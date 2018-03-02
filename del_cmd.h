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
    //获取系统信息 无参
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
    // 获取指令信息
    _Cout("帮助信息：");
    _Cout("info\t显示系统信息；");
    _Cout("cd [path]\t改变当前目录；");
    _Cout("dir [path]\t显示目录信息；");
    _Cout("md [path]\t创建目录；");
    _Cout("rd [path]\t删除目录；");
    _Cout("newfile [filepath]\t新建文件；");
    _Cout("cat [filepath]\t查看文件；");
    _Cout("copy [<host>src] [des]\t复制文件；");
    _Cout("del [filepath]\t删除文件；");
    _Cout("check\t检测系统；");
    _Cout("cls\t清屏。");
}

bool change_dir(const string &path){
    // dentry, ./ ../, 以/作为分隔，注意不存在情况 1参
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
    // dentry, 1参 或 /s + 1参
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
//        cout<<it->cur_node->attrib<<endl;
        sprintf(rwx, "%4X", it->cur_node->attrib);
        _Cout(string(rwx).substr(1) + "\t", false);
        _Cout(to_string(it->cur_node->user_id) + "\t", false); //可能需要修改
        _Cout(string(ctime(&(it->cur_node->c_time))), false);
    }
}

bool new_dir(const string &path, Uchar type = 1){
    //无/ / ./ ../ 1参 判断存在
    //已经存在的不要
	string f_type = "目录";
    if(type == 0) f_type = "文件";
    if(path.empty()){ //因为sstream所以不存在空格字符串的情况
        _Cout("无法创建" + f_type + "：" + f_type + "命名不正确。");
        return false;
    }
    int pos = path.find_last_of('/');
    string dir_name = path.substr(pos+1);
    if(dir_name.empty() || dir_name == "." || dir_name == ".."){
        _Cout("无法创建" + f_type + "：" + f_type + "命名不正确。");
        return false;
    }
    dentry *parent = NULL;
    if(pos != string::npos){ //分为 /a(pos = 0), a(pos = npos), /.../a(pos > 0)情况
        if(!get_dir(path.substr(0, pos+1), parent)){
            _Cout("无法创建" + f_type + "：找不到路径。");
            return false;
        }
    } else {
        parent = cur_den;
    }
    if(parent->cur_node->file_size / BLOCK_SIZE >= 10){
        _Cout("无法创建" + f_type + "：文件个数已达到最大。");
        return false;
    }
    for(auto it : parent->sub_dir){
        if(dir_name == it->cur_dir->file_name) {
            _Cout("无法创建" + f_type + "：目录已存在。");
            return false;
        }
    }
    auto *den = new dentry(parent);
    strcpy(den->cur_dir->file_name, dir_name.data());
    if(!alloc_inode(den->cur_dir->inode_id)){
        _Cout("无法创建" + f_type + "：文件个数已达到最大。");
        delete den;
        return false;
    }
    if(!add_dir_msg(parent->cur_node, den->cur_dir)){
        _Cout("无法创建" + f_type + "：磁盘空间不足。");
        delete den;
        return false;
    }
    den->cur_node->id = den->cur_dir->inode_id;
    set_attrib(den->cur_node, type, 7, 5, 5);
    den->cur_node->c_time = time(0);
    den->cur_node->a_time = den->cur_node->c_time;
    den->cur_node->file_size = 0;
    den->cur_node->n_blocks = 0;
    write_to_disk(block_msg->offset_inode+den->cur_node->id * sizeof(inode), *(den->cur_node)); //保存新的i节点
    parent->sub_dir.push_back(den);
    write_to_disk(block_msg->offset_inode+parent->cur_node->id * sizeof(inode), *(parent->cur_node)); //更新父i节点
    return true;
}

void del_dir(const string &path){
    // 注意/ 1参
    if(path.empty()){
        _Cout("无法删除目录：路径不正确。");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法删除目录：路径不正确。");
        return;
    }
    //文件
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("无法删除目录：该路径不是目录。");
        return;
    }
    //根目录
    if(den == root_den){
        _Cout("无法删除目录：根目录无法删除");
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
    remove_dir(den);
    _Cout("删除成功！");
}

bool new_file(const string &path){
    // 同new_dir 1参
    return new_dir(path, 0); //0表示文件类型
}

void read_file(const string &path){
    // 类似read_dir, 注意不要读dir 1参
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法读取文件：找不到指定路径。");
        return;
    }
    if((den->cur_node->attrib & (1<<12)) != 0){
        _Cout("无法读取文件：该路径不是文件。");
        return;
    }
    get_file(den->cur_node);
}

void del_file(const string &path){
    // 类似del_dir 1参
    if(path.empty()){
        _Cout("无法删除文件：路径不正确。");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("无法删除文件：路径不正确。");
        return;
    }
    //文件
    if((den->cur_node->attrib & (1<<12)) != 0){
        _Cout("无法删除文件：该路径不是文件。");
        return;
    }
    _Cout("删除中，请稍后...");
    remove_file(den);
    _Cout("删除成功！");
}

