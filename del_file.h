//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_DEL_FILE_H
#define LINUX_DEL_FILE_H

#include <vector>
#include "type.h"
#include "function.h"

template <class T>
void read_from_disk(int offset, T &tmp){ //记得传入实例, 传入前记得初始化tmp
    disk.seekg(offset, ios::beg);
    disk.read((char*)&tmp, sizeof(tmp));
}

template <class T>
void write_to_disk(int offset, T &tmp){
    disk.seekp(offset, ios::beg);
    disk.write((char*)&tmp, sizeof(tmp));
}

Uint *get_blocks(inode *node){
    int n_blocks = 0;
    auto *blocks = new Uint[node->n_blocks];
    for(int i = 0; i < 10; ++i){ //直接块
        blocks[n_blocks] = node->block_addr[i];
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    disk.seekg(block_msg->offset_block + node->block_addr[10]*BLOCK_SIZE, ios::beg);
    int block_num = BLOCK_SIZE / sizeof(Uint);
    for(int i = 0; i < block_num; ++i){ //一级块
        disk.read((char*)&blocks[n_blocks], sizeof(Uint));
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    disk.seekg(block_msg->offset_block + node->block_addr[11]*BLOCK_SIZE, ios::beg);
    int remain_num = (node->n_blocks - n_blocks - 1) / block_num + 1;
    auto *remain = new Uint[remain_num];
    for(int i = 0; i < remain_num; ++i){ //二级块对应的地址块
        disk.read((char*)&remain[i], sizeof(Uint));
    }
    for(int i = 0; i < remain_num; ++i){ //二级块
        disk.seekg(block_msg->offset_block + remain[i]*BLOCK_SIZE, ios::beg);
        for(int j = 0; j < block_num; ++j){
            disk.read((char*)&blocks[n_blocks], sizeof(Uint));
            if((++n_blocks) == node->n_blocks) return blocks;
        }
    }
}

bool get_sub_dir(dentry *&den){ //目录最多需要2048个块
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("This is not a directory.");
        return false;
    }
    Ulong file_size = den->cur_node->file_size;
    if(file_size == 0) return true;
    Uint *blocks = get_blocks(den->cur_node);
    int file_num = BLOCK_SIZE / sizeof(file);
    for(int i = 0; i < den->cur_node->n_blocks; ++i){
        disk.seekg(block_msg->offset_block+blocks[i]*BLOCK_SIZE, ios::beg);
        for(int j = 0; j < file_num; ++j){
            auto tmp = new dentry(den);
            disk.read((char*)tmp->cur_dir, sizeof(file));
            read_from_disk(block_msg->offset_inode+tmp->cur_dir->inode_id*sizeof(inode), *(tmp->cur_node));
            den->sub_dir.push_back(tmp);
            file_size -= sizeof(file);
            if(file_size == 0) return true;
        }
    }
    return false;
}

bool alloc_inode(Ushort &id){
    if(block_msg->n_free_inodes == 0) return false;
    id = 0;
    for (Uchar &map : inode_bitmap) {
        for(int j = 0; j < BYTE; ++j){
            if((map & (1 << j)) == 0){
                map = 1 << j;
                --(block_msg->n_free_inodes);
                return true;
            }
            ++id;
        }
    }
}

#endif //LINUX_DEL_FILE_H
