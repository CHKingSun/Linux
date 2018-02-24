#include <iostream>

#include "del_disk.h"

int main() {
//    std::cout<< sizeof(file)<<std::endl;
    if(!init_disk()) exit(0);
    getchar();
    save_disk();
    return 0;
}