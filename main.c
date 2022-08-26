#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Threads
pthread_t *floors;
pthread_t stopSim;

// Global Variables
int timeInterval;
int total_time;
int currentCapacity;
int currentFloor = 0;
int timing;
int numOfFloors;
int totalNumOfCalls;
int openCalls = 0;
int request_calls_key = 0;
int stop_key = 0;
int countDone = 0;
int sleeper = 0;
int fullCount = 0;
int openingGoingUpCounter = 0;
int callTimer = 0;
int done = 0;
float waitingTimeAVGpeople = 0;
float turnAroundAVGpeople = 0;
float waitingTimeAVGprocess = 0;
float turnAroundTimeAVGprocess = 0;


// Calculations
int thisTurnaround = 0;
int totalTurnaround = 0;
int totalWait = 0;
int totalNumberOfProcesses = 0;
int totalServiced = 0;

// Shared Data Structures
int **past;
int **current;
int **track;
int *callsRecieved;

// File
FILE *f_create;

// Mutex locks
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

// Function Initializations
void printCalls();

void *stop(void *);

// Create data structures
void init() {

    // Keeps track of floor from / floor at issued
    past = malloc(sizeof(int *) * numOfFloors + 1);
    for (int i = 0; i < numOfFloors + 1; i++) {
        past[i] = malloc(sizeof(int) * 8);
    }
    int a, p;
    past[0][0] = -1;
    past[0][1] = -1;
    past[0][2] = -1;
    past[0][3] = -1;
    past[0][4] = -1;
    past[0][5] = -1;
    past[0][6] = -1;
    past[0][7] = -1;
    for (a = 1; a < numOfFloors + 1; a++) {
        for (p = 0; p < 8; p++) {
            if (p == 2) {
                past[a][p] = -1;
            } else if (p == 5) {
                past[a][p] = -1;
            } else {
                past[a][p] = 0;
            }
        }
    }
    // ***



    // Keeps track of current floors options
    current = malloc(sizeof(int *) * numOfFloors + 1);
    for (int i = 0; i < numOfFloors + 1; i++) {
        current[i] = malloc(sizeof(int) * 5);
    }
    current[0][0] = -1;
    current[0][1] = -1;
    current[0][2] = -1;
    current[0][3] = -1;
    current[0][4] = -1;
    for (a = 1; a < numOfFloors + 1; a++) {
        for (p = 0; p < 5; p++) {
            if (p == 2) {
                current[a][p] = -1;
            } else {
                current[a][p] = 0;
            }
        }
    }
    // ***


    // Keeps track of current floors times
    track = malloc(sizeof(int *) * numOfFloors + 1);
    for (int i = 0; i < numOfFloors + 1; i++) {
        track[i] = malloc(sizeof(int) * 5);
    }
    track[0][0] = -1;
    track[0][1] = -1;
    track[0][2] = -1;
    track[0][3] = -1;
    track[0][4] = -1;
    for (a = 1; a < numOfFloors + 1; a++) {
        for (p = 0; p < 5; p++) {
            if (p == 2) {
                track[a][p] = -1;
            } else {
                track[a][p] = 0;
            }
        }
    }
    // ***

    // Keeps track of current calls recieved
    callsRecieved = malloc(sizeof(int *) * numOfFloors + 1);
    callsRecieved[0] = 0;
    for (a = 1; a < numOfFloors + 1; a++) {
        callsRecieved[a] = 0;
    }


}

