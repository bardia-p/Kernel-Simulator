#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define LINESIZE 1024 
#define NUMLINES 20 

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

typedef struct {
    pcb_t *front;  // Points to the node at the head of the queue's linked list. 
    pcb_t *rear;   // Points to the node at the tail of the queue's linked list.
    int size;      // The # of nodes in the queue's linked list. 
} queue_t;


pcb_t *pcb_construct(int processInfo[10]){
        pcb_t* process = malloc(sizeof(pcb_t));

        process->pid = processInfo[0];
        process->arrivalTime = processInfo[1];
        process->totalCPUTime= processInfo[2];
        process->ioFreq = processInfo[3];
        process->ioDur = processInfo[4];
        process->timeRemain = processInfo[5];
        process->ioRemain = processInfo[6];
        process->ioFreqCounter  = processInfo[7];
        process->oldState= processInfo[8];
        process->currentState= processInfo[9];

}

queue_t *queue_construct(void)
{
    queue_t *queue = malloc(sizeof(queue_t));
    // assert(queue != NULL);

    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

/* adds an element to the end of the queue */
void enqueue(queue_t *queue, pcb_t* p)
{
    //assert(queue != NULL);
    if (queue->front == NULL) {
        queue->front = p;
    } else {
        queue->rear->next = p;
    }

    queue->rear = p;
    queue->size += 1;
}

/* removes an element from the front of the queue */
pcb_t* dequeue(queue_t *queue)
{
    // assert(queue != NULL);
    int process[10];
    process[0]= queue->front->pid;
    process[1]= queue->front->arrivalTime;
    process[2]= queue->front->totalCPUTime;
    process[3]= queue->front->ioFreq;
    process[4]= queue->front->ioDur;
    process[5]= queue->front->timeRemain;
    process[6]= queue->front->ioRemain;
    process[7]= queue->front->ioFreqCounter;
    process[8]= queue->front->oldState;
    process[9]= queue->front->currentState;

    pcb_t *node_to_delete = queue->front;
    queue->front = queue->front->next;
    
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    queue->size -= 1;

    pcb_t*node_deleted =pcb_construct(process); 

    return node_deleted;
}

/* Loads the values of the test file into an array of lines */
int readFile (FILE* inputf, char file[NUMLINES][LINESIZE]){
    int i =0;
    
    while(fgets(file[i], LINESIZE, inputf)) 
    {
        file[i][strlen(file[i]) - 1] = '\0';
        i++;
    }

    return i;
}

/* Loads the values into the array of line to an array of processes */
void initializeProcesses(int processes[][10], char file[NUMLINES][LINESIZE], int numCommands){
    printf("number of commands %d\n", numCommands);

    for (int i=0; i<numCommands; i++)
    {
        pcb_t* process = malloc(sizeof(pcb_t));
        sscanf(file[i] , "%d %d %d %d %d", &processes[i][0], &processes[i][1], &processes[i][2], &processes[i][3], &processes[i][4]);
        processes[i][5] = processes[i][2];
        processes[i][6] = processes[i][4];
        processes[i][7]=0;
        processes[i][8]=0;
        processes[i][9] = 0;
    }
}

/* adds all the processes from the array to the new queue */
void addProcessestoNew(queue_t* new_queue, int processes[][10], char file[NUMLINES][LINESIZE], int numCommands){
    for (int i=0; i<numCommands; i++)
    {
        pcb_t* process = pcb_construct(processes[i]);

        enqueue(new_queue,process);
    }
}

/* Sorts the processes based on the time they arrive at */
void sortProcesses(int processes[][10], int numCommands){
    for (int i=0; i<numCommands-1; i++)
    {
        for (int j=i+1; j<numCommands; j++){
            if (processes[i][1] > processes[j][1]){
                for (int k=0; k<10;k++){
                    int temp = processes[i][k];
                    processes[i][k] = processes[j][k];
                    processes[j][k] = temp;
                }     
            }
        }
    }
}

/* Printing the current state of the processes and saving it in a file */
void displayOutput(int clock, int pid, char state1[], char state2[], FILE* outputf){
    printf("%3d %10d %10s %10s\n",clock,pid,state1, state2);
    fprintf(outputf, "%3d %10d %10s %10s\n\n",clock,pid,state1, state2);   
}

int main(int argc,char *argv[])
{
    char file[NUMLINES][LINESIZE];

    FILE *inputf = NULL; 
	FILE *outputf = NULL;

   
    inputf = fopen(argv[1], "r");
	outputf = fopen(argv[2], "w");

    int numCommands = readFile(inputf, file);

    queue_t* new_queue = queue_construct();
    queue_t* ready_queue = queue_construct();
    queue_t* waiting_queue = queue_construct();

    int processes[numCommands][10];

    initializeProcesses(processes, file, numCommands);

    sortProcesses(processes,numCommands);

    addProcessestoNew(new_queue, processes,file, numCommands);
    

    
    int clock = 0; //the main clock for the system
    int cpuBusy = 0; //makes sure only one cpu is running at a given time 


    //header of the file
    printf("%3s %8s %12s %10s\n","TIME","PID", "PREVIOUS", "CURRENT");
    fprintf(outputf, "%3s %8s %12s %10s\n\n","TIME","PID", "PREVIOUS", "CURRENT");

    pcb_t* newProcess;
    pcb_t* runningProcess;
    pcb_t* waitingProcess;

    int terminated = 0;
    int waiting = 0;
    int deleted = 0;
    
    while (terminated< numCommands){
        //goes from new to ready
        if (new_queue->size>0 && clock >= new_queue->front->arrivalTime){
            newProcess = dequeue(new_queue);
            newProcess->oldState = 0;
            newProcess->currentState = 1;
            enqueue(ready_queue, newProcess);
            displayOutput(clock,newProcess->pid, "NEW", "READY",outputf);
        }

        //goes from ready to running
        if (cpuBusy==0){
            if (ready_queue->size >0){
                //goes from ready to running
                runningProcess = dequeue(ready_queue);
                runningProcess->oldState = 1;
                runningProcess->currentState = 2;
                displayOutput(clock,runningProcess->pid, "READY", "RUNNING",outputf);
                cpuBusy = 1;
            }
        }

        //process is waiting
        if (waiting_queue->size>0 && waiting==0){
            waitingProcess = dequeue(waiting_queue);
            waiting = 1;
        }

        if (waiting==1){
            waitingProcess->ioRemain-=1;
            if (waitingProcess->ioRemain==0){
                waiting = 0;
                waitingProcess->oldState = 3;
                waitingProcess->currentState = 2;
                waitingProcess->ioRemain = waitingProcess->ioDur;
                enqueue(ready_queue,waitingProcess);
                displayOutput(clock,waitingProcess->pid, "WAITING", "READY",outputf);
            }
        }

        //process is running
        if (cpuBusy==1){
            runningProcess->ioFreqCounter+=1;

            //process is terminated
            if (runningProcess->timeRemain == 0){
                cpuBusy = 0;
                runningProcess->oldState = 2;
                runningProcess->currentState = 4;
                displayOutput(clock,runningProcess->pid, "RUNNING", "TERMINATED",outputf);
                deleted = 1;
                terminated+=1;
            }


            //goes to waiting
            else if (runningProcess->ioFreqCounter == runningProcess->ioFreq+1){
                cpuBusy = 0;
                runningProcess->oldState = 2;
                runningProcess->currentState = 3;
                runningProcess->ioFreqCounter = 0;
                enqueue(waiting_queue, runningProcess);
                displayOutput(clock,runningProcess->pid, "RUNNING", "WAITING",outputf);
            }

            else{
                runningProcess->timeRemain-=1;
            }

            if (deleted ==1){
                free(runningProcess);
                deleted = 0;
            }
        }
        

        clock++;
    }

    fclose(inputf);
    fclose(outputf);
    
    return 0;
}