#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <httplib.h>
#include <json.hpp>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <sstream>
#include <fstream>
#include "BwtFS.h"

using json = nlohmann::json;
using BwtFS::Util::Logger;

constexpr std::string_view SERVER_NAME = "BwtFS/0.1";

class HttpServer {
private:
    httplib::Server server;
    std::unordered_map<std::string, BwtFS::Node::bw_tree*> trees;
    std::mutex trees_mutex;

    // Helper function to check if string starts with prefix
    bool starts_with(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    // Helper function to read file content
    std::string read_file_content(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    // Generate HTML for root page (hardcoded)
    std::string generate_root_html() const {
        std::string html;

        // HTML document start
        html += "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        html += "    <meta charset=\"UTF-8\">\n";
        html += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html += "    <title>Welcome to BwtFS</title>\n";
        html += "    <style>\n";

        // CSS styles
        html += "        :root {\n";
        html += "            --primary-color: #4a6fa5;\n";
        html += "            --secondary-color: #6b8cae;\n";
        html += "            --accent-color: #ff7e5f;\n";
        html += "            --light-color: #f8f9fa;\n";
        html += "            --dark-color: #343a40;\n";
        html += "            --success-color: #28a745;\n";
        html += "            --danger-color: #dc3545;\n";
        html += "            --border-radius: 8px;\n";
        html += "            --box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);\n";
        html += "            --transition: all 0.3s ease;\n";
        html += "        }\n\n";

        html += "        * {\n";
        html += "            margin: 0;\n";
        html += "            padding: 0;\n";
        html += "            box-sizing: border-box;\n";
        html += "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n";
        html += "        }\n\n";

        html += "        body {\n";
        html += "            background-color: #f5f7fa;\n";
        html += "            color: var(--dark-color);\n";
        html += "            line-height: 1.6;\n";
        html += "            padding: 20px;\n";
        html += "        }\n\n";

        html += "        .container {\n";
        html += "            max-width: 800px;\n";
        html += "            margin: 0 auto;\n";
        html += "            padding: 30px;\n";
        html += "            background-color: white;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "            box-shadow: var(--box-shadow);\n";
        html += "        }\n\n";

        html += "        h1 {\n";
        html += "            color: var(--primary-color);\n";
        html += "            margin-bottom: 20px;\n";
        html += "            font-weight: 600;\n";
        html += "        }\n\n";

        html += "        p {\n";
        html += "            margin-bottom: 15px;\n";
        html += "        }\n\n";

        html += "        .storage-info {\n";
        html += "            display: flex;\n";
        html += "            justify-content: space-between;\n";
        html += "            margin: 25px 0;\n";
        html += "            padding: 15px;\n";
        html += "            background-color: var(--light-color);\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "        }\n\n";

        html += "        .storage-info p {\n";
        html += "            margin: 0;\n";
        html += "            font-weight: 500;\n";
        html += "        }\n\n";

        html += "        .upload-section, .delete-section {\n";
        html += "            margin: 25px 0;\n";
        html += "            padding: 20px;\n";
        html += "            border: 1px solid #e1e5eb;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "            transition: var(--transition);\n";
        html += "        }\n\n";

        html += "        .upload-section:hover, .delete-section:hover {\n";
        html += "            border-color: var(--primary-color);\n";
        html += "        }\n\n";

        html += "        .file-input-wrapper {\n";
        html += "            position: relative;\n";
        html += "            margin-bottom: 15px;\n";
        html += "        }\n\n";

        html += "        .file-input-wrapper input[type=\"file\"] {\n";
        html += "            width: 100%;\n";
        html += "            padding: 10px;\n";
        html += "            border: 1px solid #ddd;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "        }\n\n";

        html += "        button {\n";
        html += "            background-color: var(--primary-color);\n";
        html += "            color: white;\n";
        html += "            border: none;\n";
        html += "            padding: 10px 20px;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "            cursor: pointer;\n";
        html += "            font-weight: 500;\n";
        html += "            transition: var(--transition);\n";
        html += "        }\n\n";

        html += "        button:hover {\n";
        html += "            background-color: var(--secondary-color);\n";
        html += "            transform: translateY(-2px);\n";
        html += "        }\n\n";

        html += "        button:active {\n";
        html += "            transform: translateY(0);\n";
        html += "        }\n\n";

        html += "        #delete_path {\n";
        html += "            width: 100%;\n";
        html += "            padding: 10px;\n";
        html += "            margin-bottom: 15px;\n";
        html += "            border: 1px solid #ddd;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "        }\n\n";

