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

typedef struct Node {
    int fd;
    int inode;
    struct Node *next;
} Node;

Node * createNode(int fd){
    Node *new_node = (Node*)malloc(sizeof(Node));
    new_node->fd = fd;
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) < 0) {
        //fprintf(stderr, "Error retrieving inode");
        return NULL;
    }
    new_node->inode = (int)fd_stat.st_ino;
    new_node->next = NULL;

    return new_node;

}

int contains_fd(Node *head, int fd){
    Node *current = head;

    while (current != NULL){
        if (current->fd == fd){
            return 1;
        }
        current = current->next;
    }
    return 0;
}

Node* add_fd(Node *head, int fd){
    if (contains_fd(head, fd)){
        return head;
    }
    Node *new_node = createNode(fd);
    if (!new_node){
        return head;
    }
    if (head ==NULL || head->fd > fd){
        new_node->next = head;
        return new_node;
    }
    Node *current = head;
    while (current->next != NULL && current->next->fd < fd){
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;
    return head;
}

void free_fd(Node *head){
    Node *current = head;
    while (current != NULL){
        Node *next = current->next;
        free(current);
        current = next;
    }
}
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
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    continue;
                }
                printf("%.7s %s\n", f->PID, fd_entry->d_name);
            }
        }
        else{
            while ((entry = readdir(proc_dir))){
                char PID[20];
                snprintf(PID, sizeof(PID), "%.9s", entry->d_name);

                if (!isdigit(PID[0])){
                    continue;
                }

                if (owns_file(PID)){
                    char file_path[256];
                    snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
                    fd_path = opendir(file_path);

                    if (!fd_path){
                        //error accessing the path as a safety check
                        continue;
                    }
                    while ((fd_entry = readdir(fd_path)) != NULL){
                        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                           continue;
                        }
                        printf("%.7s %s\n", PID, fd_entry->d_name);
                    }
                    closedir(fd_path);
                }


            }
        }

    }
    if (f->system_wide){
        if (proc_dir){
            rewinddir(proc_dir);
        }
        printf("PID     FD     Filename\n");
        printf("========================\n");
        if (f->PID){
            while ((fd_entry = readdir(fd_path))  != NULL){
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    continue;
                }
                char fd_filename[1024];
                char fd_entry_link[512]; //this might be bad...
                snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", f->PID, fd_entry->d_name);
                ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
                if (length == -1){
                    printf("%.7s %s\n", f->PID, fd_entry->d_name);
                }
                else{
                    fd_filename[length] = '\0';
                    printf("%.7s %s %s\n", f->PID, fd_entry->d_name, fd_filename);
                }
            }
        }
        else{
            while ((entry = readdir(proc_dir))){
                char PID[20];
                snprintf(PID, sizeof(PID), "%.9s", entry->d_name);

                if (!isdigit(PID[0])){
                    continue;
                }

                if (owns_file(PID)){
                    char file_path[256];
                    snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
                    fd_path = opendir(file_path);

                    if (!fd_path){
                        //error accessing the path as a safety check
                        continue;
                    }
                    while ((fd_entry = readdir(fd_path)) != NULL){
                        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                           continue;
                        }
                        char fd_filename[1024];
                        char fd_entry_link[512]; //this might be bad...
                        snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", PID, fd_entry->d_name);
                        ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
                        if (length == -1){
                            printf("%.7s %s\n",PID, fd_entry->d_name);
                        }
                        else{
                            fd_filename[length] = '\0';
                            printf("%.7s %s %s\n", PID, fd_entry->d_name, fd_filename);
                        }
                    }
                    closedir(fd_path);
                }


            }
        }
    }
    if (f->Vnodes){

         if (proc_dir){
            rewinddir(proc_dir);
        }


        printf("FD     Inode\n");
        printf("============\n");
        if (f->PID){
            Node *head = NULL;
            while ((fd_entry = readdir(fd_path))  != NULL){
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    continue;
                }
                int fd = (int) strtol(fd_entry->d_name, NULL, 10);
                head = add_fd(head, fd);

            }
            Node *print_nodes = head;
            while (print_nodes != NULL){
                printf("%d      %d", print_nodes->fd, print_nodes->inode);
                print_nodes = print_nodes->next;
            }
            free_fd(head);
        }
        else{
            Node *head = NULL;
            while ((entry = readdir(proc_dir))){
                char PID[20];
                snprintf(PID, sizeof(PID), "%.9s", entry->d_name);

                if (!isdigit(PID[0])){
                    continue;
                }
                if (owns_file(PID)) {
                    char file_path[256];
                    snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
                    fd_path = opendir(file_path);

                    if (!fd_path) {
                        continue;
                    }
                    while ((fd_entry = readdir(fd_path)) != NULL){
                        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                            continue;
                        }
                        int fd = (int) strtol(fd_entry->d_name, NULL, 10);
                        head = add_fd(head, fd);
                    }
                    closedir(fd_path);
                }
            }
            Node *print_nodes = head;
            while (print_nodes != NULL){
                printf("%d      %d", print_nodes->fd, print_nodes->inode);
                print_nodes = print_nodes->next;
            }
            free_fd(head);

         }
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