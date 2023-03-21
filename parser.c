//
// Created by Petr Valik on 24.05.2022.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "permissions.h"
#include "parser.h"

int import_file_name(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *name = malloc(length + 1);
    if (name == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&name, &length, permissions_file);
    if (result == -1) {
        free(name);
        return 1;
    }
    char *token = strtok(name, " ");
    if (token == NULL) {
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    char *path = malloc(strlen(token) + 1);
    if (path == NULL) {
        free(name);
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    strcpy(path, token);
    token = strtok(NULL, " ");
    while (token != NULL) {
        char *new = malloc(strlen(path) + strlen(token) + 3);
        if (new == NULL) {
            free(path);
            free(name);
            return 1;
        }
        strcpy(new, path);
        strcat(new, " ");
        strcat(new, token);
        free(path);
        path = new;
        token = strtok(NULL, " ");
    }
    free(name);
    new_file->path = path;
    return 0;
}

int import_file_owner(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *owner = malloc(length + 1);
    if (owner == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&owner, &length, permissions_file);
    if (result == -1) {
        free(owner);
        return 1;
    }
    char *token = strtok(owner, " ");
    if (token == NULL) {
        free(owner);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(owner);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(owner);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    char *owner_name = malloc(strlen(token) + 1);
    if (owner_name == NULL) {
        free(owner);
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    strcpy(owner_name, token);
    free(owner);
    new_file->owner = owner_name;
    return 0;
}

int import_file_group(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *group = malloc(length + 1);
    if (group == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&group, &length, permissions_file);
    if (result == -1) {
        free(group);
        return 1;
    }
    char *token = strtok(group, " ");
    if (token == NULL) {
        free(group);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(group);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(group);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    char *group_name = malloc(strlen(token) + 1);
    if (group_name == NULL) {
        free(group);
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    strcpy(group_name, token);
    free(group);
    new_file->group = group_name;
    return 0;
}

int parse_user_perms(char *user_perms, import_file *new_file)
{
    char *token = strtok(user_perms, ":");
    if (token == NULL) {
        free(user_perms);
        return 1;
    }
    token = strtok(NULL, ":");
    if (token == NULL) {
        free(user_perms);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    strcpy(new_file->perms_user, token);
    free(user_perms);
    if (strlen(new_file->perms_user) != 3) {
        return 1;
    }
    return 0;
}

int import_user_perms(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *user_perms = malloc(length + 1);
    if (user_perms == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&user_perms, &length, permissions_file);
    if (result == -1) {
        free(user_perms);
        return 1;
    }
    return parse_user_perms(user_perms, new_file);
}

int parse_flags(char *flags, import_file *new_file)
{
    char *token = strtok(flags, " ");
    if (token == NULL) {
        free(flags);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(flags);
        return 1;
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(flags);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    strcpy(new_file->flags, token);
    free(flags);
    if (strlen(new_file->flags) != 3) {
        return 1;
    }
    return 0;
}

int import_flags(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    ssize_t result;
    char *flags = malloc(length + 1);
    if (flags == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    result = getline(&flags, &length, permissions_file);
    if (result == -1) {
        free(flags);
        return 1;
    }
    if (flags[0] == '#') {
        if (parse_flags(flags, new_file) != 0) {
            return 1;
        }
        if (import_user_perms(new_file, permissions_file) != 0) {
            return 1;
        }
    } else {
        strcpy(new_file->flags, "XXX");
        if (parse_user_perms(flags, new_file) != 0) {
            return 1;
        }
    }
    return 0;
}

int import_group_perms(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *group_perms = malloc(length + 1);
    if (group_perms == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&group_perms, &length, permissions_file);
    if (result == -1) {
        free(group_perms);
        return 1;
    }
    char *token = strtok(group_perms, ":");
    if (token == NULL) {
        free(group_perms);
        return 1;
    }
    token = strtok(NULL, ":");
    if (token == NULL) {
        free(group_perms);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    strcpy(new_file->perms_group, token);
    free(group_perms);
    if (strlen(new_file->flags) != 3) {
        return 1;
    }
    return 0;
}

int import_other_perms(import_file *new_file, FILE *permissions_file)
{
    size_t length = 64;
    char *other_perms = malloc(length + 1);
    if (other_perms == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        return 1;
    }
    ssize_t result = getline(&other_perms, &length, permissions_file);
    if (result == -1) {
        free(other_perms);
        return 1;
    }
    char *token = strtok(other_perms, ":");
    if (token == NULL) {
        free(other_perms);
        return 1;
    }
    token = strtok(NULL, ":");
    if (token == NULL) {
        free(other_perms);
        return 1;
    }
    token[strlen(token) - 1] = '\0';
    strcpy(new_file->perms_other, token);
    free(other_perms);
    if (strlen(new_file->flags) != 3) {
        return 1;
    }
    return 0;
}

void add_import_file(import_list *perms, import_file *new_file)
{
    if (perms->head == NULL) {
        perms->head = new_file;
        perms->tail = new_file;
    } else {
        perms->tail->next = new_file;
        perms->tail = new_file;
    }
    new_file->next = NULL;
}

int import_files(import_list *perms, FILE *permissions_file)
{
    while (true) {
        import_file *new_file = malloc(sizeof(import_file));
        perms->elements += 1;
        if (new_file == NULL) {
            fprintf(stderr, "%s\n", "malloc failed");
            return 1;
        }
        if (import_file_name(new_file, permissions_file) == 1) {
            fprintf(stderr, "%s\n", "name incorrect");
            free(new_file);
            return 1;
        }
        if (import_file_owner(new_file, permissions_file) != 0) {
            fprintf(stderr, "%s\n", "owner incorrect");
            free(new_file->path);
            free(new_file);
            return 1;
        }
        if (import_file_group(new_file, permissions_file) != 0) {
            fprintf(stderr, "%s\n", "group incorrect");
            free(new_file->path);
            free(new_file->owner);
            free(new_file);
            return 1;
        }
        if (import_flags(new_file, permissions_file) != 0) {
            fprintf(stderr, "%s\n", "flags or owner perms incorrect");
            free(new_file->path);
            free(new_file->owner);
            free(new_file);
            return 1;
        }
        if (import_group_perms(new_file, permissions_file) != 0) {
            fprintf(stderr, "%s\n", "group perms incorrect");
            free(new_file->path);
            free(new_file->owner);
            free(new_file);
            return 1;
        }
        if (import_other_perms(new_file, permissions_file) != 0) {
            fprintf(stderr, "%s\n", "other perms incorrect");
            free(new_file->path);
            free(new_file->owner);
            free(new_file);
            return 1;
        }
        add_import_file(perms, new_file);
        size_t length = 64;
        char *free_line = malloc(length + 1);
        if (free_line == NULL) {
            return 1;
        }
        ssize_t line_res = getline(&free_line, &length, permissions_file);
        free(free_line);
        if (line_res == -1) {
            break;
        }
    }
    return 0;
}
