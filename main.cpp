#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include <ctime>
#include <thread>


#define MAX_RETRIES 10


class Semaphore {

private:
    union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
    };

public:

    Semaphore () {}

    ~Semaphore () 
    {
        if (m_taken)
        {
            release ();
        }
    }


    bool init (const char * filename, char id) 
    {
        key_t key;
        if ((key = ftok(filename, id)) == -1) {
            perror("ftok");
            return false;
        }

        // try creating the semaphore with flags so that if it already
        // exists semget returns an error
        m_semaphoreId = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);

        if (m_semaphoreId >= 0) // this process got the semaphore first 
        { 
            union semun valueUnion;
            valueUnion.val = 0;
            
            // initialize the semaphore vith zero value
            if (semctl(m_semaphoreId, 0, SETVAL, valueUnion) == -1) {
                perror ("semctl");
                int e = errno;
                // free the semaphore
                semctl(m_semaphoreId, 0, IPC_RMID);
                m_semaphoreId = -1;
                errno = e;
                return false;
            }
        } 
        else if (errno == EEXIST) // some other therad got the semaphore first
        {
            int ready = 0;
            m_semaphoreId = semget(key, 1, 0);
            if (m_semaphoreId < 0)
            {   
                m_semaphoreId = -1;
                return false;
            }

            union semun semaphoreUnion;
            struct semid_ds buf;
            semaphoreUnion.buf = &buf;

            for(int i = 0; i < MAX_RETRIES && !ready; i++) {
                semctl(m_semaphoreId, 0, IPC_STAT, semaphoreUnion);
                if (semaphoreUnion.buf->sem_otime != 0) {
                    ready = 1;
                } else {
                    sleep(1);
                }
            }
            if (!ready) {
                errno = ETIME;
                return false;
            }
        } 
        else {
            return false; /* error, check errno */
        }
        return true;
    }

    bool take ()
    {
        struct sembuf semaphoreOperationArray[2];
        semaphoreOperationArray[0].sem_num = 0;
        semaphoreOperationArray[0].sem_op = 0;
        semaphoreOperationArray[0].sem_flg = SEM_UNDO;
        semaphoreOperationArray[1].sem_num = 0;
        semaphoreOperationArray[1].sem_op = 1;
        semaphoreOperationArray[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

        if (semop(m_semaphoreId, semaphoreOperationArray, 2) == -1) {
			perror("semop: semop failed");
            return false;
        } 
        m_taken = true;
        return true;
    }

    bool release ()
    {
        struct sembuf semaphoreOperationArray[1];
        semaphoreOperationArray[0].sem_num = 0;
        semaphoreOperationArray[0].sem_op = -1;
        semaphoreOperationArray[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
        if (semop(m_semaphoreId, semaphoreOperationArray, 1) == -1) {
            perror("semop: semop failed");
            return false;
        } 
        m_taken = false;
        return true;
    }


    bool remove ()
    {
        if (m_semaphoreId <= 0)
        {
            return false;
        }
        if (semctl(m_semaphoreId, 0, IPC_RMID) < 0)
        {
            return false;
        }
        return true;
    }


private:
    int m_semaphoreId = -1;
    bool m_taken = false;
};




int main(void)
{
    int pid;

    if ((pid = fork()) < 0) 
    {
        perror ("fork");
        exit (1);
    }
    if (pid == 0) 
    {
        Semaphore semaphore;
        if (!semaphore.init ("main.cpp", 'A')) {
            exit (1);
        }

        semaphore.take();
        printf ("Child took semaphore\n");
        sleep (2);
        semaphore.release();
        printf ("Child released semaphore\n");
        sleep (1);
    }
    else 
    {
        Semaphore semaphore;
        if (!semaphore.init ("main.cpp", 'A')) {
            wait (NULL);
            exit (1);
        }

        // 2ms delay is enough for the child process to acquire the semaphore first
        std::this_thread::sleep_for (std::chrono::milliseconds(2));

        semaphore.take();
        printf ("Parent took semaphore\n");
        sleep (2);
        semaphore.release();
        printf ("Parent released semaphore\n");
        sleep (1);
        wait (NULL);
    }

    return 0;
}




