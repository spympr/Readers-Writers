#include "../headers/shared_memory.h"

int shmcreate(key_t key,int size)
{
    int k = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    if(k < 0){
      printf("Error of shmget!\n");
      exit(1);
    }
    return k;
}

Entry* shmattach(int shm_id)
{
    Entry* ptr = (Entry* )shmat(shm_id, (void *)0, 0);
    if (ptr == (Entry* )(-1)) 
    {    
        printf("Error of shmat! \n");
        exit(1);
    }
    return ptr;
}

int shmdetach(Entry* shm_ptr)
{
    int temp = shmdt(shm_ptr);
    if(temp==-1)
    {
        printf("Error with detach of shared memory!\n");
        exit(1);
    }
    return temp;
}

int* shm2attach(int shm_id)
{
    int* ptr = (int* )shmat(shm_id, (void *)0, 0);
    if (ptr == (int* )(-1)) 
    {    
        printf("Error of shmat! \n");
        exit(1);
    }
    return ptr;
}
int shm2detach(int* shm_ptr)
{
    int temp = shmdt(shm_ptr);
    if(temp==-1)
    {
        printf("Error with detach of shared memory!\n");
        exit(1);
    }
    return temp;
}

int shmdelete(int shm_id)
{
    int temp = shmctl(shm_id, IPC_RMID, 0);
    if(temp==-1)
    {    
        printf("Error with deletion of shared memory!\n");
        exit(1);
    }
    return temp;
}