// 跨平台FUSE头文件引入
// 根据不同平台使用不同的FUSE版本和头文件路径
#ifdef _WIN32
    // Windows平台使用WinFSP，FUSE版本29
    #define FUSE_USE_VERSION 29
    #include <fuse.h>
#elif defined(__APPLE__)
    // macOS平台使用macFUSE，FUSE版本26（兼容性更好）
    #define FUSE_USE_VERSION 26
    #include <fuse.h>
#else
    // Linux平台使用libfuse3，FUSE版本35
    #define FUSE_USE_VERSION 35
    #include <fuse3/fuse.h>
#endif

#include <cstring>
#include <iostream>
#include "core.h"

static MemoryFS memory_fs;

// 获取文件属性（ls命令调用的核心函数）
// 跨平台处理不同平台的stat结构体差异
#ifdef _WIN32
    // Windows WinFSP使用自定义的fuse_stat结构体
static int memory_fs_getattr(const char *path, struct fuse_stat *stbuf, struct fuse_file_info *fi)
#elif defined(__APPLE__)
    // macOS macFUSE使用标准的struct stat
static int memory_fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
#else
    // Linux libfuse3也使用标准struct stat
static int memory_fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
#endif
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        return 0;
    }

    // 查一下文件系统中是否有这个文件
    auto it = memory_fs.files_.find(path);
    if (it == memory_fs.files_.end()) {
        return -ENOENT;  // 关键：文件不存在时要返回负值！
    }

    // 根据文件或目录设置不同的属性
    if (it->second.is_directory) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        stbuf->st_size = 4096;  // 目录通常显示为4096字节
    } else {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = it->second.data.size();
    }
    return 0;
}

// 读取目录内容（ls命令的核心实现）
// 跨平台处理不同平台的偏移量类型差异
#ifdef _WIN32
    // Windows使用fuse_off_t类型
static int memory_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        fuse_off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
#elif defined(__APPLE__)
    // macOS使用FUSE 2.9 API，签名略有不同
    // readdir函数在FUSE 2.9中只有5个参数，没有flags参数
static int memory_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
#else
    // Linux使用标准off_t类型
static int memory_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
#endif
{
    (void)offset; (void)fi;
#ifdef _WIN32
    // Windows WinFSP 版本 - 支持FUSE_FILL_DIR_PLUS标志
    filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    for (auto &name : memory_fs.list_files_in_dir(path))
        filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS);
#elif defined(__APPLE__)
    // macOS FUSE 2.9 API - filler函数只有4个参数
    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);
    for (auto &name : memory_fs.list_files_in_dir(path))
        filler(buf, name.c_str(), nullptr, 0);
#else
    // Linux libfuse3 版本 - 支持FUSE_FILL_DIR_PLUS标志以提升性能
    filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    for (auto &name : memory_fs.list_files_in_dir(path))
        filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS);
#endif
    return 0;
}

// 打开文件 - 跨平台统一的函数签名
// 所有平台的open函数签名都是一致的
static int memory_fs_open_fuse(const char *path, struct fuse_file_info *fi)
{
    fi->fh = memory_fs.open(path);
    return fi->fh < 0 ? -ENOENT : 0;
}

// 创建文件 - 跨平台处理不同平台的参数类型差异
#ifdef _WIN32
    // Windows使用fuse_mode_t类型
