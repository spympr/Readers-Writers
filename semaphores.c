#include "../headers/semaphores.h"

int set_sem(int sem_id)
{
    union semun sem_union;
    sem_union.val = 1;
    if ((semctl(sem_id, 0, SETVAL, sem_union)) == -1) return(0);
    return(1);
}

int sem_create(key_t key)
{
    int sem_id = semget(key, 1, 0666|IPC_CREAT|IPC_EXCL);
    if(!set_sem(sem_id))
    {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(1);
    }
    return sem_id;
}

void sem_del(int sem_id)
{
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    {
        fprintf(stderr, "Failed to delete semaphore %d\n", sem_id);
        exit(1);
    }
}

int sem_down(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; 
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) 
    {
        fprintf(stderr, "Semaphore_down failed %d\n", sem_id);
        return(0);
    }
    return(1);
}

int sem_up(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; 
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) 
    {
        fprintf(stderr, "Semaphore_up failed %d\n", sem_id);
        return(0);
    }
        return(1);
}