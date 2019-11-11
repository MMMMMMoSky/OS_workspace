#include "func_def.h"

void set_string(struct file_directory *fd, char *name)
{
    int i = 0;
    while (name[i] != '\0')
    {
        fd->name[i] = name[i];
        i++;
    }
    fd->name[i] = '\0';
}

//初始化Home根目录
void init_home()
{
    extern struct file_directory home;
    extern struct file_directory_point nowdf;
    extern struct file_directory_point olddf;
    home.name[0] = '/';
    home.name[1] = 0;
    home.left = 0;
    home.right = 0;
    home.father = 0;
    home.flag = 1;
    nowdf.fdp = &home; // now is home
    olddf.fdp = &home; // at first they are equal to each other
}
//建一个新的目录，左孩子右兄弟上面是父节点模式
void create_new_directory(struct file_directory_point *now_directory, char *name)
{
    //printf("%s\n",now_directory->fdp->name);
    struct file_directory_point find_directory;//find the current directory
    find_directory.fdp = now_directory->fdp;
    struct file_directory* new_directory;//a new directory
    new_directory = (struct file_directory*)mem_alloc(sizeof(struct file_directory));
    set_string(new_directory, name);
    new_directory->left = 0;
    new_directory->right = 0;
    new_directory->flag = 1;
    new_directory->father = now_directory->fdp;
    if (find_directory.fdp->left == 0)
    {
        find_directory.fdp->left = new_directory;
    }
    else
    {
        find_directory.fdp = find_directory.fdp->left;
        while (find_directory.fdp->right != 0)
        {
            find_directory.fdp = find_directory.fdp->right;
        }
        find_directory.fdp->right = new_directory;
    }
//   now_directory->fdp=new_directory;  make directory but don't get in
}

//建一个新的文件，左孩子右兄弟上面是父节点模式
void create_new_file(struct file_directory_point *now_directory, char *name)
{
    // printf("%s\n",now_directory->fdp->name);
    struct file_directory_point find_file;//find the current directory
    find_file.fdp = now_directory->fdp;
    struct file_directory* new_file;//cd a new directory
    new_file = (struct file_directory*)mem_alloc(sizeof(struct file_directory));
    set_string(new_file, name);
    new_file->left = 0;
    new_file->right = 0;
    new_file->flag = 0;
    if (find_file.fdp->left == 0)
    {
        find_file.fdp->left = new_file;
        new_file->father = find_file.fdp;
    }
    else
    {
        find_file.fdp = find_file.fdp->left;
        while (find_file.fdp->right != 0)
        {
            find_file.fdp = find_file.fdp->right;
        }
        find_file.fdp->right = new_file;
        new_file->father = find_file.fdp;
    }
}