static int memory_fs_create_fuse(const char* path, fuse_mode_t mode, struct fuse_file_info *fi) {
    (void)mode; // 避免未使用参数警告，目前我们的简单实现忽略mode参数
#elif defined(__APPLE__)
    // macOS使用标准mode_t类型
static int memory_fs_create_fuse(const char* path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode; // 避免未使用参数警告，目前我们的简单实现忽略mode参数
#else
    // Linux使用标准mode_t类型
static int memory_fs_create_fuse(const char* path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode; // 避免未使用参数警告，目前我们的简单实现忽略mode参数
#endif

    // 首先尝试打开文件，如果文件不存在则创建
    int fd = memory_fs.open(path);  // open函数内部会自动创建不存在的文件
    if (fd < 0) {
        // 如果打开失败，尝试显式创建文件
        if (memory_fs.create(path) < 0) {
            return -EIO; // 创建失败返回I/O错误
        }
        // 创建成功后再次尝试打开
        fd = memory_fs.open(path);
    }

    if (fd < 0) return -EIO; // 如果仍然失败，返回I/O错误
    fi->fh = fd;              // 设置文件句柄
    return 0;                 // 返回成功
}


// 读文件 - 跨平台处理不同平台的偏移量类型差异
#ifdef _WIN32
    // Windows使用fuse_off_t类型
static int memory_fs_read_fuse(const char *path, char *buf, size_t size, fuse_off_t offset,
                          struct fuse_file_info *fi)
#elif defined(__APPLE__)
    // macOS使用标准off_t类型
static int memory_fs_read_fuse(const char *path, char *buf, size_t size, off_t offset,
                          struct fuse_file_info *fi)
#else
    // Linux使用标准off_t类型
static int memory_fs_read_fuse(const char *path, char *buf, size_t size, off_t offset,
                          struct fuse_file_info *fi)
#endif
{
    return memory_fs.read(fi->fh, buf, size, offset);
}

// 写文件 - 跨平台处理不同平台的偏移量类型差异
#ifdef _WIN32
    // Windows使用fuse_off_t类型
static int memory_fs_write_fuse(const char *path, const char *buf, size_t size, fuse_off_t offset,
                           struct fuse_file_info *fi)
#elif defined(__APPLE__)
    // macOS使用标准off_t类型
static int memory_fs_write_fuse(const char *path, const char *buf, size_t size, off_t offset,
                           struct fuse_file_info *fi)
#else
    // Linux使用标准off_t类型
static int memory_fs_write_fuse(const char *path, const char *buf, size_t size, off_t offset,
                           struct fuse_file_info *fi)
#endif
{
    return memory_fs.write(fi->fh, buf, size, offset);
}

// 删除文件 - 跨平台统一的实现
static int memory_fs_unlink_fuse(const char *path){
    return memory_fs.remove(path);
}

// 关闭文件 - 跨平台统一的实现
static int memory_fs_release_fuse(const char *path, struct fuse_file_info *fi){
    return memory_fs.close(fi->fh);
}

// 获取文件系统统计信息 - 跨平台处理不同平台的结构体差异
#ifdef _WIN32
    // Windows WinFSP使用自定义的fuse_statvfs结构体
static int memory_fs_statfs(const char *path, struct fuse_statvfs *stbuf)
#elif defined(__APPLE__)
    // macOS macFUSE使用标准的struct statvfs
static int memory_fs_statfs(const char *path, struct statvfs *stbuf)
#else
    // Linux libfuse3也使用标准struct statvfs
static int memory_fs_statfs(const char *path, struct statvfs *stbuf)
#endif
{
    memset(stbuf, 0, sizeof(*stbuf));

    // 合理值（单位：block）
    stbuf->f_bsize = 4096;        // block size
    stbuf->f_frsize = 4096;
    stbuf->f_blocks = 1024 * 1024; // 总块数（比如 4GB）
    
    size_t used_size = 0;
    for (const auto& pair : memory_fs.files_) {
        used_size += pair.second.data.size();
    }
    size_t free_size = (stbuf->f_blocks * stbuf->f_bsize) - used_size;
    std::cout << "[statfs] used_size=" << used_size << " free_size=" << free_size << std::endl;
    stbuf->f_bfree  = free_size / stbuf->f_bsize;  // 空闲块数
    stbuf->f_bavail = stbuf->f_bfree; // 非特权用户可用

    stbuf->f_files  = 100000;     // 文件总数上限
    stbuf->f_ffree  = 648;     // 剩余文件数

    return 0;
}

// 文件访问权限检查 - 跨平台统一的实现
static int memory_fs_access_fuse(const char *path, int mask) {
    // 简化实现：永远允许访问所有文件
    // 在实际应用中，这里应该检查文件权限
    (void)path;     // 避免未使用参数警告
    (void)mask;     // 避免未使用参数警告
    return 0;
}

// 重命名/移动文件 - 跨平台处理不同平台的参数类型差异
#ifdef _WIN32
    // Windows使用带flags参数的rename函数
static int memory_fs_rename_fuse(const char *old_path, const char *new_path, unsigned int flags) {
    (void)flags; // flags参数暂时忽略，用于未来的原子性重命名
    return memory_fs.rename(old_path, new_path);
}
#elif defined(__APPLE__)
    // macOS使用传统的2参数rename函数
static int memory_fs_rename_fuse(const char *old_path, const char *new_path) {
    return memory_fs.rename(old_path, new_path);
}
#else
    // Linux使用带flags参数的rename函数
static int memory_fs_rename_fuse(const char *old_path, const char *new_path, unsigned int flags) {
    (void)flags; // flags参数暂时忽略，用于未来的原子性重命名
    return memory_fs.rename(old_path, new_path);
}
#endif

// 创建目录 - 跨平台处理不同平台的参数类型差异
#ifdef _WIN32
    // Windows使用fuse_mode_t类型
static int memory_fs_mkdir_fuse(const char *path, fuse_mode_t mode) {
    (void)mode; // 忽略权限参数
    return memory_fs.mkdir(path);
}
#endif

// 文件权限修改 - 跨平台处理不同平台的参数类型差异
#ifdef _WIN32
    // Windows使用fuse_mode_t类型，但通常不支持chmod操作
static int memory_fs_chmod_fuse(const char *path, fuse_mode_t mode, struct fuse_file_info *fi) {
    // Windows文件系统不支持Unix风格的权限，直接返回成功
    (void)path; (void)mode; (void)fi;
    return 0;
}

    // Windows的chown操作
static int memory_fs_chown_fuse(const char *path, fuse_uid_t uid, fuse_gid_t gid, struct fuse_file_info *fi) {
    // Windows不支持Unix风格的文件所有权，直接返回成功
    (void)path; (void)uid; (void)gid; (void)fi;
    return 0;
}
#elif defined(__APPLE__)
    // macOS使用标准mode_t类型
static int memory_fs_mkdir_fuse(const char *path, mode_t mode) {
    (void)mode; // 忽略权限参数
    return memory_fs.mkdir(path);
}

    // macOS使用标准Unix类型
static int memory_fs_chmod_fuse(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // 简化实现：忽略权限修改，总是返回成功
    (void)path; (void)mode; (void)fi;
    return 0;
}

static int memory_fs_chown_fuse(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // 简化实现：忽略所有者修改，总是返回成功
    (void)path; (void)uid; (void)gid; (void)fi;
    return 0;
}
#else
    // Linux使用标准mode_t类型
static int memory_fs_mkdir_fuse(const char *path, mode_t mode) {
    (void)mode; // 忽略权限参数
    return memory_fs.mkdir(path);
}

    // Linux使用标准Unix类型
static int memory_fs_chmod_fuse(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // 简化实现：忽略权限修改，总是返回成功
    (void)path; (void)mode; (void)fi;
    return 0;
}

static int memory_fs_chown_fuse(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    // 简化实现：忽略所有者修改，总是返回成功
    (void)path; (void)uid; (void)gid; (void)fi;
    return 0;
}
#endif

#ifdef __APPLE__
    // macOS适配函数 - getattr函数没有fuse_file_info参数
static int memory_fs_getattr_macos_adapter(const char *path, struct stat *stbuf) {
    // 调用我们的实现，忽略第三个参数
    return memory_fs_getattr(path, stbuf, nullptr);
}

    // macOS适配函数 - chmod函数没有fuse_file_info参数
static int memory_fs_chmod_macos_adapter(const char *path, mode_t mode) {
    // 调用我们的实现，忽略第三个参数
    return memory_fs_chmod_fuse(path, mode, nullptr);
}

    // macOS适配函数 - chown函数没有fuse_file_info参数
static int memory_fs_chown_macos_chown_adapter(const char *path, uid_t uid, gid_t gid) {
    // 调用我们的实现，忽略第四个参数
    return memory_fs_chown_fuse(path, uid, gid, nullptr);
}
#endif

static struct fuse_operations memory_fs_oper = {};

// static struct fuse_operations memory_fs_oper = {
//     .getattr = memory_fs_getattr,
//     .readdir = memory_fs_readdir,
//     .open    = memory_fs_open_fuse,
//     .read    = memory_fs_read_fuse,
//     .write   = memory_fs_write_fuse,
//     .unlink  = memory_fs_unlink_fuse,
//     .release = memory_fs_release_fuse,
// };


int main(){

    return 0;
}

// int main(int argc, char *argv[]){
//     // 初始化FUSE操作结构体
//     // 根据不同平台设置相应的函数指针，处理API差异
// #ifdef _WIN32
//     // Windows WinFSP API
//     memory_fs_oper.getattr = memory_fs_getattr;    // 获取文件属性
//     memory_fs_oper.readdir = memory_fs_readdir;    // 读取目录内容
//     memory_fs_oper.open    = memory_fs_open_fuse;  // 打开文件
//     memory_fs_oper.read    = memory_fs_read_fuse;  // 读取文件内容
//     memory_fs_oper.write   = memory_fs_write_fuse; // 写入文件内容
//     memory_fs_oper.unlink  = memory_fs_unlink_fuse; // 删除文件
//     memory_fs_oper.rename  = memory_fs_rename_fuse; // 重命名/移动文件
//     memory_fs_oper.release = memory_fs_release_fuse; // 关闭文件
//     memory_fs_oper.statfs  = memory_fs_statfs;     // 获取文件系统统计信息
//     memory_fs_oper.create  = memory_fs_create_fuse; // 创建文件
//     memory_fs_oper.mkdir   = memory_fs_mkdir_fuse;  // 创建目录
//     memory_fs_oper.chmod   = memory_fs_chmod_fuse;  // 修改文件权限
//     memory_fs_oper.chown   = memory_fs_chown_fuse;  // 修改文件所有者
//     memory_fs_oper.access  = memory_fs_access_fuse; // 检查文件访问权限
// #elif defined(__APPLE__)
//     // macOS FUSE 2.9 API - 需要适配不同的函数签名
//     memory_fs_oper.getattr = memory_fs_getattr_macos_adapter;     // getattr函数需要适配
//     memory_fs_oper.readdir = memory_fs_readdir;     // readdir函数签名已适配
//     memory_fs_oper.open    = memory_fs_open_fuse;   // 打开文件
//     memory_fs_oper.read    = memory_fs_read_fuse;   // 读取文件内容
//     memory_fs_oper.write   = memory_fs_write_fuse;  // 写入文件内容
//     memory_fs_oper.unlink  = memory_fs_unlink_fuse; // 删除文件
//     memory_fs_oper.rename  = memory_fs_rename_fuse; // 重命名/移动文件
//     memory_fs_oper.release = memory_fs_release_fuse; // 关闭文件
//     memory_fs_oper.statfs  = memory_fs_statfs;      // 获取文件系统统计信息
//     memory_fs_oper.create  = memory_fs_create_fuse; // 创建文件
//     memory_fs_oper.mkdir   = memory_fs_mkdir_fuse;  // 创建目录
//     memory_fs_oper.chmod   = memory_fs_chmod_macos_adapter;  // 修改文件权限（适配函数）
//     memory_fs_oper.chown   = memory_fs_chown_macos_chown_adapter;  // 修改文件所有者（适配函数）
//     memory_fs_oper.access  = memory_fs_access_fuse; // 检查文件访问权限
// #else
//     // Linux libfuse3 API
//     memory_fs_oper.getattr = memory_fs_getattr;    // 获取文件属性
//     memory_fs_oper.readdir = memory_fs_readdir;    // 读取目录内容
//     memory_fs_oper.open    = memory_fs_open_fuse;  // 打开文件
//     memory_fs_oper.read    = memory_fs_read_fuse;  // 读取文件内容
//     memory_fs_oper.write   = memory_fs_write_fuse; // 写入文件内容
//     memory_fs_oper.unlink  = memory_fs_unlink_fuse; // 删除文件
//     memory_fs_oper.rename  = memory_fs_rename_fuse; // 重命名/移动文件
//     memory_fs_oper.release = memory_fs_release_fuse; // 关闭文件
//     memory_fs_oper.statfs  = memory_fs_statfs;     // 获取文件系统统计信息
//     memory_fs_oper.create  = memory_fs_create_fuse; // 创建文件
//     memory_fs_oper.mkdir   = memory_fs_mkdir_fuse;  // 创建目录
//     memory_fs_oper.chmod   = memory_fs_chmod_fuse;  // 修改文件权限
//     memory_fs_oper.chown   = memory_fs_chown_fuse;  // 修改文件所有者
//     memory_fs_oper.access  = memory_fs_access_fuse; // 检查文件访问权限
// #endif

//     std::cout << "Starting memory_fs FUSE filesystem..." << std::endl;

//     // 检查是否提供了挂载点参数
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <mount_point>" << std::endl;
//         return 1;
//     }

//     // 为不同平台准备启动参数，使用用户提供的挂载点
// #ifdef _WIN32
//     // Windows WinFSP启动参数
//     char *my_argv[] = {
//             argv[0],
//             argv[1],                                     // 用户提供的挂载点
//             (char *)"-o", (char *)"file_system=bwtfs",     // 文件系统名称
//             (char *)"-o", (char *)"volname=bwtfs",        // 卷标名称
//             (char *)"-o", (char *)"uid=0",               // 用户ID
//             (char *)"-o", (char *)"gid=0",               // 组ID
//             NULL
//         };
// #elif defined(__APPLE__)
//     // macOS macFUSE启动参数
//     char *my_argv[] = {
//             argv[0],
//             argv[1],                                     // 用户提供的挂载点
//             (char *)"-o", (char *)"allow_other",         // 允许其他用户访问
//             (char *)"-o", (char *)"volname=bwtfs",        // 卷标名称
//             (char *)"-o", (char *)"uid=501",             // 当前用户ID（需要根据实际用户调整）
//             (char *)"-o", (char *)"gid=20",              // 当前组ID（staff组）
//             NULL
//         };
// #else
//     // Linux libfuse3启动参数
//     char *my_argv[] = {
//             argv[0],
//             argv[1],                                     // 用户提供的挂载点
//             (char *)"-o", (char *)"allow_other",         // 允许其他用户访问
//             (char *)"-o", (char *)"volname=bwtfs",        // 卷标名称
//             (char *)"-o", (char *)"uid=1000",           // 当前用户ID（需要根据实际用户调整）
//             (char *)"-o", (char *)"gid=1000",            // 当前组ID
//             NULL
//         };
// #endif

//     int my_argc = sizeof(my_argv) / sizeof(my_argv[0]) - 1;

//     // 根据不同平台调用相应的FUSE入口函数
// #ifdef _WIN32
//     // Windows使用fuse_main_real函数，需要传递结构体大小
//     std::cout << "Starting on Windows with WinFSP..." << std::endl;
//     return fuse_main_real(my_argc, my_argv, &memory_fs_oper, sizeof(memory_fs_oper), NULL);
// #elif defined(__APPLE__)
//     // macOS使用标准fuse_main函数
//     std::cout << "Starting on macOS with macFUSE..." << std::endl;
//     return fuse_main(my_argc, my_argv, &memory_fs_oper, NULL);
// #else
//     // Linux使用标准fuse_main函数
//     std::cout << "Starting on Linux with libfuse3..." << std::endl;
//     return fuse_main(my_argc, my_argv, &memory_fs_oper, NULL);
// #endif
// }
