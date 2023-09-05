#include"elevator.h"
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

//int assigned_elevator = 0; 
//pthread_mutex_t elevator_lock;
//int elevator_floor = 0;
//int elevator_direction = 1;

//int wait_at = -1;

//int is_elevator_ready = 0;
//int is_passenger_ready = 0;
//pthread_cond_t elevator_signal = PTHREAD_COND_INITIALIZER;
// pthread_cond_t passenger_signal = PTHREAD_COND_INITIALIZER;
// pthread_mutex_t passenger_lock;

static struct Elevator {
    int elevator_floor;
    int elevator_direction;
    pthread_mutex_t elevator_lock;
    pthread_mutex_t passenger_lock;

    int is_elevator_ready;
    int is_passenger_ready;

    pthread_cond_t elevator_signal;
    pthread_cond_t passenger_signal;

    int wait_at;
	
} elevators[ELEVATORS];


void scheduler_init() {
    for (int i = 0; i < ELEVATORS; i++) {
        pthread_mutex_init(&elevators[i].elevator_lock,0);
        pthread_mutex_lock(&elevators[i].elevator_lock);
        pthread_mutex_init(&elevators[i].passenger_lock,0);

        elevators[i].elevator_floor = 0;
        elevators[i].elevator_direction = 1;
        elevators[i].is_elevator_ready = 0;

        elevators[i].is_passenger_ready = 0;

        elevators[i].wait_at = -1;

        elevators[i].elevator_signal = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
        elevators[i].passenger_signal = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    }
    
}


/********************************************************************************************************************
Method2: Implementation using conditions

In this method we replace the busy polling with conditions.
TODO 1a: define USECONDITION to begin using this method. 
********************************************************************************************************************/
void passenger_request(int passenger, int from_floor, int to_floor, void (*enter)(int, int), void(*exit)(int, int)) {
    int assigned_elevator = passenger % 4; 
    // assigned_elevator++;
    // if(assigned_elevator > 3){
    //     assigned_elevator = 0;
    // }
    pthread_mutex_lock(&elevators[assigned_elevator].passenger_lock);

    // *************
    // ENTER THE LIFT
    // *************
    pthread_mutex_lock(&elevators[assigned_elevator].elevator_lock);

    // TODO 1b: Submit request to elevator by setting 'wait_at'.
    // 'wait_at' is used to store the next floor at which the elevator should stop
    // at. Replace 0 below with the correct value for this request.
    elevators[assigned_elevator].wait_at = from_floor;

    // TODO 1c:
    // After setting the 'wait_at' variable, we wait for the elevator to arrive at the
    // floor and inform us once it's ready.
    //
    // The passenger thread should wait for the 'is_elevator_ready' variable to be set
    // and the elevator sets the variable and signals this thread
    //
    // Fill in the while loop below to wait for the condition to be met.
    while(!elevators[assigned_elevator].is_elevator_ready) {
        pthread_cond_wait(&elevators[assigned_elevator].elevator_signal, &elevators[assigned_elevator].elevator_lock);
    }
    elevators[assigned_elevator].is_elevator_ready = 0;

    // enter the lift
    enter(passenger, assigned_elevator);

    // TODO 1e:
    // We've now entered the elevator. It's now the passenger's turn to notify the elevator
    // that they have entered. As in the previous TODOs, use pthread_cond_signal to notify
    // the elevator thread that the passenger is ready.
    /* FILL IN HERE */
    pthread_mutex_unlock(&elevators[assigned_elevator].elevator_lock);
    elevators[assigned_elevator].is_passenger_ready = 1;
    pthread_cond_signal(&elevators[assigned_elevator].passenger_signal);


    // *************
    // EXIT THE LIFT
    // *************
    elevators[assigned_elevator].wait_at = to_floor;

    while(!elevators[assigned_elevator].is_elevator_ready) {
        pthread_cond_wait(&elevators[assigned_elevator].elevator_signal, &elevators[assigned_elevator].elevator_lock);
    }
    elevators[assigned_elevator].is_elevator_ready = 0;

    // exit the lift
    exit(passenger, assigned_elevator);

    pthread_mutex_unlock(&elevators[assigned_elevator].elevator_lock);
    elevators[assigned_elevator].is_passenger_ready = 1;
    pthread_mutex_unlock(&elevators[assigned_elevator].passenger_lock); // belonged at the bottom of the section 

    pthread_cond_signal(&elevators[assigned_elevator].passenger_signal);
}

void elevator_ready(int elevator, int at_floor, void(*move_direction)(int, int), void(*door_open)(int), void(*door_close)(int)) {
    //if(elevator == 0) {
        if(at_floor == FLOORS-1)
            elevators[elevator].elevator_direction = -1;
        if(at_floor == 0)  
            elevators[elevator].elevator_direction = 1;

        //door_open(elevator);

        if (elevators[elevator].wait_at == at_floor) {
            // There is a passenger waiting at this floor.
            // We set wait_at back to its default value so it can be used later.
            elevators[elevator].wait_at = -1;
            door_open(elevator); // temp

            // TODO 1d:
            // The passenger thread waiting for the elevator will be waiting for the elevator
            // to be ready so that it can step into the elevator. The door is open now so the elevator
            // is ready! Time to signal the other thread.
            //
            // Signal the passenger thread using 'is_elevator_ready', 'elevator_signal' and pthread_cond_signal
            /* FILL IN CODE HERE */
            elevators[elevator].is_elevator_ready = 1;
            pthread_cond_signal(&elevators[elevator].elevator_signal);

            // TODO 1f:
            // It's now the passengers turn to notify the elevator thread once it's entered.
            // Wait for the 'is_passenger_ready' condition to be ready. Use the 'passenger_signal'
            // variable along with pthread_cond_wait to wait.
            while(!elevators[elevator].is_passenger_ready) {
               pthread_cond_wait(&elevators[elevator].passenger_signal, &elevators[elevator].elevator_lock);
            }
            elevators[elevator].is_passenger_ready = 0;

            door_close(elevator); // temp

        } else {
            // There's no passenger waiting at this floor
            // Let's release the lock which will allow other passenger threads to run
            // and check the state of the elevator
            pthread_mutex_unlock(&elevators[elevator].elevator_lock);
            usleep(1);
            pthread_mutex_lock(&elevators[elevator].elevator_lock);
        }

        //door_close(elevator);
          
        move_direction(elevator, elevators[elevator].elevator_direction);
        elevators[elevator].elevator_floor = at_floor + elevators[elevator].elevator_direction;
    //}
}