// This function handles the issue requesting and accessing and writing into the shared
// data structures.
void issue_request(int issueAtFloor) {
    pthread_mutex_lock(&lock);

    // If both slots are filled return to thread function.
    if (past[issueAtFloor][0] != 0 && past[issueAtFloor][1] != 0) {
        pthread_mutex_unlock(&lock);
        return;
    }
    // If it is the max number of floors and down slot is filled return (you cant go up any further)
    if (issueAtFloor == numOfFloors && past[issueAtFloor][0] != 0 && past[issueAtFloor][1] == 0) {
        pthread_mutex_unlock(&lock);
        return;
    }
    // If lowest floor number has its up call filled return.
    if (issueAtFloor == 1 && past[issueAtFloor][1] != 0 && past[issueAtFloor][0] == 0) {
        pthread_mutex_unlock(&lock);
        return;
    }

    // Initalize variables needed
    int floor_call_at, floor_call_to, num_passengers;
    int found = 0;
    int checkDirection = 0;
    int slot;
    int pass = 0;

    // Alg to sort based on the current filled data struct
    if (issueAtFloor == numOfFloors && past[issueAtFloor][0] == 0 && past[issueAtFloor][1] == 0) {
        slot = 0;
    } else if (issueAtFloor == 1 && past[issueAtFloor][1] == 0 && past[issueAtFloor][0] == 0) {
        slot = 1;
    } else if (past[issueAtFloor][0] == 0 && past[issueAtFloor][1] == 0) {
        srand(time(NULL) * issueAtFloor + issueAtFloor);
        slot = (rand() % 2);
    } else if (past[issueAtFloor][0] != 0 && past[issueAtFloor][1] == 0) {
        slot = 1;
    } else if (past[issueAtFloor][0] == 0 && past[issueAtFloor][1] != 0) {
        slot = 0;
    }

    // If slot is found, fill it using random number generator
    if (slot == 0) {
        srand(time(NULL) * issueAtFloor + issueAtFloor);
        while (pass == 0) {
            floor_call_to = (rand() % issueAtFloor);
            if (floor_call_to > 0) {
                pass = 1;
            }
        }
    }
    if (slot == 1) {
        srand(time(NULL) * issueAtFloor + issueAtFloor);
        while (pass == 0) {
            floor_call_to = (rand() % numOfFloors + 1);
            if (floor_call_to > issueAtFloor) {
                pass = 1;
            }
        }
    }

    // Take argument given from thread.
    floor_call_at = issueAtFloor;

    // Random number generator for number of passengers
    srand(time(NULL) * issueAtFloor + issueAtFloor);
    num_passengers = (rand() % 5) + 1;

    // Fill based on alg above
    past[floor_call_at][slot] = floor_call_to; // The number inserted is the destination of the current floor
    past[floor_call_at][slot + 3] = num_passengers; // This is the number of passengers for the given call
    track[floor_call_at][slot] = timing; // This is the current time that this is happening
    pthread_mutex_lock(&count_lock);
    callsRecieved[floor_call_at] = 1; // This is keeping track of the calls per floor recieved
    pthread_mutex_unlock(&count_lock);


    // Unlock and return
    pthread_mutex_unlock(&lock);
    return;


}

// This is the thread per floors function
void *run_floors(void *arg) {

    // Takes argument from thread (floor number)
    int threadFloorNumber = (*(int *) arg) + 1;
    int issueAtFloor = threadFloorNumber;

    // Endless loop until simulation has stopped fully.
    while (stop_key == 0) {


        // Random number generator to see if this specific floor will generate a call.
        srand(time(NULL) * issueAtFloor);
        int randomNum = (rand() % 2);
        if (randomNum == 1) {
            issue_request(issueAtFloor);
        }


        // Puts each thread into a while loop until variable is changed
        while (1) {
            if (sleeper == 1) {
                break;
            }
        }
        sleep(1);

    }

    pthread_exit(NULL);
}

