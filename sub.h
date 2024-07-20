

int runCommand (char * command, char * outPut);
int getCfd (int cfd);
int initializeSemaphor(int sem);
void initializeTransactionSharedMemory();
void releaseTransactionSharedMemory();
int subService(int pid);
void initializeSubscriptionSharedMemory();
void releaseSubscriptionSharedMemory();
void initializeMessage();

#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <netinet/in.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "keyValStore.h"
#include "sys/sem.h"

#ifndef UNTITLED_SUB_H
#define UNTITLED_SUB_H

#endif //UNTITLED_SUB_H
