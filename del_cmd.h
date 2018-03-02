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
    //��ȡϵͳ��Ϣ �޲�
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
    // ��ȡָ����Ϣ
    _Cout("������Ϣ��");
    _Cout("info\t��ʾϵͳ��Ϣ��");
    _Cout("cd [path]\t�ı䵱ǰĿ¼��");
    _Cout("dir [path]\t��ʾĿ¼��Ϣ��");
    _Cout("md [path]\t����Ŀ¼��");
    _Cout("rd [path]\tɾ��Ŀ¼��");
    _Cout("newfile [filepath]\t�½��ļ���");
    _Cout("cat [filepath]\t�鿴�ļ���");
    _Cout("copy [<host>src] [des]\t�����ļ���");
    _Cout("del [filepath]\tɾ���ļ���");
    _Cout("check\t���ϵͳ��");
    _Cout("cls\t������");
}

bool change_dir(const string &path){
    // dentry, ./ ../, ��/��Ϊ�ָ���ע�ⲻ������� 1��
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
    // dentry, 1�� �� /s + 1��
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
//        cout<<it->cur_node->attrib<<endl;
        sprintf(rwx, "%4X", it->cur_node->attrib);
        _Cout(string(rwx).substr(1) + "\t", false);
        _Cout(to_string(it->cur_node->user_id) + "\t", false); //������Ҫ�޸�
        _Cout(string(ctime(&(it->cur_node->c_time))), false);
    }
}

bool new_dir(const string &path, Uchar type = 1){
    //��/ / ./ ../ 1�� �жϴ���
    //�Ѿ����ڵĲ�Ҫ
	string f_type = "Ŀ¼";
    if(type == 0) f_type = "�ļ�";
    if(path.empty()){ //��Ϊsstream���Բ����ڿո��ַ��������
        _Cout("�޷�����" + f_type + "��" + f_type + "��������ȷ��");
        return false;
    }
    int pos = path.find_last_of('/');
    string dir_name = path.substr(pos+1);
    if(dir_name.empty() || dir_name == "." || dir_name == ".."){
        _Cout("�޷�����" + f_type + "��" + f_type + "��������ȷ��");
        return false;
    }
    dentry *parent = NULL;
    if(pos != string::npos){ //��Ϊ /a(pos = 0), a(pos = npos), /.../a(pos > 0)���
        if(!get_dir(path.substr(0, pos+1), parent)){
            _Cout("�޷�����" + f_type + "���Ҳ���·����");
            return false;
        }
    } else {
        parent = cur_den;
    }
    if(parent->cur_node->file_size / BLOCK_SIZE >= 10){
        _Cout("�޷�����" + f_type + "���ļ������Ѵﵽ���");
        return false;
    }
    for(auto it : parent->sub_dir){
        if(dir_name == it->cur_dir->file_name) {
            _Cout("�޷�����" + f_type + "��Ŀ¼�Ѵ��ڡ�");
            return false;
        }
    }
    auto *den = new dentry(parent);
    strcpy(den->cur_dir->file_name, dir_name.data());
    if(!alloc_inode(den->cur_dir->inode_id)){
        _Cout("�޷�����" + f_type + "���ļ������Ѵﵽ���");
        delete den;
        return false;
    }
    if(!add_dir_msg(parent->cur_node, den->cur_dir)){
        _Cout("�޷�����" + f_type + "�����̿ռ䲻�㡣");
        delete den;
        return false;
    }
    den->cur_node->id = den->cur_dir->inode_id;
    set_attrib(den->cur_node, type, 7, 5, 5);
    den->cur_node->c_time = time(0);
    den->cur_node->a_time = den->cur_node->c_time;
    den->cur_node->file_size = 0;
    den->cur_node->n_blocks = 0;
    write_to_disk(block_msg->offset_inode+den->cur_node->id * sizeof(inode), *(den->cur_node)); //�����µ�i�ڵ�
    parent->sub_dir.push_back(den);
    write_to_disk(block_msg->offset_inode+parent->cur_node->id * sizeof(inode), *(parent->cur_node)); //���¸�i�ڵ�
    return true;
}

void del_dir(const string &path){
    // ע��/ 1��
    if(path.empty()){
        _Cout("�޷�ɾ��Ŀ¼��·������ȷ��");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷�ɾ��Ŀ¼��·������ȷ��");
        return;
    }
    //�ļ�
    if((den->cur_node->attrib & (1<<12)) == 0){
        _Cout("�޷�ɾ��Ŀ¼����·������Ŀ¼��");
        return;
    }
    //��Ŀ¼
    if(den == root_den){
        _Cout("�޷�ɾ��Ŀ¼����Ŀ¼�޷�ɾ��");
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
    remove_dir(den);
    _Cout("ɾ���ɹ���");
}

bool new_file(const string &path){
    // ͬnew_dir 1��
    return new_dir(path, 0); //0��ʾ�ļ�����
}

void read_file(const string &path){
    // ����read_dir, ע�ⲻҪ��dir 1��
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷���ȡ�ļ����Ҳ���ָ��·����");
        return;
    }
    if((den->cur_node->attrib & (1<<12)) != 0){
        _Cout("�޷���ȡ�ļ�����·�������ļ���");
        return;
    }
    get_file(den->cur_node);
}

