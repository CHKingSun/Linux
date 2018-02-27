//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_FUNCTION_H
#define LINUX_FUNCTION_H

#include <iostream>
#include <string>
#include <conio.h>
#include "type.h"

void _Cout(string msg, bool warp = true){
    std::cout<<msg;
    if(warp) std::cout<<std::endl;
}

char _getchar(bool print = true){
    char ch[2];
    ch[0] = _getch(); ch[1] = '\0';
    if(ch[0] == '\b'){
        _Cout("\b \b", false);
        return _getchar();
    }
    if(print) _Cout(ch, false);
    else _Cout("*", false);
    return ch[0];
}
//输出多余
bool check(){
    char ch = _getchar();
    if(ch == 27){
        _Cout("\n输入已取消。");
        return false;
    } else if(ch == 'Y' || ch == 'y'){
        ch = _getchar();
        if(ch == '\r') return true;
        else if(ch == 'E' || ch == 'e'){
            ch = _getchar();
            if(ch == 'S' || ch == 's'){
                if(_getchar() == '\r') return true;
            }
        }
    } else if(ch == 'N' || ch == 'n'){
        ch = _getchar();
        if(ch == '\r') return false;
        else if(ch == 'O' || ch == 'o'){
            if(_getchar() == '\r') return false;
        }
    }
    if(ch != '\r') while (_getchar() != '\r');
    _Cout("命令不正确，请输入(Y/N)或者按ESC取消输入: ", false);
    return check();
}

void set_bitmap(Uint block_addr, bool use = true){
    if (!use) {
        bitmap[block_addr >> (3 * sizeof(Uchar))] &= ~(1 << (block_addr & (BYTE - 1)));
        disk.seekp(block_addr * BLOCK_SIZE, ios::beg);
        disk.write("\0", BLOCK_SIZE);
        ++(block_msg->n_free_blocks);
    } else {
        bitmap[block_addr >> (3 * sizeof(Uchar))] |= 1 << (block_addr & (BYTE - 1));
        --(block_msg->n_free_blocks);
    }
}

void set_inode_bitmap(Ushort inode_addr, bool use = true){
    if (!use) {
        inode_bitmap[inode_addr >> (3 * sizeof(Uchar))] &= ~(1 << (inode_addr & (BYTE - 1)));
        ++(block_msg->n_free_inodes);
    } else {
        inode_bitmap[inode_addr >> (3 * sizeof(Uchar))] |= 1 << (inode_addr & (BYTE - 1));
        --(block_msg->n_free_inodes);
    }
}

void set_attrib(inode *&node, Uchar is_dir, Uchar u, Uchar g, Uchar o){
    node->attrib = (is_dir << 12) + (u << 8) + (g << 4) + o;
    node->user_id = 0;
    node->group_id = 0;
}

void spilt(const string &path, vector<string> &parts){
    Uint last = 0;
    Uint index = path.find_first_of('/', last);
    while (index != std::string::npos) {
        parts.push_back(path.substr(last, index-last));
        last = index + 1;
        index = path.find_first_of('/', last);
    }
    if (index - last > 0) {
        parts.push_back(path.substr(last, index-last));
    }
}

#endif //LINUX_FUNCTION_H
