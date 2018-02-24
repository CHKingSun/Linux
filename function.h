//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_FUNCTION_H
#define LINUX_FUNCTION_H

#include <iostream>
#include "type.h"

void set_bitmap(Uint block_addr, int use = 1){
    block_msg.bitmap[block_addr>>(3* sizeof(Uchar))] = use << (block_addr^(BYTE-1));
}

void set_inode_bitmap(Ushort inode_addr, int use = 1){
    block_msg.inode_bitmap[inode_addr>>(3* sizeof(Uchar))] = use << (inode_addr^(BYTE-1));
}

void set_attrib(inode *&node, Uchar is_dir, Uchar r, Uchar w, Uchar x){
    node->attrib = is_dir << 12 + r << 8 + w << 4 + x;
}

void _Cout(char *msg, bool warp = true){
    std::cout<<msg;
    if(warp) std::cout<<std::endl;
}

#endif //LINUX_FUNCTION_H
