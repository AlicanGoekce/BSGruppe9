

int put( char * key, char * value);
int get( char * key, char * res);
int del( char * key, char * msg);
void initializeKeyAndValueSharedMemory();
void releaseKeyAndValueSharedMemory();

#include <string.h>
#include <stdio.h>
#include "sys/shm.h"
#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <netinet/in.h>
#include <unistd.h>
#include <sys/msg.h>

#ifndef UNTITLED_KEYVALSTORE_H
#define UNTITLED_KEYVALSTORE_H

#endif //UNTITLED_KEYVALSTORE_H