void del_file(const string &path){
    // ����del_dir 1��
    if(path.empty()){
        _Cout("�޷�ɾ���ļ���·������ȷ��");
        return;
    }
    dentry *den = NULL;
    if(!get_dir(path, den)){
        _Cout("�޷�ɾ���ļ���·������ȷ��");
        return;
    }
    //�ļ�
    if((den->cur_node->attrib & (1<<12)) != 0){
        _Cout("�޷�ɾ���ļ�����·�������ļ���");
        return;
    }
    _Cout("ɾ���У����Ժ�...");
    remove_file(den);
    _Cout("ɾ���ɹ���");
}

void copy_file(const string &src, string des){
    // ע��<host> 2��
    //�ж���û��<host> �ļ��Ƿ���� ��֧�ִ�ϵͳcopy���ⲿ
    dentry *den = NULL;
    if(des.empty()) des = ".";
    if(src.size() > 6 && src.substr(0, 6) == "<host>"){
        int pos = src.find_last_of('\\');
        if(pos == string::npos || pos == src.size()-1){
            _Cout("�޷������ļ����ļ�·������ȷ��");
            return;
        }
        string file_name = src.substr(pos+1);
        des += "/" + file_name;
        string path = src.substr(6);
        fstream fin(path, ios::in | ios::binary);
        if(!fin.is_open()){
            _Cout("�޷������ļ���Դ�ļ������ڡ�");
            return;
        }
        fin.seekg(0, ios::end);
        Ulong file_size = fin.tellg();
        fin.clear();
        Uint n_blocks = ceil((float)file_size/BLOCK_SIZE);
        if(n_blocks + n_blocks / 256 > block_msg->n_free_blocks){
            _Cout("�޷������ļ������̿ռ䲻�㡣");
            return;
        }
        if(file_size > MAX_FILE_SIZE){
            _Cout("�޷������ļ��������ļ�����С��");
            return;
        }
        if(get_dir(des, den)){
            if((den->cur_node->attrib & (1<<12)) != 0){
                _Cout("�޷������ļ�������ͬ��Ŀ¼��");
                return;
            }
            _Cout("�ļ��Ѵ��ڣ��Ƿ񸲸�(Y/N): ");
            if(!check()){
                _Cout("��ȡ�����ơ�");
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
            _Cout("�޷������ļ����ļ�·������ȷ��");
            return;
        }
        string file_name = src.substr(pos+1);
        des += "/" + file_name;
        dentry *src_den = NULL;
        if(!get_dir(src, src_den)){
            _Cout("�޷������ļ���Դ�ļ������ڡ�");
            return;
        }
        if((src_den->cur_node->attrib & (1<<12)) != 0){
            _Cout("�޷������ļ�����·�������ļ���");
            return;
        }
        if(src_den->cur_node->n_blocks + src_den->cur_node->n_blocks / 256 > block_msg->n_free_blocks){
            _Cout("�޷������ļ������̿ռ䲻�㡣");
            return;
        }
        if(src_den->cur_node->file_size > MAX_FILE_SIZE){
            _Cout("�޷������ļ��������ļ�����С��");
            return;
        }
        if(get_dir(des, den)){
            if((den->cur_node->attrib & (1<<12)) != 0){
                _Cout("�޷������ļ�������ͬ��Ŀ¼��");
                return;
            }
            _Cout("�ļ��Ѵ��ڣ��Ƿ񸲸�(Y/N): ");
            if(!check()){
                _Cout("��ȡ�����ơ�");
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
        _Cout("i�ڵ�δ����ǣ����޸���");
    }
    if(!den->sub_dir.empty() || get_sub_dir(den)){
        for(auto it : den->sub_dir){
            check_den(it, n_blocks, n_inodes);
        }
    }
}

void check_sys(){
    // ���� �޲�
    //����ÿ���ļ������i�ڵ��Ƿ���ϣ����ʣ����i�ڵ��Ƿ�����
    Uint n_blocks = 0;
    Uint n_inodes = 0;
    check_den(root_den, n_blocks, n_inodes);
    if(n_blocks + block_msg->offset_block / BLOCK_SIZE != BLOCK_NUM - block_msg->n_free_blocks){
        _Cout("ʣ��������ʵ�ʲ���Ӧ��");
    }
    if(n_inodes != MAX_INODE_NUM - block_msg->n_free_inodes){
        _Cout("ʣ��i�ڵ���ʵ�ʲ���Ӧ��");
    }
    _Cout("�����ɣ�");
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
            _Cout("����ָ�����'help'��ȡָ����Ϣ��");
        }
        _Cout(cur_path, false);
    }
}

#endif //LINUX_DEL_CMD_H
