//
// Created by Alican Gökce on 06.05.24.
//

#include "keyValStore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *key;
    char *value;
    struct Node *next;
} Node;

Node *head = NULL;

// Utility functions for data storage system
Node* createNode(char *key, char *value) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->key = strdup(key);
    newNode->value = strdup(value);
    newNode->next = NULL;
    return newNode;
}

int put(char *key, char *value) {
    Node **cur = &head;
    while (*cur) {
        if (strcmp((*cur)->key, key) == 0) {
            free((*cur)->value);
            (*cur)->value = strdup(value);
            return 1; // Key exists, value updated.
        }
        cur = &(*cur)->next;
    }
    Node *newNode = createNode(key, value);
    newNode->next = *cur;
    *cur = newNode;
    return 0; // New key-value pair added.
}

int get(char *key, char **res) {
    Node *cur = head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            *res = cur->value;
            return 1; // Key found.
        }
        cur = cur->next;
    }
    return -1; // Key not found.
}

int del(char *key) {
    Node **cur = &head, *temp;
    while (*cur) {
        if (strcmp((*cur)->key, key) == 0) {
            temp = *cur;
            *cur = (*cur)->next;
            free(temp->key);
            free(temp->value);
            free(temp);
            return 1; // Key deleted.
        }
        cur = &(*cur)->next;
    }
    return -1; // Key not found.
}



