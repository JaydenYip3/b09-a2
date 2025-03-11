#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

typedef struct Flags { // Struct for holding flags and terminal arguements
        int per_process;
        int system_wide;
        int Vnodes;
        int composite;
        int summary;
        int threshold_int;
        char* PID;
    }Flags;


int owns_file(char *PID){
    // descry: This function determines if the following PID is accessible by the user.
    // PID: Process identity value , type string.
    // returning: This function returns true or false if the PID is accessible by the user.

    char path[64];
    struct stat st;
    //intialization

    snprintf(path, sizeof(path), "/proc/%s", PID);
    //creates the path directory to the PID

    if (stat(path, &st) != 0){
        return 0;
        //checks if the PID exists
    }
    return st.st_uid == getuid();
    //returns true if the user owns the process

}

void parse_args(Flags* f, int argc, char** argv){
    // descry: Determines the state of each flag and argument in Flags f, this will determine the outputs that will be made
    // f: Custom struct made to hold true or false values and strings, type Flags.
    // argc: The number of arguments in the terminal input, type int.
    // argv: A collection of strings that were inputted from the terminal, type string array.
    // returning: No return value.

    f->per_process = 0;
    f->system_wide = 0;
    f->Vnodes = 0;
    f->composite = 0;
    f->summary = 0;
    f->threshold_int = 0;
    f->PID = NULL;
    // Preset to all false and empty

    if (argc == 1){
        f->composite = 1;
        return;
        // default output when no arguments inputted
    }

    int index = 1;
    //starts after the output file
    if (argv[index][0] != '-'){
        //checks if the first is a PID
        int len = strlen(argv[index]);
        f->PID = malloc(len + 1);
        //allocating space for the string user inputted
        if (f->PID){ //check if allocation worked
            strncpy(f->PID, argv[index],len );
            f->PID[len] = '\0';
            if (argc == 2){
                f->composite = 1;
                return;
                //if only two inputs are the output file and PID then default output
            }
        }
        else{
            fprintf(stderr, "Error allocating memory for PID arguement.");
            exit(1);
        }

        char file_path[256];
        snprintf(file_path, sizeof(file_path), "/proc/%s/fd", f->PID);
        DIR *fd_path = opendir(file_path);
        //For the input PID for its directory
        if (fd_path == NULL){
            if (!owns_file(f->PID)){
                fprintf(stderr, "You do not have permissions for PID:  %s.\n", f->PID);
                free(f->PID);
                exit(1);
                //checks if PID exits or if has permissions
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
            // sets true for per proces output
        }
        else if (strcmp(argv[index], "--systemWide") == 0){
            f->system_wide = 1;
            // sets true for system wide output
        }
        else if (strcmp(argv[index], "--Vnodes") == 0){
            f->Vnodes = 1;
            // sets true for Vnodes output
        }
        else if (strcmp(argv[index], "--composite") == 0){
            f->composite = 1;
            // sets true for composite output
        }
        else if (strcmp(argv[index], "--summary") == 0){
            f->summary = 1;
            // sets true for summary output
        }
        else if (strncmp(argv[index], "--threshold=", 12) == 0){
            //checks if --threshold is the first 12 characters
            if (strlen(argv[index]) == 12){
                fprintf(stderr, "Error no value present after --threshold=X");
                exit(1);
                //if there is no X value
            }
            f->threshold_int =  (int)strtol(argv[index]+ 12, NULL, 10);

            if (f->threshold_int <= 0){
                // tests if threshold is a valid number
                //should catch if letters input as well

                fprintf(stderr, "Please pick a threshold strictly greater than 0.\n");
                exit(1);
            }


        }
        else{
            //if runs through all flags and none match then the flag is unknown
            fprintf(stderr, "There is an unrecognized flag or arguement inputted. \n");
            exit(1);
        }
    }
}
void per_process_output_single_PID(Flags* f, DIR *fd_path, struct dirent *fd_entry){
    // descry: Draws the outputs for the table for the per process flag for a singular process
    // f: Custom struct made to hold true or false values and strings, type Flags.
    // fd_path: directory path to the PID fd directory, type DIR.
    // fd_entry: pointer to a file in a directory (iterates through all in a PID, type struct dirent
    // returning: No return value.
    while ((fd_entry = readdir(fd_path))  != NULL){
        //iterates through each fd in the directory
        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
            // removes teh fds with values of "." and ".."
            continue;
        }
        printf("         %-7.7s %s\n", f->PID, fd_entry->d_name);
        // print format for output
    }

}

