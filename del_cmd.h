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
    //TODO ��ȡϵͳ��Ϣ �޲�
    _Cout("������Ϣ��");
    _Cout(string("\t���̴�С��") + to_string(BLOCK_NUM) + "KB");
    _Cout(string("\t���ÿռ䣺") + to_string(BLOCK_NUM - block_msg->n_free_blocks) + "KB");
    _Cout(string("\tʣ��ռ䣺") + to_string(block_msg->n_free_blocks) + "KB");
    _Cout(string("\tʣ��ռ�ռ�ȣ�") + to_string((double)block_msg->n_free_blocks/BLOCK_NUM) + "%");
    _Cout("��ϸ��Ϣ��");
    _Cout(string("\t���С��") + to_string(BLOCK_SIZE) + "�ֽ�");
    _Cout(string("\t�������") + to_string(BLOCK_NUM));
    _Cout(string("\tʣ��������") + to_string(block_msg->n_free_blocks));
    _Cout(string("\ti�ڵ�������") + to_string(MAX_INODE_NUM));
    _Cout(string("\tʣ��i�ڵ������") + to_string(block_msg->n_free_inodes));
}

void get_help(){
    //TODO ��ȡָ����Ϣ
}

bool change_dir(const string &path){
    //TODO dentry, ./ ../, ��/��Ϊ�ָ���ע�ⲻ������� 1��
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷��ı�Ŀ¼���Ҳ���ָ��·����");
        return false;
    }
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("�޷��ı�Ŀ¼����·������Ŀ¼��");
        return false;
    }
    cur_den = den;
    den = NULL;
    return true;
}

void read_dir(const string &path, bool flag = false){
    //TODO dentry, 1�� �� /s + 1��
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷���ȡĿ¼���Ҳ���ָ��·����");
        return;
    }
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("�޷���ȡĿ¼����·������Ŀ¼��");
        return;
    }
    _Cout("�ļ�/Ŀ¼��\t����\t��С(KB)\tȨ��\t�����û�\t����ʱ��");
    for(auto it : den->sub_dir){
        _Cout(string(it->cur_dir->file_name) + "\t", false);
        if((it->cur_node->attrib & (1<<12)) == 0) _Cout("�ļ�\t", false);
        else _Cout("Ŀ¼\t", false);
        _Cout(to_string(it->cur_node->n_blocks) + "\t", false);
        char rwx[5];
        sprintf(rwx, "%X", it->cur_node->attrib);
        _Cout(string(rwx).substr(1) + "\t", false);
        _Cout(to_string(it->cur_node->user_id) + "\t", false); //������Ҫ�޸�
        _Cout(string(ctime(&(it->cur_node->c_time))), false);
    }
}

void new_dir(string path){
    //TODO ��/ / ./ ../ 1�� �жϴ���
    //�Ѿ����ڵĲ�Ҫ
    if(path.empty()){ //��Ϊsstream���Բ����ڿո��ַ��������
        _Cout("�޷�����Ŀ¼��Ŀ¼��������ȷ��");
        return;
    }
    int pos = path.find_last_of('/');
    string dir_name = path.substr(pos+1);
    if(dir_name.empty()){
        _Cout("�޷�����Ŀ¼��Ŀ¼��������ȷ��");
        return;
    }
    dentry *parent = NULL;
    if(pos != string::npos){ //��Ϊ /a(pos = 0), a(pos = npos), /.../a(pos > 0)���
        if(!get_dir(path.substr(0, pos+1), parent)){
            _Cout("�޷�����Ŀ¼���Ҳ���·����");
            return;
        }
    } else {
        parent = cur_den;
    }
    if(parent->cur_node->file_size / BLOCK_SIZE >= 10){
        _Cout("�޷�����Ŀ¼���ļ������Դﵽ���");
        return;
    }
    for(auto it : parent->sub_dir){
        if(dir_name == it->cur_dir->file_name) {
            _Cout("�޷�����Ŀ¼��Ŀ¼�Ѵ��ڡ�");
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
    write_to_disk(block_msg->offset_inode+den->cur_node->id * sizeof(inode), *(den->cur_node)); //�����µ�i�ڵ�
    parent->sub_dir.push_back(den);
    add_dir_msg(parent->cur_node, den->cur_dir);
    write_to_disk(block_msg->offset_inode+parent->cur_node->id * sizeof(inode), *(parent->cur_node)); //���¸�i�ڵ�
}

void del_dir(string path){
    //TODO ע��/ 1��
    if(path.empty()){
        _Cout("�޷�ɾ��Ŀ¼��·������ȷ��");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷�ɾ��Ŀ¼��·������ȷ��");
        return;
    }
    //��Ŀ¼
    if(den == root_den){
        _Cout("�޷�ɾ��Ŀ¼����Ŀ¼�޷�ɾ��");
        return;
    }
    //�ļ�
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("�޷�ɾ��Ŀ¼����·������Ŀ¼��");
        return;
    }
    //��Ϊ��Ŀ¼
    if(den->cur_node->file_size != 0){
        _Cout("��Ŀ¼��Ϊ�գ��Ƿ�ɾ����(Y/N): ", false);
        if(!check()){
            _Cout("��ȡ��ɾ����");
            return;
        }
    }
    _Cout("ɾ���У����Ժ�...");
    if(!remove_dir(den)) _Cout("ɾ��ʧ�ܣ�");
    else _Cout("ɾ���ɹ���");
}

void new_file(string path){
    //TODO ͬnew_dir 1��
}

void read_file(string path){
    //TODO ����read_dir, ע�ⲻҪ��dir 1��
}

void copy_file(string des, string src){
    //TODO ע��<host> 2��
}

void del_file(string path){
    //TODO ����del_dir 1��
}

void check_sys(){
    //TODO ���� �޲�
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
            _Cout("ϵͳ���˳�......");
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
            _Cout("����ָ�����'help'��ȡָ����Ϣ��");
        }
        _Cout(cur_path, false);
    }
}

#endif //LINUX_DEL_CMD_H
