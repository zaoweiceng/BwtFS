#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>
#include <map>
#include "BwtFS.h"

// 因为beast、http、net是作用域，所以可以直接用namespace重命名
// 但是tcp是一个类，所以只能通过using重命名
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using BwtFS::Util::Logger;

constexpr std::string_view SERVER_NAME = "BwtFS/0.1";

class http_connection : public std::enable_shared_from_this<http_connection> {
    private:
        tcp::socket _socket;
        static std::unordered_map<std::string, BwtFS::Node::bw_tree*> _trees; // 存储所有的bw_tree对象，key是token
        beast::flat_buffer _buffer{1024*1024*10};
        http::request_parser<http::dynamic_body> _request;// 请求头
        http::response<http::dynamic_body> _response; // 回应
        bool is_finished = false; // 是否完成请求
        // net::steady_timer _deadline{ // 定时器
        //     _socket.get_executor(),
        //     std::chrono::seconds(60)
        // };

        void read_request() {
            auto self = shared_from_this(); // 伪闭包
            auto config = BwtFS::Config::getInstance();
            _request.body_limit(std::stoi(config["server"]["max_body_size"])); // 设置请求体的大小限制
            http::async_read(_socket, _buffer, _request,
                [self](beast::error_code ec, std::size_t bytes_transferred) {
                    LOG_DEBUG << "Read request size: " << bytes_transferred;
                    LOG_DEBUG << "Request method: " << self->_request.get().method_string();
                    LOG_DEBUG << "Request target: " << self->_request.get().target();
                    if (self->_request.get().target() == "") {
                        // 忽略空请求
                        self->_response.result(http::status::not_found);
                        self->_response.set(http::field::content_type, "text/plain");
                        beast::ostream(self->_response.body()) << "Not Found";
                        self->write_response();
                        return;
                    }
                    boost::ignore_unused(bytes_transferred);
                    if (!ec) {
                        self->process_request();
                    }else{
                        LOG_ERROR << "Read request error: " << ec.to_string();
                        self->_socket.close(ec);
                        return;
                    }
                });
        }


        void check_deadline() {
            // auto self = shared_from_this(); // 伪闭包
            // _deadline.async_wait([self](boost::system::error_code ec) {
            //     if (!ec) {
            //         self->_socket.close(ec);
            //     }else{
            //         LOG_ERROR << "Deadline error: " << ec.to_string();
            //     }
            // });
        }

        void process_request() {
            _response.version(_request.get().version());
            _response.keep_alive(false); // true是长连接,false是短连接
            // LOG_INFO << "Processing request: " 
            //     << _request.get().method_string() << " " 
            //     << _request.get().target() << ", size: " 
            //     << _request.get().body().size();
            switch (_request.get().method()) {
            case http::verb::get:
                _response.result(http::status::ok);
                _response.set(http::field::server, SERVER_NAME);
                create_get_response();
                is_finished = true;
                break;
            case http::verb::post:
                _response.result(http::status::ok);
                _response.set(http::field::server, SERVER_NAME);
                _response.keep_alive(true);
                create_post_response();
                break;
            default:
                _response.result(http::status::method_not_allowed);
                _response.set(http::field::content_type, "text/plain");
                _response.set(http::field::server, SERVER_NAME);
                beast::ostream(_response.body()) << "Invalid request-method"
                    << std::string(_request.get().method_string()) << "'";
                is_finished = true;
                break;
            }

            write_response();
        }

