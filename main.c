#include <sys/wait.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../headers/shared_memory.h"
#include "../headers/semaphores.h"
    
//Function, which is going to give us a realistic time to wait into the critical section.
double expo_time(double lambda)
{
    double u = rand() / (RAND_MAX+1.0);
    return -log(1-u)/lambda;
}

int main (int argc, char* argv[])
{
    srand(time(NULL));
    int peers, entries, pid, Iteration_Times,status=0;
    float ratio;
    key_t key;

    if(argc != 5){
        printf("Error! You have to insert 4 arguments! \nTry... ./main Num_of_Peers Num_of_Entries Ratio Iteration_Times \n");
        return -1;
    }
    else{
        peers = atoi(argv[1]);
        entries = atoi(argv[2]);
        ratio = atof(argv[3]);
        Iteration_Times = atoi(argv[4]);
        if(peers<=0 || entries<=0 || Iteration_Times<=0)
            printf("Error! You have to insert positive numbers! \n");
        if(ratio<0.0 || ratio>1.0)
            printf("Error! Ratio out of bounds! \n");
    }

    //Create the key...
    if ((key = ftok("source/main.c", 'R')) == -1) //producing a random key for shared memory
    {                
        printf("Error! Cannot create key!   \n");
        exit(1);
    }

    //Print some significant values.
    printf("Number of peers: %d\n", peers);
    printf("Number of entries: %d\n", entries);
    printf("Ratio number: %.2f\n", ratio);
    printf("Number of iterations: %d\n\n", Iteration_Times);
    
    //Creating shared memory...and getting access into it(made up from Entries).
    int size = entries*sizeof(Entry);
    int shm_id = shmcreate(key,size);
    Entry* shm_ptr = shmattach(shm_id);
    //printf("Just created and attached shared memory!\n");
    
    //Creating 2nd shared memory...and getting access into it(made up from integers).
    int size2 = 4*sizeof(int);
    int shm2_id = shmcreate(key,size2);
    int* shm2_ptr = shm2attach(shm2_id);
    //printf("Just created and attached the 2nd shared memory!\n\n");

    //For each entry initialize members with zero, create one semaphore for readers and writers and
    // one semaphore only for readers 
    for(int j=0;j<entries;j++)
    {
        shm_ptr[j].now_reader_counter = 0;
        shm_ptr[j].reader_counter = 0;
        shm_ptr[j].write_counter = 0;
        shm_ptr[j].rw_sem_id = sem_create((key_t)(j+1)*532);
        //printf("Just created a (r-w)-semaphore with id=%d of entry %d\n", shm_ptr[j].rw_sem_id,j);
        shm_ptr[j].r_sem_id = sem_create((key_t)(j+1)*1254);
        //printf("Just created a r-semaphore with id=%d of entry %d\n\n", shm_ptr[j].r_sem_id,j);
    }   
    
    //Initialize the 4 values into 2nd shared memory.
    shm2_ptr[0] = (ceil(ratio*peers*Iteration_Times)); //Available for the moment readers
    shm2_ptr[1] = (peers*Iteration_Times - ceil(ratio*peers*Iteration_Times)); //Available for the moment writers
    shm2_ptr[2] = 0; //Sum of all peer's reads.
    shm2_ptr[3] = 0; //Sum of all peer's writes.
    
    //Creating peers child processes 
    for(int i=0; i<peers; i++)  
    {                     
        pid = fork();
        if(pid==-1)
        {
            printf("Function fork() failed!\n");
            return -1;
        }
        else if(pid==0) //If it is a child, don't fork()
            break;
    }
    
    //Initialization of some useful variables.
    int isreader; // If it's zero then he's a reader, otherwise (if it's 1) he's a writer.
	int peer_reads = 0; //Sum of reads of a peer.
	int peer_writes = 0; //Sum of writes of a peer.
    time_t start_up,end_up;
    double waiting_time = 0.0,total_up;
    srand((unsigned int) getpid());   
    
    //Case 0: It's a child.
    if(pid==0)
    {
        for(int i=0;i<Iteration_Times;i++)
        {
            isreader = rand() % 2; //Give me either a reader or a writer.
            int random_num_of_entry = (rand() % entries); //Give me a random Entry.

            //Case 1: It's a Reader or we have no Writers (and we want some more Readers)
            if((isreader==0 || shm2_ptr[1]==0) && shm2_ptr[0]>0)
            {
                shm2_ptr[0]--;
                
                start_up = time(NULL);
                if(!(sem_down(shm_ptr[random_num_of_entry].r_sem_id)))
                    exit(1);
                end_up = time(NULL);
                total_up = (double)end_up - start_up;
                waiting_time += total_up;

                shm_ptr[random_num_of_entry].now_reader_counter++;

                if(shm_ptr[random_num_of_entry].now_reader_counter==1)
                {
                    if(!(sem_down(shm_ptr[random_num_of_entry].rw_sem_id)))
                        exit(1);
                }

                if(!(sem_up(shm_ptr[random_num_of_entry].r_sem_id)))
                    exit(1);
                
                peer_reads++;
                shm_ptr[random_num_of_entry].reader_counter++;
                sleep(expo_time(0.5));
                //printf("*Reader %d in entry %d,which has %d readers now and %d readers until now! %f - %d\n\n", getpid(), random_num_of_entry, shm_ptr[random_num_of_entry].now_reader_counter, shm_ptr[random_num_of_entry].reader_counter,total_up,t);

                if(!(sem_down(shm_ptr[random_num_of_entry].r_sem_id)))
                    exit(1);
                
                shm_ptr[random_num_of_entry].now_reader_counter--;
                if(shm_ptr[random_num_of_entry].now_reader_counter==0)
                {
                    if(!(sem_up(shm_ptr[random_num_of_entry].rw_sem_id)))
                        exit(1);
                }    
                
                if(!(sem_up(shm_ptr[random_num_of_entry].r_sem_id)))
                    exit(1);
                
            }
            //Case 2: It's a Writer or we have no Readers (and we want some more Writers)
            if((isreader==1 || shm2_ptr[0]==0) && shm2_ptr[1]>0)
            {
                shm2_ptr[1]--;
                
                start_up = time(NULL);
                if(!(sem_down(shm_ptr[random_num_of_entry].rw_sem_id)))
                    exit(1);
                end_up = time(NULL);
                total_up = (double)end_up - start_up;
                waiting_time += total_up;
                //////Critical Section/////
                peer_writes++;
                shm_ptr[random_num_of_entry].write_counter++;
                sleep(expo_time(0.1));
                //printf("*Writer %d in entry %d,which has %d writers until now! %f - %d\n\n", getpid(), random_num_of_entry, shm_ptr[random_num_of_entry].write_counter,total_up,t);

                if(!(sem_up(shm_ptr[random_num_of_entry].rw_sem_id)))
                    exit(1);
            }    

        }
        shm2_ptr[2]+=peer_reads; //Add each peer's reads to sum of all peer's reads.
        shm2_ptr[3]+=peer_writes; //Add each peer's writes to sum of all peer's writes.
        printf(" ____________________________________________________________________\n");
        printf("|Peer:%d, Read times:%d, Write times:%d, Average Waiting Time:%.2f|\n",getpid(),peer_reads,peer_writes,waiting_time/Iteration_Times);
        exit(0);
    }
    //Case 1: Father.
    else    
    {
        //Father wait as long as all of your children have finished their tasks.
        for(int j=0;j<peers;j++)
            wait(&status);
        printf(" _____________________________________________________________________\n");
        printf("\nAll peers have read %d times and have writen %d times!\n\n", shm2_ptr[2],shm2_ptr[3]);
        
        int entries_reads = 0;
        int entries_writes = 0;
        
        //Process to calculate sum of all entries reads and writes.
        for(int j=0;j<entries;j++)
        {
            entries_reads+=shm_ptr[j].reader_counter;
            entries_writes+=shm_ptr[j].write_counter;
        }
        printf("All entries have been read %d times and have been writen %d times!\n\n", entries_reads,entries_writes);

        //Deletion of all semaphores.       
        for(int j=0;j<entries;j++)
        {            
            //printf("Just deleted a (r-w)-semaphore with id=%d of entry %d\n", shm_ptr[j].rw_sem_id,j);
            sem_del(shm_ptr[j].rw_sem_id);
            //printf("Just deleted a r-semaphore with id=%d of entry %d\n\n", shm_ptr[j].r_sem_id,j);
            sem_del(shm_ptr[j].r_sem_id);
        }
        
        //Detachment of shared memory.
        shmdetach(shm_ptr);
        shmdelete(shm_id);
        //printf("Just detached and deleted shared memory!\n");  
        //Detachment of 2nd shared memory.
        shm2detach(shm2_ptr);
        shmdelete(shm2_id);
        //printf("Just detached and deleted 2nd shared memory!\n\n"); 
    }

}