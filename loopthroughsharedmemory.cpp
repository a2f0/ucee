#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "messages.h"
#include "string.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>


//http://stackoverflow.com/questions/7261699/how-to-pass-data-using-shared-memory-and-save-the-received-data-to-file


#define SEMKEYPATH "/home"      /* Path used on ftok for semget key  */
#define SEMKEYID 1              /* Id used on ftok for semget key    */
#define NUMSEMS 2

int main(){
  int shmid;
  key_t key;
  void* shm;
  struct OrderManagementMessage omm;
  key_t semkey;
  int semid;
  struct sembuf operations[2];
  int rc;
  long long int copiedthroughsharedmemory=0;
  //name of shared memory segment
  key = 9041;

  //key_t key;
  //int semid;

  //key = ftok("/tmp", 'b');
  //semid = semget(key, 10, 0666 | IPC_CREAT);

  /*
   * Create the segment.
   */

  if ((shmid = shmget(key,sizeof(struct OrderManagementMessage),0666|IPC_CREAT)) < 0) {
    perror("shmget");
    return -1;
  }
  printf("segment id:%d\n", shmid);
  shm = shmat(shmid, NULL, 0);
  semkey = ftok(SEMKEYPATH,SEMKEYID);
  if ( semkey == (key_t)-1 )
  {
    printf("main: ftok() for sem failed\n");
    return -1;
  }
  printf("semkey: %d\n",semkey);
  semid = semget( semkey, NUMSEMS, 0666);
  if ( semid == -1 )
  {
    //printf("main: semget() failed\n");
    printf("Error in semget(): %s\n", strerror(errno));
    return -1;
  }
  for(;;) {

    /* Generate an IPC key for the semaphore set and the shared      */
    /* memory segment.  Typically, an application specific path and  */
    /* id would be used to generate the IPC key.                     */


    operations[0].sem_num = 1;
    operations[0].sem_op =  0;
    operations[0].sem_flg = 0;
    operations[1].sem_num = 0;
    operations[1].sem_op =  1;
    operations[1].sem_flg = 0;
    printf("calling semop and blocking\n");
    rc = semop( semid, operations, 2 );
    printf("semval 0: %d\n", semctl(semid, 0, GETVAL, 0));
    printf("semval 1: %d\n", semctl(semid, 1, GETVAL, 0));
    printf("semval 2: %d\n", semctl(semid, 2, GETVAL, 0));
    printf("non-blocking state has been reached\n");
    if (rc == -1)
    {
      printf("Error in semop(): %s\n", strerror(errno));
      return -1;
    }

    /*begin action*/
    memcpy(&omm,shm,sizeof(omm));
    printf("idnew message read from shared memory...\n");
    printf("order type: %d\n", omm.type);
    printf("Order type: %d\n",omm.payload.order.order_type);
    printf("Timestamp: %llu\n", omm.payload.order.timestamp );
    char order_id[33];
    strncpy(order_id, omm.payload.order.order_id, 32);
    order_id[32] = '\0';
    printf("order_id char array: %s.\n", order_id);
    printf("ending.\n");
    copiedthroughsharedmemory++;
    printf("copiedthroughsharedmemory: %llu\n", copiedthroughsharedmemory);
    /*end action*/
    operations[0].sem_num = 0;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;
    rc = semop( semid, operations, 1 );
    if (rc == -1)
    {
      printf("main: semop() failed\n");
      return -1;
    }
  }
}


/*
  void trap() {
          rc = semop( semid, operations, 2 );
                  if (rc == -1)
                          {
                                      printf("main: semop() failed\n");
                                                  return -1;
                                                          }
                                                                  rc = shmdt(shm);
                                                                          if (rc==-1)
                                                                                  {
                                                                                              printf("main: shmdt() failed\n");
                                                                                                          return -1;
                                                                                                                  }
                                                                                                                  }
*/
