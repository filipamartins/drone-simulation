
#ifndef EXTERN_H
#define EXTERN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#include "structs.h"
#include "files.h"
#include "drone_movement.h"
#include "central.h"
#include "linkedlists.h"

#define PIPE_NAME "/tmp/input_pipe"
#define TYPE_DRONE 3

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN int num_warehouses;
EXTERN int num_drones;
EXTERN int freq_abast;
EXTERN int quant_abast;
EXTERN int time_unit;
EXTERN int num_prod;
EXTERN Point coord_max;

EXTERN int shm_id, shm_id_stats;
EXTERN int simulation_pid;
EXTERN int central_pid;
EXTERN int mqid;


EXTERN volatile sig_atomic_t exit_requested;
//THREADS
EXTERN pthread_t supply_thread;
EXTERN pthread_t pipe_thread;

//PTR SHARED MEMORY
EXTERN Warehouse *shm_war;
EXTERN Stats *shm_stats;

//EXTERN Drone *drones;
EXTERN Product *product_types;
EXTERN Point drone_bases[4];

EXTERN FILE *fp2;

//SEMAPHORES
EXTERN sem_t *mutex_war;
EXTERN sem_t *mutex_stats;
EXTERN sem_t *mutex_centr;
EXTERN sem_t *mutex_log_file;

EXTERN ListDrone list_drone;
EXTERN ListOrder list_head;

#endif