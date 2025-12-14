#include "server.hpp"

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

        std::string host = config["server"]["host"];
        unsigned short port = static_cast<unsigned short>(std::stoi(config["server"]["port"]));

        // Create and start the HTTP server
        HttpServer server;

        LOG_INFO << "Server started at: http://" << host << ":" << port;

        // Start server (this will block until server stops)
        server.start(host, port);

    }
    catch (const std::exception& e) {
        LOG_ERROR << "Error: " << e.what();
        std::cerr << "Error: " << e.what() << std::endl;

#ifdef _WIN32
        system("pause");
#endif
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    system("pause");
#endif
    return 0;
}