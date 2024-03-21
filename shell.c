#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "read.h"
#include "write.h"
#include "receive.h"
#include "transmit.h"

// Launches the s-talk program
int main (int argc, char * argv[])
{
    // Checks if arguments are correct
    if (argc!=4)
    {
        printf("Please enter valid arguments \n");
        printf("Valid argument strucure is: \n");
        printf("s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    // Storing arguments
    char* myPort = argv[1];
    char* theirHostname = argv[2];
    char* theirPort = argv[3];

    // Creates a shared list
    List* list = List_create();

    // Initializes the four modules
    // Modules are running in an infinite loop, they will exit when given the exit code "!"
    SetupRead(list);
    SetupTransmit(theirHostname, theirPort, list);
    SetupReceiver(myPort, list);
    SetupWriter(list);

    // Cleans up the threads
    CloseRead();
    CloseTransmit();
    CloseReceiver();
    CloseWriter();

    // Frees the list
    // At this point, the list should already be empty
    List_free(list, free);

    return 0;
}