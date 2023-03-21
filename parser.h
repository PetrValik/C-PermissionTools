//
// Created by Petr Valik on 24.05.2022.
//

#include <stdio.h>

typedef struct import_list
{
    struct import_file *head;
    struct import_file *tail;
    int return_value;
    int elements;
    int used;
} import_list;

typedef struct import_file
{
    char *path;
    char *owner;
    char *group;
    char flags[4];

    char perms_user[4];
    char perms_group[4];
    char perms_other[4];
    struct import_file *next;
} import_file;

int import_files(import_list *perms, FILE *permissions_file);
