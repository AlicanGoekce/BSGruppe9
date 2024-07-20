

#include "sub.h"

#define TRUE 1

int connection_fd, sem_id, transactionId, *transaction, msgId; //int mit dem Namen connection_fd wird erstellt, hier sollen dann die Clientdaten drin gespeichert werden

struct sembuf enter, leave; // Structs für den Semaphor

struct text_message { // struct für die Nachrichtenwarteschlange, kurz Aufbau einer Nachricht da drin
    long mtype; // message Typ einfach die ProzessId
    char mtext[100]; // Die eigentliche message
};

typedef struct KeyAndSub { // Struct um Subscriber ProzessId und keys miteinander zu verbinden
    char key[100];
    int pid;
} KeyAndSub;

int *keyAndSubSize, subId, subSizeId; // Die jeweiligen Structs, Ids für das Shared memory und die Size dazu
struct KeyAndSub *keysAndSubs;

int initializeSemaphor(int sem) { // Der Semaphor aus main wird an sub.c übergeben und die structs
    sem_id = sem;                   // für den Semaphor werden mit den jeweiligen Daten gefüllt

    enter.sem_num = leave.sem_num = 0; // Semaphor 0 in der Gruppe
    enter.sem_flg = leave.sem_flg = SEM_UNDO;
    enter.sem_op = -1; // blockieren, DOWN-Operation
    leave.sem_op = 1; // freigeben, UP-Operation

    return 0;
}

int putSubscriber(char * key, char * output, char * getRes) { // Subscriber werden im shared memory gespeichert
    if (strcmp(getRes, "key_notfound") == 0 || strcmp(getRes, "key_nonexistent") == 0) {
        sprintf(output, "SUB:%s:%s", key, getRes); // Wenn der key nicht existiert dann macht er so in output rein
        return 0;
    }
    for (int i = 0; i < *keyAndSubSize; i++) { // Wenn schon vorhanden dann halt nicht
        if (strcmp(keysAndSubs[i].key, key) == 0 && keysAndSubs[i].pid == getpid()) {
            strcpy(output, "That key was already subscribed!");
            return 0;
        }
    }
    sprintf(output, "SUB:%s:%s", key, getRes); // Ausgabe für den Client wird erstellt und in output gespeichert
    keysAndSubs[*keyAndSubSize].pid = getpid(); // Die Prozess Id vom gerade laufenden Prozess
    strcpy(keysAndSubs[*keyAndSubSize].key, key); // und der Key der übergeben wird, werden zusammen gespeichert
    *keyAndSubSize += 1; // Die Size vom KeyAndSub struct wird erhöht
    return 0;
}

int notifySubscriber(char *key, char *output) { // Die Subscriber von einem Key werden benachrichtigt
    if (*keyAndSubSize == 0)
        return 0;
    int v;
    struct text_message msg; // message struct wird angelegt
    strcpy(msg.mtext, output); // und der command wird als message übergeben
    for (int i = 0; i < *keyAndSubSize; i++) { // Jetzt werden alle Subscriber durchsucht bis man die findet die was zu
        if (strcmp(keysAndSubs[i].key, key) == 0) { // dem übergebenen Key wissen wollten
            if (keysAndSubs[i].pid != getpid()) { // Wenn man nicht selbst derjenige ist, der subscribt hat
                msg.mtype = keysAndSubs[i].pid; // Dann wird die ProzessId als message Typ übergeben
                v = msgsnd(msgId, &msg, strlen(output) + 1, 0); // und die message in die Warteschlange gepackt
                if (v < 0) {
                    printf("Error writing to queue!\n"); // Fehler, wenn es nicht geklappt hat
                }
            }
        }
    }
    return 0;
}

void delSubscription(char *key) { // Wenn ein Key gelöscht wird, so wird auch jeder subscriber zu dem gelöschten key
    if (*keyAndSubSize != 0) {
        int deletedElements = 0;    // aus der subscriber liste mit dem Eintrag zum jeweiligen Key gelöscht
        for (int i = 0; i < *keyAndSubSize; ++i) {
            if (i == *keyAndSubSize - 1 && strcmp(keysAndSubs[i].key, key) == 0) {
                deletedElements++;
                break;
            }
            if (strcmp(keysAndSubs[i].key, key) == 0) {
                keysAndSubs[i] = keysAndSubs[i + 1];
                deletedElements++;
            }
        }
        keyAndSubSize -= deletedElements;
    }
}

