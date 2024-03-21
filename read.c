#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "list.h"
#include "read.h"
#include "receive.h"
#include "transmit.h"

static List* LList;
static pthread_t ReaderThread;
char* Message;
char BufferStorage[256];
int Bytes;


int RdSetBytes(int BytesArg)
{
    memset(BufferStorage, 0, 256);
    BytesArg = read(0, BufferStorage, 256);
    return BytesArg;
}

char* RdSetMessageBuffer(char* MessageArg, int BytesArg)
{
  MessageArg = (char*)malloc(sizeof(char) * (BytesArg + 1));
  strncpy(MessageArg, BufferStorage, BytesArg);
  MessageArg[BytesArg] = '\0';

  return MessageArg;
}

static void* ReadUnload() {
    while (1) {

        int Iteration = 0;

        while (1) {
            Iteration++;

            printf(">");  // This will display the prompt to the user
            fflush(stdout);  // This will ensure that the prompt is displayed immediately

            Bytes = RdSetBytes(Bytes);
            if (Bytes == -1) {
                printf("Error: not enough bytes allocated");
                exit(-1);
            }

            Message = RdSetMessageBuffer(Message,Bytes);

            int Res = List_input(LList,Message);
            if (Res == -1) {
                printf("Error: Message cannot be read");
            }

            if (!strcmp(Message, "!\n") && strlen(Message)==2)
            {
              SignalTransmit();
              CancelReceiver();
              CancelTransmit();
              exit(1);
            }

            if (BufferStorage[Bytes - 1] == '\n' || Iteration == 100) {
                SignalTransmit();
                break;
            }
        }
    }
    return NULL;
}

void SetupRead(List* ListArg) {
    LList = ListArg;

    int ReadingThread =  pthread_create(&ReaderThread, NULL, ReadUnload, NULL);
    if (ReadingThread != 0) {
        printf("Error: Threads not created properly");
        exit(-1);
    }
}

void CancelRead() 
{
    pthread_cancel(ReaderThread);
}

void CloseRead() 
{
    pthread_join(ReaderThread, NULL);
}
