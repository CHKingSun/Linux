//
// Created by KingSun on 2018/2/24.
//

/**
 * 16������λһ���ֽ�
 */

#ifndef LINUX_DEL_DISK_H
#define LINUX_DEL_DISK_H

#include <ctime>
#include "type.h"
#include "function.h"
#include "del_file.h"
#include "debug.h"

void save_disk(){
    write_to_disk(0, *block_msg);
    write_to_disk(block_msg->offset_bitmap, bitmap);
    write_to_disk(block_msg->offset_inode_bitmap, inode_bitmap);
}

void init_disk(){
    _Cout("��ʼ����......");
    disk.close();
    disk.open(file_sys_name, ios::out | ios::binary);
    disk.seekp(BLOCK_NUM*BLOCK_SIZE, ios::beg); //write
    disk.write("#", 1);
    block_msg = new super_block;
	memset(bitmap, 0, sizeof(bitmap));
	memset(inode_bitmap, 0, sizeof(inode_bitmap));
    block_msg->offset_bitmap = ceil((float)sizeof(super_block) / BLOCK_SIZE) * BLOCK_SIZE; //ƫ�����Ϳ��ƫ������һ��
    block_msg->offset_inode_bitmap = block_msg->offset_bitmap + ceil((float)sizeof(bitmap) / BLOCK_SIZE) * BLOCK_SIZE;
    block_msg->offset_inode = block_msg->offset_inode_bitmap + ceil((float)sizeof(inode_bitmap) / BLOCK_SIZE) * BLOCK_SIZE;
    block_msg->offset_block = block_msg->offset_inode + sizeof(inode) * MAX_INODE_NUM;
    block_msg->n_free_inodes = MAX_INODE_NUM;
    block_msg->n_free_blocks = BLOCK_NUM - block_msg->offset_block / BLOCK_SIZE;
    for(int i = 0; i < block_msg->offset_block / BLOCK_SIZE; ++i) set_bitmap(i);
    auto *root_dir = new inode;
    alloc_inode(root_dir->id);
    root_dir->c_time = time(0);
	root_dir->a_time = root_dir->c_time;
    root_dir->file_size = 0;
    root_dir->n_blocks = 0;
    set_attrib(root_dir, 1, 7, 5, 5);
	//write_to_disk(0, "Hello World!");
    write_to_disk(block_msg->offset_inode+root_dir->id * sizeof(inode), *root_dir); //Ҫע��ƫ�Ƶ�������ʲô
//    print(root_dir);
//    print(block_msg);
    set_inode_bitmap(0);
    save_disk();
	//exit(0);
    delete root_dir;
    root_dir = NULL;
    delete block_msg;
    block_msg = NULL;
    disk.close();
    _Cout("��ʼ����ɣ�");
}

bool load_disk(){
    _Cout("ϵͳ�����У����Ժ�......");
    disk.open(file_sys_name, ios::in | ios::out | ios::binary);
    block_msg = new super_block;
    read_from_disk(0, *block_msg);
    read_from_disk(block_msg->offset_bitmap, bitmap);
    read_from_disk(block_msg->offset_inode_bitmap, inode_bitmap);
    root_den = new dentry;
    root_den->parent = root_den;
    root_den->cur_dir = new file("/", 0);
    root_den->cur_node = new inode;
    read_from_disk(block_msg->offset_inode+root_den->cur_dir->inode_id*sizeof(inode), *(root_den->cur_node));
    if(!get_sub_dir(root_den)){
        _Cout("ϵͳ����ʧ�ܣ������˳�......");
        return false;
    }
    cur_den = root_den;
//    print(block_msg);
//    print(root_den);
    _Cout("��������ɣ�");
    return true;
}

bool initial(){
    disk.open(file_sys_name, ios::in | ios::binary);
    if(!disk.is_open()){
        _Cout("------------ϵͳ��δ��ʼ��------------");
        _Cout("�Ƿ���г�ʼ����(Y/N): ", false);
        disk.close();
        if(!check()){
            _Cout("��ʼ����ȡ���������˳�......");
            return false;
        } else{
            init_disk();
        }
    }
    disk.close();
    return load_disk();
}

void sys_exit(){
    _Cout("���ڱ������ݣ����Ժ�......");
    save_disk();
    delete block_msg;
    block_msg = NULL;
    delete root_den;
    root_den = NULL;
    disk.close();
}

#endif //LINUX_DEL_DISK_H
