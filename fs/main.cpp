#ifdef _WIN32
  #define FUSE_USE_VERSION 29
  #include <fuse.h>
#else
 #define FUSE_USE_VERSION 35
  #include <fuse3/fuse.h>
#endif

#include <cstring>
#include <iostream>
#include "core.h"

static MyFS myfs;

// 获取文件属性（ls 调用）
#ifdef _WIN32
static int myfs_getattr(const char *path, struct fuse_stat *stbuf, struct fuse_file_info *fi)
#else
static int myfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
#endif
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        return 0;
    }

    // 查一下文件系统中是否有这个文件
    auto it = myfs.files_.find(path);
    if (it == myfs.files_.end()) {
        return -ENOENT;  // 关键：文件不存在时要返回负值！
    }

    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = it->second.data.size();
    return 0;
}

// 读取目录内容（ls）
#ifdef _WIN32
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        fuse_off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
#else
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
#endif
{
#ifdef _WIN32
    // Windows WinFSP 版本
    filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    for (auto &name : myfs.list_files())
        filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS);
#else
    // Linux libfuse 版本
    filler(buf, ".", nullptr, 0, 0);
    filler(buf, "..", nullptr, 0, 0);
    for (auto &name : myfs.list_files())
        filler(buf, name.c_str(), nullptr, 0, 0);
#endif
    return 0;
}

// 打开文件
#ifdef _WIN32
static int myfs_open_fuse(const char *path, struct fuse_file_info *fi)
#else
static int myfs_open_fuse(const char *path, struct fuse_file_info *fi)
#endif
{
    fi->fh = myfs.open(path);
    return fi->fh < 0 ? -ENOENT : 0;
}

static int myfs_create_fuse(const char* path, fuse_mode_t mode, struct fuse_file_info *fi) {
    int fd = myfs.open(path);  // 如果 open 本身不创建，则你要手动 new 一个文件
    if (fd < 0) {
        // 如果 open 没有自动创建，则明确创建：
        myfs.create(path);      // TODO
        fd = myfs.open(path);
    }

    if (fd < 0) return -EIO;
    fi->fh = fd;
    return 0;
}


// 读文件
#ifdef _WIN32
static int myfs_read_fuse(const char *path, char *buf, size_t size, fuse_off_t offset,
                          struct fuse_file_info *fi)
#else
static int myfs_read_fuse(const char *path, char *buf, size_t size, off_t offset,
                          struct fuse_file_info *fi)
#endif
{
    return myfs.read(fi->fh, buf, size, offset);
}

// 写文件
#ifdef _WIN32
static int myfs_write_fuse(const char *path, const char *buf, size_t size, fuse_off_t offset,
                           struct fuse_file_info *fi)
#else
static int myfs_write_fuse(const char *path, const char *buf, size_t size, off_t offset,
                           struct fuse_file_info *fi)
#endif
{
    return myfs.write(fi->fh, buf, size, offset);
}

// 删除文件
static int myfs_unlink_fuse(const char *path){
    return myfs.remove(path);
}

// 关闭文件
static int myfs_release_fuse(const char *path, struct fuse_file_info *fi){
    return myfs.close(fi->fh);
}

#ifdef _WIN32
static int myfs_statfs(const char *path, struct fuse_statvfs *stbuf)
#else
static int myfs_statfs(const char *path, struct statvfs *stbuf)
#endif
{
    memset(stbuf, 0, sizeof(*stbuf));

    // 合理值（单位：block）
    stbuf->f_bsize = 4096;        // block size
    stbuf->f_frsize = 4096;
    stbuf->f_blocks = 1024 * 1024; // 总块数（比如 4GB）
    
    size_t used_size = 0;
    for (const auto& pair : myfs.files_) {
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

static int myfs_access_fuse(const char *path, int mask) {
    // 永远允许访问（简单实现）
    return 0;
}

static int myfs_chmod_fuse(const char *path, fuse_mode_t mode, struct fuse3_file_info *fi) {
    // 不存权限，直接成功
    return 0;
}

static int myfs_chown_fuse(const char *path, fuse_uid_t uid, fuse_gid_t gid, struct fuse3_file_info *fi) {
    // 忽略所有者设置
    return 0;
}

static struct fuse_operations myfs_oper = {};

// static struct fuse_operations myfs_oper = {
//     .getattr = myfs_getattr,
//     .readdir = myfs_readdir,
//     .open    = myfs_open_fuse,
//     .read    = myfs_read_fuse,
//     .write   = myfs_write_fuse,
//     .unlink  = myfs_unlink_fuse,
//     .release = myfs_release_fuse,
// };

int main(int argc, char *argv[]){
    myfs_oper.getattr = myfs_getattr;
    myfs_oper.readdir = myfs_readdir;
    myfs_oper.open    = myfs_open_fuse;
    myfs_oper.read    = myfs_read_fuse;
    myfs_oper.write   = myfs_write_fuse;
    myfs_oper.unlink  = myfs_unlink_fuse;
    myfs_oper.release = myfs_release_fuse;
    // windows
    myfs_oper.statfs  = myfs_statfs;
    myfs_oper.create  = myfs_create_fuse;
    myfs_oper.chmod   = myfs_chmod_fuse;
    myfs_oper.chown   = myfs_chown_fuse,
    myfs_oper.access  = myfs_access_fuse,

    std::cout << "Starting MyFS FUSE filesystem..." << std::endl;
    char *my_argv[] = {
            argv[0],
            (char *)"X:",                       // 挂载点
            // (char *)"-o", (char *)"VolumePrefix=MyFileSystem",  // WinFSP 专用参数
            (char *)"-o", (char *)"file_system=MyFS", // 文件系统名称
            // (char *)"-o", (char *)"debug",     // 输出调试日志
            (char *)"-o", (char *)"volname=MyFS", // 卷标
            (char *)"-o", (char *)"uid=0",
            (char *)"-o", (char *)"gid=0",
            NULL
        };

    int my_argc = sizeof(my_argv) / sizeof(my_argv[0]) - 1;
    #ifdef _WIN32
        return fuse_main_real(my_argc, my_argv, &myfs_oper, sizeof(myfs_oper), NULL);
    #else
        return fuse_main(my_argc, my_argv, &myfs_oper, NULL);
    #endif
}
