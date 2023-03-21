//
// Created by Petr Valik on 10.05.2022.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dirent.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "pwd.h"
#include "grp.h"
#include "parser.h"
#include "permissions.h"

void free_files(dir *directory)
{
    file *next;
    file *actual = directory->file_head;
    while (actual != NULL) {
        next = actual->next;
        free(actual->path);
        free(actual->owner);
        free(actual->group);
        free(actual);
        actual = next;
    }
}

void free_dir(dir *directory)
{
    dir *next;
    dir *actual = directory->dir_head;
    free_files(directory);
    while (actual != NULL) {
        next = actual->next;
        free_dir(actual);
        free(actual->path);
        free(actual->owner);
        free(actual->group);
        free(actual);
        actual = next;
    }
}

void print_dir(FILE *output, dir *directory, int first)
{
    if (first != 0) {
        fprintf(output, "\n");
    }
    fprintf(output, "# file: %s\n", directory->path);
    fprintf(output, "# owner: %s\n", directory->owner);
    fprintf(output, "# group: %s\n", directory->group);
    if (directory->flags[0] != 'X') {
        fprintf(output, "# flags: %s\n", directory->flags);
    }
    fprintf(output, "user::%s\n", directory->perms_user);
    fprintf(output, "group::%s\n", directory->perms_group);
    fprintf(output, "other::%s\n", directory->perms_other);
}

void print_file(FILE *output, file *info_file)
{
    fprintf(output, "\n");
    fprintf(output, "# file: %s\n", info_file->path);
    fprintf(output, "# owner: %s\n", info_file->owner);
    fprintf(output, "# group: %s\n", info_file->group);
    fprintf(output, "user::%s\n", info_file->perms_user);
    fprintf(output, "group::%s\n", info_file->perms_group);
    fprintf(output, "other::%s\n", info_file->perms_other);
}

int alphabet_check(char *original, char *new)
{
    unsigned int x = strlen(original);
    if (strlen(original) > strlen(new)) {
        x = strlen(new);
    }
    for (unsigned int i = 0; i < x; i++) {
        if (original[i] > new[i]) {
            return 0;
        } else if (original[i] < new[i]) {
            return 1;
        }
    }
    if (strlen(original) > strlen(new)) {
        return 0;
    }
    return 1;
}

void insert_dir_to_export(dir *directory, dir *new_directory)
{
    dir *actual = directory->dir_head;
    while (actual != NULL) {
        if (alphabet_check(actual->path, new_directory->path) == 0) {
            if (actual->prev != NULL) {
                actual->prev->next = new_directory;
            }
            if (actual == directory->dir_head) {
                new_directory->prev = NULL;
                directory->dir_head = new_directory;
            }
            new_directory->prev = actual->prev;
            actual->prev = new_directory;
            new_directory->next = actual;
            return;
        }
        actual = actual->next;
    }
    if (directory->dir_head == NULL) {
        directory->dir_head = new_directory;
        directory->dir_tail = new_directory;
        new_directory->prev = NULL;
    } else {
        directory->dir_tail->next = new_directory;
        new_directory->prev = directory->dir_tail;
        directory->dir_tail = new_directory;
    }
    new_directory->next = NULL;
}

void insert_file_to_export(dir *directory, file *new_file)
{
    file *actual = directory->file_head;
    while (actual != NULL) {
        if (alphabet_check(actual->path, new_file->path) == 0) {
            if (actual->prev != NULL) {
                actual->prev->next = new_file;
            }
            if (actual == directory->file_head) {
                directory->file_head = new_file;
            }
            new_file->prev = actual->prev;
            actual->prev = new_file;
            new_file->next = actual;
            return;
        }
        actual = actual->next;
    }
    if (directory->file_head == NULL) {
        directory->file_head = new_file;
        directory->file_tail = new_file;
        new_file->prev = NULL;
    } else {
        directory->file_tail->next = new_file;
        new_file->prev = directory->file_tail;
        directory->file_tail = new_file;
    }
    new_file->next = NULL;
}

