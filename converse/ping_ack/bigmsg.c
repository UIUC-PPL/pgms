#include <stdio.h>
#include <time.h>
#include <converse.h>
#include "bigmsg.cpm.h"

CpvDeclare(int, bigmsg_index);
CpvDeclare(int, recv_count);

#define MSG_SIZE 8
#define MSG_COUNT 100

typedef struct myMsg
{
  char header[CmiMsgHeaderSizeBytes];
  int payload[MSG_SIZE];
} *message;

CpmInvokable bigmsg_stop()
{
  CsdExitScheduler();
}


void bigmsg_handler(void *vmsg)
{
  int i, next;
  message msg = (message)vmsg;
  if (CmiMyPe()==1) {
    CpvAccess(recv_count) = 1 + CpvAccess(recv_count);
    if(CpvAccess(recv_count) == MSG_COUNT) {
      CmiPrintf("\nTesting recvd data");
      for (i=0; i<MSG_SIZE; i++) {
        if (msg->payload[i] != i) {
          CmiPrintf("Failure in bigmsg test, data corrupted.\n");
          exit(1);
        }
      }
      CmiFree(msg);
      msg = (message)CmiAlloc(sizeof(struct myMsg));
      for (i=0; i<MSG_SIZE; i++) msg->payload[i] = i;
      CmiSetHandler(msg, CpvAccess(bigmsg_index));
      CmiSyncSendAndFree(0, sizeof(struct myMsg), msg);
    } else
      CmiFree(msg);
  } else { //Pe-0 receives ack
    CmiPrintf("\nReceived ack on PE-#%d", CmiMyPe());
    CmiFree(msg);
    Cpm_bigmsg_stop(CpmSend(CpmALL));
  }
}

void bigmsg_init()
{
  int i, k;
  struct myMsg *msg;
  if (CmiNumPes()<2) {
    CmiPrintf("note: bigmsg requires at least 2 processors, skipping test.\n");
    CmiPrintf("exiting.\n");
    CsdExitScheduler();
    Cpm_bigmsg_stop(CpmSend(CpmALL));
  } else {
    if(CmiMyPe()==0) {
      for(k=0;k<MSG_COUNT;k++) {
//        CmiPrintf("\nSending msg number #%d\n", k);
        msg = (message)CmiAlloc(sizeof(struct myMsg));
        for (i=0; i<MSG_SIZE; i++) msg->payload[i] = i;
        CmiSetHandler(msg, CpvAccess(bigmsg_index));
        CmiSyncSendAndFree(1, sizeof(struct myMsg), msg);
      }
    }
  }
}

void bigmsg_moduleinit(int argc, char **argv)
{
  CpvInitialize(int, bigmsg_index);
  CpvInitialize(int, recv_count);
  CpvAccess(bigmsg_index) = CmiRegisterHandler(bigmsg_handler);
  void CpmModuleInit(void);
  void CfutureModuleInit(void);
  void CpthreadModuleInit(void);

  CpmModuleInit();
  CfutureModuleInit();
  CpthreadModuleInit();
  CpmInitializeThisModule();
  // Set runtime cpuaffinity
  CmiInitCPUAffinity(argv);
  // Initialize CPU topology
  CmiInitCPUTopology(argv);
  // Wait for all PEs of the node to complete topology init
  CmiNodeAllBarrier();

  // Update the argc after runtime parameters are extracted out
  argc = CmiGetArgc(argv);
  if(CmiMyPe()==0)
    bigmsg_init();
}

int main(int argc, char **argv)
{
  ConverseInit(argc,argv,bigmsg_moduleinit,0,0);
}
