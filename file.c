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

    memcpy(hd_buf + node->blk.index, node, sizeof(struct file_directory));
    write_disk(hd_buf_blk, hd_buf);  // (已经修复, hd_buf_blk) 之前有这么大的bug怎么能正确运行的...
}

// 分配一个新的索引节点, 并更新内存和硬盘中的记录
void new_index_node_pos(struct blk_ptr *res)
{
    // 遍历, 查找第一个 0 即可
    res->block = 6;
    res->index = 0;
    int idx = 0, bit = 1;
    while (hd_usage[0][idx] & bit) {
        if (bit == 0x80) { bit = 1; idx++; }
        else bit <<= 1;
        res->index += sizeof(struct file_directory);
        if (res->index >= 1024) {
            res->block++;
            res->index = 0;
        }
    }

    // 现在位置已经确定, 更新内存和硬盘中的存储
    hd_usage[0][idx] |= bit;
    write_disk(0, hd_usage[0]);
}

// 将索引节点的记录位置为空
void free_index_node(struct blk_ptr *p)
{
    // 先计算出 p 是第几个索引节点
    int num = (p->block - 6) * 16 + (p->index) / sizeof(struct file_directory);
    // 然后计算下标和与数
    int idx = num / 8, bit = 1 << (num & 7);
    hd_usage[0][idx] &= ~bit;  // 将 bit 位清空
    write_disk(0, hd_usage[0]);
}

// 分配一个新的硬盘块, 用于储存文件, 更新硬盘和内存中的记录
// 初始化该块为一个空文件
int new_file_block()
{
    // 从 256 号开始为储存文件的硬盘块
    // hd_usage[1] ~ hd_usage[5] 储存它们的使用情况
    int ret = 256;  // 从 256 号开始遍历
    int id = 1, idx = 0, bit = 1;  // 直接 hd_usage[1][0] 的第一位对应第 256 个硬盘块
    while (hd_usage[id][idx] & bit) {
        if (bit == 0x80) {
            bit = 1;
            if (idx == 1023) { idx = 0; id++; }
            else idx++;
        }
        else bit <<= 1;
        ret++;
    }

    // 位置确定, 更新内存和硬盘中的记录位
    hd_usage[id][idx] |= bit;
    write_disk(id, hd_usage[id]);

    // 将该文件块初始化为全0
    hd_buf_blk = ret;
    for (int i = 0; i < 1024; i++) {
        hd_buf[i] = 0;
    }
    write_disk(hd_buf_blk, hd_buf);

    return ret;
}

// 释放一个文件的所有硬盘块, 更新硬盘和内存中的记录
// 传入的是起始块号
void free_file_block(int blk)
{
    hd_buf_blk = -1;
    while (1) {
        // 在记录位中释放该块, 根据 blk 计算 id, idx, bit
        int num = blk - 256;
        int id = num / 8192 + 1, idx = (num % 8192) / 8, bit = 1 << (num & 7);
        hd_usage[id][idx] &= ~bit;
        write_disk(id, hd_usage[id]);

        // 读入该块, 判断是否还有下一块
        read_disk(blk, hd_buf);
        if (*(uint*)(hd_buf + 1020) <= 1000) break;  // 不超过 1k 说明该文件在当前块结束
        blk = *(uint*)(hd_buf + 1016);
    }
}

void unformatted_hard_disk()
{
    printf("Unformatted hard disk: do you want to format it? (y/n)");
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
    for (int i = 0; i < 6; i++) {
        write_disk(i, hd_usage[i]);
    }

    // 2. create root index
    new_index_node_pos(&path_root.blk);
    set_string(&path_root, "/");
    path_root.father = path_root.left = path_root.right = 0;
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

    if (!(hd_usage[0][0] & 1) ||             // 检查根目录索引节点是否记录为已使用
            load_index_node(&path_root, &tmp) || // 加载根节点并检查位置储存是否正确
            path_root.rblk.index < 1024 ||       // 根节点不可能有 right 指针
            path_root.father != 0 ||             // 根节点的 father 必须为 0, 除了根节点的 father 之外, 其他节点的 father, left, right 都没有意义
            load_child_node(path_now)) {         // 加载其他节点

        unformatted_hard_disk();

    }
}

// 建一个新的目录, 左孩子右兄弟上面是父节点模式
// 与新建文件夹不同的是: 新建文件不需要在硬盘中分配块, 只需要更新索引即可
struct file_directory* create_new_directory(struct file_directory *path, const char *name)
{
    struct file_directory* new_directory;
    new_directory = (struct file_directory*)mem_alloc(sizeof(struct file_directory));

