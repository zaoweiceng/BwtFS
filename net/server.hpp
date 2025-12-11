#ifndef __SERVER_HPP__
#define __SERVER_HPP__
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
            case http::verb::delete_:
                _response.result(http::status::ok);
                _response.set(http::field::server, SERVER_NAME);
                _response.keep_alive(false);
                create_delete_response();
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

        bool starts_with(const std::string& str, const std::string& prefix) {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }

        void create_get_response(){
            if (_request.get().target() == "/"){
                _response.set(http::field::content_type, "text/html");
                beast::ostream(_response.body())
                    << R"(
                        <!DOCTYPE html>
                        <html lang="en">
                        <head>
                            <meta charset="UTF-8">
                            <meta name="viewport" content="width=device-width, initial-scale=1.0">
                            <title>Welcome to BwtFS</title>
                            <style>
                                :root {
                                    --primary-color: #4a6fa5;
                                    --secondary-color: #6b8cae;
                                    --accent-color: #ff7e5f;
                                    --light-color: #f8f9fa;
                                    --dark-color: #343a40;
                                    --success-color: #28a745;
                                    --danger-color: #dc3545;
                                    --border-radius: 8px;
                                    --box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
                                    --transition: all 0.3s ease;
                                }
                                
                                * {
                                    margin: 0;
                                    padding: 0;
                                    box-sizing: border-box;
                                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                                }
                                
                                body {
                                    background-color: #f5f7fa;
                                    color: var(--dark-color);
                                    line-height: 1.6;
                                    padding: 20px;
                                }
                                
                                .container {
                                    max-width: 800px;
                                    margin: 0 auto;
                                    padding: 30px;
                                    background-color: white;
                                    border-radius: var(--border-radius);
                                    box-shadow: var(--box-shadow);
                                }
                                
                                h1 {
                                    color: var(--primary-color);
                                    margin-bottom: 20px;
                                    font-weight: 600;
                                }
                                
                                p {
                                    margin-bottom: 15px;
                                }
                                
                                .storage-info {
                                    display: flex;
                                    justify-content: space-between;
                                    margin: 25px 0;
                                    padding: 15px;
                                    background-color: var(--light-color);
                                    border-radius: var(--border-radius);
                                }
                                
                                .storage-info p {
                                    margin: 0;
                                    font-weight: 500;
                                }
                                
                                .upload-section, .delete-section {
                                    margin: 25px 0;
                                    padding: 20px;
                                    border: 1px solid #e1e5eb;
                                    border-radius: var(--border-radius);
                                    transition: var(--transition);
                                }
                                
                                .upload-section:hover, .delete-section:hover {
                                    border-color: var(--primary-color);
                                }
                                
                                .file-input-wrapper {
                                    position: relative;
                                    margin-bottom: 15px;
                                }
                                
                                .file-input-wrapper input[type="file"] {
                                    width: 100%;
                                    padding: 10px;
                                    border: 1px solid #ddd;
                                    border-radius: var(--border-radius);
                                }
                                
                                button {
                                    background-color: var(--primary-color);
                                    color: white;
                                    border: none;
                                    padding: 10px 20px;
                                    border-radius: var(--border-radius);
                                    cursor: pointer;
                                    font-weight: 500;
                                    transition: var(--transition);
                                }
                                
                                button:hover {
                                    background-color: var(--secondary-color);
                                    transform: translateY(-2px);
                                }
                                
                                button:active {
                                    transform: translateY(0);
                                }
                                
                                #delete_path {
                                    width: 100%;
                                    padding: 10px;
                                    margin-bottom: 15px;
                                    border: 1px solid #ddd;
                                    border-radius: var(--border-radius);
                                }
                                
                                #info {
                                    margin-top: 20px;
                                    padding: 15px;
                                    border-radius: var(--border-radius);
                                    background-color: var(--light-color);
                                    min-height: 50px;
                                }
                                
                                .progress-bar {
                                    width: 100%;
                                    height: 10px;
                                    background-color: #e1e5eb;
                                    border-radius: 5px;
                                    margin-top: 10px;
                                    overflow: hidden;
                                }
                                
                                .progress {
                                    height: 100%;
                                    background-color: var(--accent-color);
                                    width: 0%;
                                    transition: width 0.3s ease;
                                }
                                
                                .success-message {
                                    color: var(--success-color);
                                    font-weight: 500;
                                }
                                
                                .error-message {
                                    color: var(--danger-color);
                                    font-weight: 500;
                                }
                                
                                footer {
                                    margin-top: 30px;
                                    text-align: center;
                                    color: #6c757d;
                                    font-size: 0.9em;
                                }
                            </style>
                        </head>
                        <body>
                            <div class="container">
                                <h1>Welcome to BwtFS</h1>
                                <p>This is a simple HTTP server for BwtFS file storage system.</p>
                                
                                <div class="storage-info">
                                    <p>Total Storage: <strong>)" 
                                    << BwtFS::System::getBwtFS()->getFileSize() / BwtFS::MB
                                    <<  R"( MB</strong></p>
                                    <p>Free Space: <strong>)"
                                    << BwtFS::System::getBwtFS()->getFreeSize() / BwtFS::MB
                                    << R"( MB</strong></p>
                                </div>
                                
                                <div class="upload-section">
                                    <h3>Upload File</h3>
                                    <div class="file-input-wrapper">
                                        <input type="file" id="file" aria-label="Select file to upload">
                                    </div>
                                    <button onclick="uploadFile(document.getElementById('file').files[0]) ">Upload File</button>
                                    <div class="progress-bar" id="progressBar" style="display: none;">
                                        <div class="progress" id="progress"></div>
                                    </div>
                                </div>
                                
                                <div class="delete-section">
                                    <h3>Delete File</h3>
                                    <input type="text" id="delete_path" placeholder="Enter file token to delete" aria-label="File token to delete">
                                    <button onclick="deleteFile(document.getElementById('delete_path').value) ">Delete File</button>
                                </div>
                                
                                <div id="info"></div>
                                
                                <footer>
                                    BwtFS File Storage System &copy; 2025
                                </footer>
                            </div>

                            <script>
                                async function deleteFile(token) {
                                    if (!token) {
                                        showMessage("Please enter a file token to delete.", "error");
                                        return;
                                    }
                                    
                                    try {
                                        const response = await fetch('/delete/' + encodeURIComponent(token), {
                                            method: 'DELETE',
                                            headers: {
                                                'Content-Type': 'application/json'
                                            }
                                        });
                                        
                                        const infoElement = document.getElementById('info');
                                        if (response.ok) {
                                            showMessage("File deleted successfully.", "success");
                                            document.getElementById('delete_path').value = '';
                                        } else {
                                            try {
                                                const errorText = await response.text();
                                                showMessage(`Failed to delete file. Status: ${response.status} - ${response.statusText}: ${errorText}`, "error");
                                            } catch (error) {
                                                showMessage(`Failed to delete file. Status: ${response.status} - ${response.statusText}: Failed to read error message`, "error");
                                            }
                                        }
                                    } catch (error) {
                                        showMessage(`Error deleting file: ${error.message}`, "error");
                                    }
                                }

                                async function uploadFile(file) {
                                    if (!file) {
                                        showMessage("Please select a file to upload.", "error");
                                        return;
                                    }

                                    const CHUNK_SIZE = 1024 * 1024; // 1MB
                                    const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
                                    const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);
                                    
                                    // Show progress bar
                                    const progressBar = document.getElementById('progressBar');
                                    const progressElement = document.getElementById('progress');
                                    progressBar.style.display = 'block';
                                    progressElement.style.width = '0%';

                                    try {
                                        for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
                                            const start = chunkIndex * CHUNK_SIZE;
                                            const end = Math.min(start + CHUNK_SIZE, file.size);
                                            const chunk = file.slice(start, end);

                                            const response = await fetch('/upload', {
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
                                            });

                                            if (!response.ok) {
                                                throw new Error('Network response was not ok');
                                            }

                                            // Update progress
                                            const progressPercent = Math.round(((chunkIndex + 1) / totalChunks) * 100);
                                            progressElement.style.width = `${progressPercent}%`;

                                            if (chunkIndex + 1 === totalChunks) {
                                                const data = await response.json();
                                                showMessage(`File uploaded successfully. Token: <strong>${data.token}</strong>`, "success");
                                                progressBar.style.display = 'none';
                                            }
                                        }
                                    } catch (error) {
                                        showMessage(`Error uploading file: ${error.message}`, "error");
                                        progressBar.style.display = 'none';
                                    }
                                }

                                function showMessage(message, type) {
                                    const infoElement = document.getElementById('info');
                                    infoElement.innerHTML = message;
                                    infoElement.className = type === "success" ? "success-message" : "error-message";
                                }
                            </script>
                        </body>
                        </html>
                    )";
                    // << "<html>\n"
                    // << "<head><title>Welcome to BwtFS</title></head>\n"
                    // << "<body>\n"
                    // << "<div style=\"text-align: center;\">\n"
                    // << "<h1>Welcome to BwtFS</h1>\n"
                    // << "<p>This is a simple HTTP server of BwtFS.</p>\n"
                    // << "<p>System total Size: "
                    // << BwtFS::System::getBwtFS()->getFileSize() / BwtFS::MB
                    // << " MB</p>\n"
                    // << "<p>System Free Size: "
                    // << BwtFS::System::getBwtFS()->getFreeSize() / BwtFS::MB
                    // << " MB</p>\n"
                    // << "<input type=\"file\" name=\"choose file\" id=\"file\" />\n"
                    // << "<button onclick=\"uploadFile(document.getElementById('file').files[0])\">upload</button>\n"
                    // << "<br/>"
                    // << "<input type=\"text\" id=\"delete_path\" placeholder=\"Enter file token to delete\" />\n"
                    // << "<button onclick=\"deleteFile(document.getElementById('delete_path').value)\">Delete</button>\n"
                    // << "<p id=\"info\"></p>\n"
                    // << "<script>\n"
                    // << R"(
                    // async function deleteFile(token) {
                    //     if (!token) {
                    //         alert("Please enter a file token to delete.");
                    //         return;
                    //     }
                    //     try {
                    //         const response = await fetch('/delete/' + encodeURIComponent(token), {
                    //             method: 'DELETE',
                    //             headers: {
                    //                 'Content-Type': 'application/json'
                    //             }
                    //         })
                    //         if (response.ok) {
                    //             var info = document.getElementById('info');
                    //             info.innerHTML = "File deleted successfully.";
                    //         } else {
                    //             var info = document.getElementById('info');
                    //             response.text().then(errorText => {
                    //                 info.innerHTML = "Failed to delete file. Status: " + response.status + 
                    //                     " - " + response.statusText + ": " + errorText;
                    //             }).catch(error => {
                    //                 info.innerHTML = "Failed to delete file. Status: " + response.status + 
                    //                     " - " + response.statusText + ":Failed to read error message";
                    //             });
                    //         }
                    //     } catch (error) {
                    //         var info = document.getElementById('info');
                    //         info.innerHTML = "Error deleting file: " + error.message;
                    //     }
                    // }
                    // async function uploadFile(file) {
                    //     const CHUNK_SIZE = 1024 * 1024; // 1MB
                    //     const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
                    //     const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);

                    //     for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
                    //         const start = chunkIndex * CHUNK_SIZE;
                    //         const end = Math.min(start + CHUNK_SIZE, file.size);
                    //         const chunk = file.slice(start, end);

                    //         await fetch('/upload', {
                    //             method: 'POST',
                    //             headers: {
                    //                 'Content-Type': 'application/octet-stream',
                    //                 'X-File-Id': fileId,
                    //                 'Connection': 'keep-alive',
                    //                 'X-Chunk-Index': chunkIndex,
                    //                 'X-Total-Chunks': totalChunks,
                    //                 'X-File-Size': file.size,
                    //                 'X-File-Type': file.type
                    //             },
                    //             body: chunk
                    //         }).then(response => {
                    //             if (!response.ok) {
                    //                 throw new Error('Network response was not ok');
                    //             }
                    //             var info = document.getElementById('info');
                    //             if (chunkIndex + 1 === totalChunks) {
                    //                 return response.json().then(data => {
                    //                     info.innerHTML = "Token: " + data.token;
                    //                 });
                    //             } else {
                    //                 info.innerHTML = "<br/>" + 
                    //                     Math.round(((chunkIndex + 1) / totalChunks) * 100) + "% uploaded.";
                    //             }
                    //             return response.json();
                    //         });
                    //     }
                    // }
                    // )"
                    // << "</script>\n"
                    // << "</div>\n"
                    // << "</body>\n"
                    // << "</html>\n";
            }
            std::string path = _request.get().target();
            if (path.length() > 1 && path[0] == '/') {
                path = path.substr(1);
                LOG_DEBUG << "path is " << path;
                if (path.length() < 20 || path.find('/') != std::string::npos) {
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

        void create_delete_response() {
            std::string path = _request.get().target();
            if (starts_with(path, "/delete/")) {
                path = path.substr(8); // 删除前缀 "/delete/"
                if (path.length() < 30) {
                    LOG_ERROR << "Invalid token";
                    _response.result(http::status::bad_request);
                    _response.set(http::field::content_type, "text/plain");
                    _response.set(http::field::server, SERVER_NAME);
                    beast::ostream(_response.body()) << "Invalid token";
                    return;
                }
                try {
                    LOG_DEBUG << "Deleting file" << path;
                    BwtFS::Node::bw_tree tree(path, true);
                    LOG_DEBUG << "File tree initialized for deletion: " << path;
                    try {
                        tree.delete_file();
                    } catch (const std::exception& e) {
                        LOG_ERROR << "Error deleting file: " << e.what();
                        _response.result(http::status::not_found);
                        _response.set(http::field::content_type, "text/plain");
                        _response.set(http::field::server, SERVER_NAME);
                        beast::ostream(_response.body()) << "File not found\r\n";
                        return;
                    }

                    _response.result(http::status::ok);
                    _response.set(http::field::content_type, "application/json");
                    _response.set(http::field::server, SERVER_NAME);
                    boost::json::object obj;
                    obj["status"] = "success";
                    obj["message"] = "File deleted successfully";
                    beast::ostream(_response.body()) << boost::json::serialize(obj);
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error: " << e.what();
                    _response.result(http::status::not_found);
                    _response.set(http::field::content_type, "text/plain");
                    _response.set(http::field::server, SERVER_NAME);
                    beast::ostream(_response.body()) << "File not found\r\n";
                }
            } else {
                _response.result(http::status::bad_request);
                _response.set(http::field::content_type, "text/plain");
                _response.set(http::field::server, SERVER_NAME);
                beast::ostream(_response.body()) << "Invalid delete request";
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
        }

        ~http_connection() {
        }
};

#endif // __SERVER_HPP__