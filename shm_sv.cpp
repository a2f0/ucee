#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;

int main(){
char c;
int shmid;
key_t mykey=5678;
size_t mysize = 27;
char *shm, *s;

if( (shmid = shmget(mykey, mysize, IPC_CREAT | 0666)) < 0)
	cout << "Error: shmget" << endl;

if ((shm = (char*) shmat(shmid, NULL, 0)) == (char*) -1) {
	cout << "Error: shmat" << endl;
}

s=shm;

for (c='a'; c <= 'z'; c++)
	*s++ = c;
*s=NULL;

while (*shm != '*')
	sleep(1);

return 0;

}