int subService(int pid) { // Hier werden alle Nachrichten für den gerade laufenden Prozess, mit der jeweiligen ProzessId,
    long v;             // von der Nachrichtenwarteschlange ausgelesen und auf der Konsole ausgegeben
    struct text_message msg;
    while (TRUE) {
        v = msgrcv(msgId, &msg, 100, pid, 0); // Hier ausgelesen!
        if (v < 0)
            printf("No message in queue!\n");
        else {
            write(connection_fd, msg.mtext, strlen(msg.mtext)); // Und hier einmal ausgegeben
            write(connection_fd, "\n", sizeof("\n")); // Einfach nochmal ein Zeilenumbruch
        }
    }
}

int getCfd (int cfd) { //Holt den Clienten, oder eher gesagt die Daten des Clienten für die sub.c
    connection_fd = cfd;
    return 0;
}

typedef int (*StoreCommand)(char *key, char *val); //Definiert eine unbestimmte Funktion

void runningCommand(StoreCommand storeCommand, char *command, char *outPut, char *tmp, char *key, char *val) { //hier werden Funktionen ausgeführt,
    if (val != NULL) {                                                              //mit den Werten die übergeben werden
        storeCommand(key, val);             //wichtig der value (val) Wert darf nicht NULL sein
        sprintf(outPut, "%s:%s:%s", tmp, key, val); //dann wird auch die Funktion die ausgeführt wird in der variablen command
    }                                       //gespeichert in der Form "Funktion:Key:Message/Wert"
    else strcpy(outPut, "Not a valid command!"); //Ansonsten wird in command "Not a valid command!" gespeichert und auch ausgegeben
    puts(outPut);
}

