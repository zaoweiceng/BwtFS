#include "BwtFS.h"

bool write_file(const std::string& system_path, const std::string& file_path) {
    auto system = BwtFS::System::getBwtFS();
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
    return true;
}

int main(int argc, char* argv[]) {
    // 初始化
    init();
    auto config = BwtFS::Config::getInstance();
    LOG_INFO << "BwtFS initialized.";
    if (argc == 0){
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
    }
    return 0;
}