#include <stdlib.h>
#include<stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define LINESIZE 256 
#define NUMLINES 10 

typedef struct pcb {
    int pid;
    int arrivalTime;
    int totalCPUTime;
    int ioFreq;
    int ioDur;
    
    int timeRemain; //keeps track of the remaining time of the cpu
    int ioRemain; //keeps track of the remaining time of I/O
    int ioFreqCounter; //keeps track of the time for the next I/O

    int oldState;
    int currentState;

    struct pcb *next;
} pcb_t;

/*
Loads the values of the test file into an array of lines
*/
int readFile (FILE* inputf, char file[NUMLINES][LINESIZE]){
    int i =0;
    
    while(fgets(file[i], LINESIZE, inputf)) 
    {
        file[i][strlen(file[i]) - 1] = '\0';
        i++;
    }

    return i;
}

/* 
Loads the values into the array of line to an array of processes
*/
void initializeProcesses(pcb_t processes[], char file[NUMLINES][LINESIZE], int numCommands){
    printf("number of commands %d\n", numCommands);

    for (int i=0; i<numCommands; i++)
    {
        sscanf(file[i] , "%i %i %i %i %i", &processes[i].pid, &processes[i].arrivalTime, &processes[i].totalCPUTime, &processes[i].ioFreq, &processes[i].ioDur);

        processes[i].currentState= 0;
        processes[i].timeRemain = processes[i].totalCPUTime;
        processes[i].ioRemain = processes[i].ioDur;
        processes[i].ioFreqCounter  = 0;
    }
}

/* 
Sorts the processes based on the time they arrive at
*/
void sortProcesses(pcb_t processes[], int numCommands){
    for (int i=0; i<numCommands-1; i++)
    {
        for (int j=i+1; j<numCommands; j++){
            if (processes[i].arrivalTime > processes[j].arrivalTime){
                pcb_t temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
}

/* Process is transferred from the new state to ready*/
void loadProcess(pcb_t processes[], int processIndex){
    processes[processIndex].currentState = 1;
}

/* Process is transferred from the running state to terminate*/
void killProcess(pcb_t processes[], int processIndex){
    processes[processIndex].currentState = 4;
}

/* Process is transferred from the running state to waiting*/
void ioEnter(pcb_t processes[], int processIndex){
    processes[processIndex].currentState = 3;
    processes[processIndex].ioFreqCounter = 0;
    processes[processIndex].ioRemain = processes[processIndex].ioDur;
}

/* Process is transferred from the waiting state to ready*/
void ioLeave(pcb_t processes[], int processIndex){
    processes[processIndex].currentState = 1;
    processes[processIndex].ioRemain = processes[processIndex].ioDur;
}

/* Process is transferred from the ready state to running*/
void runProcess(pcb_t processes[], int processIndex){
    processes[processIndex].currentState = 2;
}

/*Printing the current state of the processes and saving it in a file*/
void displayOutput(int clock, int pid, char state1[], char state2[], FILE* outputf){
    printf("%3d %10d %10s %10s\n",clock,pid,state1, state2);
    fprintf(outputf, "%3d %10d %10s %10s\n",clock,pid,state1, state2);   
}

int main(int argc,char *argv[])
{
    char file[NUMLINES][LINESIZE];

    FILE *inputf = NULL; 
	FILE *outputf = NULL;

   
    inputf = fopen(argv[1], "r");
	outputf = fopen(argv[2], "w");

    int numCommands = readFile(inputf, file);

    pcb_t processes[numCommands];

    initializeProcesses(processes, file, numCommands);

    sortProcesses(processes, numCommands);


    int clock = 0; //the main clock for the system
    int cpuBusy = 0; //makes sure only one cpu is running at a given time 
    int processesDone = 0; //keeps track of all of the finished processes to terminate the program
    
    int waiting = 0; //keeps track of whether a process is already waiting for I/O or not 
    int firstWaiting = -1; //keeps track of whether a process is the first one starting

    //header of the file
    printf("%3s %8s %12s %10s\n","TIME","PID", "PREVIOUS", "CURRENT");
    fprintf(outputf, "%3s %8s %12s %10s\n","TIME","PID", "PREVIOUS", "CURRENT");


    while (processesDone< numCommands){
        // check all the processes
        for (int i=0; i<numCommands; i++){     
            // if there is a new process
            if (clock>=processes[i].arrivalTime && processes[i].currentState ==0){
                loadProcess(processes,i);
                displayOutput(clock,processes[i].pid, "NEW", "READY",outputf);
            }   

            //if the process is running 
            if (processes[i].currentState == 2){

                //if the process finished executing
                if (processes[i].timeRemain ==0){
                    killProcess(processes,i);
                    displayOutput(clock,processes[i].pid, "RUNNING", "TERMINATED", outputf);
                    cpuBusy = 0;
                    processesDone+=1;
                }

                //if the process is ready for I/O
                if (processes[i].ioFreqCounter == processes[i].ioFreq && processes[i].currentState == 2){
                    ioEnter(processes,i);
                    displayOutput(clock,processes[i].pid, "RUNNING", "WAITING", outputf);
                    cpuBusy = 0;
                    if (firstWaiting == -1){
                        firstWaiting = i;
                    }
                }
                processes[i].timeRemain -=1;
                processes[i].ioFreqCounter +=1;
            }

            //if the process is waiting
            if (processes[i].currentState == 3 && (waiting == 0 || firstWaiting == i)){
                processes[i].ioRemain-=1;
                firstWaiting = i;
                waiting = 1;
            }

            //if the process is done with I/O
            if (processes[i].ioRemain ==-1 && processes[i].currentState == 3){
                ioLeave(processes,i);  
                waiting = 0;
                firstWaiting = -1;
                displayOutput(clock,processes[i].pid, "WAITING", "READY", outputf);
            }

            //if the process is ready to run
            if (processes[i].currentState ==1 && cpuBusy==0){
                runProcess(processes,i);
                displayOutput(clock,processes[i].pid, "READY", "RUNNING", outputf);
                cpuBusy = 1;
            }
        }

        clock++;
    }
     
    return 0;
}