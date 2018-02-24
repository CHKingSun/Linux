//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_DEL_FILE_H
#define LINUX_DEL_FILE_H

#include <vector>
#include "type.h"
#include "function.h"

void seekg(int offset){
    disk.seekg(block_msg.first_block+offset, ios::beg);
}

void seekp(int offset){
    disk.seekp(block_msg.first_block+offset, ios::beg);
}

Uint *get_blocks(inode *node){
    int n_blocks = 0;
    auto *blocks = new Uint[node->n_blocks];
    for(int i = 0; i < 10; ++i){ //直接块
        blocks[n_blocks] = node->block_addr[i];
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    seekg(node->block_addr[10]);
    int block_num = BLOCK_SIZE / sizeof(Uint);
    for(int i = 0; i < block_num; ++i){ //一级块
        disk.read((char*)&blocks[n_blocks], sizeof(Uint));
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    seekg(node->block_addr[11]);
    int remain_num = (node->n_blocks - n_blocks - 1) / block_num + 1;
    auto *remain = new Uint[remain_num];
    for(int i = 0; i < remain_num; ++i){ //二级块对应的地址块
        disk.read((char*)&remain[i], sizeof(Uint));
    }
    for(int i = 0; i < remain_num; ++i){ //二级块
        seekg(remain[i]);
        for(int j = 0; j < block_num; ++j){
            disk.read((char*)&blocks[n_blocks], sizeof(Uint));
            if((++n_blocks) == node->n_blocks) return blocks;
        }
    }
}

bool get_sub_dir(std::vector<file*> &sub_dir, inode *node){ //目录最多需要2048个块
    if((node->attrib ^ (1<<12)) == 0){
        _Cout("This is not a directory.");
        return false;
    }
    Ulong file_size = node->file_size;
    if(file_size == 0) return true;
    Uint *blocks = get_blocks(node);
    int file_num = BLOCK_SIZE / sizeof(file);
    for(int i = 0; i < node->n_blocks; ++i){
        seekg(blocks[i]);
        for(int j = 0; j < file_num; ++j){
            auto tmp = new file;
            disk.read((char*)tmp, sizeof(file));
            sub_dir.push_back(tmp);
            file_size -= sizeof(file);
            if(file_size == 0) return true;
        }
    }
    return false;
}

#endif //LINUX_DEL_FILE_H
