#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>  
#include <openssl/md5.h> 
#include <time.h>

#include "semun.h"
#include "SemInterface.h" 






int main()
{              
    int running = 1;

    int semp3_id;           
    int semp4_id;
    int semp5_id;
    pid_t pid;

    /* Semaphores */
    semp5_id = semget((key_t)4567, 1, 0666 | IPC_CREAT);  //semp5  for shm  ENC2->P2
    if (semp5_id < 0) {
        perror("Could not create sem");
        exit(3);
    }

    semp4_id = semget((key_t)2345, 1, 0666 | IPC_CREAT);  //semp4 for shm ENC1 <-> CHAN
    if (semp4_id < 0) {
        perror("Could not create sem");
        exit(3);
    }

    semp3_id = semget((key_t)7654, 1, 0666 | IPC_CREAT);  //semp3 for ENC1 <-> CHAN
    if (semp3_id < 0) {
        perror("Could not create sem");
        exit(3);
    }



    if( -1 == (pid = fork())){
        perror("error in fork\n");
        exit(1);
    }



    void *shared_memory4 = (void *)0;
    struct shared_used* shared_stuffs4;     //saving address


    if(pid != 0){       //ENC2 



        void *shared_memory3 = (void *)0;
        struct shared_used* shared_stuffs3;     //saving address
        int shmid;
        char* checksum_old;
        char* checksum_new;
        int noise;


        shmid = shmget((key_t)3456, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between CHAN -> ENC2
        if (shmid == -1) {
            fprintf(stderr , "shmget failed\n");   
            exit(EXIT_FAILURE);
        }

        shared_memory3 = shmat(shmid, (void *)0, 0);  
        if (shared_memory3 == (void *)-1) {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }

        shared_stuffs3 = (struct shared_used*)shared_memory3;   

        shmid = shmget((key_t)4567, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2 -> P1

        if (shmid == -1) {
            fprintf(stderr , "shmget failed\n");    
            exit(EXIT_FAILURE);
        }

        shared_memory4 = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
        if (shared_memory4 == (void *)-1) {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }

        shared_stuffs4 = (struct shared_used*)shared_memory4;   //attach in shm4


        shared_stuffs4->P1 = 1;         //first give input P1
        shared_stuffs4->redo = 0;

        while(running){

            P(semp4_id);

            /*this condition is only for comunication P2->P1 and retransmit msg*/
            if(shared_stuffs4->P1 == 0 || shared_stuffs3->redo == 1){

                strncpy(shared_stuffs3->some_text, shared_stuffs4->some_text, TEXT_SIZE);    //copy text in shm2
                MD5(shared_stuffs3->some_text, sizeof(shared_stuffs3->some_text), shared_stuffs3->checksum);    //encrypt

                if (strncmp(shared_stuffs4->some_text, "TERM", 4) == 0) {       

                    V(semp3_id);     //wake up process CHAN, P2 terminated
                    break;          //dettach and finish process
                }

                shared_stuffs4->P1 = 1;     //prepare for moving P1->P2 
                V(semp3_id);                //wake up CHAN
                continue;
            }


            if(shared_stuffs4->P1 == 1){

                checksum_old = strdup(shared_stuffs3->checksum);
                checksum_new = strdup(MD5(shared_stuffs3->some_text, sizeof(shared_stuffs3->some_text), NULL));
                noise = memcmp(checksum_old, checksum_new, MD5_DIGEST_LENGTH);         //if noise is 0 transfer message without problem otherwise retransmit


                if(!noise){     //if there isn't noise transfer the message without problem

                    strncpy(shared_stuffs4->some_text, shared_stuffs3->some_text, sizeof(shared_stuffs3->some_text));    //copy text to shared memory 4
                    strncpy(shared_stuffs4->checksum, shared_stuffs3->checksum, sizeof(shared_stuffs3->checksum));    //copy checksum to shared memory 4 - dont change -
                    printf("\n(P1->P2) noise = %d\n", noise);

                }
                else{    //if there is noise raise the flag "redo" and resent the message from ENC1   

                    printf("\n(P1->P2) noise = %d P1 resend please wait.....\n", noise);
                    shared_stuffs3->redo = 1;    
                    V(semp3_id);    //informs CHAN the message has noise

                }

                /*CHAN informs ENC2, P1 want terminate*/
                if (strncmp(shared_stuffs3->some_text, "TERM", 4) == 0) {       

                    V(semp5_id);     //wake up process P2, P1 terminated
                    break;          
                }

                if(!noise)          //if there isn't noise send message to P2
                    V(semp5_id);

            }

        }

        /*this is for ECN2*/
        if (shmdt(shared_memory3) == -1) 
            fprintf(stderr, "shmdt failed in shared memory 3 ENC2\n");

        if (shmdt(shared_memory4) == -1) 
            fprintf(stderr, "shmdt failed in shared memory 4 from ENC2\n");



        }
        else{
            int shmid;
            char buffer[BUFSIZ];
            

            printf("\nYou Connected to server\n");
            printf("\nPlease wait message from P1\n");
            


            shmid = shmget((key_t)4567, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC2->P2
            if (shmid == -1) {
                fprintf(stderr , "shmget failed\n");   
                exit(EXIT_FAILURE);
            }

            shared_memory4 = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
            if (shared_memory4 == (void *)-1) {
                fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
            }

            shared_stuffs4 = (struct shared_used*)shared_memory4;   //attach in shm4

            shared_stuffs4->redo = 0;   //redo = 1, resent message from P2

            while(running){
                
                
                P(semp5_id);

                if(shared_stuffs4->P1 == 1){

                    if (strncmp(shared_stuffs4->some_text, "TERM", 4) == 0) {   
                        printf("\nP1 terminated the system\n");

                        if (shmdt(shared_memory4) == -1)    //dettach shm
                            fprintf(stderr, "shmdt failed in shared memory 4 from P2\n");

                        detach();      //detach shared memories/semaphores

                        break;      //go out from loop to dettach memmory
                    }

                    printf("\nmessage from P1: %s", shared_stuffs4->some_text);
                    shared_stuffs4->P1 = 0;        //inform ENC2 the message came without problems we are ready to send a message from P2
                    
                }

                if(shared_stuffs4->P1 == 0){       

                    printf("\nEnter a message: ");    //ENC1 wait P1

                    fgets(buffer, BUFSIZ, stdin);	                        //write text and save it in buffer
                    strncpy(shared_stuffs4->some_text, buffer, TEXT_SIZE);  //copy text to shm

                    if (strncmp(shared_stuffs4->some_text, "TERM", 4) == 0) {     
                        printf("\nYou terminated the system\n");

                        if (shmdt(shared_memory4) == -1) //dettach shm
                            fprintf(stderr, "shmdt failed in shared memory 4 from P2\n");

                        V(semp4_id);     //wake up process ENC2, P2 terminated

                        break;          
                    }
                    
                }

                V(semp4_id);
            }   
        }   

}




