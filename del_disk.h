//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_DEL_DISK_H
#define LINUX_DEL_DISK_H

#include <ctime>
#include "type.h"
#include "function.h"
#include "del_file.h"

bool check(){
    char ch = getchar();
    if(ch == 27){
        _Cout("\n������ȡ����");
        return false;
    } else if(ch == 'Y' || ch == 'y'){
        ch = getchar();
        if(ch == '\n') return true;
        else if(ch == 'E' || ch == 'e'){
            ch = getchar();
            if(ch == 'S' || ch == 's'){
                if(getchar() == '\n') return true;
            }
        }
    } else if(ch == 'N' || ch == 'n'){
        ch = getchar();
        if(ch == '\n') return false;
        else if(ch == 'O' || ch == 'o'){
            if(getchar() == '\n') return false;
        }
    }
    while (getchar() != '\n');
    _Cout("�����ȷ��������(Y/N)���߰�ESCȡ������: ", false);
    return check();
}

bool init_disk(){
    disk.open(file_sys_name, ios::out | ios::in | ios::binary);
    if(!disk.is_open()){
        _Cout("------------ϵͳ��δ��ʼ��------------");
        _Cout("�Ƿ���г�ʼ����(Y/N): ", false);
        if(!check()){
            _Cout("��ʼ����ȡ���������˳�......");
            disk.close();
            return false;
        } else{
            _Cout("��ʼ����......");
            disk.close();
            disk.open(file_sys_name, ios::out | ios::binary);
            disk.seekp(BLOCK_NUM*BLOCK_SIZE, ios::beg); //write
            disk.write("#", 1);
            disk.flush();
            block_msg.first_block = 1295;
            block_msg.n_free_inodes = MAX_INODE_NUM;
            block_msg.n_free_blocks = BLOCK_NUM - block_msg.first_block;
            for(int i = 0; i < block_msg.first_block; ++i) set_bitmap(i);
            auto *root_dir = new inode;
            root_dir->c_time = time(0);
            root_dir->file_size = 0;
            root_dir->n_blocks = 0;
            set_attrib(root_dir, 1, 7, 5, 5);
            block_msg.inode_table[0] = *root_dir;
            set_inode_bitmap(0);
            root_dir = NULL;
            _Cout("��ʼ����ϣ�");
            _Cout("ϵͳ������......");
        }
    } else{
        _Cout("ϵͳ������......");
        disk.seekg(0, ios::beg); //read
        disk.read((char*)&block_msg, sizeof(super_block));
    }
    cur_den = new dentry;
    auto *cur_dir = new file("/", 0);
    cur_den->parent = cur_den;
    cur_den->cur_dir = cur_dir;
    if(!get_sub_dir(cur_den->sub_dir, &block_msg.inode_table[cur_dir->inode_id])){
        _Cout("ϵͳ����ʧ�ܣ�");
        return false;
    }
    _Cout("ϵͳ������ɣ�");
    return true;
}

void save_disk(){
    _Cout("���ڱ������ݣ����Ժ�......");
    disk.seekp(0, ios::beg);
    disk.write((char*)&block_msg, sizeof(super_block));
    disk.flush();
    delete cur_den;
    disk.close();
}

#endif //LINUX_DEL_DISK_H