int file_owner_group(struct stat stats, file *new_file)
{
    struct passwd *pw;
    struct group *grp;
    pw = getpwuid(stats.st_uid);
    grp = getgrgid(stats.st_gid);
    char *owner_name = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    if (owner_name == NULL) {
        return 1;
    }
    char *group_name = malloc(sizeof(char) * strlen(grp->gr_name) + 1);
    if (group_name == NULL) {
        free(owner_name);
        return 1;
    }
    strcpy(owner_name, pw->pw_name);
    strcpy(group_name, grp->gr_name);
    new_file->owner = owner_name;
    new_file->group = group_name;
    return 0;
}

void file_permissions(struct stat stats, file *new_file)
{
    strcpy(new_file->perms_user, "---");
    if (stats.st_mode & S_IRUSR)
        new_file->perms_user[0] = 'r';
    if (stats.st_mode & S_IWUSR)
        new_file->perms_user[1] = 'w';
    if (stats.st_mode & S_IXUSR)
        new_file->perms_user[2] = 'x';

    strcpy(new_file->perms_group, "---");
    if (stats.st_mode & S_IRGRP)
        new_file->perms_group[0] = 'r';
    if (stats.st_mode & S_IWGRP)
        new_file->perms_group[1] = 'w';
    if (stats.st_mode & S_IXGRP)
        new_file->perms_group[2] = 'x';

    strcpy(new_file->perms_other, "---");
    if (stats.st_mode & S_IROTH)
        new_file->perms_other[0] = 'r';
    if (stats.st_mode & S_IWOTH)
        new_file->perms_other[1] = 'w';
    if (stats.st_mode & S_IXOTH)
        new_file->perms_other[2] = 'x';
}

int load_file(char *path, struct stat stats, file *new_file)
{
    if (file_owner_group(stats, new_file) == 1) {
        free(new_file);
        return 1;
    }
    new_file->path = path;
    file_permissions(stats, new_file);
    return 0;
}

int dir_owner_group(struct stat stats, dir *new_dir)
{
    struct passwd *pw;
    struct group *grp;
    pw = getpwuid(stats.st_uid);
    grp = getgrgid(stats.st_gid);
    char *owner_name = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    if (owner_name == NULL) {
        return 1;
    }
    char *group_name = malloc(sizeof(char) * strlen(grp->gr_name) + 1);
    if (group_name == NULL) {
        free(owner_name);
        return 1;
    }
    strcpy(owner_name, pw->pw_name);
    strcpy(group_name, grp->gr_name);
    new_dir->owner = owner_name;
    new_dir->group = group_name;
    return 0;
}

void dir_permissions(struct stat stats, dir *new_dir)
{
    strcpy(new_dir->perms_user, "---");
    if (stats.st_mode & S_IRUSR)
        new_dir->perms_user[0] = 'r';
    if (stats.st_mode & S_IWUSR)
        new_dir->perms_user[1] = 'w';
    if (stats.st_mode & S_IXUSR)
        new_dir->perms_user[2] = 'x';

    strcpy(new_dir->perms_group, "---");
    if (stats.st_mode & S_IRGRP)
        new_dir->perms_group[0] = 'r';
    if (stats.st_mode & S_IWGRP)
        new_dir->perms_group[1] = 'w';
    if (stats.st_mode & S_IXGRP)
        new_dir->perms_group[2] = 'x';

    strcpy(new_dir->perms_other, "---");
    if (stats.st_mode & S_IROTH)
        new_dir->perms_other[0] = 'r';
    if (stats.st_mode & S_IWOTH)
        new_dir->perms_other[1] = 'w';
    if (stats.st_mode & S_IXOTH)
        new_dir->perms_other[2] = 'x';

    strcpy(new_dir->flags, "---");
    if (stats.st_mode & S_ISUID)
        new_dir->flags[0] = 's';
    if (stats.st_mode & S_ISGID)
        new_dir->flags[1] = 's';
    if (stats.st_mode & 512)
        new_dir->flags[2] = 't';
    if (strcmp(new_dir->flags, "---") == 0) {
        strcpy(new_dir->flags, "XXX");
    }
}

