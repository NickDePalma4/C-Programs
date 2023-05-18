# Hash-Tree

This project implements a hash tree algorithm using Jenkins One at a Time Hash and multithreading to calculate a 32-bit unsigned int hash value of a user-specified file. The hash value acts as a fingerprint or unique identifier for the file. The program utilizes a binary tree structure to equally distribute blocks of the file among multiple threads, resulting in efficient parallel processing.

## Features

- **Multithreaded Hashing:** The program employs multithreading to divide the input file into multiple blocks, allowing concurrent hash value computation. Each thread is responsible for computing the hash value of a specific range of blocks.

- **Binary Hash Tree:** A complete binary tree of threads is constructed, with each leaf thread assigned a range of consecutive blocks. Interior threads compute the hash value of their assigned blocks by combining the hash values of their child threads.

- **Block Assignment to Threads:** The program ensures that each thread is assigned a specific range of blocks based on its thread number. This assignment enables efficient distribution of the hashing workload among the threads.

## Usage

To run the hash-tree program, follow these steps:

1. Compile the program by executing the following command:
           
        gcc htree.c -o htree -Wall -Werror -std=gnu99 -pthread
        

2. Run the program by providing the path to the input file and the number of threads as command-line arguments:

        ./htree <filename> <num_threads>
        

    *Replace `<input_file>` with the path to the file you want to calculate the hash value for. Specify `<num_threads>` as the desired number of threads for parallel processing.*

3. Wait for the program to complete execution. Once finished, the program will display the calculated hash value of the input file.

## Report

A comprehensive experimental study has been conducted to analyze the performance of the implemented multi-threaded hashing algorithm. The study aims to measure the speedup achieved by increasing the number of threads for various input files. The report, located in the same directory as the source code, provides detailed findings and analysis.

The report consists of the following sections:

- **Experimental Setup:** Describes the hardware used for the experiments, including machine specifications and the number of CPUs.

- **Experimental Procedure:** Explains the procedure followed for each input file, including measuring the time taken to compute the hash value for different thread configurations.

- **Graphs and Speedup Calculation:** Presents graphs showcasing the relationship between the number of threads and the computation time. Additionally, calculates the speedup achieved using the formula: `speedup = (time taken for single thread) / (time taken for n threads)`.

- **Observations and Conclusion:** Provides observations on the behavior of computation time as the number of threads increases. Analyzes the achieved speedup and discusses whether it is proportional to the number of threads. The section also highlights any interesting findings and concludes with a summary of the study's outcomes.

Please refer to the report for detailed insights into the performance of the hash-tree program.

## Dependencies

The hash-tree program has the following dependencies:

- C compiler (e.g., GCC)
- POSIX Threads (pthreads)

Ensure that you have a compatible C compiler installed, along with the necessary POSIX Threads library.

## Disclaimer

Please note that the source code provided in this repository is available for viewing purposes only. It was originally developed as a project for a class, and while you are welcome to examine the code and learn from it, it should not be reused or submitted for class assignments or any other academic or commercial purposes. Using this code for such purposes would violate academic integrity policies and may have consequences.

## License

The code in this repository is provided under the MIT License.

### MIT License

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to view and run the code contained in this repository, subject to the following conditions:

1. The code may be used for personal, educational, or evaluation purposes.
2. The code may not be used for commercial purposes without explicit written permission.
3. The code may not be redistributed or modified without the author's permission.
4. Any derivative work based on this code must clearly attribute the original author.

THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For any questions or inquiries regarding the usage of this code, please contact the author directly.


## Viewing the Source Code

To view the source code, navigate to the appropriate directory in this repository. The code is provided in files with the ".c" extension. You can open these files using any text editor or an Integrated Development Environment (IDE) of your choice. Feel free to explore the code, understand the implementation, and learn from it.

However, remember that it is important to develop your own solutions for class assignments and projects. Reusing or submitting this code as your own would be a violation of academic integrity and can have serious consequences. Use this code responsibly and solely for educational purposes.


## Acknowledgments

This project makes use of the Jenkins One at a Time Hash algorithm, originally developed by Bob Jenkins. Special thanks to Bob Jenkins for his contributions to the field of hash functions.

If you encounter any issues or have suggestions for improvements, please feel free to submit an issue or pull request on the project's GitHub repository.

Happy hashing!