void copy_file(const string &src, string des){
    // 注意<host> 2参
    //判断有没有<host> 文件是否存在 不支持从系统copy到外部
    dentry *den = NULL;
    if(des.empty()) des = ".";
    if(src.size() > 6 && src.substr(0, 6) == "<host>"){
        int pos = src.find_last_of('\\');
        if(pos == string::npos || pos == src.size()-1){
            _Cout("无法复制文件：文件路径不正确。");
            return;
        }
        string file_name = src.substr(pos+1);
        des += "/" + file_name;
        string path = src.substr(6);
        fstream fin(path, ios::in | ios::binary);
        if(!fin.is_open()){
            _Cout("无法复制文件：源文件不存在。");
            return;
        }
        fin.seekg(0, ios::end);
        Ulong file_size = fin.tellg();
        fin.clear();
        Uint n_blocks = ceil((float)file_size/BLOCK_SIZE);
        if(n_blocks + n_blocks / 256 > block_msg->n_free_blocks){
            _Cout("无法复制文件：磁盘空间不足。");
            return;
        }
        if(file_size > MAX_FILE_SIZE){
            _Cout("无法复制文件：超过文件最大大小。");
            return;
        }
        if(get_dir(des, den)){
            if((den->cur_node->attrib & (1<<12)) != 0){
                _Cout("无法复制文件：存在同名目录。");
                return;
            }
            _Cout("文件已存在，是否覆盖(Y/N): ");
            if(!check()){
                _Cout("已取消复制。");
                return;
            }
            remove_dir(den);
        }
        if(!new_file(des)) return;
        get_dir(des, den);
        den->cur_node->n_blocks = n_blocks;
        den->cur_node->file_size = file_size;
        copy_file(fin, den->cur_node);
        fin.close()
    } else{
        int pos = src.find_last_of('/', -1);
        if(pos == src.size()-1){
            _Cout("无法复制文件：文件路径不正确。");
            return;
        }
        string file_name = src.substr(pos+1);
        des += "/" + file_name;
        dentry *src_den = NULL;
        if(!get_dir(src, src_den)){
            _Cout("无法复制文件：源文件不存在。");
            return;
        }
        if((src_den->cur_node->attrib & (1<<12)) != 0){
            _Cout("无法复制文件：该路径不是文件。");
            return;
        }
        if(src_den->cur_node->n_blocks + src_den->cur_node->n_blocks / 256 > block_msg->n_free_blocks){
            _Cout("无法复制文件：磁盘空间不足。");
            return;
        }
        if(src_den->cur_node->file_size > MAX_FILE_SIZE){
            _Cout("无法复制文件：超过文件最大大小。");
            return;
        }
        if(get_dir(des, den)){
            if((den->cur_node->attrib & (1<<12)) != 0){
                _Cout("无法复制文件：存在同名目录。");
                return;
            }
            _Cout("文件已存在，是否覆盖(Y/N): ");
            if(!check()){
                _Cout("已取消复制。");
                return;
            }
            remove_dir(den);
        }
        if(!new_file(des)) return;
        get_dir(des, den);
        den->cur_node->n_blocks = src_den->cur_node->n_blocks;
        den->cur_node->file_size = src_den->cur_node->file_size;
        copy_file(src_den->cur_node, den->cur_node);
    }
    den->cur_node->a_time = time(0);
}

void check_den(dentry *den, Uint &n_blocks, Uint &n_inodes){
    n_blocks += den->cur_node->n_blocks;
    ++n_inodes;
    if((inode_bitmap[den->cur_node->id >> (3 * sizeof(Uchar))] & (1 << (den->cur_node->id & (BYTE - 1))) == 0)){
        set_inode_bitmap(den->cur_node->id);
        _Cout("i节点未被标记，已修复。");
    }
    if(!den->sub_dir.empty() || get_sub_dir(den)){
        for(auto it : den->sub_dir){
            check_den(it, n_blocks, n_inodes);
        }
    }
}

void check_sys(){
    // 待定 无参
    //遍历每个文件，检查i节点是否对上，检查剩余块和i节点是否有误
    Uint n_blocks = 0;
    Uint n_inodes = 0;
    check_den(root_den, n_blocks, n_inodes);
    if(n_blocks + block_msg->offset_block / BLOCK_SIZE != BLOCK_NUM - block_msg->n_free_blocks){
        _Cout("剩余块个数与实际不对应！");
    }
    if(n_inodes != MAX_INODE_NUM - block_msg->n_free_inodes){
        _Cout("剩余i节点与实际不对应！");
    }
    _Cout("检测完成！");
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
        } else if(cmd == "cls"){
            system("cls");
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
            if(cur_den == root_den) cur_path = "/# ";
            else cur_path = cur_den->cur_path.substr(2) + "/" + cur_den->cur_dir->file_name + "# ";
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
            sin>>src>>des;
            copy_file(src, des);
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
