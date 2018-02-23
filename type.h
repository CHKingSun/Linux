//
// Created by KingSun on 2018/2/18.
//

#ifndef LINUX_TYPE_H
#define LINUX_TYPE_H

#define MAX_NAME_LEN 128

typedef unsigned int Uint;

enum file_pms{ //file permissions
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

enum file_type{
    I_FILE,
    I_DIR
};

struct inode{
    int uid;
    file_pms permission;
    file_type type;
    Uint size;
    Uint blocks_addr;
    Uint n_blocks;
};

struct file{
    char file_name[MAX_NAME_LEN];

};

struct dir: file{

};

#endif //LINUX_TYPE_H
