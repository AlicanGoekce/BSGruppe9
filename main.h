
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include <netinet/in.h>
#include <unistd.h>

#include "keyValStore.h"
#include "sub.h"
#include "sys/shm.h"
#include "sys/sem.h"

#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H

#endif //UNTITLED_MAIN_H