    // 设置结构体信息
    set_string(new_directory, name);
    new_directory->father = path;
    new_directory->left = new_directory->right = 0;
    new_directory->lblk.index = new_directory->rblk.index = 1024;

    // 与新建文件不同: 这里设置为 -1; 只有这一句话不同, 其他都一样
    new_directory->start_block = -1;

    // 获取该节点在硬盘中的储存位置, 并更新内存和硬盘中的记录位
    new_index_node_pos(&new_directory->blk);

    // 找到 path->left->right 链表的尾节点, 将 new_directory 插入其后, 并更新该节点
    // 虽然逆序插入 O(1), 但是会增加一次硬盘读写, 所以实际效率更低
    if (path->left == 0) {
        path->left = new_directory; 
        path->left = new_directory;
        path->lblk.block = new_directory->blk.block;
        path->lblk.index = new_directory->blk.index;
        save_index_node(path);
    }
    else {
        struct file_directory* rfa = path->left;
        while (rfa->right) rfa = rfa->right;
        rfa->right = new_directory;
        rfa->rblk.block = new_directory->blk.block;
        rfa->rblk.index = new_directory->blk.index;
        save_index_node(rfa);
    }

    // 在硬盘的索引树中更新 new_directory 节点
    save_index_node(new_directory);

    return new_directory;
}

// 建立新的文件, 调用前必须保证 path 合法并且该 path 目录下不存在 name 文件名/文件夹名
// 返回新建的文件节点指针
struct file_directory* create_new_file(struct file_directory *path, const char *name)
{
    struct file_directory* new_file;
    new_file = (struct file_directory*)mem_alloc(sizeof(struct file_directory));

    // 设置结构体信息
    set_string(new_file, name);
    new_file->father = path;
    new_file->left = new_file->right = 0;
    new_file->lblk.index = new_file->rblk.index = 1024;

    // 在硬盘中创建该文件的第一个块
    new_file->start_block = new_file_block();

    // 获取该节点在硬盘中的储存位置, 并更新内存和硬盘中的记录位
    new_index_node_pos(&new_file->blk);

    // 找到 path->left->right 链表的尾节点, 将 new_file 插入其后, 并更新该节点
    // 虽然逆序插入 O(1), 但是会增加一次硬盘读写, 所以实际效率更低
    if (path->left == 0) {
        path->left = new_file; 
        path->left = new_file;
        path->lblk.block = new_file->blk.block;
        path->lblk.index = new_file->blk.index;
        save_index_node(path);
    }
    else {
        struct file_directory* rfa = path->left;
        while (rfa->right) rfa = rfa->right;
        rfa->right = new_file;
        rfa->rblk.block = new_file->blk.block;
        rfa->rblk.index = new_file->blk.index;
        save_index_node(rfa);
    }

    // 在硬盘的索引树中更新 new_file 节点
    save_index_node(new_file);

    return new_file;
}

// 删除 p 指向的文件/空文件夹
// 如果是空文件夹, 那么只需要把该索引节点释放, 并更新内存和硬盘中的记录
// 如果是文件, 那么还需要释放该文件所占用的所有文件块
void remove_file(struct file_directory *p)
{
    // 如果是文件, 释放该文件占用的所有文件块
    if (p->start_block >= 0) {
        free_file_block(p->start_block);
    }

    // 文件一定有父指针, 并且没有 left 指针, 要找到该文件在 right 链表中的上一个节点
    struct file_directory *fa = p->father->left;

    // 特判: p 就是第一个节点
    if (fa == p) {
        // 在内存和硬盘中更新 p 的上一个节点的内容
        p->father->left = p->right;
        if (p->right) {
            p->father->lblk.block = p->right->blk.block;
            p->father->lblk.index = p->right->blk.index;
        }
        else p->father->lblk.index = 1024;
        save_index_node(p->father);
    }
    else {
        // p 不是第一个节点, 找到 right 链表中的父节点
        while (fa->right != p) fa = fa->right;
        // 在内存和硬盘中更新 fa 
        fa->right = p->right;
        if (p->right) {
            fa->rblk.block = p->right->blk.block;
            fa->rblk.index = p->right->blk.index;
        }
        else fa->rblk.index = 1024;
        save_index_node(fa);
    }

    // 将 p 的记录位释放, 将 p 从内存中释放
    free_index_node(&p->blk);
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
