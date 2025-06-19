#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <boost/asio.hpp>
#include "util/log.h"

using BwtFS::Util::Logger;
using boost::asio::ip::tcp;

class Client {
    private:
        boost::asio::io_context io_context;
        tcp::socket socket;
        boost::asio::streambuf buffer;

        void connectToServer(const std::string& host, const std::string& port) {
            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve(host, port);
            boost::asio::connect(socket, endpoints);
        }

        void sendCommand(const std::string& command) {
            boost::asio::write(socket, boost::asio::buffer(command));
        }

        std::string readLine() {
            boost::asio::read_until(socket, buffer, "\r\n");
            std::istream is(&buffer);
            std::string line;
            std::getline(is, line);
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return line;
        }

        std::string readBulkString() {
            std::string length_line = readLine();
            // LOG_DEBUG << "Received bulk string length: " << length_line;
            if (length_line == "-1"){
                return "$-1\r\n";
            }
            try {
                return readLine();
            } catch (...) {
                throw std::runtime_error("Invalid bulk string length");
            }
        }

        std::string readBulkString(const std::string& length_line) {
            if (length_line == "-1"){
                return "$-1\r\n";
            }
            try {
                return readLine();
            } catch (...) {
                throw std::runtime_error("Invalid bulk string length");
            }
        }

        std::string processLine(std::string& line){
            if (line.empty()) {
                throw std::runtime_error("Received empty line");
            }
            if (line[0] == '+') {
                return line.substr(1);
            } else if (line[0] == '-') {
                return line.substr(1);
            } else if (line[0] == ':') {
                return line.substr(1);
            } else if (line[0] == '$') {
                return readBulkString(line.substr(1));
            } else if (line[0] == '*') {
                return readResponse();
            } else {
                throw std::runtime_error("Unknown response type: " + line);
            }
        }

        std::string readResponse() {
            char type;
            boost::asio::read(socket, boost::asio::buffer(&type, 1));
            switch (type){
                case '+':
                case '-':
                case ':':
                    return readLine();
                case '$':
                    return readBulkString();
                case '*': {
                    std::string count_line = readLine();
                    if (count_line == "-1") {
                        return "*-1\r\n";
                    }
                    try {
                        int count = std::stoi(count_line);
                        LOG_DEBUG << "Received array length: " << count;
                        std::string response = std::to_string(count) + "\r\n";
                        for (int i = 0; i < count; ++i) {
                            std::string line = readLine();
                            response += processLine(line) + "\r\n";
                        }
                        return response;
                    } catch (...) {
                        throw std::runtime_error("Invalid array length");
                    }
                }
                default:
                    throw std::runtime_error("Unknown response type");
                }
            }

            std::string formatCommand(const std::vector<std::string>& args) {
                std::stringstream ss;
                ss << "*" << args.size() << "\r\n";
                for (const auto& arg : args) {
                    ss << "$" << arg.size() << "\r\n" << arg << "\r\n";
                }
                // std::cout << "Formatted command: " << ss.str();
                return ss.str();
            }

    public:
        Client(const std::string& host = "127.0.0.1", const std::string& port = "6379")
            : socket(io_context) {
            connectToServer(host, port);
        }

        ~Client() {
            if (socket.is_open()) {
                socket.close();
            }
        }

        std::string executeCommand(const std::vector<std::string>& args) {
            if (args.empty()) {
                throw std::invalid_argument("Command arguments cannot be empty");
            }
            std::string command = formatCommand(args);
            sendCommand(command);
            // LOG_DEBUG << "Sent command: " << command;
            return readResponse();
        }

        std::string set(const std::string& key, const std::string& value, const std::string& expiration = "") {
            if (expiration.empty()) {
                return executeCommand({"SET", key, value});
            } else {
                return executeCommand({"SET", key, value, "EX", expiration});
            }
        }

        std::string setnx(const std::string& key, const std::string& value) {
            return executeCommand({"SETNX", key, value});
        }
    
        std::string get(const std::string& key) {
            return executeCommand({"GET", key});
        }
        
        std::string ping(const std::string& message = "") {
            if (message.empty()) {
                return executeCommand({"PING"});
            } else {
                return executeCommand({"PING", message});
            }
        }
        
        std::string del(const std::string& key) {
            return executeCommand({"DEL", key});
        }
        
        std::string exists(const std::string& key) {
            return executeCommand({"EXISTS", key});
        }

        std::string keys(const std::string& pattern = "*") {
            return executeCommand({"KEYS", pattern});
        }

        std::string auth(const std::string& password) {
            return executeCommand({"AUTH", password});
        }
};

int main(){
    try {
        Client client("127.0.0.1", "6379");
        std::cout << "Connected to server." << std::endl;
        std::string response = client.auth("123456");
        std::cout << "Auth response: " << response << std::endl;
        response = client.ping();
        std::cout << "Ping response: " << response << std::endl;
        std::string operation, key, value;
        while (true) {
            std::cout << "Enter operation (set/get/del/exists/keys/exit): ";
            std::cin >> operation;
            if (operation == "exit") {
                break;
            }
            if (operation == "set") {
                std::cout << "Enter key: ";
                std::cin >> key;
                std::cout << "Enter value: ";
                std::cin >> value;
                response = client.set(key, value);
                std::cout << "Set response: " << response << std::endl;
            } else if (operation == "get") {
                std::cout << "Enter key: ";
                std::cin >> key;
                response = client.get(key);
                std::cout << "Get response: " << response << std::endl;
            } else if (operation == "del") {
                std::cout << "Enter key: ";
                std::cin >> key;
                response = client.del(key);
                std::cout << "Del response: " << response << std::endl;
            } else if (operation == "exists") {
                std::cout << "Enter key: ";
                std::cin >> key;
                response = client.exists(key);
                std::cout << "Exists response: " << response << std::endl;
            } else if (operation == "keys") {
                response = client.keys("*");
                std::cout << "Keys response: " << response << std::endl;
            } else {
                std::cout << "Unknown operation." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}