int load_dir(char *path, struct stat stats, dir *new_dir)
{
    if (dir_owner_group(stats, new_dir) == 1) {
        free(new_dir);
        return 1;
    }
    dir_permissions(stats, new_dir);
    new_dir->path = path;
    return 0;
}

int accepting_file(struct stat stats)
{
    if ((S_ISCHR(stats.st_mode))) {
        fprintf(stderr, "%s\n", "character device");
        return 1;
    } else if ((S_ISBLK(stats.st_mode))) {
        fprintf(stderr, "%s\n", "block device");
        return 1;
    } else if ((S_ISFIFO(stats.st_mode))) {
        fprintf(stderr, "%s\n", "FIFO");
        return 1;
    } else if ((S_ISLNK(stats.st_mode))) {
        fprintf(stderr, "%s\n", "symbolic link");
        return 1;
    } else if ((S_ISSOCK(stats.st_mode))) {
        fprintf(stderr, "%s\n", "socket");
        return 1;
    }
    return 0;
}

int rec_export(dir *directory)
{
    DIR *actual_dir = opendir(directory->path);
    struct dirent *actual_file = NULL;
    if (actual_dir == NULL) {
        fprintf(stderr, "%s\n", "opendir failed");
        return 1;
    }
    while ((actual_file = readdir(actual_dir)) != NULL) {
        if (strcmp(".", actual_file->d_name) == 0 || strcmp("..", actual_file->d_name) == 0) {
            continue;
        }
        char *new_name = malloc(sizeof(char) * (strlen(directory->path) + strlen(actual_file->d_name)) + 2);
        if (new_name == NULL) {
            return 1;
        }
        if (strcmp(directory->path, ".") == 0) {
            strcpy(new_name, actual_file->d_name);
        } else {
            strcpy(new_name, directory->path);
            strcat(new_name, "/");
            strcat(new_name, actual_file->d_name);
        }
        struct stat path_stat;

        if (stat(new_name, &path_stat) == -1) {
            free(new_name);
            fprintf(stderr, "%s\n", "stat failed");
            closedir(actual_dir);
            return 1;
        }
        if (accepting_file(path_stat) != 0) {
            free(new_name);
            closedir(actual_dir);
            return 1;
        }
        if (S_ISREG(path_stat.st_mode)) {
            file *new_file = malloc(sizeof(file));
            if (new_file == NULL) {
                free(new_name);
                closedir(actual_dir);
                return 1;
            }
            new_file->path = new_name;
            memset(new_file, 0, sizeof(file));
            new_file->path = new_name;
            if (load_file(new_name, path_stat, new_file) == 1) {
                free(new_name);
                free(new_file);
                closedir(actual_dir);
                return 1;
            }
            insert_file_to_export(directory, new_file);
        } else if (S_ISDIR(path_stat.st_mode)) {
            dir *new_dir = malloc(sizeof(dir));
            if (new_dir == NULL) {
                free(new_name);
                closedir(actual_dir);
                return 1;
            }
            memset(new_dir, 0, sizeof(dir));
            if (load_dir(new_name, path_stat, new_dir) == 1) {
                free(new_name);
                free(new_dir);
                closedir(actual_dir);
                return 1;
            }
            insert_dir_to_export(directory, new_dir);
            if (rec_export(new_dir) == 1) {
                closedir(actual_dir);
                return 1;
            }
        } else {
            free(new_name);
            closedir(actual_dir);
            fprintf(stderr, "%s\n", "unexpected type of file");
            return 1;
        }
    }
    closedir(actual_dir);
    return 0;
}

void print_files(FILE *permissions_file, dir *directory)
{
    file *actual = directory->file_head;
    while (actual != NULL) {
        print_file(permissions_file, actual);
        actual = actual->next;
    }
}

