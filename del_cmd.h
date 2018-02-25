//
// Created by KingSun on 2018/2/25.
//

#ifndef LINUX_DEL_CMD_H
#define LINUX_DEL_CMD_H

#include <string>
#include <iostream>

#include "function.h"

using namespace std;

void del_cmd(){
    string cmd;
    while (cin>>cmd){
        if(cmd == "exit"){
            _Cout("系统已退出......");
            break;
        } else if(cmd == "info"){

        } else if(cmd == "");
    }
}

#endif //LINUX_DEL_CMD_H