void per_process_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the per process flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    int count = 0;
    // counter for the ouptput format
    while ((entry = readdir(proc_dir))){
        // iterates through each PID in the /proc/
        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);

        if (!isdigit(PID)){
            //PIDs only contain numbers
            continue;
        }

        if (owns_file(PID)){
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(file_path);
            //path of the current PID's file directory

            if (!fd_path){
                //error accessing the path as a safety check
                continue;
            }

            while ((fd_entry = readdir(fd_path)) != NULL){
                //iterates through each fd in the directory
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    // removes teh fds with values of "." and ".."
                    continue;
                }
                printf("%-8d %-7.7s  %s\n", count++, PID, fd_entry->d_name);
                //prints the formatted output
            }
            closedir(fd_path);
            //closes directory for safety
        }


    }

}
void system_Wide_output_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry){
    // descry: Draws the outputs for the table for the system Wide flag for a singular process
    // f: Custom struct made to hold true or false values and strings, type Flags.
    // fd_path: directory path to the PID fd directory, type DIR.
    // fd_entry: pointer to a file in a directory (iterates through all in a PID, type struct dirent
    // returning: No return value.

    if (fd_path) {
        //checks if fd_path was used
        rewinddir(fd_path);
        //resets the fd_path to the beginning of the fds
    }
        while ((fd_entry = readdir(fd_path))  != NULL){
            //iterates through each fd in the directory
            if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                // removes teh fds with values of "." and ".."
                continue;
            }

            char fd_filename[1024];
            char fd_entry_link[512]; //this might be bad...

            snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", f->PID, fd_entry->d_name);
            //gets the specific PID and FD to use for the directory path.

            ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
            // obtains and checks if the PID and FD path is valid
            if (length == -1){
                //invalid PID and FD path
                printf("%.7s %s\n", f->PID, fd_entry->d_name);
            }
            else{
                fd_filename[length] = '\0';
                //delimeter
                printf("         %-7.7s %-7.7s %s\n", f->PID, fd_entry->d_name, fd_filename);
                //prints the formatted output
            }
        }
}
void system_Wide_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the system Wide flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    int count = 0;
    // counter for the ouptput format

    while ((entry = readdir(proc_dir))){
         // iterates through each PID in the /proc/

        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);
        //gets the PID

        if (!isdigit(PID)){
            // PIDs can only be digits
            continue;
        }

        if (owns_file(PID)){
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(file_path);
            //path of the current PID's file directory

            if (!fd_path){
                //error accessing the path as a safety check
                continue;
            }
            while ((fd_entry = readdir(fd_path)) != NULL){
                //iterates through each fd in the directory
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    // removes teh fds with values of "." and ".."
                    continue;
                }
                char fd_filename[1024];
                char fd_entry_link[512]; //this might be bad...
                snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", PID, fd_entry->d_name);
                //gets the specific PID and FD to use for the directory path.
                ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
                // obtains and checks if the PID and FD path is valid
                if (length == -1){
                    //invalid path
                    printf("%.7s %s\n",PID, fd_entry->d_name);
                }
                else{
                    fd_filename[length] = '\0';
                    printf("%-8d %-7.7s %-7.7s %s\n", count++, PID, fd_entry->d_name, fd_filename);
                    //prints the formatted output
                }
            }
            closedir(fd_path);
        }


    }
}
void Vnodes_ouptput_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry){
    // descry: Draws the outputs for the table for the Vnodes flag for a singular process
    // f: Custom struct made to hold true or false values and strings, type Flags.
    // fd_path: directory path to the PID fd directory, type DIR.
    // fd_entry: pointer to a file in a directory (iterates through all in a PID, type struct dirent
    // returning: No return value.

    if (fd_path) {
        //checks if fd_path was used
        rewinddir(fd_path);
        //resets the fd_path to the beginning of the fds
    }
    while ((fd_entry = readdir(fd_path)) != NULL) {
        //iterates through each fd in the directory
        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
            // removes teh fds with values of "." and ".."
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "/proc/%s/fd/%s", f->PID, fd_entry->d_name);
        //gets the specific PID and FD to use for the directory path.


        struct stat file_stat;
        int ret = fstatat(AT_FDCWD, full_path, &file_stat, AT_SYMLINK_NOFOLLOW);
        // retreives metadata into file_stat

        if (ret < 0) {
            // invalid path
            continue;
        }
        printf("         %-14.14s %ld\n", fd_entry->d_name, file_stat.st_ino);
        //output formatted
    }
}