        void create_get_response(){
            if (_request.get().target() == "/"){
                _response.set(http::field::content_type, "text/html");
                beast::ostream(_response.body())
                    << "<html>\n"
                    << "<head><title>Welcome to BwtFS</title></head>\n"
                    << "<body>\n"
                    << "<div style=\"text-align: center;\">\n"
                    << "<h1>Welcome to BwtFS</h1>\n"
                    << "<p>This is a simple HTTP server of BwtFS.</p>\n"
                    << "<p>System total Size: "
                    << BwtFS::System::getBwtFS()->getFileSize() / BwtFS::MB
                    << " MB</p>\n"
                    << "<p>System Free Size: "
                    << BwtFS::System::getBwtFS()->getFreeSize() / BwtFS::MB
                    << " MB</p>\n"
                    << "<input type=\"file\" name=\"上传文件\" id=\"file\" />\n"
                    << "<br/>\n"
                    << "<button onclick=\"uploadFile(document.getElementById('file').files[0])\">upload</button>\n"
                    << "<p id=\"token\"></p>\n"
                    << "<script>\n"
                    << R"(
                    async function uploadFile(file) {
                        const CHUNK_SIZE = 1024 * 1024; // 1MB
                        const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
                        const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);

                        for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
                            const start = chunkIndex * CHUNK_SIZE;
                            const end = Math.min(start + CHUNK_SIZE, file.size);
                            const chunk = file.slice(start, end);

                            await fetch('/upload', {
                                method: 'POST',
                                headers: {
                                    'Content-Type': 'application/octet-stream',
                                    'X-File-Id': fileId,
                                    'Connection': 'keep-alive',
                                    'X-Chunk-Index': chunkIndex,
                                    'X-Total-Chunks': totalChunks,
                                    'X-File-Size': file.size,
                                    'X-File-Type': file.type
                                },
                                body: chunk
                            }).then(response => {
                                if (!response.ok) {
                                    throw new Error('Network response was not ok');
                                }
                                var token = document.getElementById('token');
                                if (chunkIndex + 1 === totalChunks) {
                                    return response.json().then(data => {
                                        token.innerHTML = "Token: " + data.token;
                                    });
                                } else {
                                    token.innerHTML = "<br/>" + 
                                        Math.round(((chunkIndex + 1) / totalChunks) * 100) + "% uploaded.";
                                }
                                return response.json();
                            });
                        }
                    }
                    )"
                    << "</script>\n"
                    << "</div>\n"
                    << "</body>\n"
                    << "</html>\n";
            }
            std::string path = _request.get().target();
            if (path.length() > 1 && path[0] == '/') {
                path = path.substr(1);
                LOG_DEBUG << "path is " << path;
                if (path.length() < 30 || path.find('/') != std::string::npos) {
                    LOG_ERROR << "Invalid token";
                    _response.result(http::status::bad_request);
                    _response.set(http::field::content_type, "text/plain");
                    _response.set(http::field::server, SERVER_NAME);
                    beast::ostream(_response.body()) << "Invalid token";
                    return;
                }
                try{
                    BwtFS::Node::bw_tree tree(path);
                    _response.result(http::status::ok);
                    _response.set(http::field::content_type, "application/octet-stream");
                    _response.set(http::field::server, SERVER_NAME);
                    _response.set(http::field::content_disposition,
                        "attachment; filename=\"file\""); 
                    _response.chunked(true);
                    
                    beast::error_code ec;
                    if(ec) {
                        LOG_ERROR << "Write header error: " << ec.message();
                        return;
                    }
                    // 分块发送文件内容
                    constexpr size_t chunk_size = 4095; // 4KB chunks
                    int index = 0;
                    auto& body = _response.body();
                    while(true){
                        auto data = tree.read(index, chunk_size);
                        body.commit(boost::asio::buffer_copy(
                            body.prepare(data.size()),
                            boost::asio::buffer(data.data(), data.size()) // 读取数据
                        ));
                        index += data.size();
                        if (data.size() < chunk_size){
                            break;
                        }
                    }
                    http::write(_socket, _response, ec);
                    if(ec) {
                        std::cerr << "Write last chunk error: " << ec.message() << std::endl;
                    }
                }catch (const std::exception& e) {
                    LOG_ERROR << "Error: " << e.what();
                    _response.result(http::status::not_found);
                    _response.set(http::field::content_type, "text/plain");
                    _response.set(http::field::server, SERVER_NAME);
                    beast::ostream(_response.body()) << "File not found\r\n";
                }
            }
        }

        void write_response() {
            auto self = shared_from_this(); // 伪闭包
            _response.content_length(_response.body().size()); // 响应的长度
            http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t) {
                // 只关闭服务器发送端，客户端收到服务器的响应后，也关闭客户端的发送端
                    if (ec) {
                        LOG_ERROR << "Write response error: " << ec.message();
                    } else {
                        LOG_DEBUG << "Response sent successfully";
                    }
                    if (self->is_finished) {
                        self->_socket.shutdown(tcp::socket::shutdown_send, ec);
                        if (ec) {
                            LOG_ERROR << "Shutdown socket error: " << ec.message();
                        } else {
                            LOG_DEBUG << "Socket shutdown successfully";
                        }
                    }
                });
        }

        void create_post_response() {
            if (_request.get().target() == "/upload") {
                try {
                    // 保持连接活跃
                    _response.set(http::field::connection, "keep-alive");
                    
                    // 获取分块信息
                    int chunk_index = std::stoi(std::string(_request.get()["X-Chunk-Index"]));
                    int total_chunks = std::stoi(std::string(_request.get()["X-Total-Chunks"]));
                    std::string file_id = std::string(_request.get()["X-File-Id"]);
                    if (file_id.empty()) {
                        LOG_ERROR << "File ID is empty";
                        _response.result(http::status::bad_request);
                        _response.set(http::field::content_type, "text/plain");
                        beast::ostream(_response.body()) << "File ID is required";
                        return;
                    }
                    if (_trees.find(file_id) == _trees.end()) {
                        // 如果没有找到对应的树，则创建一个新的树
                        _trees[file_id] = new BwtFS::Node::bw_tree();
                    }
                    // 确保正确读取请求体
                    auto body_size = _request.get().body().size();
                    if (body_size > 0) {
                        auto body_data = _request.get().body().data();
                        std::string chunk_data(boost::asio::buffers_begin(body_data),
                                            boost::asio::buffers_end(body_data));
                        
                        _trees[file_id]->write(chunk_data.data(), chunk_data.size());
                        
                        // LOG_DEBUG << "Received chunk: " << chunk_index 
                        //         << " of " << total_chunks
                        //         << ", size: " << body_size;
                    } else {
                        LOG_WARNING << "Empty chunk received: " << chunk_index;
                    }

                    // 准备响应
                    _response.set(http::field::content_type, "application/json");
                    
                    if (chunk_index + 1 == total_chunks) {
                        // LOG_INFO << "All chunks received, total chunks: " << total_chunks;
                        _trees[file_id]->flush(); // 完成所有分块的写入
                        _trees[file_id]->join();
                        
                        boost::json::object obj;
                        obj["status"] = "success";
                        obj["token"] = _trees[file_id]->get_token();
                        obj["message"] = "File uploaded successfully";
                        beast::ostream(this->_response.body()) << boost::json::serialize(obj);
                        delete _trees[file_id]; // 删除树对象
                        _trees.erase(file_id); // 从map中删除
                    } else {
                        boost::json::object obj;
                        obj["status"] = "chunk_received";
                        obj["chunk"] = chunk_index;
                        beast::ostream(_response.body()) << boost::json::serialize(obj);
                    }
                    
                    _response.content_length(_response.body().size());
                    _response.result(http::status::ok);
                    _response.prepare_payload();
                    is_finished = true;
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error processing upload: " << e.what();
                    _response.result(http::status::internal_server_error);
                    _response.set(http::field::content_type, "application/json");
                    beast::ostream(_response.body()) << R"({"status":"error","message":")" << e.what() << "\"}";
                    _response.prepare_payload();
                }
            } else {
                _response.result(http::status::not_found);
                _response.set(http::field::content_type, "text/plain");
                beast::ostream(_response.body()) << "File not found\r\n";
            }
        }

    public:
        http_connection(tcp::socket&& socket) : _socket(std::move(socket)) {
        }

        void start() {
            read_request(); // 读请求
            // check_deadline(); // 检查超时
        }

        ~http_connection() {
            LOG_DEBUG << "Connection closed: " << 
                _socket.remote_endpoint().address().to_string() << 
                ":" << _socket.remote_endpoint().port();
        }
};