        html += "        #info {\n";
        html += "            margin-top: 20px;\n";
        html += "            padding: 15px;\n";
        html += "            border-radius: var(--border-radius);\n";
        html += "            background-color: var(--light-color);\n";
        html += "            min-height: 50px;\n";
        html += "        }\n\n";

        html += "        .progress-bar {\n";
        html += "            width: 100%;\n";
        html += "            height: 10px;\n";
        html += "            background-color: #e1e5eb;\n";
        html += "            border-radius: 5px;\n";
        html += "            margin-top: 10px;\n";
        html += "            overflow: hidden;\n";
        html += "        }\n\n";

        html += "        .progress {\n";
        html += "            height: 100%;\n";
        html += "            background-color: var(--accent-color);\n";
        html += "            width: 0%;\n";
        html += "            transition: width 0.3s ease;\n";
        html += "        }\n\n";

        html += "        .success-message {\n";
        html += "            color: var(--success-color);\n";
        html += "            font-weight: 500;\n";
        html += "        }\n\n";

        html += "        .error-message {\n";
        html += "            color: var(--danger-color);\n";
        html += "            font-weight: 500;\n";
        html += "        }\n\n";

        html += "        footer {\n";
        html += "            margin-top: 30px;\n";
        html += "            text-align: center;\n";
        html += "            color: #6c757d;\n";
        html += "            font-size: 0.9em;\n";
        html += "        }\n";
        html += "    </style>\n";
        html += "</head>\n<body>\n";
        html += "    <div class=\"container\">\n";
        html += "        <h1>Welcome to BwtFS</h1>\n";
        html += "        <p>This is a simple HTTP server for BwtFS file storage system.</p>\n\n";

        html += "        <div class=\"storage-info\">\n";
        html += "            <p>Total Storage: <strong id=\"total_size\">Loading...</strong></p>\n";
        html += "            <p>Free Space: <strong id=\"free_size\">Loading...</strong></p>\n";
        html += "        </div>\n\n";

        html += "        <div class=\"upload-section\">\n";
        html += "            <h3>Upload File</h3>\n";
        html += "            <div class=\"file-input-wrapper\">\n";
        html += "                <input type=\"file\" id=\"file\" aria-label=\"Select file to upload\">\n";
        html += "            </div>\n";
        html += "            <button onclick=\"uploadFile(document.getElementById('file').files[0])\">Upload File</button>\n";
        html += "            <div class=\"progress-bar\" id=\"progressBar\" style=\"display: none;\">\n";
        html += "                <div class=\"progress\" id=\"progress\"></div>\n";
        html += "            </div>\n";
        html += "        </div>\n\n";

        html += "        <div class=\"delete-section\">\n";
        html += "            <h3>Delete File</h3>\n";
        html += "            <input type=\"text\" id=\"delete_path\" placeholder=\"Enter file token to delete\" aria-label=\"File token to delete\">\n";
        html += "            <button onclick=\"deleteFile(document.getElementById('delete_path').value)\">Delete File</button>\n";
        html += "        </div>\n\n";

        html += "        <div id=\"info\"></div>\n\n";

        html += "        <footer>\n";
        html += "            BwtFS File Storage System &copy; 2025\n";
        html += "        </footer>\n";
        html += "    </div>\n\n";

        html += "    <script>\n";

