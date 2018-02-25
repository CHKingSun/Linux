//
// Created by KingSun on 2018/2/24.
//

#ifndef LINUX_FUNCTION_H
#define LINUX_FUNCTION_H

#include <iostream>
#include "type.h"

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

void _Cout(char *msg, bool warp = true){
    std::cout<<msg;
    if(warp) std::cout<<std::endl;
}

#endif //LINUX_FUNCTION_H
