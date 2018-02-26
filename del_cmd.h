//
// Created by KingSun on 2018/2/25.
//

#ifndef LINUX_DEL_CMD_H
#define LINUX_DEL_CMD_H

#include <string>
#include <iostream>
#include <sstream>

#include "function.h"
#include "del_file.h"

using namespace std;

void get_info(){
    //TODO 获取系统信息 无参
    _Cout("磁盘信息：");
    _Cout(string("\t磁盘大小：") + to_string(BLOCK_NUM) + "KB");
    _Cout(string("\t已用空间：") + to_string(BLOCK_NUM - block_msg->n_free_blocks) + "KB");
    _Cout(string("\t剩余空间：") + to_string(block_msg->n_free_blocks) + "KB");
    _Cout(string("\t剩余空间占比：") + to_string((double)block_msg->n_free_blocks/BLOCK_NUM) + "%");
    _Cout("详细信息：");
    _Cout(string("\t块大小：") + to_string(BLOCK_SIZE) + "字节");
    _Cout(string("\t块个数：") + to_string(BLOCK_NUM));
    _Cout(string("\t剩余块个数：") + to_string(block_msg->n_free_blocks));
    _Cout(string("\ti节点总数：") + to_string(MAX_INODE_NUM));
    _Cout(string("\t剩余i节点个数：") + to_string(block_msg->n_free_inodes));
}

void get_help(){
    //TODO 获取指令信息
}

bool change_dir(const string &path){
    //TODO dentry, ./ ../, 以/作为分隔，注意不存在情况 1参
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法改变目录：找不到指定路径。");
        return false;
    }
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("无法改变目录：该路径不是目录。");
        return false;
    }
    cur_den = den;
    den = NULL;
    return true;
}

void read_dir(const string &path, bool flag = false){
    //TODO dentry, 1参 或 /s + 1参
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法读取目录：找不到指定路径。");
        return;
    }
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("无法读取目录：该路径不是目录。");
        return;
    }
    _Cout("文件/目录名\t类型\t大小(KB)\t权限\t所属用户\t创建时间");
    for(auto it : den->sub_dir){
        _Cout(string(it->cur_dir->file_name) + "\t", false);
        if((it->cur_node->attrib & (1<<12)) == 0) _Cout("文件\t", false);
        else _Cout("目录\t", false);
        _Cout(to_string(it->cur_node->n_blocks) + "\t", false);
        char rwx[5];
        sprintf(rwx, "%X", it->cur_node->attrib);
        _Cout(string(rwx).substr(1) + "\t", false);
        _Cout(to_string(it->cur_node->user_id) + "\t", false); //可能需要修改
        _Cout(string(ctime(&(it->cur_node->c_time))), false);
    }
}

void new_dir(string path){
    //TODO 无/ / ./ ../ 1参 判断存在
    //已经存在的不要
    if(path.empty()){ //因为sstream所以不存在空格字符串的情况
        _Cout("无法创建目录：目录命名不正确。");
        return;
    }
    int pos = path.find_last_of('/');
    string dir_name = path.substr(pos+1);
    if(dir_name.empty()){
        _Cout("无法创建目录：目录命名不正确。");
        return;
    }
    dentry *parent = NULL;
    if(pos != string::npos){ //分为 /a(pos = 0), a(pos = npos), /.../a(pos > 0)情况
        if(!get_dir(path.substr(0, pos+1), parent)){
            _Cout("无法创建目录：找不到路径。");
            return;
        }
    } else {
        parent = cur_den;
    }
    if(parent->cur_node->file_size / BLOCK_SIZE >= 10){
        _Cout("无法创建目录：文件个数以达到最大。");
        return;
    }
    for(auto it : parent->sub_dir){
        if(dir_name == it->cur_dir->file_name) {
            _Cout("无法创建目录：目录已存在。");
            return;
        }
    }
    auto *den = new dentry(parent);
    strcpy(den->cur_dir->file_name, dir_name.data());
    alloc_inode(den->cur_dir->inode_id);
    den->cur_node->id = den->cur_dir->inode_id;
    set_attrib(den->cur_node, 1, 7, 5, 5);
    den->cur_node->c_time = time(0);
    den->cur_node->a_time = den->cur_node->c_time;
    den->cur_node->file_size = 0;
    den->cur_node->n_blocks = 0;
    write_to_disk(block_msg->offset_inode+den->cur_node->id * sizeof(inode), *(den->cur_node)); //保存新的i节点
    parent->sub_dir.push_back(den);
    add_dir_msg(parent->cur_node, den->cur_dir);
    write_to_disk(block_msg->offset_inode+parent->cur_node->id * sizeof(inode), *(parent->cur_node)); //更新父i节点
}

void del_dir(string path){
    //TODO 注意/ 1参
    if(path.empty()){
        _Cout("无法删除目录：路径不正确。");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法删除目录：路径不正确。");
        return;
    }
    //根目录
    if(den == root_den){
        _Cout("无法删除目录：根目录无法删除");
        return;
    }
    //文件
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("无法删除目录：该路径不是目录。");
        return;
    }
    //不为空目录
    if(den->cur_node->file_size != 0){
        _Cout("该目录不为空，是否删除？(Y/N): ", false);
        if(!check()){
            _Cout("已取消删除。");
            return;
        }
    }
    _Cout("删除中，请稍后...");
    if(!remove_dir(den)) _Cout("删除失败！");
    else _Cout("删除成功！");
}

void new_file(string path){
    //TODO 同new_dir 1参
}

void read_file(string path){
    //TODO 类似read_dir, 注意不要读dir 1参
}

void copy_file(string des, string src){
    //TODO 注意<host> 2参
}

void del_file(string path){
    //TODO 类似del_dir 1参
}

void check_sys(){
    //TODO 待定 无参
}

void del_cmd(){
    string line;
	string cur_path = "/# ";
    _Cout(cur_path, false);
    while (getline(cin, line)){
        stringstream sin(line);
        string cmd;
        sin>>cmd;
        if(cmd == "exit"){
            _Cout("系统已退出......");
            break;
        } else if(cmd == "help"){
            get_help();
        } else if(cmd == "info"){
            get_info();
        } else if(cmd == "cd"){
            string path;
            sin>>path;
            if(change_dir(path)){
                if(cur_den == root_den) cur_path = "/# ";
                else cur_path = cur_den->cur_path.substr(2) + "/" + cur_den->cur_dir->file_name + "# ";
            }
        } else if(cmd == "dir"){
            string path;
            sin>>path;
            read_dir(path);
        } else if(cmd == "md"){
            string path;
            sin>>path;
            new_dir(path);
        } else if(cmd == "rd"){
            string path;
            sin>>path;
            del_dir(path);
        } else if(cmd == "newfile"){
            string path;
            sin>>path;
            new_file(path);
        } else if(cmd == "cat"){
            string path;
            sin>>path;
            read_file(path);
        } else if(cmd == "copy"){
            string des, src;
            sin>>des>>src;
            copy_file(des, src);
        } else if(cmd == "del"){
            string path;
            sin>>path;
            del_file(path);
        } else if(cmd == "check"){
            check_sys();
        } else{
            _Cout("错误指令！输入'help'获取指令信息。");
        }
        _Cout(cur_path, false);
    }
}

#endif //LINUX_DEL_CMD_H