        // JavaScript code
        html += "        window.onload = function() {\n";
        html += "            fetch('/system_size')\n";
        html += "                .then(response => response.json())\n";
        html += "                .then(data => {\n";
        html += "                    document.getElementById('total_size').textContent = (data.system_size / (1024 * 1024)).toFixed(2) + ' MB';\n";
        html += "                })\n";
        html += "                .catch(error => {\n";
        html += "                    document.getElementById('total_size').textContent = 'Error loading';\n";
        html += "                });\n\n";

        html += "            fetch('/free_size')\n";
        html += "                .then(response => response.json())\n";
        html += "                .then(data => {\n";
        html += "                    document.getElementById('free_size').textContent = (data.free_size / (1024 * 1024)).toFixed(2) + ' MB';\n";
        html += "                })\n";
        html += "                .catch(error => {\n";
        html += "                    document.getElementById('free_size').textContent = 'Error loading';\n";
        html += "                });\n";
        html += "        };\n\n";

        html += "        async function deleteFile(token) {\n";
        html += "            if (!token) {\n";
        html += "                showMessage('Please enter a file token to delete.', 'error');\n";
        html += "                return;\n";
        html += "            }\n\n";

        html += "            try {\n";
        html += "                const response = await fetch('/delete/' + encodeURIComponent(token), {\n";
        html += "                    method: 'DELETE',\n";
        html += "                    headers: {\n";
        html += "                        'Content-Type': 'application/json'\n";
        html += "                    }\n";
        html += "                });\n\n";

        html += "                const infoElement = document.getElementById('info');\n";
        html += "                if (response.ok) {\n";
        html += "                    showMessage('File deleted successfully.', 'success');\n";
        html += "                    document.getElementById('delete_path').value = '';\n";
        html += "                    window.onload();\n";
        html += "                } else {\n";
        html += "                    try {\n";
        html += "                        const errorText = await response.text();\n";
        html += "                        showMessage('Failed to delete file. Status: ' + response.status + ' - ' + response.statusText + ': ' + errorText, 'error');\n";
        html += "                    } catch (error) {\n";
        html += "                        showMessage('Failed to delete file. Status: ' + response.status + ' - ' + response.statusText + ': Failed to read error message', 'error');\n";
        html += "                    }\n";
        html += "                }\n";
        html += "            } catch (error) {\n";
        html += "                showMessage('Error deleting file: ' + error.message, 'error');\n";
        html += "            }\n";
        html += "        }\n\n";

        html += "        async function uploadFile(file) {\n";
        html += "            if (!file) {\n";
        html += "                showMessage('Please select a file to upload.', 'error');\n";
        html += "                return;\n";
        html += "            }\n\n";

        html += "            const CHUNK_SIZE = 1024 * 1024;\n";
        html += "            const totalChunks = Math.ceil(file.size / CHUNK_SIZE);\n";
        html += "            const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);\n\n";

        html += "            const progressBar = document.getElementById('progressBar');\n";
        html += "            const progressElement = document.getElementById('progress');\n";
        html += "            progressBar.style.display = 'block';\n";
        html += "            progressElement.style.width = '0%';\n\n";

        html += "            try {\n";
        html += "                for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {\n";
        html += "                    const start = chunkIndex * CHUNK_SIZE;\n";
        html += "                    const end = Math.min(start + CHUNK_SIZE, file.size);\n";
        html += "                    const chunk = file.slice(start, end);\n\n";

        html += "                    const response = await fetch('/upload', {\n";
        html += "                        method: 'POST',\n";
        html += "                        headers: {\n";
        html += "                            'Content-Type': 'application/octet-stream',\n";
        html += "                            'X-File-Id': fileId,\n";
        html += "                            'Connection': 'keep-alive',\n";
        html += "                            'X-Chunk-Index': chunkIndex,\n";
        html += "                            'X-Total-Chunks': totalChunks,\n";
        html += "                            'X-File-Size': file.size,\n";
        html += "                            'X-File-Type': file.type\n";
        html += "                        },\n";
        html += "                        body: chunk\n";
        html += "                    });\n\n";

