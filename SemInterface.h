#include <sys/sem.h>
#include <sys/shm.h>  
#include <openssl/md5.h> 
#include <time.h>

#define TEXT_SIZE 120


int set_semvalue(int );     //initialize semaphore from 1
void del_semvalue(int );    //delete semaphore
int P(int );                //DOWN semaphore
int V(int );                //UP semaphore
int detach(void);           //detach all semaphores and shared memory!
int length_text(char* );    //return the length of a text (used to change the letter when there is noise)

struct shared_used{  
           
    char some_text[TEXT_SIZE];      
    char checksum[MD5_DIGEST_LENGTH];  
    int redo;                   // this is a boolean flag, this segment will be empty until message arrive to P2 
    int P1;                     //P1 = 1 (P1 has the input / P2 output) / P1 = 0 (P2 has the input / P1 output)
};