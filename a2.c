#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

#define PROC "/proc/"

struct Flags {
        int per_process;
        int system_wide;
        int Vnodes;
        int composite;
        int summary;
        int threshold_int;
        char* PID;
    };

void parse_args(struct Flags* f, int argc, char** argv){
    f->per_process = 0;
    f->system_wide = 0;
    f->Vnodes = 0;
    f->composite = 0;
    f->summary = 0;
    f->threshold_int = 0;
    f->PID = NULL;

    if (argc == 1){
        f->composite = 1;
        return;
    }

    int index = 1;
    if (argv[index][0] != '-'){
        int len = strlen(argv[index]);
        f->PID = malloc(len + 1);
        if (f->PID){
            strncpy(f->PID, argv[index],len );
            f->PID[len] = '\0';
        }
        else{
            fprintf(stderr, "Error allocating memory for PID arguement.");
        }
    }
    for (; index < argc; index++){
        if (strcmp(argv[index], "--per-process") == 0){
            f->per_process = 1;
        }
        else if (strcmp(argv[index], "--systemWide") == 0){
            f->system_wide = 1;
        }
        else if (strcmp(argv[index], "--Vnodes") == 0){
            f->Vnodes = 1;
        }
        else if (strcmp(argv[index], "--composite") == 0){
            f->composite = 1;
        }
        else if (strcmp(argv[index], "--summary") == 0){
            f->summary = 1;
        }
        else if (strncmp(argv[index], "--threshold=", 12) == 0){
            if (strlen(argv[index]) == 12){
                fprintf(stderr, "Error no value present after --threshold=X");
                exit(1);
            }
            f->threshold_int =  (int)strtol(argv[index][12], NULL, 10);
        }
    }
}

void table_output(struct Flags* f){
    if (f->PID){
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "/proc/%s/fd", f->PID);
        FILE *fp = fopen(file_path, "r");
        if (fp == NULL){
            fprintf(stderr, "File with PID: %s not found.", f->PID);
            exit(1);
        }
        char line[256];
        while (fgets(line, sizeof(line), fp)){
            printf("%s",line);
        }
    }


    if (f->per_process){
        if (f->PID){
        }
        else{
        }

    }
    if (f->system_wide){

    }
    if (f->Vnodes){

    }
    if (f->composite){

    }
    if (f->summary){

    }
    if (f->threshold_int){

    }

    return;
}

int main(int argc, char** argv){

    typedef struct Flags Flags;

    Flags input_flags;
    parse_args(&input_flags, argc, argv);



    return 0;
}