// Simulation run function
void run() {


    // Initialize the threads
    int i, j;
    floors = (pthread_t *) malloc(sizeof(pthread_t) * numOfFloors);
    int tmp[numOfFloors];


    // Create a for loop to offically create the threads needed
    for (j = 0; j < numOfFloors; j++) {
        tmp[j] = j;
        if ((i = pthread_create(&floors[j], NULL, run_floors, (void *) &tmp[j])) != 0) {
            printf("thread creation failed. %d\n", j);
        }
    }

    // Initialize call_requests
    // sleeper = 1 means it starts
    // always set countDown to 0 and sleep 1 to allow time for all threads to start correctly
    sleep(1);
    sleeper = 1;
    countDone = 0;
    sleep(1); // leave time for all threads to leave waiting loop
    sleeper = 0;


    // Check if there is no going up from immediate call request.
    pthread_mutex_lock(&lock);
    for (int i = 1; i < numOfFloors + 1; i++) {
        if (past[i][1] > 0) {
            openingGoingUpCounter++;
        }
    }
    if (timing == 0 && openingGoingUpCounter == 0) {
        past[1][1] = numOfFloors;
        past[1][4] = 1;
    }
    printCalls();
    pthread_mutex_unlock(&lock);


    int direction = 1;
    currentFloor = 1;

    // Create the stop thread, that checks when the time is done and stops the simulation.
    pthread_create(&stopSim, NULL, stop, (void *) NULL);

    // The loop simulation keeps track of each floor.
    pthread_mutex_lock(&lock);
    while (done == 0) {


        i = currentFloor;

        // The following algorithm checks if the current floor has calls that are already being serviced
        int currentFloorBeingHandleUpCheck = 0;
        int currentFloorBeingHandleDownCheck = 0;
        int subtractingCapacity = 0;
        int sleepDropoffTimeCheck = 0;
        for (int j = 1; j < numOfFloors + 1; j++) {
            if (past[j][1] == i) {
                if (past[j][7] == 1) {
                    currentFloorBeingHandleUpCheck++;
                    current[j][1] = past[j][4];
                }
            }
            if (past[j][0] == i) {
                if (past[j][6] == 1) {
                    currentFloorBeingHandleDownCheck++;
                    current[j][0] = past[j][3];
                }
            }
        }

        // If the direction is up and there is a call that can be dropped off at this location then enter.
        if (direction == 1 && currentFloorBeingHandleUpCheck > 0) { // direction up dropoff
            for (int k = 1; k < numOfFloors + 1; k++) {
                if (current[k][1] > 0) {
                    subtractingCapacity = subtractingCapacity + current[k][1];
                    // found dropoff space (k is place in past that can be set back to 0
                    current[k][1] = 0;

                    // - reset call
                    past[k][1] = 0;

                    // - reset number of passengers + add to sum
                    totalServiced = totalServiced + past[k][4];
                    past[k][4] = 0;

                    // - reset picked up/confirmation number
                    past[k][7] = 0;

                    // - reset time added + add to waiting time sum and turnaround
                    totalWait = totalWait + (track[k][4] - track[k][1]);
                    totalTurnaround = totalTurnaround + ((timing) - track[k][1]);
                    track[k][1] = 0;
                    track[k][4] = 0;

                    totalNumberOfProcesses = totalNumberOfProcesses + 1;
                }
            }
            sleep(2);
            printf("Time %d: The elevator dropped off %d passengers at Floor %d.\n", timing, subtractingCapacity,
                   currentFloor);
            fprintf(f_create, "Time %d: The elevator dropped off %d passengers at Floor %d.\n", timing,
                    subtractingCapacity, currentFloor);
            timing = timing + 2;
            if (callTimer + timeInterval < timing) {
                pthread_mutex_unlock(&lock);
                sleep(1);
                sleeper = 1;
                countDone = 0;
                sleep(1); // leave time for all threads to leave waiting loop
                sleeper = 0;
                callTimer = callTimer + timeInterval;
                printCalls();
                pthread_mutex_lock(&lock);
            }

            sleepDropoffTimeCheck++;
            currentCapacity = currentCapacity - subtractingCapacity;
        }


        if (direction == 0 && currentFloorBeingHandleDownCheck > 0) { // direction down dropoff
            for (int k = 1; k < numOfFloors + 1; k++) {
                if (current[k][0] > 0) {
                    subtractingCapacity = subtractingCapacity + current[k][0];
                    // found dropoff space (k is place in past that can be set back to 0
                    current[k][0] = 0;

                    // - reset call
                    past[k][0] = 0;

                    // - reset number of passengers + add to sum
                    totalServiced = totalServiced + past[k][3];
                    past[k][3] = 0;

                    // - reset picked up/confirmation number
                    past[k][6] = 0;

                    // - reset time added + add to waiting time sum and turnaround
                    totalWait = totalWait + (track[k][3] - track[k][0]);
                    totalTurnaround = totalTurnaround + ((timing) - track[k][0]);
                    track[k][0] = 0;
                    track[k][3] = 0;

                    totalNumberOfProcesses = totalNumberOfProcesses + 1;
                }
            }
            sleep(2);
            printf("Time %d: The elevator dropped off %d passengers at Floor %d.\n", timing, subtractingCapacity,
                   currentFloor);
            fprintf(f_create, "Time %d: The elevator dropped off %d passengers at Floor %d.\n", timing,
                    subtractingCapacity, currentFloor);
            timing = timing + 2;
            if (callTimer + timeInterval < timing) {
                pthread_mutex_unlock(&lock);
                sleep(1);
                sleeper = 1;
                countDone = 0;
                sleep(1); // leave time for all threads to leave waiting loop
                sleeper = 0;
                callTimer = callTimer + timeInterval;
                printCalls();
                pthread_mutex_lock(&lock);
            }

            sleepDropoffTimeCheck++;
            currentCapacity = currentCapacity - subtractingCapacity;
        }

        // Check to see if direction needs to switch. ****
        int lowestSpot = 0;
        int highestSpot = 0;

        for (int q = 1; q < numOfFloors + 1; q++) {
            if (past[q][0] > 0 || past[q][1] > 0) {
                lowestSpot = q;
                break;
            }
        }
        for (int l = numOfFloors; l > 0; l--) {
            if (past[l][0] > 0 || past[l][1] > 0) {
                highestSpot = l;
                break;
            }
        }
        if (direction == 1) {
            if (i == highestSpot) {
                direction = 0;
            }
        } else if (direction == 0) {
            if (i == lowestSpot) {
                direction = 1;
            }
        }

        // Pickup algorithm
        if (direction == 1 && past[i][1] > 0 &&
            ((past[i][4] + currentCapacity) <= 10) && past[i][7] == 0) { //direction up pickup
            past[i][7] = 1;
            track[i][4] = timing;
            currentCapacity = past[i][4] + currentCapacity;
            if (sleepDropoffTimeCheck == 0) {
                sleep(2);
            }
            printf("Time %d: The elevator picked up %d passengers at Floor %d.\n", timing, past[i][4], currentFloor);
            fprintf(f_create, "Time %d: The elevator picked up %d passengers at Floor %d.\n", timing, past[i][4],
                    currentFloor);
            if (sleepDropoffTimeCheck == 0) {
                timing = timing + 2;
                if (callTimer + timeInterval < timing) {
                    pthread_mutex_unlock(&lock);
                    sleep(1);
                    sleeper = 1;
                    countDone = 0;
                    sleep(1); // leave time for all threads to leave waiting loop
                    sleeper = 0;
                    callTimer = callTimer + timeInterval;
                    printCalls();
                    pthread_mutex_lock(&lock);
                }
            }
        }

        // Pickup for going down.
        if (direction == 0 && past[i][0] > 0 &&
            ((past[i][3] + currentCapacity)) <= 10 && past[i][6] == 0) { //direction down pickup
            past[i][6] = 1;
            track[i][3] = timing;
            currentCapacity = past[i][3] + currentCapacity;
            if (sleepDropoffTimeCheck == 0) {
                sleep(2);
            }
            printf("Time %d: The elevator picked up %d passengers at Floor %d.\n", timing, past[i][3], currentFloor);
            fprintf(f_create, "Time %d: The elevator picked up %d passengers at Floor %d.\n", timing, past[i][3],
                    currentFloor);
            if (sleepDropoffTimeCheck == 0) {
                timing = timing + 2;
                if (callTimer + timeInterval < timing) {
                    pthread_mutex_unlock(&lock);
                    sleep(1);
                    sleeper = 1;
                    countDone = 0;
                    sleep(1); // leave time for all threads to leave waiting loop
                    sleeper = 0;
                    callTimer = callTimer + timeInterval;
                    printCalls();
                    pthread_mutex_lock(&lock);
                }

            }
        }



        // Change floor number
        if (direction == 1 && done == 0) {
            currentFloor++;

            if (callTimer + timeInterval < timing) {
                pthread_mutex_unlock(&lock);
                sleep(1);
                sleeper = 1;
                countDone = 0;
                sleep(1); // leave time for all threads to leave waiting loop
                sleeper = 0;
                callTimer = callTimer + timeInterval;
                printCalls();
                pthread_mutex_lock(&lock);
            }

            timing = timing + timeInterval;
            sleep(3);
            printf("Going up...\n");
            fprintf(f_create, "Going up...\n");
            sleep(5);

            pthread_mutex_unlock(&lock);
            sleep(1);
            sleeper = 1;
            countDone = 0;
            sleep(1); // leave time for all threads to leave waiting loop
            sleeper = 0;
            callTimer = callTimer + timeInterval;
            printCalls();
            pthread_mutex_lock(&lock);


        } else if (direction == 0 && done == 0) {
            currentFloor--;

            if (callTimer + timeInterval < timing) {
                pthread_mutex_unlock(&lock);
                sleep(1);
                sleeper = 1;
                countDone = 0;
                sleep(1); // leave time for all threads to leave waiting loop
                sleeper = 0;
                callTimer = callTimer + timeInterval;
                printCalls();
                pthread_mutex_lock(&lock);
            }

            timing = timing + timeInterval;
            sleep(3);
            printf("Going down...\n");
            fprintf(f_create, "Going down...\n");
            sleep(5);


            pthread_mutex_unlock(&lock);
            sleep(1);
            sleeper = 1;
            countDone = 0;
            sleep(1); // leave time for all threads to leave waiting loop
            sleeper = 0;
            callTimer = callTimer + timeInterval;
            printCalls();
            pthread_mutex_lock(&lock);


        }


    }


    return;


}
// This function stops the simulation
void *stop(void *arg) {
    // Stops the simulation, outputting counters etc

    while (1) {
        if (timing >= total_time) {
            stop_key = 1;
            done = 1;
            for (int j = 0; j < numOfFloors; j++) {
                pthread_join(floors[j], NULL);
            }
            break;
        }
    }


    if (totalNumberOfProcesses == 0) {
    } else {
        waitingTimeAVGpeople = (float) totalWait / (float) totalServiced;
        turnAroundAVGpeople = (float) totalTurnaround / (float) totalServiced;

        waitingTimeAVGprocess = (float) totalWait / (float) totalNumberOfProcesses;
        turnAroundTimeAVGprocess = (float) totalTurnaround / (float) totalNumberOfProcesses;
    }

    // Output the counters.
    printf("\nFinal Output Counters:\n");
    printf("The number of passengers that the elevator has serviced: %d\n", totalServiced);
    printf("Average wait time for all the passengers serviced (dropped off): %.2f\n", waitingTimeAVGpeople);
    printf("Average turnaround time for all the passengers serviced (dropped off): %.2f\n", turnAroundAVGpeople);
    printf("Average wait time for all the calls that were requested and finished: %.2f\n", waitingTimeAVGprocess);
    printf("Average turnaround time for all the calls that were requested and finished: %.2f\n",
           turnAroundTimeAVGprocess);


    fprintf(f_create, "\nFinal Output Counters:\n");
    fprintf(f_create, "The number of passengers that the elevator has serviced: %d\n", totalServiced);
    fprintf(f_create, "Average wait time for all the passengers serviced: %.2f\n", waitingTimeAVGpeople);
    fprintf(f_create, "Average turnaround time for all the passengers serviced: %.2f\n", turnAroundAVGpeople);
    fprintf(f_create, "Average wait time for all the calls that were requested and finished: %.2f\n",
            waitingTimeAVGprocess);
    fprintf(f_create, "Average turnaround time for all the calls that were requested and finished: %.2f\n",
            turnAroundTimeAVGprocess);

    pthread_exit(NULL);

}

