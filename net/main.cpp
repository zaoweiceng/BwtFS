#include "server.hpp"

void http_server(tcp::acceptor& acceptor) {
    auto socket = std::make_shared<tcp::socket>(acceptor.get_executor());
    
    acceptor.async_accept(*socket, [&acceptor, socket](boost::system::error_code ec) {
        if (!ec) {
            std::make_shared<http_connection>(std::move(*socket))->start();
        } else {
            LOG_ERROR << "Accept error: " << ec.message();
        }
        http_server(acceptor);
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
        http_server(acceptor);
        LOG_INFO << "Server started at: http://" << address.to_string() << ":" << port;
        ioc.run();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        system("pause");
        return EXIT_FAILURE;
    }
    system("pause");
    return 0;
}
