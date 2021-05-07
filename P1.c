//P1 is the server system
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


int length_text(char* );


int main(int argc, char* argv[])
{
    if (argc != 2) { 
        printf("You must enter a probability, try again\n");
        exit(0);
    }


    pid_t pid;      	//fork identifier
    int running = 1;
    	 

    int shmid;
    void *shared_memory = (void *)0;
    struct shared_used* shared_stuffs;   
	char buffer[BUFSIZ];

    int semp1_id;       //semaphore for P1          
    int semp2_id;       //semaphore for ENC1 
    int semp3_id;       //semaphore for CHAN       
    int semp4_id;       //semaphore for ENC2
    int semp5_id;       //semaphore for P2





    //Create semaphore for sychronize process
    semp1_id = semget((key_t)4321, 1, 0666 | IPC_CREAT);  
    if (semp1_id < 0) {
        perror("Could not create sem");
        exit(3);
    }

    semp2_id = semget((key_t)6543, 1, 0666 | IPC_CREAT);  
    if (semp2_id < 0) {
        perror("Could not create sem");
        exit(3);
    }

    semp3_id = semget((key_t)7654, 1, 0666 | IPC_CREAT); 
    if (semp3_id < 0) {
        perror("Could not create sem");
        exit(3);
    }

    semp4_id = semget((key_t)2345, 1, 0666 | IPC_CREAT);  
    if (semp4_id < 0) {
        perror("Could not create sem");
        exit(3);
    }


    semp5_id = semget((key_t)4567, 1, 0666 | IPC_CREAT); 
    if (semp5_id < 0) {
        perror("Could not create sem");
        exit(3);
    }




    if (-1 == (pid = fork())) /* spawn child process */     
    { perror ("error in fork");
        exit (1);
    }

    if (pid != 0) { //P1

        printf("P1 process begin\n");

        shmid = shmget((key_t)1234, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between P1 -> ENC1
        if (shmid == -1) {
            fprintf(stderr , "shmget failed\n");  
            exit(EXIT_FAILURE);
        }

        shared_memory = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
        if (shared_memory == (void *)-1) {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }

        shared_stuffs = (struct shared_used *)shared_memory;


        shared_stuffs->redo = 0;            //0:the message is ok, 1:the message has noise    
        shared_stuffs->P1 = 1;              //first P1 use input

        while(running) {

            if(shared_stuffs->P1 == 0){     //receive the message and print it to stdout

                if (strncmp(shared_stuffs->some_text, "TERM", 4) == 0) {      

                    if (shmdt(shared_memory) == -1)     
                        fprintf(stderr, "shmdt failed in shared memory 1 from P1\n");

                    printf("\nP2 terminated the system\n");

                    detach();   //detach all shared memories  and semaphores   
                    break;           
                }

                printf("\nmessage from P2: %s", shared_stuffs->some_text);


                shared_stuffs->P1 = 1;
            }



            if(shared_stuffs->P1 == 1){
                
                printf("\nEnter a message: ");    //ENC1 wait P1 


                fgets(buffer, BUFSIZ, stdin);	         //write text and save it in buffer
                strncpy(shared_stuffs->some_text, buffer, TEXT_SIZE);  //copy text to shm


                if (strncmp(shared_stuffs->some_text, "TERM", 4) == 0) {       

                    if (shmdt(shared_memory) == -1)     //P1 dettach shm
                        fprintf(stderr, "shmdt failed in shared memory 1 from P1\n");

                    printf("\nYou terminated the system\n");

                    V(semp2_id);    //wake up process ENC1, P1 terminated

                    break;
                }

                shared_stuffs->P1 = 0;      //prepare to receive message from P2

            }

            V(semp2_id);
            P(semp1_id);
        }



    } else {    //ENC1->CHAN  or CHAN->ENC1  

        void *shared_memory2 = (void *)0;
        struct shared_used* shared_stuffs2;     //saving address
        pid_t pid2;                             //pid for CHAN process



        if( -1 == (pid2 = fork())){
            perror("error in fork");
            exit(1);
        }


        if (pid2 != 0){             //ENC1

            char* checksum_old;
            char* checksum_new;
            int noise;


            shmid = shmget((key_t)1234, sizeof(struct shared_used), 0666 | IPC_CREAT);
            if (shmid == -1) {
                fprintf(stderr , "shmget failed\n");    
                exit(EXIT_FAILURE);
            }

            shared_memory = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
            if (shared_memory == (void *)-1) {
                fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
            }

            shared_stuffs = (struct shared_used *)shared_memory;        //attach shm1 P1->ENC1


            shmid = shmget((key_t)2345, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC1 -> CHAN
            if (shmid == -1) {
                fprintf(stderr , "shmget failed\n");    
                exit(EXIT_FAILURE);
            }

            shared_memory2 = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
            if (shared_memory2 == (void *)-1) {
                fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
            }
            
            shared_stuffs2 = (struct shared_used *) shared_memory2;        //attach shm ENC1->CHAN
            shared_stuffs2->P1 = 1;      
            shared_stuffs2->redo = 0;

            while(running){ 

                P(semp2_id);
                
                if(shared_stuffs2->P1 == 0){
                    
                    if(shared_stuffs2->redo == 1)       //Message came again, check again
                        shared_stuffs2->redo = 0;

                    checksum_old = shared_stuffs2->checksum;
                    checksum_new = MD5(shared_stuffs2->some_text, sizeof(shared_stuffs2->some_text), NULL);
                    noise = memcmp(checksum_old, checksum_new, MD5_DIGEST_LENGTH);         //if noise is 0 transfer message without problem otherwise retransmit


                    if(!noise){     //if there isn't noise transfer the message without problem 

                        strncpy(shared_stuffs->some_text, shared_stuffs2->some_text, sizeof(shared_stuffs2->some_text));    //copy text to shared memory 4
                        strncpy(shared_stuffs->checksum, shared_stuffs2->checksum, sizeof(shared_stuffs2->checksum));    //copy checksum to shared memory 4 
                        shared_stuffs2->P1 = 1;     //prepare to send message P1
                        
                        if (strncmp(shared_stuffs2->some_text, "TERM", 4) == 0) {  
                            V(semp1_id);    //wake up process P1, process P2 terminated
                            break;         
                        }

                        printf("\n(P2->P1) noise = %d\n", noise);

                        V(semp1_id);            //go to P1 process

                        continue;             //pass the current process

                    }
                    else{           //if there is noise raise the flag "redo" and resent the message from ENC1   

                        printf("\n(P2->P1) noise = %d P2 resend please wait...\n", noise);
                        shared_stuffs2->redo = 1;
                        V(semp3_id);            //go to the previous process

                    }

                    /*CHAN informs ENC2, P1 wants to tereminate*/
                    if (strncmp(shared_stuffs2->some_text, "TERM", 4) == 0) {  
                        V(semp1_id);    //wake up process P1, process P2 terminated
                        break;          
                    }

                }



                if(shared_stuffs2->P1 == 1){    //if shm2 -> redo == 1  retransmit the message

                    if(shared_stuffs2->redo == 1)
                        shared_stuffs2->redo = 0;   //ENC1 resend the message

                    strncpy(shared_stuffs2->some_text, shared_stuffs->some_text, TEXT_SIZE);    //copy text in shm2

                    //encrypt message
                    MD5(shared_stuffs2->some_text, sizeof(shared_stuffs2->some_text), shared_stuffs2->checksum);


                    if (strncmp(shared_stuffs->some_text, "TERM", 4) == 0) {      

                        V(semp3_id);            //wake up process CHAN, process P1 terminated.
                        break;                  //dettach and finish process
                    }

                    shared_stuffs2->P1 = 0;     //prepare to receive message for moving P2->P1
                    V(semp3_id);
                }

            }

            if (shmdt(shared_memory2) == -1) 
                fprintf(stderr, "shmdt failed in shared memory 3 ENC2\n");

            if (shmdt(shared_memory) == -1) 
                fprintf(stderr, "shmdt failed in shared memory 4 from ENC2\n");

        } 
        else {                  //CHAN -> ENC2


            void *shared_memory3 = (void *)0;
            struct shared_used* shared_stuffs3;    
            int length;
            int random_number;
            int term;
            pid_t pid3;


            if( -1 == (pid3 = fork())){
                perror("error in fork");
                exit(1);
            }


            if (pid3 != 0){         //CHAN



                shmid = shmget((key_t)2345, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between ENC1 -> CHAN
                if (shmid == -1) {
                    fprintf(stderr , "shmget failed\n");    
                    exit(EXIT_FAILURE);
                }

                shared_memory2 = shmat(shmid, (void *)0, 0);  //default arguments making shm attach
                if (shared_memory3 == (void *)-1) {
                    fprintf(stderr, "shmat failed\n");
                    exit(EXIT_FAILURE);
                }

                shared_stuffs2 = (struct shared_used *)shared_memory2;        //attach shm ENC1->CHAN


                shmid = shmget((key_t)3456, sizeof(struct shared_used), 0666 | IPC_CREAT);      //shm between CHAN->ENC1
                if (shmid == -1) {
                    fprintf(stderr , "shmget failed\n");   
                    exit(EXIT_FAILURE);
                }

                shared_memory3 = shmat(shmid, (void *)0, 0);  //default arguments making shm attach, attach CHAN->ENC1 shm
                if (shared_memory3 == (void *)-1) {
                    fprintf(stderr, "shmat failed\n");
                    exit(EXIT_FAILURE);
                }


                srand(time(0));
                shared_stuffs3 = (struct shared_used*) shared_memory3;
                
                shared_stuffs3->P1 = 1;         //first start P1
                shared_stuffs3->redo = 0;       //initialize flag redo

                while(running){

                    P(semp3_id);

                    if(shared_stuffs3->P1 == 0){    //P2 -> P1

                        if(shared_stuffs2->redo == 1){
                            shared_stuffs3->redo = 1;       //ENC2 must resent the message
                            V(semp4_id);
                            P(semp3_id);                //wait ENC2 to send the message again
                            shared_stuffs3->redo = 0;   //CHAN resent the message
                        }

                        strncpy(shared_stuffs2->some_text, shared_stuffs3->some_text, sizeof(shared_stuffs2->some_text));    //copy text to shared memory 3
                        strncpy(shared_stuffs2->checksum, shared_stuffs3->checksum, sizeof(shared_stuffs3->checksum));    //copy checksum to shared memory 3

                        length = length_text(shared_stuffs3->some_text);

                        random_number = rand() % 100+1;        //(o - 100)

                        /*if the message is TERM dont alterate it*/
                        term = strncmp(shared_stuffs3->some_text, "TERM", 4);   

                        //from 0-10 there is noise with a probability, and from 11 - there isn't noise
                        if ((random_number < atoi(argv[1])) && (term != 0)) { 
                            //noise hits the text
                            shared_stuffs2->some_text[rand()%length] = 'x'; 

                        }else{ 

                            shared_stuffs3->P1 = 1;   

                            /*ENC1 info CHAN P1 want terminate*/
                            if (strncmp(shared_stuffs3->some_text, "TERM", 4) == 0){       
        
                                V(semp2_id);            //wake up ENC1, process P2 terminated
                                break;           
                            } 
                            V(semp2_id);        //wake up ENC1
                            continue;        
                        }

                        V(semp2_id);
                    }



                    if(shared_stuffs3->P1 == 1){    //P1->P2
                            
                        if(shared_stuffs3->redo == 1){
                            shared_stuffs2->redo = 1;   //info ENC1 to retransmit the message
                            shared_stuffs2->P1 = 1;    
                            V(semp2_id);
                            P(semp3_id);
                            shared_stuffs3->redo = 0;  //EC2 resend the message 
                        }

                        //copy text to shared memory 3
                        strncpy(shared_stuffs3->some_text, shared_stuffs2->some_text, sizeof(shared_stuffs2->some_text));

                        //copy checksum to shared memory 3
                        strncpy(shared_stuffs3->checksum, shared_stuffs2->checksum, sizeof(shared_stuffs2->checksum));   
                        
                        length = length_text(shared_stuffs3->some_text);

                        random_number = rand() % 100+1;        //(o - 100 probability msg pass without noise)

                        term = strncmp(shared_stuffs3->some_text, "TERM", 4);   

                        /*if the messege is TERM dont alteration the message*/
                        if ((random_number < atoi(argv[1])) && (term != 0)) { 
                            //noise hits the text
                            shared_stuffs3->some_text[rand()%length] = 'x'; 
                            shared_stuffs2->P1 = 1;

                        } else
                            shared_stuffs3->P1 = 0;     //prepare to receive message from P2->P1

                        /*ENC1 info CHAN, P1 want terminate*/
                        if (strncmp(shared_stuffs2->some_text, "TERM", 4) == 0){    

                            V(semp4_id);         //wake up ENC2 process P1 terminated.
                            break;             
                        }  

                        V(semp4_id);
                    }

                }

                /*this is for CHAN*/
                if (shmdt(shared_memory2) == -1)    
                    fprintf(stderr, "shmdt failed in shared memory 2 from CHAN\n");
                
                if (shmdt(shared_memory3) == -1) 
                    fprintf(stderr, "shmdt failed in shared memor 3 from CHAN\n");


            } 
            else exit(0);  //useless child
	    }
         
     }

}



