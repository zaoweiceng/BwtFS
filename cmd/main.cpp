#include "BwtFS.h"

bool write_file(const std::string& system_path, const std::string& file_path) {
    auto system = BwtFS::System::openBwtFS(system_path);
    auto file = std::fstream();
    file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR << "打开文件失败: " << file_path;
        return false;
    }
    char buffer[8190];
    BwtFS::Node::bw_tree tree;
    while (true) {
        file.read(buffer, sizeof(buffer));
        auto size = file.gcount();
        if (size == 0) {
            break;
        }
        tree.write(buffer, size);
    }
    tree.flush();
    file.close();
    tree.join();
    auto token = tree.get_token();
    LOG_INFO << "文件写入成功，Token: " << token;
    return true;
}

/*
    TODO: 
        实现文件拖拽上传，自动识别文件路径
        根据token获取文件并下载到指定路径或者当前目录
        实现文件删除
*/
int main(int argc, char* argv[]) {
    // 初始化
    init();
    auto config = BwtFS::Config::getInstance();
    LOG_INFO << "BwtFS initialized.";
    LOG_DEBUG << argc << " arguments provided.";
    if (argc == 1){
        std::cout << "请输入文件路径：";
        std::string path;
        std::cin >> path;
        auto system = BwtFS::System::openBwtFS(path);
        if (system == nullptr) {
            LOG_ERROR << "打开文件系统失败！";
            return 1;
        }
        std::cout << "请输入要写入的文件路径：";
        std::string file_path;
        std::cin >> file_path;
        auto file = std::fstream();
        file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR << "打开文件失败！";
            return 1;
        }
        char buffer[8190];
        BwtFS::Node::bw_tree tree;
        while (true) {
            file.read(buffer, sizeof(buffer));
            auto size = file.gcount();
            if (size == 0) {
                break;
            }
            tree.write(buffer, size);
        }
        tree.flush();
        file.close();
        tree.join();
        auto token = tree.get_token();
        LOG_INFO << "文件写入成功，Token: " << token;
    }else if (argc == 2) {
        if (std::string(argv[1]) == "create"){
            std::cout << "请输入文件系统路径：";
            std::string path;
            std::cin >> path;
            std::cout << "请输入文件系统大小（单位：MB，默认" << 
                        std::stoull(config["system"]["size"]) / BwtFS::MB << ", 输入0为默认值）：";
            size_t file_size = std::stoull(config["system"]["size"]);
            std::cin >> file_size;
            if (file_size == 0) {
                file_size = std::stoull(config["system"]["size"]);
            }else{
                file_size *= BwtFS::MB; // 转换为字节
            }
            LOG_INFO << "创建文件系统：" << path << "，大小：" << file_size << " 字节";
            BwtFS::System::File::createFile(path, file_size);
            BwtFS::System::initBwtFS(path);
            std::cout << "文件系统已创建：" << path << std::endl;
        }
    }else if (argc == 3){
        LOG_DEBUG << argv[1] << " " << argv[2];
        if (std::string(argv[1]).find(".bwt") != std::string::npos) {
            if (!write_file(argv[1], argv[2])) {
                LOG_ERROR << "写入文件失败！";
                return 1;
            } else {
                LOG_INFO << "文件写入成功！";
            }
        }else{
            if (!write_file(argv[2], argv[1])) {
                LOG_ERROR << "写入文件失败！";
                return 1;
            } else {
                LOG_INFO << "文件写入成功！";
            }
        }
    }
    system("pause");
    return 0;
}