void print_dirs(FILE *permissions_file, dir *directory)
{
    dir *actual = directory->dir_head;
    if (strcmp(directory->path, ".") == 0) {
        print_dir(permissions_file, directory, 0);
    } else {
        print_dir(permissions_file, directory, 1);
    }
    while (actual != NULL) {
        print_dirs(permissions_file, actual);
        actual = actual->next;
    }
    print_files(permissions_file, directory);
}

int export(FILE *permissions_file)
{
    dir *root = malloc(sizeof(dir));
    memset(root, 0, sizeof(dir));
    if (root == NULL) {
        return 1;
    }
    struct stat stats;
    if (stat(".", &stats) != 0) {
        free(root);
        return 1;
    }
    if (dir_owner_group(stats, root) == 1) {
        free(root);
        return 1;
    }
    dir_permissions(stats, root);
    char *new_path = malloc(sizeof(char) * 2);
    if (new_path == NULL) {
        free(root->group);
        free(root->owner);
        free(root);
        return 1;
    }
    root->path = new_path;
    strcpy(new_path, ".");
    if (rec_export(root) == 1) {
        free_dir(root);
        free(root->path);
        free(root->owner);
        free(root->group);
        free(root);
        return 1;
    }
    print_dirs(permissions_file, root);
    free_dir(root);
    free(root->path);
    free(root->owner);
    free(root->group);
    free(root);
    return 0;
}

/** end of export **/

/** start of import **/

void free_list(import_list *perms)
{
    import_file *actual = perms->head;
    import_file *next;
    while (actual != NULL) {
        free(actual->path);
        free(actual->group);
        free(actual->owner);
        next = actual->next;
        free(actual);
        actual = next;
    }
    free(perms);
}

void set_rights(import_list *perms, import_file *perm_file)
{
    mode_t rights_to_add = 0;
    if (perm_file->perms_user[0] == 'r') {
        rights_to_add += S_IRUSR;
    }
    if (perm_file->perms_user[1] == 'w') {
        rights_to_add += S_IWUSR;
    }
    if (perm_file->perms_user[2] == 'x') {
        rights_to_add += S_IXUSR;
    }
    if (perm_file->perms_group[0] == 'r') {
        rights_to_add += S_IRGRP;
    }
    if (perm_file->perms_group[1] == 'w') {
        rights_to_add += S_IWGRP;
    }
    if (perm_file->perms_group[2] == 'x') {
        rights_to_add += S_IXGRP;
    }
    if (perm_file->perms_other[0] == 'r') {
        rights_to_add += S_IROTH;
    }
    if (perm_file->perms_other[1] == 'w') {
        rights_to_add += S_IWOTH;
    }
    if (perm_file->perms_other[2] == 'x') {
        rights_to_add += S_IXOTH;
    }
    if (perm_file->flags[0] != 'X') {
        if (perm_file->flags[0] != '-') {
            rights_to_add += S_ISUID;
        }
        if (perm_file->flags[1] != '-') {
            rights_to_add += S_ISGID;
        }
        if (perm_file->flags[2] != '-') {
            rights_to_add += 512;
        }
    }
    if (chmod(perm_file->path, rights_to_add) == -1) {
        fprintf(stderr, "%s\n", "unable to set rights");
        perms->return_value = 1;
    }
}

import_file *in_perm_list(import_list *perms, char *new_name)
{
    import_file *file = perms->head;
    while (file != NULL) {
        if (strcmp(file->path, new_name) == 0) {
            perms->used += 1;
            return file;
        }
        file = file->next;
    }
    return NULL;
}

