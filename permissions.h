//
// Created by Petr Valik on 10.05.2022.
//

#include <stdio.h>

/** export dir contain two lists(one of his dirs and one of his files),
 *  also contain pointer to next dir in current directory also contain

 *  all necessary attributes for dir to export **/

typedef struct dir
{
    char *path;
    char *owner;
    char *group;
    char flags[4];
    char perms_user[4];
    char perms_group[4];
    char perms_other[4];
    struct dir *dir_head;
    struct dir *dir_tail;
    struct file *file_head;
    struct file *file_tail;
    struct dir *next;
    struct dir *prev;
} dir;

typedef struct file
{
    char *path;
    char *owner;
    char *group;
    char perms_user[4];
    char perms_group[4];
    char perms_other[4];
    struct file *next;
    struct file *prev;
} file;

int export(FILE *permissions_file);

int import(FILE *permissions_file);
