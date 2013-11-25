#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "messages.h"
#include "printing.h"
#include "keys.h"

//This is used to send the message through a System V message queue
struct message_msgbuf {
  long mtype;  /* must be positive */
  struct OrderManagementMessage omm;
};

int main()
{
  printf("Hello world!\n");
  key_t key = ftok(METOCMKEY1,'b');
  int msqid = msgget(key, 0666| IPC_CREAT);
  struct message_msgbuf mmb;
  while(msgrcv(msqid,&mmb,sizeof(struct message_msgbuf),2,0)!=-1){
    struct OrderManagementMessage omm = mmb.omm;
    printOrderManagementMessage(&omm);
  };
  return 0;
};
