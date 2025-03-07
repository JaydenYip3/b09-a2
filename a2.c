#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

struct Flags {
        int per_process;
        int system_wide;
        int Vnodes;
        int composite;
        int summary;
        int threshold_int;
        char* PID;
    };
int owns_file(char *PID){
    char path[64];
    struct stat st;

    snprintf(path, sizeof(path), "/proc/%s", PID);

    if (stat(path, &st) != 0){
        return 0;
    }
    return st.st_uid == getuid();

}
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
            exit(1);
        }
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "/proc/%s/fd", f->PID);
        DIR *fd_path = opendir(file_path);
        if (fd_path == NULL){
            if (!owns_file(f->PID)){
                fprintf(stderr, "You do not have permissions for PID:  %s.\n", f->PID);
                free(f->PID);
                exit(1);
            }
            fprintf( stderr, "No such process with PID: %s.\n", f->PID);
            free(f->PID);
            exit(1);
        }
        //printf("opened %s\n", file_path);
        index++;
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
            f->threshold_int =  (int)strtol(argv[index]+ 12, NULL, 10);
        }
    }
}

void table_output(struct Flags* f){
    //printf("%s", f->PID);
    struct dirent *entry;
    struct dirent *fd_entry;
    DIR *fd_path;
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir){
        fprintf(stderr, "Erorr in opening '/proc' directory.");
        exit(1);
    }
    if (f->PID){
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "/proc/%s/fd", f->PID);
        fd_path = opendir(file_path);
        if (!fd_path){
            fprintf(stderr, "Erorin opening path: %s", file_path);
            exit(1);
        }
    }

    if (f->per_process){
        printf("PID     FD\n");
        printf("==========\n");
        if (f->PID){
            while ((fd_entry = readdir(fd_path))  != NULL){
                printf("%.7s %s\n", f->PID, fd_entry->d_name);
            }
        }
        else{
            while ((entry = readdir(proc_dir))){
                char PID[256];
                strncpy(PID, entry->d_name, sizeof(PID) - 1);
                PID[sizeof(PID) - 1] = '\0';

                if (!isdigit(PID[0])){
                    continue;
                }

                if (owns_file(PID)){
                    char file_path[256];
                    snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
                    fd_path = opendir(file_path);
                    while ((fd_entry = readdir(fd_path)) != NULL){
                        printf("%.7s %s\n", PID, fd_entry->d_name);
                    }
                    closedir(file_path);
                }


            }
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
    table_output(&input_flags);



    return 0;
}