        html += "                    if (!response.ok) {\n";
        html += "                        throw new Error('Network response was not ok');\n";
        html += "                    }\n\n";

        html += "                    const progressPercent = Math.round(((chunkIndex + 1) / totalChunks) * 100);\n";
        html += "                    progressElement.style.width = progressPercent + '%';\n\n";

        html += "                    if (chunkIndex + 1 === totalChunks) {\n";
        html += "                        const data = await response.json();\n";
        html += "                        showMessage('File uploaded successfully. Token: <strong>' + data.token + '</strong>', 'success');\n";
        html += "                        progressBar.style.display = 'none';\n";
        html += "                        window.onload();\n";
        html += "                    }\n";
        html += "                }\n";
        html += "            } catch (error) {\n";
        html += "                showMessage('Error uploading file: ' + error.message, 'error');\n";
        html += "                progressBar.style.display = 'none';\n";
        html += "            }\n";
        html += "        }\n\n";

        html += "        function showMessage(message, type) {\n";
        html += "            const infoElement = document.getElementById('info');\n";
        html += "            infoElement.innerHTML = message;\n";
        html += "            infoElement.className = type === 'success' ? 'success-message' : 'error-message';\n";
        html += "        }\n";
        html += "    </script>\n";
        html += "</body>\n";
        html += "</html>";

        return html;
    }

