#pragma once
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//Shared memory storages a struct
typedef struct data
{ 
    int now_reader_counter;
    int reader_counter;
    int write_counter;
    int rw_sem_id;
    int r_sem_id;
} Entry;

//Create shared memory and connect
int shmcreate(key_t, int );
//Deleting shared memory segment
int shmdelete(int);

//Getting a pointer to the segment
Entry* shmattach(int );
//Detaching segment
int shmdetach(Entry* );

//Getting a pointer to 2nd segment
int* shm2attach(int );
//Detaching 2nd segment
int shm2detach(int* );