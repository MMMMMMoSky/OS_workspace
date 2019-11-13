#include "func_def.h"

struct file_directory path_root;  // root
struct file_directory *path_now;  // file system path now
byte hd_usage[6][1024];           // 前 6 个硬盘块, 储存索引树/硬盘块的使用情况 (详见README)

// hd_buf 缓存一个硬盘块(-1表示没有), hd_buf_blk 表示块号
int hd_buf_blk;
byte hd_buf[1024];

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

// 从硬盘的某一个位置加载索引节点, 并判断加载位置与节点自身储存的位置是否一致
byte load_index_node(struct file_directory *node, struct blk_ptr *pos)
{
    if (pos->block != hd_buf_blk) {
        hd_buf_blk = pos->block;
        read_disk(hd_buf_blk, hd_buf);
    }

    // 或许可以使用一句 memcpy 搞定
    // byte *index = hd_buf + pos->index;
    // node->blk = *((struct blk_ptr*)index); index += sizeof(struct blk_ptr);
    // node->lblk = *((struct blk_ptr*)index); index += sizeof(struct blk_ptr);
    // node->rblk = *((struct blk_ptr*)index); index += sizeof(struct blk_ptr);
    // node->left = *((struct file_directory**)index); index += sizeof(struct file_directory*);
    // node->right = *((struct file_directory**)index); index += sizeof(struct file_directory*);
    // node->father = *((struct file_directory**)index); index += sizeof(struct file_directory*);
    // node->start_block = *((int*)index); index += sizeof(int);
    // memcpy(node->name, index, MAX_NAME_BYTE);
    memcpy(node, hd_buf + pos->index, sizeof(struct file_directory));

    return node->blk.block != pos->block || node->blk.index != pos->index;
}

// 将一个索引节点储存到硬盘中
void save_index_node(struct file_directory *node)
{
    if (node->blk.block != hd_buf_blk) {
        hd_buf_blk = node->blk.block;
        read_disk(hd_buf_blk, hd_buf);
    }

    memcpy(hd_buf_blk + node->blk.index, node, sizeof(struct file_directory));
    write_disk(hd_buf_blk, hd_buf_blk);
}

void unformatted_hard_disk()
{
    printf("\nUnformatted hard disk: do you want to format it? (y/n)");
    char line[4];
    getline(line, 4);
    if (line[0] == 'n') {
        printf("Ok, you can shut down now...");
        while (1);
    }

    // 1. empty usage
    for (int i = 0; i < 1024; i++) {
        hd_usage[0][i] = hd_usage[1][i] = hd_usage[2][i] = 0;
        hd_usage[3][i] = hd_usage[4][i] = hd_usage[5][i] = 0;
    }
    hd_usage[0][7] |= 0x80;  // root node usage
    for (int i = 0; i < 6; i++) {
        write_disk(i, hd_usage[i]);
    }

    // 2. create root index
    set_string(&path_root, "/");
    path_root.father = path_root.left = path_root.right = 0;
    path_root.blk.block = 6, path_root.blk.index = 0;
    path_root.lblk.index = path_root.rblk.index = 1024;
    path_root.start_block = -1;
    save_index_node(&path_root);
}

// 根据 node 的 lblk 和 rblk 从硬盘加载它的左右子节点
// 检查位置是否一致并设置指针
byte load_child_node(struct file_directory* node)
{
    if (node->lblk.index < 1024) {
        node->left = (struct file_directory*)mem_alloc(sizeof(struct file_directory));
        if (load_index_node(node->left, &node->lblk) ||  // 先加载左子节点
            load_child_node(node->left)) {               // 然后递归加载左子树
            mem_free(node->left, sizeof(struct file_directory));
            return 1;
        }
    }
    else {
        node->left = 0;
    }
    if (node->rblk.index < 1024) {
        node->right = (struct file_directory*)mem_alloc(sizeof(struct file_directory));
        if (load_index_node(node->right, &node->rblk) ||
            load_child_node(node->right)) {
            mem_free(node->right, sizeof(struct file_directory));
            return 1;
        }
    }
    else {
        node->right = 0;
    }

    // 设置 father 
    for (struct file_directory* p = node->left; p; p = p->right) {
        p->father = node;
    }
    return 0;
}

// 初始化文件系统
// TODO: 从磁盘加载
void init_file_system()
{
    hd_buf_blk = -1;
    path_now = &path_root;

    // 加载前 6 个块
    for (int i = 0; i < 6; i++) {
        read_disk(i, hd_usage[i]); 
    }

    struct blk_ptr tmp;
    tmp.block = 6, tmp.index = 0;

    if (!(hd_usage[0][7] & 0x80) ||          // 检查根目录索引节点是否记录为已使用
        load_index_node(&path_root, &tmp) || // 加载根节点并检查位置储存是否正确
        path_root.rblk.index < 1024 ||       // 根节点不可能有 right 指针
        path_root.father != 0 ||             // 根节点的 father 必须为 0, 除了根节点的 father 之外, 其他节点的 father, left, right 都没有意义
        load_child_node(path_now)) {         // 加载其他节点

        unformatted_hard_disk();

    }
}

// 建一个新的目录, 左孩子右兄弟上面是父节点模式
struct file_directory* create_new_directory(struct file_directory *path, const char *name)
{
    // 在目前的设计中, 目录和文件仅有 flag 一项不同, 所以可以复用代码
    struct file_directory *new_directory = create_new_file(path, name);
    new_directory->start_block = -1;
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
    new_file->start_block = 0;  // TODO: real file system
    return new_file;
}

// 删除 p 指向的文件
void remove_file(struct file_directory *p)
{
    // 文件一定有父指针, 并且没有 left 指针, 要找到该文件在 right 链表中的上一个节点
    struct file_directory *fa = p->father->left;

    // 特判: p 就是第一个节点
    if (fa == p) {
        p->father->left = p->right;
        mem_free(p, sizeof(struct file_directory));
        return;
    }

    while (fa->right != p) fa = fa->right;
    fa->right = p->right;
    mem_free(p, sizeof(struct file_directory));
}

// 删除 p 指向的文件夹
void remove_directory(struct file_directory *p)
{
    // 防止野指针: 如果当前路径在被删除的目录之下, 那么把当前路径重新设置到 /
    if (p == path_now) {
        path_now = &path_root;
    }

    // 递归删除左子树中的内容 
    for (struct file_directory* i = p->left; i; i = i->right) {
        remove_directory(i);
    }

    // 当文件夹为空的时候, 就可以当作文件删除了, 但是根目录不可以删除
    if (p != &path_root) {
        remove_file(p);
    }
}