public:
    HttpServer() {
        setup_routes();
    }

    void setup_routes() {
        // Enable CORS
        server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, X-File-Id, X-Chunk-Index, X-Total-Chunks, X-File-Size, X-File-Type, Connection");
            return httplib::Server::HandlerResponse::Unhandled;
        });

        // Handle OPTIONS requests for CORS
        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            return;
        });

        // Root endpoint - serve hardcoded HTML page
        server.Get("/", [this](const httplib::Request&, httplib::Response& res) {
            res.set_content(generate_root_html(), "text/html");
        });

        // Favicon
        server.Get("/favicon.ico", [](const httplib::Request&, httplib::Response& res) {
            std::ifstream file("./favicon.ico", std::ios::binary);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                res.set_content(content, "image/x-icon");
            } else {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
            }
        });

        // Robots.txt
        server.Get("/robots.txt", [](const httplib::Request&, httplib::Response& res) {
            res.set_content("User-agent: *\nDisallow: /delete/\n", "text/plain");
        });

        // System size endpoint
        server.Get("/system_size", [](const httplib::Request&, httplib::Response& res) {
            json obj;
            obj["system_size"] = BwtFS::System::getBwtFS()->getFileSize();
            res.set_content(obj.dump(), "application/json");
        });

        // Free size endpoint
        server.Get("/free_size", [](const httplib::Request&, httplib::Response& res) {
            json obj;
            obj["free_size"] = BwtFS::System::getBwtFS()->getFreeSize();
            res.set_content(obj.dump(), "application/json");
        });

        // File download endpoint
        server.Get("(.*)", [this](const httplib::Request& req, httplib::Response& res) {
            std::string path = req.matches[1];

            // Skip paths that are already handled
            if (path == "" || path == "/" || path == "/favicon.ico" || path == "/robots.txt" ||
                path == "/system_size" || path == "/free_size") {
                return;
            }

            // Remove leading slash if present
            if (path.length() > 1 && path[0] == '/') {
                path = path.substr(1);
            }

            LOG_DEBUG << "Requested file path: " << path;

            if (path.length() < 20 || path.find('/') != std::string::npos) {
                LOG_ERROR << "Invalid token: " << path;
                res.status = 400;
                res.set_content("Invalid token", "text/plain");
                return;
            }

            try {
                BwtFS::Node::bw_tree tree(path);

                // Read file data
                std::ostringstream file_data;
                constexpr size_t chunk_size = 4096;
                int index = 0;

                while (true) {
                    auto data = tree.read(index, chunk_size);
                    file_data.write(reinterpret_cast<const char*>(data.data()), data.size());
                    index += data.size();
                    if (data.size() < chunk_size) {
                        break;
                    }
                }

                res.set_content(file_data.str(), "application/octet-stream");
                res.set_header("Content-Disposition", "attachment; filename=\"file\"");
            } catch (const std::exception& e) {
                LOG_ERROR << "Error reading file: " << e.what();
                res.status = 404;
                res.set_content("File not found", "text/plain");
            }
        });

        // File upload endpoint
        server.Post("/upload", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                // Get chunk information
                int chunk_index = std::stoi(req.get_header_value("X-Chunk-Index"));
                int total_chunks = std::stoi(req.get_header_value("X-Total-Chunks"));
                std::string file_id = req.get_header_value("X-File-Id");

                if (file_id.empty()) {
                    LOG_ERROR << "File ID is empty";
                    res.status = 400;
                    res.set_content("{\"status\":\"error\",\"message\":\"File ID is required\"}", "application/json");
                    return;
                }

                std::lock_guard<std::mutex> lock(trees_mutex);

                if (trees.find(file_id) == trees.end()) {
                    // Create new tree for this file
                    trees[file_id] = new BwtFS::Node::bw_tree();
                }

                // Write chunk data
                if (req.body.size() > 0) {
                    trees[file_id]->write(const_cast<char*>(req.body.data()), req.body.size());
                } else {
                    LOG_WARNING << "Empty chunk received: " << chunk_index;
                }

                json response_obj;

                if (chunk_index + 1 == total_chunks) {
                    // All chunks received
                    trees[file_id]->flush();
                    trees[file_id]->join();

                    response_obj["status"] = "success";
                    response_obj["token"] = trees[file_id]->get_token();
                    response_obj["message"] = "File uploaded successfully";

                    // Clean up
                    delete trees[file_id];
                    trees.erase(file_id);
                } else {
                    response_obj["status"] = "chunk_received";
                    response_obj["chunk"] = chunk_index;
                }

                res.set_content(response_obj.dump(), "application/json");
            } catch (const std::exception& e) {
                LOG_ERROR << "Error processing upload: " << e.what();
                res.status = 500;
                json error_obj;
                error_obj["status"] = "error";
                error_obj["message"] = e.what();
                res.set_content(error_obj.dump(), "application/json");
            }
        });

        // File delete endpoint
        server.Delete("/delete/(.*)", [this](const httplib::Request& req, httplib::Response& res) {
            std::string path = req.matches[1];

            if (path.length() < 30) {
                LOG_ERROR << "Invalid token: " << path;
                res.status = 400;
                res.set_content("Invalid token", "text/plain");
                return;
            }

            try {
                LOG_DEBUG << "Deleting file: " << path;
                BwtFS::Node::bw_tree tree(path, true);

                tree.delete_file();

                json response_obj;
                response_obj["status"] = "success";
                response_obj["message"] = "File deleted successfully";

                res.set_content(response_obj.dump(), "application/json");
            } catch (const std::exception& e) {
                LOG_ERROR << "Error deleting file: " << e.what();
                res.status = 404;
                res.set_content("File not found", "text/plain");
            }
        });
    }

    void start(const std::string& host, int port) {
        LOG_INFO << "Server starting at http://" << host << ":" << port;
        auto config = BwtFS::Config::getInstance();
        size_t max_body_size = std::stoull(config["server"]["max_body_size"]); // Default 100MB

        // Set server settings
        server.set_payload_max_length(max_body_size); // 100MB max payload

        // Start server
        if (!server.listen(host.c_str(), port)) {
            LOG_ERROR << "Failed to start server";
            throw std::runtime_error("Failed to start server");
        }
    }

    ~HttpServer() {
        // Clean up any remaining trees
        std::lock_guard<std::mutex> lock(trees_mutex);
        for (auto& pair : trees) {
            delete pair.second;
        }
        trees.clear();
    }
};

#endif // __SERVER_HPP__