//
// Created by KingSun on 2018/2/25.
//

/**
 * use to print data to debug.
 */

#ifndef LINUX_DEBUG_H
#define LINUX_DEBUG_H

#include <iostream>
#include "type.h"

void print(inode *node){
    cout<<"attrib: "<<node->attrib<<endl;
    cout<<"id: "<<node->id<<endl;
    cout<<"user_id: "<<node->user_id<<endl;
    cout<<"group_id: "<<node->group_id<<endl;
    cout<<"file_size: "<<node->file_size<<endl;
    cout<<"n_blocks: "<<node->n_blocks<<endl;
    cout<<"c_time: "<<node->c_time<<endl;
    cout<<"a_time: "<<node->a_time<<endl;
    cout<<"block_addr: ";
    for (Uint i : node->block_addr){
        cout<<i<<"\t";
    }
    cout<<endl;
}

void print(file *dir){
    cout<<"file_name: "<<dir->file_name<<"\tinode_id: "<<dir->inode_id<<endl;
}

void print(super_block *super){
    cout<<"n_free_blocks: "<<super->n_free_blocks<<endl;
    cout<<"n_free_inodes: "<<super->n_free_inodes<<endl;
    cout<<"offset_bitmap: "<<super->offset_bitmap<<endl;
    cout<<"offset_inode_bitmap: "<<super->offset_inode_bitmap<<endl;
    cout<<"offset_inode: "<<super->offset_inode<<endl;
    cout<<"offset_inode_bitmap: "<<super->offset_inode_bitmap<<endl;
    cout<<"offset_block: "<<super->offset_block<<endl;
}

void print(dentry *den){
    cout<<"file: "<<endl;
    print(den->cur_dir);
    cout<<"inode: "<<endl;
    print(den->cur_node);
    cout<<"sub_dir"<<endl;
    for(auto it : den->sub_dir){
        cout<<"\t";
        print(it->cur_dir);
    }
}

#endif //LINUX_DEBUG_H