void check_user_group(import_list *perms, import_file *perm_file, char *name)
{
    struct passwd *pw;
    struct group *grp;
    struct stat path_stat;
    if (stat(perm_file->path, &path_stat) == -1) {
        perms->return_value = 2;
        fprintf(stderr, "%s\n", "stat failed");
        return;
    }
    pw = getpwuid(path_stat.st_uid);
    grp = getgrgid(path_stat.st_gid);
    if (pw == NULL) {
        fprintf(stderr, "User of file %s is incorrect\n", name);
        perms->return_value = 1;
    } else if (strcmp(pw->pw_name, perm_file->owner) != 0) {
        fprintf(stderr, "User of file %s is incorrect\n", name);
        perms->return_value = 1;
    }
    if (grp == NULL) {
        fprintf(stderr, "Group of file %s is incorrect\n", name);
        perms->return_value = 1;
    } else if (strcmp(grp->gr_name, perm_file->group) != 0) {
        fprintf(stderr, "Group of file %s is incorrect\n", name);
        perms->return_value = 1;
    }
}

int try_set(import_list *perms, char *name, import_file *perm_file)
{
    check_user_group(perms, perm_file, name);
    if (perms->return_value == 2) {
        return 1;
    }
    set_rights(perms, perm_file);
    return 0;
}

int rec_import(import_list *perms, char *father)
{
    int return_value = 0;
    DIR *dir = opendir(father);
    struct dirent *file = NULL;
    if (dir == NULL) {
        fprintf(stderr, "%s\n", "opendir failed");
        return 1;
    }
    while ((file = readdir(dir)) != NULL) {
        if (strcmp(".", file->d_name) == 0 || strcmp("..", file->d_name) == 0) {
            continue;
        }
        char *new_name = malloc(sizeof(char) * (strlen(father) + strlen(file->d_name)) + 2);
        if (new_name == NULL) {
            closedir(dir);
            return 1;
        }
        if (strcmp(father, ".") == 0) {
            strcpy(new_name, file->d_name);
        } else {
            strcpy(new_name, father);
            strcat(new_name, "/");
            strcat(new_name, file->d_name);
        }
        import_file *perm_file = in_perm_list(perms, new_name);
        if (perm_file == NULL) {
            free(new_name);
            continue;
        }
        struct stat path_stat;
        if (stat(new_name, &path_stat) == -1) {
            free(new_name);
            fprintf(stderr, "%s\n", "stat failed");
            closedir(dir);
            return 1;
        }
        if (accepting_file(path_stat) != 0) {
            free(new_name);
            closedir(dir);
            return 1;
        }
        if (S_ISREG(path_stat.st_mode)) {
            if (try_set(perms, file->d_name, perm_file) != 0) {
                free(new_name);
                closedir(dir);
                return 1;
            }
        } else if (S_ISDIR(path_stat.st_mode)) {
            if (try_set(perms, file->d_name, perm_file) != 0) {
                free(new_name);
                closedir(dir);
                return 1;
            }
            if (rec_import(perms, new_name) != 0) {
                free(new_name);
                closedir(dir);
                return 1;
            }
        } else {
            free(new_name);
            closedir(dir);
            fprintf(stderr, "%s\n", "unexpected type of file");
            return 1;
        }
        free(new_name);
    }
    closedir(dir);
    return return_value;
}

void initialize_perm(import_list *perms)
{
    perms->head = NULL;
    perms->tail = NULL;
    perms->return_value = 0;
    perms->used = 0;
    perms->elements = 0;
}

int import(FILE *permissions_file)
{
    import_list *perms = malloc(sizeof(import_list));
    if (perms == NULL) {
        return 1;
    }
    initialize_perm(perms);
    if (import_files(perms, permissions_file) != 0) {
        free_list(perms);
        return 1;
    }
    import_file *perm_file = in_perm_list(perms, ".");
    if (perm_file != NULL) {
        if (try_set(perms, ".", perm_file) != 0) {
            free_list(perms);
        }
    }
    int return_value = rec_import(perms, ".");
    if (perms->elements != perms->used) {
        fprintf(stderr, "%s\n", "all files in permission file were not used");
        return_value = 1;
    }
    if (perms->return_value != 0) {
        return_value = 1;
    }
    free_list(perms);
    return return_value;
}
