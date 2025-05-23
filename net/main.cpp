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
        beast::flat_buffer _buffer{1024*1024*10};
        http::request_parser<http::dynamic_body> _request;// 请求头
        http::response<http::dynamic_body> _response; // 回应
        net::steady_timer _deadline{ // 定时器
            _socket.get_executor(),
            std::chrono::seconds(60)
        };

        void read_request() {
            auto self = shared_from_this(); // 伪闭包
            auto config = BwtFS::Config::getInstance();
            _request.body_limit(std::stoi(config["server"]["max_body_size"])); // 设置请求体的大小限制
            http::async_read(_socket, _buffer, _request,
                [self](beast::error_code ec, std::size_t bytes_transferred) {
                    LOG_DEBUG << "Read request size: " << bytes_transferred;
                    // boost::ignore_unused(bytes_transferred); // 没用到bytes_transferred参数，编译器会警告，这里直接忽略掉
                    if (!ec) {
                        self->process_request();
                    }else{
                        LOG_ERROR << "Read request error: " << ec.message();
                        self->_socket.close(ec);
                        return;
                    }
                });
        }


        void check_deadline() {
            auto self = shared_from_this(); // 伪闭包
            _deadline.async_wait([self](boost::system::error_code ec) {
                if (!ec) {
                    self->_socket.close(ec);
                }
            });
        }

        void process_request() {
            _response.version(_request.get().version());
            _response.keep_alive(true); // true是长连接,false是短连接
            switch (_request.get().method()) {
            case http::verb::get:
                _response.result(http::status::ok);
                _response.set(http::field::server, SERVER_NAME);
                create_get_response();
                break;
            case http::verb::post:
                _response.result(http::status::ok);
                _response.set(http::field::server, SERVER_NAME);
                create_post_response();
                break;
            default:
                _response.result(http::status::method_not_allowed);
                _response.set(http::field::content_type, "text/plain");
                _response.set(http::field::server, SERVER_NAME);
                beast::ostream(_response.body()) << "Invalid request-method"
                    << std::string(_request.get().method_string()) << "'";
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
                    // 上传文件按钮，调用js上传文件，文件上传完成之后，返回文件的id
                    << "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"return uploadFile(event)\">"
                    << "<input type=\"file\" name=\"file\" id=\"file\" />\n"
                    << "<input type=\"submit\" value=\"Upload\" />\n"
                    << "</form>\n"
                    << "<script>\n"
                    << "function uploadFile(event) {\n"
                    << "    event.preventDefault();\n"
                    << "    var formData = new FormData();\n"
                    << "    var fileInput = document.getElementById('file');\n"
                    << "    var file = fileInput.files[0];\n"
                    << "    if (!file) {\n"
                    << "        alert('请选择上传的文件.');\n"
                    << "        return;\n"
                    << "    }\n"
                    << "    formData.append('file', file);\n"
                    << "    var xhr = new XMLHttpRequest();\n"
                    << "    xhr.open('POST', '/upload', true);\n"
                    << "    xhr.onreadystatechange = function() {\n"
                    << "        if (xhr.readyState == 4 && xhr.status == 200) {\n"
                    << "            var response = JSON.parse(xhr.responseText);\n"
                    << "            if (response.status == 'success') {\n"
                    << "                alert('File uploaded successfully! File Token: ' + response.token);\n"
                    << "            } else {\n"
                    << "                alert('File upload failed!');\n"
                    << "            }\n"
                    << "        }\n"
                    << "    };\n"
                    << "    xhr.send(formData);\n"
                    << "    return false;\n"
                    << "}\n"
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
                self->_socket.shutdown(tcp::socket::shutdown_send, ec);
                self->_deadline.cancel();
                });
        }

        void create_post_response() {
            if (_request.get().target() == "/upload") {
                BwtFS::Node::bw_tree tree;
                if (_request.get()[boost::beast::http::field::transfer_encoding] == "chunked") {
                    // 对于分块传输，我们需要逐块处理
                    auto handle_chunk = [&](const std::string& chunk) {
                        tree.write(const_cast<char*>(chunk.c_str()), chunk.size());
                    };
                    // 读取分块数据
                    auto it = _request.get().body().data().begin();
                    auto end = _request.get().body().data().end();
                    while (it != end) {
                        auto chunk_size = std::distance(it, end);
                        if (chunk_size <= 0) break;
                        std::string chunk(static_cast<const char*>((*it).data()), (*it).size());
                        handle_chunk(chunk);
                        ++it;
                    }
                    
                } else {
                    // 对于普通请求，直接写入整个body
                    auto& buffer = _request.get().body();
                    std::string body_data;
                    body_data.reserve(buffer.size());
                    for (auto const& seq : buffer.data()) {
                        body_data.append(static_cast<const char*>(seq.data()), seq.size());
                    }
                    tree.write(const_cast<char*>(body_data.data()), body_data.size());
                }
                tree.flush();
                tree.join();
                this->_response.set(http::field::content_type, "application/json");
                boost::json::object obj;
                obj["status"] = "success";
                obj["token"] = tree.get_token();
                obj["message"] = "File uploaded successfully";
                beast::ostream(this->_response.body()) << boost::json::serialize(obj);
                this->_response.content_length(this->_response.body().size());
                this->_response.result(http::status::ok);
            }
            else {
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
            check_deadline(); // 检查超时
        }
        ~http_connection() {
            LOG_INFO << "Connection closed: " << 
                _socket.remote_endpoint().address().to_string() << 
                ":" << _socket.remote_endpoint().port();
        }
};

void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
    
    acceptor.async_accept(socket, [&](boost::system::error_code ec) {
        if (!ec) {
            // 创建一个http_connection新实例，并调用start函数
            std::make_shared<http_connection>(std::move(socket))->start();
        }

        http_server(acceptor, socket);
        });
}

int main(void){
    try {
        init();
        auto config = BwtFS::Config::getInstance();
        LOG_INFO <<  config["system"]["path"];
        auto system = BwtFS::System::openBwtFS(config["system"]["path"]);
        auto const address = net::ip::make_address(config["server"]["address"]);
        unsigned short port = static_cast<unsigned short>(std::stoi(config["server"]["port"]));
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor( ioc,{address,port} );
        tcp::socket socket(ioc);
        http_server(acceptor, socket);
        LOG_INFO << "Server started at " << address.to_string() << ":" << port;
        ioc.run();

    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
