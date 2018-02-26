//
// Created by KingSun on 2018/2/18.
//

#ifndef LINUX_TYPE_H
#define LINUX_TYPE_H

#include <fstream>
#include <cmath>
#include <vector>
#include <cstring>

using namespace std;

typedef unsigned short Ushort;
typedef unsigned int Uint;
typedef unsigned long Ulong;
typedef unsigned char Uchar;

const int BYTE = sizeof(Uchar) * 8; //8
const int BLOCK_SIZE = 1024;
const int BLOCK_NUM = 1024 * 100;
const int FILE_SIZE = 64; //sizeof(file)
const int MAX_NAME_LEN = (FILE_SIZE - sizeof(Ushort)) / sizeof(char);
const int MAX_INODE_NUM = 1 << 14;
const int MAX_FILE_SIZE = 1 << 16;

struct inode{ //80
    Ushort attrib, id;
    Uint user_id, group_id;
    Uint block_addr[13]; //直接块 10 | 一级块 256 | 二级块 256*256=65536
    Ulong file_size; //n_blocks = file_size / BLOCK_SIZE;
    Uint n_blocks;
    time_t c_time, a_time;
//    Uchar not_use[44];
};

struct file{
    file(){}
    file(const char *f, Ushort id):inode_id(id){
        strcpy(file_name, f);
    };
    char file_name[MAX_NAME_LEN];
    Ushort inode_id;
};

struct super_block{ //1 block
    Uint n_free_blocks;
    Uint n_free_inodes;
    Uint offset_bitmap; //sizeof(super_block)
    Uint offset_inode_bitmap; //sizeof(super_block) + sizeof(bitmap)
    Uint offset_inode; //sizeof(super_block) + sizeof(bitmap) + sizeof(inode_bitmap)
    Uint offset_block;
};

struct dentry{
    dentry(){}
    dentry(dentry *p): parent(p){
        cur_dir = new file;
        cur_node = new inode;
        cur_path = parent->cur_path + "/" + parent->cur_dir->file_name;
    }
    ~dentry(){
        delete(cur_dir);
        delete(cur_node);
        sub_dir.erase(sub_dir.begin(), sub_dir.end());
    }
    dentry *parent;
    file *cur_dir;
    string cur_path;
    inode *cur_node;
    std::vector<dentry*> sub_dir;
};

//inode inode_table[MAX_INODE_NUM]; //sizeof(inode) * MAX_INODE_NUM blocks
Uchar inode_bitmap[MAX_INODE_NUM/BYTE]; //2 blocks
Uchar bitmap[BLOCK_NUM/BYTE]; //13 blocks

std::fstream disk;
super_block *block_msg = NULL;
dentry *root_den = NULL;
dentry *cur_den = NULL;
const char file_sys_name[] = "os.bin";

#endif //LINUX_TYPE_H
