
# Matrix Multiplier Program

by Bardia Parmoun

Released on: 18/02/2021

## DESCRIPTION
- This program is used to simulate the behaviour of a kernel.
- It takes a list of processes and following the behaviour that is specificied in the following state diagram
<p align="center">
<img src="images/statediagrams.JPG" />
</p>
  
- Each process starts from new and is then moved to ready which then moves to running and waits for an I/O which can later move to waiting and get back to the queue. After a process is done executing it goes to terminated.

## RUNNING THE PROGRAM
1. To run the program first run the makefile to compile the executable file by typing:
```shell
$ gcc kernel_simulator.c -o kernel_simulator
```
2. After that an executable with the name of kernel_simulator has been generated which can then be used with the test cases:
```shell
$ ./kernel_simulator test1.txt out1.text
```
- Note that the testcases are located in the testcases folder and you need to make sure the file and the test file are located in the same folder. 

- Some examples of the outputs are in the sample outputs folder
## USING THE PROGRAM
To run the program first a testcase must be provided with a list of processes such as the following example:
```
1111 5 50 20 10
2222 10 50 20 10
```
- First column: the process id
- Second column: the arrival time of the process
- Third column: the I/O frequency
- Fourth column: the I/O duration

After that the program generates an output file which looks like the following: 
```
TIME      PID     PREVIOUS    CURRENT 
  5       1111        NEW      READY
  5       1111      READY    RUNNING
 10       2222        NEW      READY
 26       1111    RUNNING    WAITING
 26       2222      READY    RUNNING
 35       1111    WAITING      READY
 47       2222    RUNNING    WAITING
 48       1111      READY    RUNNING
 56       2222    WAITING      READY
 68       1111    RUNNING    WAITING
 68       2222      READY    RUNNING
 77       1111    WAITING      READY
 88       2222    RUNNING    WAITING
 89       1111      READY    RUNNING
 97       2222    WAITING      READY
 99       1111    RUNNING TERMINATED
 99       2222      READY    RUNNING
109       2222    RUNNING TERMINATED
```
## CREDITS
Author: Bardia Parmoun

Copyright © 2021 Bardia Parmoun. All rights reserved
