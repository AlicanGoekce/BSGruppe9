

#include "keyValStore.h"

typedef struct KeyAndValue {
    char key[100];
    char value[100];
} KeyAndValue; //Das ist einfach ein Objekt in dem ein Key und ein Value/Message gespeichert wird

struct KeyAndValue * keysAndValues; //Das ist dann ein Array aus diesen KeyAndValue Objekten

int *keysAndValuesSize; //Das ist die Größe des Arrays was wir gerade erstellt haben

int keyId, idSize;

int put( char * key, char * value){ //Hier wird geschaut, ob die übergebenen Werte denn auch gültige Werte sind
    if (key == NULL || value == NULL)
        return 0; //Wenn nicht, dann soll die Funktion halt auch nicht ausgeführt werden
    for (int i = 0; i < *keysAndValuesSize; i++) {
        if (strcmp(keysAndValues[i].key, key) == 0) { //Gibt es den übergeben Key schon, so wird nur die value dazu
            strcpy(keysAndValues[i].value, value); //verändert und die Größe des Arrays bleibt gleich
            return 0; //Und hiermit wird die Funktion verlassen
        }
    }
    strcpy(keysAndValues[*keysAndValuesSize].key, key); //Wenn die Werte aber gültig sind, dann werden diese in dem vorher definierten
    strcpy(keysAndValues[*keysAndValuesSize].value, value); //Array gespeichert
    *keysAndValuesSize += 1; //Und die jetzige Größe des Arrays wird durch den Pointer zur Größe angezeigt

    return 0;
}

int get( char * key, char * res){ //Hier wird auch geschaut, ob die Werte gültig sind
    strcpy(res, "key_notfound"); //Die Ursprungsmessage die gegeben werden soll ist immer "key_notfound"
    if (key == NULL || *keysAndValuesSize == 0) {
        return 0; //Wenn der key der übergeben wurde, ungültig ist oder das Array leer ist, dann wird die Ursprungsmessage ausgegeben
    }
    for (int i = 0; i < *keysAndValuesSize; i++){ //Sollte der Key aber existieren dann wird der jeweilige Value dazu zurückgegeben
        if (strcmp(keysAndValues[i].key, key) == 0) {
            strcpy(res, keysAndValues[i].value);
            return 0;
        }
    }
    strcpy(res, "key_nonexistent"); //Wenn man alles durchlaufen ist, dann kann es den Key ja gar nicht erst geben
    return 0;
}

int del( char * key, char * msg ){ //Die Ursprungsmessage ist hier das ein Key gar nicht existiert
    strcpy(msg, "key_nonexistent");
    if (*keysAndValuesSize == 0) { //Sollte das Array leer sein, so wird die Ursprungmessage zurückgegeben
        return 0;
    }
    for (int i = 0; i < *keysAndValuesSize; i++){ //Hier wird einfach nur nach dem übergebenen Key in dem Array gesucht
        if (strcmp(keysAndValues[i].key, key) == 0 && i != *keysAndValuesSize - 1) { // Und dieser Eintrag wird dann aus
            for (int n = i + 1; n < *keysAndValuesSize; n++) { // dem Array gelöscht und die größe des Arrays wird dementsprechend angepasst
                keysAndValues[n - 1] = keysAndValues[n];
                strcpy(msg, "key_deleted"); //Die message die dann zurückgegeben wird ist das der Key, bzw.
            }                       //der Eintrag mit dem key gelöscht wurde
            return 0;
        } else if (i == *keysAndValuesSize - 1 && strcmp(keysAndValues[i].key, key) == 0) {
            strcpy(msg, "key_deleted");
            break;
        }
    }
    *keysAndValuesSize -= 1;
    return 0;
}

void initializeKeyAndValueSharedMemory() {
    keyId = shmget(IPC_PRIVATE, sizeof(KeyAndValue) * 100, IPC_CREAT|0777); //Hier wird einmal das keysAndValues
    keysAndValues = (struct KeyAndValue *) shmat(keyId, 0, 0);      //Array im Shared Memory gespeichert

    idSize = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0777);  //Und hier wird die Größe des Arrays gespeichert
    keysAndValuesSize = (int *) shmat(idSize, 0, 0);
    *keysAndValuesSize = 0; //Die größe wird auch erstmal auf 0 gesetzt
}

void releaseKeyAndValueSharedMemory() {
    shmdt(keysAndValuesSize);
    shmdt(keysAndValues);
}