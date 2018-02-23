//
// Created by KingSun on 2018/2/18.
//

#ifndef LINUX_TYPE_H
#define LINUX_TYPE_H

#include <ctime>
#include <fstream>

typedef unsigned short Ushort;
typedef unsigned int Uint;
typedef unsigned long Ulong;
typedef unsigned char Uchar;

const int BLOCK_SIZE = 1024;
const int BLOCK_NUM = 1024 * 100;
const int MAX_NAME_LEN = 64 - sizeof(Ushort);
const int MAX_INODE_NUM = 1<<16;

struct inode{ //128
    Uint user_id;
    Uint group_id;
    Ushort attrib;
    Ulong file_size; //n_blocks = file_size / BLOCK_SIZE;
    Uint block_addr[13];
    Ulong create_time, m_time, a_time;
    Uchar not_use[48];
};

struct file{
    Uchar file_name[MAX_NAME_LEN];
    Ushort inode_id;
};

struct super_block{ //12812 13blocks
    Uchar bitmap[BLOCK_NUM/ (sizeof(Uchar)*8)];
    Uint n_free_blocks;
    Uint n_free_inodes;
    Uint first_block;
};

std::fstream disk;
int inode_table[MAX_INODE_NUM]; //8192 blocks

#endif //LINUX_TYPE_H
