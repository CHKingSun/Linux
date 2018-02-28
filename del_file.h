//
// Created by KingSun on 2018/2/24.
//

//只有在读取、新建操作时才有new操作

#ifndef LINUX_DEL_FILE_H
#define LINUX_DEL_FILE_H

#include <vector>
#include <ctime>
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
    disk.flush();
}

Uint *get_blocks(inode *node){
    if(node->n_blocks == 0) return NULL;
    int n_blocks = 0;
    auto *blocks = new Uint[node->n_blocks];
    for(int i = 0; i < 10; ++i){ //直接块
        blocks[n_blocks] = node->block_addr[i];
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    disk.seekg(node->block_addr[10]*BLOCK_SIZE, ios::beg);
    int block_num = BLOCK_SIZE / sizeof(Uint);
    for(int i = 0; i < block_num; ++i){ //一级块
        disk.read((char*)&blocks[n_blocks], sizeof(Uint));
        if((++n_blocks) == node->n_blocks) return blocks;
    }
    disk.seekg(node->block_addr[11]*BLOCK_SIZE, ios::beg);
    int remain_num = (node->n_blocks - n_blocks - 1) / block_num + 1;
    auto *remain = new Uint[remain_num];
    for(int i = 0; i < remain_num; ++i){ //二级块对应的地址块
        disk.read((char*)&remain[i], sizeof(Uint));
    }
    for(int i = 0; i < remain_num; ++i){ //二级块
        disk.seekg(remain[i]*BLOCK_SIZE, ios::beg);
        for(int j = 0; j < block_num; ++j){
            disk.read((char*)&blocks[n_blocks], sizeof(Uint));
            if((++n_blocks) == node->n_blocks) {
                delete[] remain;
                return blocks;
            }
        }
    }
}

bool get_sub_dir(dentry *&den){ //目录最多需要2048个块
    if((den->cur_node->attrib & (1<<12)) == 0){
        cout<<"This is not a directory."<<endl;
        return false;
    }
    Ulong file_size = den->cur_node->file_size;
    if(file_size == 0) return true;
    Uint *blocks = get_blocks(den->cur_node);
    int file_num = BLOCK_SIZE / sizeof(file);
    for(int i = 0; i < den->cur_node->n_blocks; ++i){
//        disk.seekg(blocks[i]*BLOCK_SIZE, ios::beg);
        for(int j = 0; j < file_num; ++j){
            auto tmp = new dentry(den);
//            disk.read((char*)tmp->cur_dir, sizeof(file));
            read_from_disk(blocks[i]*BLOCK_SIZE+j*sizeof(file), *(tmp->cur_dir));
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
        for(char j = 0; j < BYTE; ++j){
            if((map & (1 << j)) == 0){
                map |= 1 << j;
                --(block_msg->n_free_inodes);
                return true;
            }
            ++id;
        }
    }
    return false;
}

bool has_dir(const string &dir_name, dentry *parent, dentry *&sub_dir){
    if(dir_name.empty()){
        sub_dir = parent;
        return true;
    }
    for(auto it : parent->sub_dir){
        if(dir_name == it->cur_dir->file_name) {
            sub_dir = it;
            if(((it->cur_node->attrib & (1<<12)) != 0) && it->sub_dir.empty()) get_sub_dir(it);
            return true;
        }
    }
    return false;
}

bool get_dir(const string &path, dentry *&den){
    //目录可以为空，获取当前目录
	if (path.empty()) {
		den = cur_den;
		return true;
	}
    vector<string> parts;
    spilt(path, parts);
    if(parts[0].empty()){
        den = root_den;
    } else if(parts[0] == "."){
        den = cur_den;
    } else if(parts[0] == ".."){
        den = cur_den->parent;
    } else if(!has_dir(parts[0], cur_den, den)){
        return false;
    }
    for(auto it = parts.begin()+1; it != parts.end(); ++it){
        if(!has_dir((*it), den, den)){
            return false;
        }
    }
    return true;
}

bool alloc_block(Uint &addr){
    if(block_msg->n_free_blocks == 0) return false;
    addr = block_msg->offset_block / BLOCK_SIZE; //第一个块
    for(int i = addr / BYTE; i < BLOCK_NUM / BYTE; ++i){
        for(char j = 0; j < BYTE; ++j){
            if((bitmap[i] & (1 << j)) == 0){
                bitmap[i] |= 1 << j;
                --(block_msg->n_free_blocks);
                return true;
            }
            ++addr;
        }
    }
    return false;
}

bool add_dir_msg(inode *&node, file *dir){
    if((node->attrib & (1<<12)) == 0){
        cout<<"This is not a directory."<<endl;
        return false;
    }
    if(node->file_size / BLOCK_SIZE >= 10){
        cout<<"files num over."<<endl;
        return false;
    }
    if(node->file_size % BLOCK_SIZE == 0){ //块刚刚好放满
        if(!alloc_block(node->block_addr[node->file_size/BLOCK_SIZE])) return false;
        ++(node->n_blocks);
    }
    write_to_disk(node->block_addr[node->file_size/BLOCK_SIZE] * BLOCK_SIZE
                  + node->file_size%BLOCK_SIZE, *dir);
    node->file_size += sizeof(file);
    node->a_time = time(0);
    return true;
}

void remove_dir_msg(dentry *&parent, dentry *&den){
    Ulong file_size = 0;
	vector<dentry*>::iterator del_it;
    for(auto it = parent->sub_dir.begin(); it != parent->sub_dir.end(); ++it){
        if((*it) == den){
			del_it = it;
            continue;
        }
        write_to_disk(parent->cur_node->block_addr[file_size/BLOCK_SIZE]*BLOCK_SIZE
                      + file_size%BLOCK_SIZE, *((*it)->cur_dir));
        file_size += sizeof(file);
    }
	parent->sub_dir.erase(del_it);
    if(file_size % BLOCK_SIZE == 0){
        --(parent->cur_node->n_blocks);
        set_bitmap(parent->cur_node->block_addr[file_size % BLOCK_SIZE], false);
    }
    parent->cur_node->file_size = file_size;
    parent->cur_node->a_time = time(0);
}

void remove_file(dentry *&den){
    //找到块，删除块，删除节点，更新父节点信息
    Uint *blocks = get_blocks(den->cur_node);
    for(Uint i = 0; i < den->cur_node->n_blocks; ++i){
        set_bitmap(blocks[i], false);
    }
    set_inode_bitmap(den->cur_node->id, false);
    remove_dir_msg(den->parent, den);
    write_to_disk(block_msg->offset_inode+den->parent->cur_node->id * sizeof(inode), *(den->parent->cur_node)); //更新父i节点
    delete den;
    den = NULL;
}

void remove_dir(dentry *&den){
    if(den->sub_dir.empty()) get_sub_dir(den);
//    for (auto &it : den->sub_dir) {
//        if((it->cur_node->attrib & (1<<12)) == 0) remove_file(it);
//        else remove_dir(it);
//    }
    while(!den->sub_dir.empty()){
        if((den->sub_dir[den->sub_dir.size()-1]->cur_node->attrib & (1<<12)) == 0) remove_file(den->sub_dir[den->sub_dir.size()-1]);
        else remove_dir(den->sub_dir[den->sub_dir.size()-1]);
    }
    if(den == cur_den) cur_den = den->parent;
    set_inode_bitmap(den->cur_node->id, false);
    remove_dir_msg(den->parent, den);
    write_to_disk(block_msg->offset_inode+den->parent->cur_node->id * sizeof(inode), *(den->parent->cur_node)); //更新父i节点
    delete den;
	den = NULL;
}

void get_file(inode *node){ //考虑最后一个的长度
    Uint *blocks = get_blocks(node);
    if(blocks == NULL) return;
    char msg[BLOCK_SIZE+1];
    for(Uint i = 0; i < node->n_blocks-1; ++i){
//        read_from_disk(blocks[i]*BLOCK_SIZE, msg);
        disk.seekg(blocks[i]*BLOCK_SIZE, ios::beg);
        disk.read(msg, BLOCK_SIZE);
        msg[BLOCK_SIZE] = '\0';
        _Cout(msg, false);
    }
    disk.seekp(blocks[node->n_blocks-1]*BLOCK_SIZE, ios::beg);
    disk.read(msg, node->file_size-(node->n_blocks-1)*BLOCK_SIZE);
    if(node->file_size-(node->n_blocks-1)*BLOCK_SIZE < BLOCK_SIZE) msg[node->file_size-(node->n_blocks-1)*BLOCK_SIZE] = '\0';
    _Cout(msg);
}

bool alloc_blocks(inode *&node, Uint *&blocks){
    if(node->n_blocks + node->n_blocks / 256 > block_msg->n_free_blocks) return false;
    if(node->n_blocks == 0) return true;
    blocks = new Uint[node->n_blocks];
    for(int i = 0; i < node->n_blocks; ++i) alloc_block(blocks[i]); //一定可以分配到
    //写入inode
    int n_blocks = 0;
    for(int i = 0; i < 10; ++i){
        node->block_addr[i] = blocks[n_blocks];
        if((++n_blocks) == node->n_blocks) return true;
    }
    alloc_block(node->block_addr[10]);
    disk.seekp(node->block_addr[10]*BLOCK_SIZE, ios::beg);
    int block_num = BLOCK_SIZE / sizeof(Uint);
    for(int i = 0; i < block_num; ++i){ //一级块
        disk.write((char*)&blocks[n_blocks], sizeof(Uint));
        if((++n_blocks) == node->n_blocks) return true;
    }
    alloc_block(node->block_addr[11]);
    disk.seekp(node->block_addr[11]*BLOCK_SIZE, ios::beg);
    int remain_num = (node->n_blocks - n_blocks - 1) / block_num + 1;
    auto *remain = new Uint[remain_num];
    for(int i = 0; i < remain_num; ++i) { //二级块的块 确定<256
        alloc_block(remain[i]);
        disk.write((char*)remain[i], sizeof(Uint));
    }
    for(int i = 0; i < remain_num; ++i){ //二级块
        disk.seekp(remain[i]*BLOCK_SIZE, ios::beg);
        for(int j = 0; j < block_num; ++j){
            disk.write((char*)&blocks[n_blocks], sizeof(Uint));
            if((++n_blocks) == node->n_blocks) {
                delete[] remain;
                return true;
            }
        }
    }
}

void copy_file(inode *src, inode *&des){
    Uint *blocks = NULL;
    if(!alloc_blocks(des, blocks)){
        _Cout("无法复制文件：磁盘空间已满。");
        return;
    }
    if(blocks == NULL) return;
    Uint *src_blocks = get_blocks(src);
    char msg[BLOCK_SIZE];
    for(int i = 0; i < des->n_blocks; ++i){
        read_from_disk(src_blocks[i]*BLOCK_SIZE, msg);
        write_to_disk(blocks[i]*BLOCK_SIZE, msg);
    }
    write_to_disk(block_msg->offset_inode+des->id*sizeof(inode), *des);
}

void copy_file(fstream &fin, inode *&node){
    //确定可以存文件
    Uint *blocks = NULL;
    if(!alloc_blocks(node, blocks)){
        _Cout("无法复制文件：磁盘空间已满。");
        return;
    }
    if(blocks == NULL) return;
    char msg[BLOCK_SIZE];
    fin.seekg(0, ios::beg);
    for(Uint i = 0; i < node->n_blocks-1; ++i){
        fin.read(msg, BLOCK_SIZE);
//        cout<<msg<<endl;
        write_to_disk(blocks[i]*BLOCK_SIZE, msg);
    }
    fin.read(msg, node->file_size-(node->n_blocks-1)*BLOCK_SIZE);
    disk.seekp(blocks[node->n_blocks-1]*BLOCK_SIZE, ios::beg);
    disk.write(msg, node->file_size-(node->n_blocks-1)*BLOCK_SIZE);
    write_to_disk(block_msg->offset_inode+node->id*sizeof(inode), *node);
}

#endif //LINUX_DEL_FILE_H
