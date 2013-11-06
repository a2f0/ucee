#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;

int main(){
//char c;
int shmid;
key_t mykey=5678;
size_t mysize = 27;
char *shm, *s;

if( (shmid = shmget(mykey, mysize, 0666)) < 0)
	cout << "Error: shmget" << endl;

if ((shm = (char*) shmat(shmid, NULL, 0)) == (char*) -1) {
	cout << "Error: shmat" << endl;
}

for (s=shm; *s != NULL; s++)
	putchar(*s);
putchar('\n');

*shm = '*';

return 0;

}
