//
// Created by KingSun on 2018/2/18.
//

#ifndef LINUX_TYPE_H
#define LINUX_TYPE_H

#include <fstream>
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
    Ushort attrib;
    Uint user_id, group_id;
    Uint block_addr[13]; //直接块 10 | 一级块 256 | 二级块 256*256=65536
    Ulong file_size; //n_blocks = file_size / BLOCK_SIZE;
    Uint n_blocks;
    time_t c_time, m_time;
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

struct super_block{ //1325580 1295blocks
    inode inode_table[MAX_INODE_NUM]; //1280 blocks
    Uchar inode_bitmap[MAX_INODE_NUM/BYTE]; //2 blocks
    Uchar bitmap[BLOCK_NUM/BYTE]; //12.5 blocks
    Uint n_free_blocks;
    Uint n_free_inodes;
    Uint first_block;
};

struct dentry{
    dentry *parent;
    file *cur_dir;
    std::vector<file*> sub_dir;
};

std::fstream disk;
super_block block_msg;
dentry *cur_den;
const char file_sys_name[] = "os.bin";

#endif //LINUX_TYPE_H
