#include <iostream>

#include "del_disk.h"

void exec(){
    if(!initial()) exit(0);
    getchar();
    sys_exit();
}

template <class T>
void test(int offset, T &tmp){ //记得传入实例
    std::cout<< sizeof(tmp)<<std::endl;
}

void test(){
    std::cout<< sizeof(inode)<<std::endl;
    cur_den = new dentry;
    cur_den->cur_node = new inode;
    test(0, *(cur_den->cur_node));
}

int main() {
    //test();
    exec();
    return 0;
}