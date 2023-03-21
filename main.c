#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "permissions.h"

void help(char *message)
{
    fprintf(stderr, "%s\n", message);
    fprintf(stdout, "first argument must be -i | --import or e- || --export\n");
    fprintf(stdout, "second argument is file for or with permissions\n");
    fprintf(stdout, "third argument is optional specified directory where program run\n");
}

int main(int argc, char *argv[])
{
    if (argc <= 2 || argc >= 5) {
        help("not enouch arguments");
        return EXIT_FAILURE;
    }
    bool return_value = true;
    FILE *permissions_file;
    if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--export") == 0) {
        permissions_file = fopen(argv[2], "w");
        if (!permissions_file) {
            fprintf(stderr, "%s\n", "fopen_permission_file");
            return EXIT_FAILURE;
        }
        if (argc == 4) {
            if (chdir(argv[3]) == -1) {
                fprintf(stderr, "%s\n", "Can't enter directory");
                fclose(permissions_file);
                return EXIT_FAILURE;
            }
        }
        return_value = export(permissions_file);

    } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--import") == 0) {
        permissions_file = fopen(argv[2], "r");
        if (!permissions_file) {
            fprintf(stderr, "%s\n", "fopen_permission_file");
            return EXIT_FAILURE;
        }
        if (argc == 4) {
            if (chdir(argv[3]) == -1) {
                fprintf(stderr, "%s\n", "Can't enter directory");
                fclose(permissions_file);
                return EXIT_FAILURE;
            }
        }
        return_value = import(permissions_file);

    } else {
        fprintf(stderr, "%s\n", "Invalid argument");
        return EXIT_FAILURE;
    }

    fclose(permissions_file);
    if (return_value == 0) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
