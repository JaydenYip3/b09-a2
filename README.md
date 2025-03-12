# Recreating the System-Wide FD Tables

# 1. Metadata:
Author: Jayden

Date: March 11, 2025


# 2. Introduction:
This is a simple C program that simply keeps track of open files, assignation of file descriptors, and process used by the OS.

# 3. Desiption of how you solve/approach the problem
I solved this my solving them sequentially. After all the setup of the flags and inputs (which was similar to A1) I started with per-process. I first had to do research on understanding the /proc/__/fd/ format. I played around in a linux system using ls -l /proc and such to visualize what I was actually doing. After that it was a matter of applying to what I saw. I used specific tools in C such as DIR and dirent to help navigate the directories and point to the items in the directory. After the per process it was pretty similar throughout where you have to either one set /proc/PID/fd/ to be a constant PID or a dynamic one where you iterate through, so the rest of the flags were similar in that manner.

# 4. Implementation
I implemented my code in modules, where each module (function) has a singular responsiblity (for the most part).
-  >int owns_file(char *PID)

This function tests to see if the current user has permission or access to the file and uses stat to test if PID exits.
-  >parse_args(Flags* f, int argc, char** argv)

This function takes in the arguments and saves their inputs in the Flags struct so it can be referenced later for printing and this uses DIR to read the directory path and test if PID leads to a valid directory path.
- >void per_process_output_single_PID(Flags* f, DIR *fd_path, struct dirent *fd_entry)

This function draws the content inside the table, it uses DIR to get to a directory and dirent to point at each item in the directory.
- >void per_process_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This function outputs for the all running processes, it uses DIR and dirent to each directory of all PIDs and their /proc/PID/fd/ and it uses dirent to point at the PIDs in /proc and FDs in /proc/PID/fd/.
- >void system_Wide_output_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry)

This function uses both DIR and dirent similar to per-process for a single PID however it includes
readlink to read the directory link.

- >void system_Wide_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This function again uses both DIR and dirent in a similar fashion to per_process_output_multiple_PID but also uses readlink() to help extract the path of the current PID,FD.

- >void Vnodes_ouptput_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry)

This function uses the DIR and dirent the same way as the previous but also uses fstatat to retrieve the metadata of the path, ultimately to find the inode of the fd.

- >void Vnodes_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This function behaves similarily with the multiple_PID processes but also uses fstatat to extract the file metadata to get the inode.

- >void composite_output_single_PID(Flags* f,DIR* fd_path, struct dirent *fd_entry)

This behaves similar to all the combined single_PID outputs together and does not add any new feature or usage than the code prior to it.

- >void composite_output_multiple_PID(DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This code again is similar to all the combined multiple_PID outputs together and has no new features within it.

- >void summary_output( DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This function essentially goes to all PIDs and starts counting their FDs by reading each entry and prints it out.

- > void threshold_output(Flags* f,DIR* fd_path, DIR* proc_dir, struct dirent *fd_entry, struct dirent *entry)

This function does a similar job to the summary_output but before printing it has a check if the number of FDs is greater than the threshold=X, then it prints with it is **STRICTLY GREATER THAN X**.

- > void table_output(struct Flags* f)

This function just simply connects the pieces together, that being all the output function and it intializes the paths of /proc and the path of /proc/PID/fd if PID is given.

- > int main(int argc, char** argv)

This function just simply defines the Flags struct and calls functions parse_args and table_output.



# 5. Flow Chart
![Flow Chart](./flowchart.png)

# 6. Compiling Code
To compile the code you should run:
> gcc -Werror -Wall -std=c99 -D_GNU_SOURCE  -o output a2.c

Note it is vital for -D_GNU_SOURCE because it allows readlink to work.

To run the code, there are optional flags and arguments:
> ./output [PID] [--per-process] [--systemWide] [--Vnodes] [--composite] [--summary] [--threshold=X]

Note it is mandatory if you want a specific PID then the PID must be the first argument.

All the other tags orders should not matter.

If no tag is present then it will default to composite.

The X in threshold should be an integer value, else it will exit or get truncated, and X is inclusive.

# 7. Expected Results


# How it works:
1. **Parsing Arguments**
    - The *parse_agruements()* function determines the user input from the execution and
    saves it all to a struct
2. **Data Collection**
    - The *get_cpu_times()* functions reads from /proc/stat to get CPU usage over tiem
    - The *get_memory_usage()* function retrieves total and used memory from the system.
    - The *get_core_info()* function gets the number of CPU cores and their max frequency.
3. **Graphs and Displays**
    - Memory Graph: The *print_memory_graph()* function generates an ASCII representation of memory usage over time.
        - **IMPORTANT**: The graph's y-axis is seperated by 10 segments where each segment represents a **percentage range**, that benig the very bottom one being from 0-10% all the way to the very top one being 90-100%
    - CPU Graph: The *print_cpu_graph()* function creates an ASCII visualization of CPU usage over the sampled time period.
        - The graph's y-axis is seperated by 12 segments where each represent a portion of the memory. With the bottom being 0 (**not the x-axis**) and top being the system rated memory
    - Cores Grid: The *print_core_info()* function formats and displays the number of cores and their frequencies in a structured format.
4. **Timing**
    - The *custom_sleep()* function implements a custom way to delay sampling





# Author
**Jayden Yip**
February 7, 2025