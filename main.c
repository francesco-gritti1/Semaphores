
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>



union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};




int main(void)
{

    // OSS: in this example semaphores can be created without bothering that other processes
    // could be concurrently creating them since the child process is forked afterward

    key_t key;
    int semid;
    int pid;
    int nSemaphoreOperations;
    struct sembuf semaphoreOperationArray[2];

    // get the key
    if ((key = ftok("main.c", 'A')) == -1) {
        perror("ftok");
        exit(1);
    }

    // create the shared semaphore
    if ((semid = semget (key, 1, IPC_CREAT | 0666)) == -1) {
        perror ("semget");
        exit (1);
    }

    // set semaphore value to zero
    union semun valueUnion;
    valueUnion.val = 0;
    
    if (semctl(semid, 0, SETVAL, valueUnion) == -1) {
        perror ("semctl");
        exit (1);
    }



    if ((pid = fork()) < 0) {
        perror ("fork");
        exit (1);
    }
    if (pid == 0) {
        // child process
        nSemaphoreOperations = 2;

        //OSS the following operations are performed ATOMICALLY: both or none.
        // this means that first the process will be waiting for the semaphore
        // value to become zero and then it will set increment it taking control
        semaphoreOperationArray[0].sem_num = 0;
        semaphoreOperationArray[0].sem_op = 0;
        semaphoreOperationArray[0].sem_flg = SEM_UNDO;

        semaphoreOperationArray[1].sem_num = 0;
        semaphoreOperationArray[1].sem_op = 1;
         // there is no need to wait since the operation is perfomed atomically
         // after semaphore's value become zero
        semaphoreOperationArray[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

        if (semop(semid, semaphoreOperationArray, nSemaphoreOperations) == -1) {
			perror("semop: semop failed");
        } 
        else {
            printf ("Child took semaphore\n");
            sleep (2);
            nSemaphoreOperations = 1;

            // free the semaphore
            semaphoreOperationArray[0].sem_num = 0;
            semaphoreOperationArray[0].sem_op = -1;
            semaphoreOperationArray[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
            if (semop(semid, semaphoreOperationArray, nSemaphoreOperations) == -1) {
                perror("semop: semop failed");
            } 
            else {
                printf ("Child released semaphore\n");
                sleep (1);
            }
        }
    }

    else {
        // parent process
       nSemaphoreOperations = 2;

        //OSS the following operations are performed ATOMICALLY: both or none.
        // this means that first the process will be waiting for the semaphore
        // value to become zero and then it will set increment it taking control
        semaphoreOperationArray[0].sem_num = 0;
        semaphoreOperationArray[0].sem_op = 0;
        semaphoreOperationArray[0].sem_flg = SEM_UNDO;

        semaphoreOperationArray[1].sem_num = 0;
        semaphoreOperationArray[1].sem_op = 1;
         // there is no need to wait since the operation is perfomed atomically
         // after semaphore's value become zero
        semaphoreOperationArray[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

        if (semop(semid, semaphoreOperationArray, nSemaphoreOperations) == -1) {
			perror("semop: semop failed");
        } 
        else {
            printf ("Parent took semaphore\n");
            sleep (2);
            nSemaphoreOperations = 1;

            // free the semaphore
            semaphoreOperationArray[0].sem_num = 0;
            semaphoreOperationArray[0].sem_op = -1;
            semaphoreOperationArray[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
            if (semop(semid, semaphoreOperationArray, nSemaphoreOperations) == -1) {
                perror("semop: semop failed");
            } 
            else {
                printf ("Parent released semaphore\n"); 
                sleep (1);
            }
        }
        wait (NULL);
    }
    return 0;
}