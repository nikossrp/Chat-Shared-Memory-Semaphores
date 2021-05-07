#include <stdio.h>
#include <stdlib.h>
// #include <sys/sem.h>
#include "semun.h"
#include "SemInterface.h"




int set_semvalue(int sem_id)
{
	union semun sem_union;
	sem_union.val = 0;	//xekinaw ton semaphore se up katastash me to SETVAL thetw thn timh tou sem etsi
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);	//ama apetixe
	return(1);
}

void del_semvalue(int sem_id)
{
	union semun sem_union;
	/*To IPC_RMID apodesmevei tn domh*/
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) // 0 giati den exei nohma se poio sygekrimeno stoixeio tha dolepsoume
	fprintf(stderr, "Failed to delete semaphore\n");
}

int P(int sem_id)		//down sem
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;		//exoume enan shmaforo sto systhma
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {	//ena array apo praxhs, to 1 perigrafh ton arithmo twn praxewn panw sto semaphore
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

int V(int sem_id)	//up sem
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}


int length_text(char* text)         //return the length of a string
{
    int i;
    int count = 0;

    for(i = 0; text[i] != '\0'; i++)
        count++;
    
    return count-1;
}


int detach(void)
{
    /* Destroy all shared memory (4 shm) */
    int shmid;

    shmid = shmget((key_t)1234, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2->P2
    if (shmid == -1) {
        fprintf(stderr , "shmget failed\n");    //Lab1 > shm_producer
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
	}
        

    shmid = shmget((key_t)2345, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2->P2
    if (shmid == -1) {
        fprintf(stderr , "shmget failed\n");    //Lab1 > shm_producer
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
	}
        

    shmid = shmget((key_t)3456, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2->P2
    if (shmid == -1) {
        fprintf(stderr , "shmget failed\n");    //Lab1 > shm_producer
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
	}
        

    shmid = shmget((key_t)4567, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2->P2
    if (shmid == -1) {
        fprintf(stderr , "shmget failed\n");    //Lab1 > shm_producer
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
	}





    /*Destroy all semaphores*/

    int semid;



    semid = semget((key_t)2345, 1, 0666 | IPC_CREAT);  //sem4  for shm  CHAN->P2
    if (semid < 0) {
        perror("Could not create sem");
        exit(3);
    }
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }



    semid = semget((key_t)4567, 1, 0666 | IPC_CREAT);  //sem4  for shm  CHAN->P2
    if (semid < 0) {
        perror("Could not create sem");
        exit(3);
    }
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }



    semid = semget((key_t)4321, 1, 0666 | IPC_CREAT);  //sem4  for shm  CHAN->P2
    if (semid < 0) {
        perror("Could not create sem");
        exit(3);
    }
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }


    semid = semget((key_t)6543, 1, 0666 | IPC_CREAT);  //sem4  for shm  CHAN->P2
    if (semid < 0) {
        perror("Could not create sem");
        exit(3);
    }
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }



    semid = semget((key_t)7654, 1, 0666 | IPC_CREAT);  //sem4  for shm  CHAN->P2
    if (semid < 0) {
        perror("Could not create sem");
        exit(3);
    }
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }


    return 1;   //destroyed successfully

}
