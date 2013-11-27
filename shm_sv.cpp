#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <cstring>
#define SHM_KEY 9991
#define SEM_KEY 9999991
using namespace std;

int shm_id, semset_id; //global for cleanup()
union semun{
int val;
struct semid_ds *buf;
ushort *array;
};
void cleanup(int); //cleanup for semaphores


void cleanup(int n){
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(semset_id, 0, IPC_RMID, NULL);
}


int set_sem_value(int semset_id, int semnum, int val){
        union semun initval;
        initval.val = val;
        if (semctl(semset_id, semnum, SETVAL, initval) == -1)
                printf("ERROR:  semctl");
return 0;
}

int wait_and_lock(int semset_id){

        struct sembuf actions[2];
        actions[0].sem_num = 0;
        actions[0].sem_flg = SEM_UNDO;
        actions[0].sem_op = 0;
        actions[1].sem_num = 1;
        actions[1].sem_flg = SEM_UNDO;
        actions[1].sem_op = +1;

        if(semop(semset_id, actions, 2) == -1)
                printf("ERROR:  semop---locking");
return 0;
}

int release_lock(int semset_id){
        struct sembuf actions[1];
        actions[0].sem_num=1;
        actions[0].sem_flg=SEM_UNDO;
        actions[0].sem_op=-1;

        if(semop(semset_id,actions,1) == -1)
                printf("ERROR:  semop---unlocking");
return 0;
}



int main(){
char c;
int i;
key_t mykey=5678;
size_t mysize = 27;
char *mem_ptr, *s;


if( (shm_id = shmget(SHM_KEY, mysize, IPC_CREAT | 0777)) < 0)
	cout << "Error: shmget" << endl;

if ((mem_ptr = (char*) shmat(shm_id, NULL, 0)) == (char*) -1) {
	cout << "Error: shmat" << endl;
}

if ( (semset_id = semget (SEM_KEY, 2, (IPC_CREAT | 0777 ))) == -1)
	printf("ERROR:  semget");

set_sem_value (semset_id, 0, 0);
set_sem_value (semset_id, 1, 0);

for(i=0; i<10; i++){
wait_and_lock(semset_id);
strcpy(mem_ptr, "WRITINGTOSHMTEXT");
release_lock(semset_id);
}

//cleanup(0);
return 0;
}