// Print the calls after each call request.
void printCalls() {

    // Print array
    int a, p;

    pthread_mutex_lock(&count_lock);

    int count = 0;
    int callCounts = 0;

    for (a = 1; a < numOfFloors + 1; a++) {
        if (callsRecieved[a] != 0) {
            callCounts++;
        }
    }
    if (callCounts > 0) {
        printf("Time %d: Calls received at floor/s ", callTimer);
        fprintf(f_create, "Time %d: Calls received at floor/s ", callTimer);
    }
    // Print array
    for (a = 1; a < numOfFloors + 1; a++) {
        if (callsRecieved[a] != 0 && count == 0) {
            printf("%d", a);
            fprintf(f_create, "%d", a);
            count++;
        } else if (callsRecieved[a] != 0 && count != 0) {
            printf(", %d", a);
            fprintf(f_create, ", %d", a);
            count++;
        }

    }
    if (count > 0) {
        printf(".\n");
        fprintf(f_create, ".\n");
    } else {
        printf("Time %d: No calls received.\n", callTimer);
        fprintf(f_create, "Time %d: No calls received.\n", callTimer);
    }


    for (a = 1; a < numOfFloors + 1; a++) {
        callsRecieved[a] = 0;
    }

    pthread_mutex_unlock(&count_lock);


}

int main(int argc, char *argv[]) {

    // Command Line Arguments
    if (argc != 4) {
        printf("usage: ./executable [num_floors] [time_interval] [total_time]\n");
        exit(1);
    }

    // Conversions needed
    numOfFloors = atoi(argv[1]);
    timeInterval = atoi(argv[2]);
    total_time = atoi(argv[3]);

    // Initalize a few different variables needed.
    currentCapacity = 0;
    timing = 0;
    currentFloor = 1;
    totalNumOfCalls = (numOfFloors * 2) - 2;

    // Open the file in writing mode.
    f_create = fopen("elevator.log", "w");

    // This function creates all the data structures necessary.
    init();

    // Run the simulation.
    run();

    // Join the stopping thread. Ending the simulation.
    pthread_join(stopSim, NULL);

}




