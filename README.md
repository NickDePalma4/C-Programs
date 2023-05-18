# C-Programs

This is a collection of projects that I have made for my UNIX programming class at UT Dallas, the following programs are described below. 

## Simple-Shell:

  ### This project implements a command line interpreter or shell, similar to BASH, however a little simpler. The program implements the following features:
        
   **- The Shell:** Execute commands provided by the user by invoking fork() and execvp().
        
   **- Builtins:** commands cd, exit, and history have been added as builtins. Both cd and exit behave the same as BASH, but history has the 
                   following syntax and behavior:
                    
                    - history [-c] [offset] 
                    
                    - history (without arguments) displays the last 100 commands the user ran, with
                      an offset next to each command. The offset is the index of the command in the
                      list, and valid values are 0 to 99, inclusive. 0 is the oldest command. 
                      Once more than 100 commands are executed, removes the oldest entry from the list to make room for
                      the newer commands. Note that history is also a command itself and therefore
                      will appear in the list of commands. If the user ran invalid commands,
                      those should also appear in the list.

                    - history -c clears the entire history, removing all entries. For example, running
                      history immediately after history -c should show history as the sole entry in
                      the list.
            
                    - history [offset] executes the command in history at the given offset. Prints an
                      error message if the offset is not valid.
                      
   **- Pipes:** Allows for piped commands using the '|' character, like the BASH interpreter, however it does not work on 
                the builtin commands (cd, exit, history).
                   
                   
## Hash-Tree: 

  ### This project uses Jenkins One at a Time Hash along with a user selected number of threads to calculate a 32-bit unsigned int hash value of a user specified file to act as a sort of file fingerprint. It uses a binary tree to equally distribute blocks (4096 bytes per block). The program includes the following:
  
   **- The Hash Tree:** using multithreading and a binary tree structure, the hash tree works 
          in the following manner:
                  
                   - Multi-Threaded Hashing: 
                      
                      This project aims to implement a multi-threaded hashing algorithm using a binary hash tree. 
                      The steps involved are as follows:

                      - Dividing the File: The input file is divided into multiple blocks, where each block represents a basic unit of data 
                        access. Let's assume there are n blocks and m threads, where n is divisible by m. Each thread is responsible for computing 
                        the hash value of n/m consecutive blocks.

                      - Creating the Binary Hash Tree: A complete binary tree of threads is created, with each leaf thread assigned n/m consecutive 
                        blocks. Each leaf thread computes the hash value of its assigned blocks and returns the hash value to its parent thread using 
                        the pthread_exit() call. Interior threads compute the hash value of their assigned blocks and wait for their children to return 
                        hash values.

                      - Combining Hash Values: The hash values returned by the child threads are combined to form a string. If an interior thread has 
                        both left and right child threads, the combined string is formed as <computed hash value + hash value from left child + hash value
                        from right child>. If an interior thread has no right child, only the hash value of the interior thread and the hash value from the 
                        left child are combined.
            
                      - Computing the Final Hash Value: The interior thread then computes the hash value of the concatenated string using the same hashing 
                        algorithm as before and returns the value to its parent. The value returned by the root thread is considered the hash value of the file.


                   - Block Assignment to Threads:

                      Each thread is assigned a number when it is created, starting from 0.
                      The number of the left child of thread i is 2 * i + 1.
                      The number of the right child of thread i is 2 * i + 2.
                      For a thread with number i, n/m consecutive blocks starting from block number i * n/m are assigned.
                      For example, let's consider a scenario where n = 100 and m = 4. The block assignment would be as follows:

                      Thread 0: Blocks 0 through 24
                      Thread 1: Blocks 25 through 49
                      Thread 2: Blocks 50 through 74
                      Thread 3: Blocks 75 through 99
                      
   **- The Report:** An experimental study was conducted to analyze the performance of the implemented multi-threaded hashing algorithm. The study aimed to measure the speedup achieved by increasing the number of threads for various input files. The report can be found in the same directory as the project, and provides detailed findings and analysis. The report follows a structured approach, including the following sections:

                  - Experimental Setup: It describes the hardware used for the experiments, including the machine specifications 
                    and the number of CPUs. The 'lscpu' command was used to gather relevant information about the CPU architecture 
                    and core count.

                  - Experimental Procedure: The report explains the procedure followed for each input file, which involved measuring 
                    the time taken to compute the hash value for different thread configurations, ranging from 1 to 256 threads.

                  - Graphs and Speedup Calculation: The report presents two graphs: one with time on the y-axis and the number of threads 
                    on the x-axis for each input file, and another with speedup on the y-axis and the number of threads on the x-axis. The 
                    speedup was calculated using the formula: speedup = (time taken for single thread) / (time taken for n threads).

                  - Observations and Conclusion: The report provides observations on the behavior of the computation time as the number of threads 
                    increases. It discusses whether the time taken always decreases, analyzes the achieved speedup, and examines whether the speedup 
                    is proportional to the number of threads increased. Additionally, it highlights any interesting findings and concludes with a summary 
                    of the study's outcomes.   
                    
## Server: 

  ### This project involves implementing a database server (dbserver) and a client (dbclient) application using sockets in C that can handle multiple clients concurrently using threads.

   **Database Server (dbserver):** The server application that initializes a socket to listen for connection requests from clients. 
   Upon accepting a connection, it creates a new thread (handler) to handle the client's requests. 
   The server continuously waits for new requests, and a handler thread terminates only when its client closes the connection.

   **Handler Thread:** Each handler thread processes requests from a dbclient and sends appropriate responses. 
   Requests can be either "PUT" or "GET" messages. For "PUT" messages, the handler appends the data record to the database file. 
   For "GET" messages, the handler searches the database for a matching record and sends it back to the client if found.

   **Database Client (dbclient):** The client application that establishes a connection with the server. It prompts the user to choose
   operations such as "PUT", "GET", or "QUIT". For "PUT", the client sends a message with the record's name and ID to the server. 
   For "GET", the client sends a message with the ID to retrieve the corresponding record. The client displays the results received from the server.
     