// void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
//     acceptor.async_accept(socket, [&](boost::system::error_code ec) {
//         if (!ec) {
//             // 创建一个http_connection新实例，并调用start函数
//             std::make_shared<http_connection>(std::move(socket))->start();
//         }else {
//             LOG_ERROR << "Accept error: " << ec.to_string();
//         }
//         http_server(acceptor, socket);
//     });
// }
void http_server(tcp::acceptor& acceptor) {
    // 每次 async_accept 前创建一个新的 socket
    auto socket = std::make_shared<tcp::socket>(acceptor.get_executor());
    
    acceptor.async_accept(*socket, [&acceptor, socket](boost::system::error_code ec) {
        if (!ec) {
            std::make_shared<http_connection>(std::move(*socket))->start();
        } else {
            LOG_ERROR << "Accept error: " << ec.message();
        }
        http_server(acceptor);  // 继续接受新连接
    });
}

std::unordered_map<std::string, BwtFS::Node::bw_tree*> http_connection::_trees;

int main(int argc, char* argv[]){
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: " << argv[0] << " [config_file_path]\n";
        std::cout << "Default config file path is ./bwtfs.ini\n";
        return 0;
    }
    try {
        init();
        auto config = BwtFS::Config::getInstance();
        std::string system_file_path = config["system"]["path"];
        if (argc == 2){
            system_file_path = argv[1];
            config["system"]["path"] = system_file_path;
        }
        auto system = BwtFS::System::openBwtFS(system_file_path);
        auto const address = net::ip::make_address(config["server"]["address"]);
        unsigned short port = static_cast<unsigned short>(std::stoi(config["server"]["port"]));
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor( ioc,{address,port} );
        // tcp::socket socket(ioc);
        http_server(acceptor);
        LOG_INFO << "Server started at: http://" << address.to_string() << ":" << port;
        ioc.run();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    system("pause");
    return 0;
}
