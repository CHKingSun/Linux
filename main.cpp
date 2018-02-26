#include <iostream>

#include "del_disk.h"
#include "del_cmd.h"

void exec(){
    if(!initial()) exit(0);
    del_cmd();
    sys_exit();
}

template <class T>
void test(int offset, T &tmp){ //记得传入实例
    std::cout<< sizeof(tmp)<<std::endl;
}

void test(){
    char rwx[3];
    sprintf(rwx, "%X", 0x1755);
    cout<<rwx<<endl;
    vector<string> p;
    spilt(" ", p);
    cout<<p.size()<<endl;
    for(const auto &it : p){
        cout<<it<<"\t";
    }
    cout<<endl;
    std::cout<< sizeof(inode)<<std::endl;
    cur_den = new dentry;
    cur_den->cur_node = new inode;
    test(0, *(cur_den->cur_node));
}

int main() {
//    test();
    exec();
    return 0;
}