int runCommand (char * command, char * outPut) {
    const char *putCommand = "PUT"; //char Arrays mit den Werten {'P','U','T'} usw. werden erstellt und können nicht verändert werden
    const char *getCommand = "GET";
    const char *delCommand = "DEL";
    const char *quitCommand = "QUIT";
    const char *subCommand = "SUB";

    const char delim[4] = {' ', '\r', '\n', '\0'}; //char Array mit den Werten Leerzeichen, Enter, Zeilenumbruch und Ende des
    //char Arryays ('\0')

    char *token;    //char Arrays den Namen token, tmp ,tmpkey, tmpVal
    char *tmp;
    char *tmpKey = NULL; //tmpkey und tmpVal sind hier erstmal NULL gesetzt
    char *tmpVal = NULL; //sprich da steht nichts drin

    token = strtok(command, delim); //In token wird der erste String/das erste Wort aus dem command oder hier dem Input Buffer/
    int counter = 0;                    //dem was der Client schreibt gesetzt und da wird darauf geachtet das der cut vom ersten Wort
    //Ich bin ein Counter                 entweder auf Leerzeichen, Enter, oder '\0' gesetzt wird, die Werte werden aus dem Array delim
    //entnommen

    if (command != NULL) { //Es wird geguckt ob der command überhaupt existiert, bzw. ob der Client überhaupt etwas geschrieben hat
        while (token != NULL && counter < 4) { //Wenn der token, also das erste Wort was wir vorher aus dem command gezogen haben
            if (counter == 0) {                 //nicht NULL ist und der counter < 4 ist
                tmp = token;                    //dann wird tmp = dem was in token steht gesetzt und
                if (strcmp(tmp, quitCommand) == 0) // es wird geguckt ob tmp bzw. token der quit command ist
                    break;              //wenn es ein quit ist, dann soll er hier auch bitte direkt aus der schleife raus
            } else if (counter == 1) { //ansonsten wird geguckt ob der counter = 1 ist dann ist der token der key
                tmpKey = token;     // und tmpKey wird = dem was in Token steht gesetzt
//                if (strcmp(tmp, subCommand) == 0)   // es wird geschaut ob der command sub ist, dann braucht man nur den key und nichts weiter
//                    break;
            } else if (counter == 2) {//Das selbe nur dieses mal mit Value/Message
                if (strcmp(tmp, putCommand) == 0) { //Wenn aber Put command ist, dann wird geschaut ob value auch Integer ist
                    int count = 0;              //Sonst ist das kein valider command
                    for (int i = 0; i < strlen(token); ++i) { //Hier wird der Value durchgegangen und geschaut ob der auch
                        if (!isdigit(token[i]))     //nur aus digits besteht, also ob der Value auch Integer ist
                            break;
                        count++;
                    }
                    if (count == strlen(token)) { //Wenn ja selbes Spiel wie vorher
                        tmpVal = token;
                    } else break;
                } else {        //Und wenn nicht, wird unterbrochen
                    tmpVal = token; //Wenn es kein Put command ist, dann ist selbes Spiel wie immer
                    counter++;
                }
            } else if (counter == 3) { //Und wenn der counter = 3 ist, soll er wieder die Schleife verlassen
                break;
            }
            token = strtok(NULL, delim); //Jedes Mal am Ende der Schleife wird in token, das nächste Wort aus dem Command gespeichert
            counter++; //und danach wird der counter erhöht
        }

        if (strcmp(tmp, putCommand) == 0) { //Ist der command ein put command
            if (*transaction != 0) { // transaction zeigt das PUT und DEL gerade belegt sind, wodurch diese nicht ausgeführt werden
                strcpy(outPut, "Put oder Del sind gerade besetzt!");
            } else { // Der Put command wird einmal mit einem Down blockiert
                semop(sem_id, &enter, 1); // hier
                *transaction += 1; // Das wird für andere mit dem shared memory transaction signalisiert
                runningCommand(put, command, outPut, tmp, tmpKey, tmpVal); //dann wird der put command ausgeführt
                notifySubscriber(tmpKey, outPut); // Die Subscriber werden benachrichtigt
                semop(sem_id, &leave, 1); // mit einem Up wird der Prozess wieder freigegeben
                *transaction -= 1; // Und das wird wieder mir dem shared memory transaction signalisiert
            }
            return 0;
        }                   //übergebenen Key und dem übergebenen Value ausgeführt und einmal auf der Konsole ausgegeben
        if (strcmp(tmp, getCommand) == 0) { //Dasselbe für den get command
            runningCommand(get, command, outPut, tmp, tmpKey, tmpVal);
            puts(tmpKey);
            return 0;
        }
        if (strcmp(tmp, delCommand) == 0) { //Dasselbe del command
            if (*transaction != 0) { // Siehe Put command
                strcpy(outPut, "Put oder Del sind gerade besetzt!");
            } else {
                semop(sem_id, &enter, 1);
                *transaction += 1;
                runningCommand(del, command, outPut, tmp, tmpKey, tmpVal);
                notifySubscriber(tmpKey, outPut);
                delSubscription(tmpKey); // extra: Hier werden alle Subscriber einträge zu dem übergebenen Key gelöscht
                semop(sem_id, &leave, 1);
                *transaction -= 1;
            }
            return 0;
        }
        if (strcmp(tmp, subCommand) == 0) { // bei einem Sub Command soll ein Prozess für einen bestimmten Key subscribt werden
            if (tmpVal == NULL) { // checkt, ob die sub Syntax richtig ist
                strcpy(outPut, "SUB <KEY> <MSG>!");
                return 0;
            } // tmpVal ist hier wichtig, da wir über den get command schauen können ob der key überhaupt existiert
            runningCommand(get, command, outPut, tmp, tmpKey, tmpVal); // und get braucht ein tmpVal
            putSubscriber(tmpKey, outPut, tmpVal);
            return 0;
        }
        if (strcmp(tmp, quitCommand) == 0) { //wenn der command = QUIT war, dann wird
            strcpy(outPut, "Ending connection!"); //in dem übergebenen char Array reingeschrieben das die connection beendet wird
            return -1; //und zeitgleich wird quit = 1 gesetzt, wodurch wir aufhören weiterzulesen was der Client denn schreibt
        }
    }
    strcpy(outPut, "Not a valid command!"); //Wenn alle möglichen Fälle durchgelaufen sind und keiner stattfand, dann wird
    return 0;       //in den übergebenen char Array "Not a valid command!" geschrieben und der server soll weiterhin alles vom
}                   //Client mitlesen

void initializeTransactionSharedMemory() { // Siehe KeyValStore Shared Memory
    transactionId = shmget(IPC_PRIVATE, sizeof(int) * 100, IPC_CREAT|0777); //Hier wird einmal das keysAndValues
    transaction = (int *) shmat(transactionId, 0, 0);      //Array im Shared Memory gespeichert
    *transaction = 0;
}

void releaseTransactionSharedMemory() {
    shmdt(transaction);
}

void initializeSubscriptionSharedMemory() {
    subId = shmget(IPC_PRIVATE, sizeof(KeyAndSub) * 100, IPC_CREAT | 0777); //Hier werden die subscriber gespeichert
    keysAndSubs = (struct KeyAndSub *) shmat(subId, 0, 0);

    subSizeId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);  //Und hier wird die Größe des Arrays gespeichert
    keyAndSubSize = (int *) shmat(subSizeId, 0, 0);
    *keyAndSubSize = 0; //Die größe wird auch erstmal auf 0 gesetzt
}

void releaseSubscriptionSharedMemory() {
    shmdt(keysAndSubs);
    shmdt(keyAndSubSize);
}

void initializeMessage() { // Nachrichtenwarteschlange mit msgId als Id wird erstellt
    msgId = msgget(IPC_PRIVATE, IPC_CREAT|0666); // Im Skript stand 0666 deswegen hier nicht 0777 wie überall sonst
}