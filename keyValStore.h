//
// Created by Alican Gökce on 06.05.24.
//

#ifndef PRAKBS21_KEYVALSTORE_H
#define PRAKBS21_KEYVALSTORE_H

int put(char* key, char* value);
int get(char* key, char** res);
int del(char* key);

#endif //PRAKBS21_KEYVALSTORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
