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

union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};

int wait_and_lock(int semset_id){

        union semun sem_info;
	struct sembuf actions[2];
        actions[0].sem_num = 1;
        actions[0].sem_flg = SEM_UNDO;
        actions[0].sem_op = 0;
        actions[1].sem_num = 0;
        actions[1].sem_flg = SEM_UNDO;
        actions[1].sem_op = +1;

        if(semop(semset_id, actions, 2) == -1)
                printf("ERROR:  semop---locking");
return 0;
}

int release_lock(int semset_id){

	union semun sem_info;
        struct sembuf actions[1];
        actions[0].sem_num=0;
        actions[0].sem_flg=SEM_UNDO;
        actions[0].sem_op=-1;

        if(semop(semset_id,actions,1) == -1)
                printf("ERROR:  semop---unlocking");
return 0;
}


int main(){
int shm_id, semset_id;
key_t mykey=5678;
size_t mysize = 27;
char *mem_ptr, *s;

if( (shm_id = shmget(SHM_KEY, mysize, 0777|IPC_CREAT)) < 0)
	cout << "Error: shmget" << endl;

if ((mem_ptr = (char*) shmat(shm_id, NULL, 0)) == (char*) -1) {
	cout << "Error: shmat" << endl;
}


semset_id = semget(SEM_KEY, 2, 0);
wait_and_lock(semset_id);
printf("Shared memory says:  %s",mem_ptr);
release_lock(semset_id);



shmdt(mem_ptr);
return 0;

}
