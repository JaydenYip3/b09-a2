# Recreating the System-Wide FD Tables

# 1. Metadata:
Author: Jayden
Date: March 11, 2025


# Features
- Real-Time System Monitoring
    - CPU Usage: Percent of the CPU used
    - Memory USage: Amount of RAM used (in GB)
- Configurable Sampling
    - Number of samples
    - Time of delay between samples
- Cores Info
    - Number of cores
    - Max. frequency of cores
- Graphical Interface
    - ASCII bar graph for both memory and CPU
    samples

# Prerequisites
- C Compiler (e.g., gcc)
- Compatible system (linux)

# Usage
After compiling:
>gcc -lm -std=c99 -Werror -Wall -o myMonitoringTool a1.c

Note: Its important for -lm to be there as it supports for functions such as ceil()... etc

**CLA Syntax**
>./myMonitoringTool [samples [tdelay]] [--memory] [--cpu] [--cores] [--samples=N] [--tdelay=T]

If no arguements are passed then all the information is present with default
values of samples = 20 and tdelay = 500000 microseconds (0.5 seconds)

# Paramters
1. ><'samples'>
    - Optional Integer defualt 20
    - Represents how many times the data will be sampled
2. ><'tdelay'>
    - Optional Integer default 500000
    - Respresents the delay between the samples
3. **Options**
    - '--memory': Shows memory usage graph
    - '--cpu': Shows CPU usage graph
    - '--cores': Shows number of cores and max frequency
    - '--samples=N' Custom number of samples
    - '--tdelay=T' Custom delay between samples

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