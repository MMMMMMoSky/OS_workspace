#include "func_def.h"

struct file_directory path_root;  // root
struct file_directory *path_now;  // file system path now

void set_string(struct file_directory *fd, const char *name)
{
    int i = 0;
    while (name[i] != '\0')
    {
        fd->name[i] = name[i];
        i++;
    }
    fd->name[i] = '\0';
}

// 初始化文件系统
// TODO: 从磁盘加载
void init_file_system()
{
    set_string(&path_root, "/");
    path_root.father = path_root.left = path_root.right = 0;
    path_root.flag = 1;
    path_now = &path_root;
}

// 建一个新的目录, 左孩子右兄弟上面是父节点模式
struct file_directory* create_new_directory(struct file_directory *path, const char *name)
{
    // 在目前的设计中, 目录和文件仅有 flag 一项不同, 所以可以复用代码
    struct file_directory *new_directory = create_new_file(path, name);
    new_directory->flag = 1;
    return new_directory;
}

// 建立新的文件, 调用前必须保证 path 合法并且该 path 目录下不存在 name 文件名/文件夹名
// 新建成功则返回新建的文件节点指针
struct file_directory* create_new_file(struct file_directory *path, const char *name)
{
    struct file_directory* new_file;
    new_file = (struct file_directory*)mem_alloc(sizeof(struct file_directory));
    new_file->right = path->left;
    new_file->father = path;
    path->left = new_file;
    set_string(new_file, name);
    new_file->flag = 0;
    return new_file;
}

