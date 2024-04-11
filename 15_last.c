#include <stdio.h>
#include <string.h>
#include "15_last.h"
#include "main_func.h"

int main(int argc, char **argv) {

    if (argc < 3){
        printf("Too few arguments");
    }
    if (strcmp(argv[1], "--file") != 0){
        printf("Incorrect command");
    }
    if (strcmp(argv[3], "--create") == 0) {
        create(argv[2], argc, argv);
    }
    if (strcmp(argv[3], "--extract") == 0) {
        extract(argv[2]);
    }
    if (strcmp(argv[3], "--list") == 0) {
        list(argv[2]);
    }

    return 0;
}