void Vnodes_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the Vnodes flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    int count = 0;
     // counter for the ouptput format

    while ((entry = readdir(proc_dir))){
        // iterates through each PID in the /proc/
        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);
        //gets the PID

        if (!isdigit(PID)){
            // PIDs can only be digits
            continue;
        }
        if (owns_file(PID)) {
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(file_path);
            //path of the current PID's file directory

            if (!fd_path) {
                //error accessing the path as a safety check
                continue;
            }
            while ((fd_entry = readdir(fd_path)) != NULL){
                 //iterates through each fd in the directory
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                     // removes teh fds with values of "." and ".."
                    continue;
                }
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "/proc/%s/fd/%s", PID, fd_entry->d_name);
                //gets the specific PID and FD to use for the directory path.

                struct stat file_stat;
                int ret = fstatat(AT_FDCWD, full_path, &file_stat, AT_SYMLINK_NOFOLLOW);
                // retreives metadata into file_stat

                if (ret < 0) {
                    // invalid path
                    continue;
                }
                printf("%-8d %-14.14s %ld\n", count++, fd_entry->d_name, file_stat.st_ino);
                //output formatted

            }
            closedir(fd_path);
        }
    }

}
void composite_output_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry){
    // descry: Draws the outputs for the table for the Composite flag for a singular process
    // f: Custom struct made to hold true or false values and strings, type Flags.
    // fd_path: directory path to the PID fd directory, type DIR.
    // fd_entry: pointer to a file in a directory (iterates through all in a PID, type struct dirent
    // returning: No return value.

    if (fd_path) {
        //checks if fd_path was used
        rewinddir(fd_path);
        //resets the fd_path to the beginning of the fds
    }
    while ((fd_entry = readdir(fd_path))  != NULL){
         //iterates through each fd in the directory
        if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
            // removes teh fds with values of "." and ".."
            continue;
        }
        char fd_filename[1024];
        char fd_entry_link[512]; //this might be bad...
        snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", f->PID, fd_entry->d_name);
        //gets the specific PID and FD to use for the directory path.
        ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
        // obtains and checks if the PID and FD path is valid

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "/proc/%s/fd/%s", f->PID, fd_entry->d_name);
        //gets the specific PID and FD to use for the directory path.


        struct stat file_stat;
        int ret = fstatat(AT_FDCWD, full_path, &file_stat, AT_SYMLINK_NOFOLLOW);
        // retreives metadata into file_stat

        if (ret < 0) {
            // invalid path
            continue;
        }
        if (length == -1){
            //invalid path
            printf("         %-7.7s %-7.7s %s       %ld\n", f->PID, fd_entry->d_name, "", file_stat.st_ino);
        }
        else{
            //prints out the correctly formatted output
            fd_filename[length] = '\0';
            printf("         %-7.7s %-7.7s %s       %ld\n", f->PID, fd_entry->d_name, fd_filename, file_stat.st_ino);
        }

    }
}
void composite_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the composite flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    int count = 0;
     // counter for the ouptput format
    while ((entry = readdir(proc_dir))){
        // iterates through each PID in the /proc/
        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);
         //gets the PID
        if (!isdigit(PID)){// PIDs can only be digits
            continue;
        }

        if (owns_file(PID)){
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(file_path);
            //path of the current PID's file directory

            if (!fd_path){
                //error accessing the path as a safety check
                continue;
            }

            while ((fd_entry = readdir(fd_path)) != NULL){
                 //iterates through each fd in the directory
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    // removes teh fds with values of "." and ".."
                    continue;
                }
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "/proc/%s/fd/%s", PID, fd_entry->d_name);
                //gets the specific PID and FD to use for the directory path.
                struct stat file_stat;
                int ret = fstatat(AT_FDCWD, full_path, &file_stat, AT_SYMLINK_NOFOLLOW);
                // retreives metadata into file_stat
                if (ret < 0) {
                    // invalid path
                    continue;
                }
                char fd_filename[1024];
                char fd_entry_link[512]; //this might be bad...
                snprintf(fd_entry_link, sizeof(fd_entry_link), "/proc/%.9s/fd/%s", PID, fd_entry->d_name);
                ssize_t length = readlink(fd_entry_link, fd_filename, sizeof(fd_filename) - 1);
                if (length == -1){
                     //invalid path
                    printf("%-8d %-7.7s %-7.7s %s       %ld\n", count++, entry->d_name, fd_entry->d_name, "", file_stat.st_ino);
                }
                else{
                    //prints formatted output
                    fd_filename[length] = '\0';
                    printf("%-8d %-7.7s %-7.7s %s       %ld\n", count++, entry->d_name, fd_entry->d_name, fd_filename, file_stat.st_ino);
                }

            }
            closedir(fd_path);
        }


    }
}
void summary_output( DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the summary flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    while ((entry = readdir(proc_dir)) != NULL){
        // iterates through each PID in the /proc/
        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);
        //gets the PID

        if (!isdigit(PID)) {
            // PIDs can only be digits
            continue;
        }

        if (owns_file(PID)) {
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(file_path);
            //path of the current PID's file directory

            if (!fd_path) {
                //checks if path is not NULL
                continue;
            }

            int fd_count = 0;
            //counter for how many FDs in a PID
            while ((fd_entry = readdir(fd_path)) != NULL) {
                // each FD in a PID
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                    // removes teh fds with values of "." and ".."
                    continue;
                }
                fd_count++;
                //adds the valid FD
            }
            closedir(fd_path);

            printf("%.7s (%d),   ", PID, fd_count);
            //prints output
        }
    }
}
void threshold_output(Flags* f,DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry){
    // descry: Draws the outputs for the table for the summary flag for all the running processes
    // fd_path: directory path to the PID fd directory, type DIR.
    // proc_dir: Directory to /proc/ where it contains all the PIDs, type DIR
    // fd_entry: pointer to a file in adirectory (iterates through all in a PID, type struct dirent
    // entry: pointer to a file in a directory, iterates through all the PIDs, type struct dirent
    // returning: No return value.

    while ((entry = readdir(proc_dir)) != NULL){
        // iterates through each PID in the /proc/
        char PID[20];
        snprintf(PID, sizeof(PID), "%.9s", entry->d_name);
        //gets the PID
        if (!isdigit(PID)) {
            // PIDs can only be digits
            continue;
        }

        if (owns_file(PID)) {
            //checks if has access to the PID
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "/proc/%s/fd", PID);
            fd_path = opendir(fd_path);
            //path of the current PID's file directory

            if (!fd_path) {
                //checks if path is not NULL
                continue;
            }

            int fd_count = 0;
            //counter for how many FDs in a PID
            while ((fd_entry = readdir(fd_path)) != NULL) {
                // each FD in a PID
                if (strcmp(fd_entry->d_name, ".") == 0 || strcmp(fd_entry->d_name, "..") == 0) {
                     // removes teh fds with values of "." and ".."
                    continue;
                }
                fd_count++;
                //adds the valid FD
            }
            closedir(fd_path);
            if (fd_count >= f->threshold_int){
                // only prints output if the PID contains more FDs than the threshold inputted
                printf("%.7s (%d),   ", PID, fd_count);
            }

        }
    }
}
void table_output(struct Flags* f){
    // descry: Draws the tables for the ouptputs and uses functions to fill the tables
    // f: Holds all the flags and arguments
    // returning: No return value.

    //printf("%s", f->PID);
    struct dirent *entry;
    struct dirent *fd_entry;
    DIR *fd_path;
    DIR *proc_dir = opendir("/proc");
    //specific directories and paths used in the modular methods

    if (!proc_dir){
        // Catches system error if /proc does not open properly
        fprintf(stderr, "Erorr in opening '/proc' directory.");
        exit(1);
    }
    if (f->PID){
         // Checks if PID is a valid path.
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "/proc/%s/fd", f->PID);
        fd_path = opendir(file_path);
        if (!fd_path){
            fprintf(stderr, "Error in opening path: %s\n", file_path);
            exit(1);
        }
    }

    if (f->per_process){
        // if flag per_process is true
        printf("         PID     FD\n");
        printf("        ============\n");
        //printed for formatting output table

        if (f->PID){
            per_process_output_single_PID(f, fd_path, fd_entry);
            // if PID is provided
        }
        else{
            per_process_output_multiple_PID(fd_path, proc_dir, fd_entry, entry);
            // no PID thus it prints all the PIDS
        }
        printf("        ============\n\n");
        //printed for formatting output table

    }
    if (f->system_wide){
        // if flag system_wide is true

        if (proc_dir) {
            //resets proc dir if used earlier in per_process
            rewinddir(proc_dir);
        }
        printf("         PID     FD      Filename\n");
        printf("        ========================================\n");
        if (f->PID){
            system_Wide_output_single_PID(f, fd_path,fd_entry);
            // if PID is provided
        }
        else{
            system_Wide_output_multiple_PID(fd_path,proc_dir,fd_entry,entry);
             // no PID thus it prints all the PIDS
        }
        printf("        ========================================\n\n");
    }
    if (f->Vnodes) {
        // if flag Vnodes is true
        if (proc_dir) {
            //resets proc dir if used earlier
            rewinddir(proc_dir);
        }

        printf("         FD            Inode\n");
        printf("        ========================================\n");

        if (f->PID) {
            Vnodes_ouptput_single_PID(f, fd_path,fd_entry);
             // if PID is provided
            }
        else{
            Vnodes_output_multiple_PID(fd_path,proc_dir,fd_entry,entry);
            // no PID thus it prints all the PIDS

         }
         printf("        ========================================\n\n");
    }
    if (f->composite){
        // if flag composite is true
        if (proc_dir) {
            //resets proc dir if used earlier
            rewinddir(proc_dir);
        }
        printf("         PID     FD       Filename       Inode\n");
        printf("        ========================================\n");
        if (f->PID){
            composite_output_single_PID(f,fd_path,fd_entry);
            // if PID is provided
        }
        else{
            composite_output_multiple_PID(fd_path,proc_dir,fd_entry,entry);
            // no PID thus it prints all the PIDS
        }
        printf("        ========================================\n");

    }
    if (f->summary){
        // if flag summary is true
        if (proc_dir) {
            //resets proc dir if used earlier
            rewinddir(proc_dir);
        }

        printf("         Summary Table\n");
        printf("        ================\n"); //table format
        summary_output(fd_path, proc_dir,fd_entry,entry);
        //prints for the summary flag output
        printf("\n\n");
    }
    if (f->threshold_int){
        // if flag threshold is true
        if (proc_dir) {
            //resets proc dir if used earlier
            rewinddir(proc_dir);
        }

        printf("#Offending processes\n");//table format
        threshold_output(f, fd_path, proc_dir, fd_entry, entry);
        //prints for the threshold flag output
        printf("\n\n");
    }

    return;
}

int main(int argc, char** argv){
    // descry: Initializes Flags and calls the functions needed sequentially
    // argc: The number of arguments in the terminal input, type int.
    // argv: A collection of strings that were inputted from the terminal, type string array.
    // returning: default if it ran properly.

    Flags input_flags;
    //intialize flags
    parse_args(&input_flags, argc, argv);
    //sets up flags, reads and saves
    table_output(&input_flags);
    //draws tables and outputs accordinly to flags



    return 0;//default safe ouptput
}