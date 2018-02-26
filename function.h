//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_FUNCTION_H
#define LINUX_FUNCTION_H

#include <iostream>
#include <string>
#include "type.h"

void _Cout(string msg, bool warp = true){
    std::cout<<msg;
    if(warp) std::cout<<std::endl;
}

bool check(){
    char ch = getchar();
    if(ch == 27){
        _Cout("\n输入已取消。");
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
    if(ch != '\n') while (getchar() != '\n');
    _Cout("命令不正确，请输入(Y/N)或者按ESC取消输入: ", false);
    return check();
}

void set_bitmap(Uint block_addr, bool use = true){
    if(use) bitmap[block_addr>>(3* sizeof(Uchar))] |= 1 << (block_addr&(BYTE-1));
	else bitmap[block_addr >> (3 * sizeof(Uchar))] &= ~(1 << (block_addr&(BYTE - 1)));
}

void set_inode_bitmap(Ushort inode_addr, bool use = true){
    if(use) inode_bitmap[inode_addr>>(3* sizeof(Uchar))] |= 1 << (inode_addr&(BYTE-1));
	else inode_bitmap[inode_addr >> (3 * sizeof(Uchar))] &= ~(1 << (inode_addr&(BYTE - 1)));
}

void set_attrib(inode *&node, Uchar is_dir, Uchar u, Uchar g, Uchar o){
    node->attrib = (is_dir << 12) + (u << 8) + (g << 4) + o;
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
