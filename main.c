#include <sys/wait.h>
#include "main.h" //Header File von main, wo die ganzen libraries drin stehen


#define TRUE 1          //TRUE wird hier einfach 1 gesetzt || TRUE = 1;
#define BUFSIZE 1024    //BUFSIZE wird hier einfach 1024 gesetzt || BUFSIZE = 1024;

int main() {

    int sock, connection_fd, pid, sem_id;//ints werden erstellt mit den jeweiligen Namen
    unsigned short marker[1]; // Variable sem_id für die Semaphorgruppe und
    // aus technischen Gründen eine Variable marker[1]

    struct sockaddr_in server, client;  //sockaddr_in erstellt mit Namen server und client
    //sockaddr_in ist einfach eine Klasse die eine Adresse und einen Port speichert

    socklen_t client_len;               //socklen_t erstellt mit Namen client_len
    //socklen_t ist eine Klasse mit einer Adresslänge

    char in[BUFSIZE], out[BUFSIZE];     //char Arrays mit den Namen in und out werden erstellt, mit der Größe BUFSIZE, also 1024

    client_len = sizeof(client);        //client_len auf die Größe von client gesetzt

    sock = socket(AF_INET, SOCK_STREAM, 0); //In sock wird jetzt ein neuer socket erstellt
    //socket(IPv4, verbindungorientierter socket ist oder TCP, protocol = 0);


    if (sock < 0) {         //Ein Socket ist wenn er normal erstellt werden kann gleich 1, sprich wenn er kleiner 1 ist,
        perror("\nCreating stream socket"); //dann konnte der Socket nicht erstellt werden
        exit(-1);       //perror ist wie printf
    }       //Wenn der Socket nicht erstellt werden kann soll die Initialiesierung in sock verlassen werden ( exit(-1) )

    server.sin_family = AF_INET;       //Die Initialisierung vom Server AF_INT = IPv4
    server.sin_addr.s_addr = INADDR_ANY;    //INADDR_ANY heißt der Server läuft auf dem localhost oder "127.0.0.1" (ist das gleiche)
    server.sin_port = htons(5678);  //Der Port wird auf 5678 gestellt, hier wichtig manche Ports sind reserviert,
    //aber Ports über einem bestimmten Wert können frei besetzt werden
    //Nur Clients auf dem localhost, die sich über den Port mit dem Server verbinden wollen,
    //können das auch machen

    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT|0777); //Eine Semaphor wird angelegt mit einem Teilnehmer in der Gruppe
    if (sem_id == -1) { // Fehlermeldung wenn die Gruppe nicht angelegt werden kann
        perror("Die Gruppe konnte nicht angelegt werden!");
        exit(1);
    }

    marker[0] = 1;
    semctl(sem_id, 1, SETALL, marker); //alle Sempaphore werden auf 1 gesetzt

    int *transaction; // einfach ein shared memory der angibt das gerade die Leitung belegt ist für PUT oder DEL Befehle
    initializeSemaphor(sem_id); // Semaphor wird übergeben an sub.c
    initializeTransactionSharedMemory();

    int brt = bind(sock, (struct sockaddr *) &server, sizeof(server)); //Der Server, den wir vorher erstellt haben,
    if (brt < 0) {  //wird an den Socket gebunden den wir erstellt haben und es wird wieder geschaut, ob das überhaupt
        perror("\nSocket couldn't be bound"); //funktioniert. Wenn nicht, dann eben wieder Fehlermeldung und exit
        exit(-1);
    }

    puts("Socket bound");   //Ausgabe auf dem Terminal das der Socket gebunden ist

    int lrt = listen(sock, 5);  //Hier lauscht der Socket jetzt nach neuen Clienten, die sich mit dem Server verbinden wollen
    if (lrt < 0) {  //Wenn es nicht geht Fehlermeldung und exit
        perror("\nSocket couldn't be listen"); //Die 5 bei listen, steht für die Anzahl der Clienten die er max akzeptieren dürfte
        exit(-1);
    }

    puts("\nWaiting for incoming connection\n"); //Ausgabe auf dem Terminal das nach connections gelauscht wird

    initializeKeyAndValueSharedMemory(); //Shared Memory für die Keys und Values wird angelegt
    int quit = 0; //Eine variable die ursprünglich 0 sein soll
    int clientNumberId, *clientNumber; //Neue ints für Anzahl der Clients

    clientNumberId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0644); //Shared Memory für die
    clientNumber = (int *) shmat(clientNumberId, 0, 0); //Anzahl der Clients wird angelegt
    *clientNumber = 0;  //Die ursprungs Anzahl wird 0 gesetzt

    initializeSubscriptionSharedMemory(); // Shared memory für subscriptions wird angelegt in sub.c
    initializeMessage(); // Nachrichtenwarteschlange wird angelegt in sub.c

    while (TRUE) { //while (TRUE) oder while (1) ist einfach eine Endlosschleife
        connection_fd = accept(sock, (struct sockaddr *) &client, &client_len);
        //Ein neuer client wird an den Server auf dem Socket angenommen
        //Die Adresse des clients ist hier einfach eine Adresse, die automatisch ausgelesen wird
        //client_len ist ja vorher schon als die Länge bzw. die Größe von client gesetzt worden
        //daher auch die addr_len client_len
        if (connection_fd < 0) { //Wenn die Verbindung nicht funktioniert hat, wieder exit
            exit(-1);
        }
        *clientNumber += 1; //Da ein Client verbunden wurde, wird die client Anzahl erhöht
        puts("Connection established"); //Ausgabe auf dem Terminal das der Client verbunden ist
        if ((pid = fork()) == 0) { // Extra Prozess aufspaltung, sodass der eine Teil, komplett für die Nachrichtenwarteschlange verantwortlich ist
            getCfd(connection_fd);  //Die Daten vom client werden an sub.c übergeben
            if ((pid = fork()) == 0) { //Ein neuer Prozess wird erstellt und der child-prozess wird in der if bearbeitet
                printf("\nNew child process established, by client no.%d\n", *clientNumber);
                while (quit == 0 &&
                       read(connection_fd, in, BUFSIZE) > 0) { //solange quit = 0 ist und der Server noch Eingaben
                    //vom Clienten lesen kann (read() > 0), soll der Server weiterhin
                    //die Eingaben vom Clienten lesen

                    quit = runCommand(in, out); //definiert in sub.c
                    //Grob gesagt die commands die gelesen werden, werden ausgeführt und bei quit wird aufgehört hier zu lesen

                    write(connection_fd, out,strlen(out));   //Der Inhalt vom eingehenden Buffer (dem was der Client an den Server
                    //schreibt), soll ausgegeben werden strlen ist dann einfach dazu da
                    //die maximale Länge von dem was ausgegben werden soll
                    //auch auf die Länge von dem was in dem out Buffer steht zu setzen
                    write(connection_fd, "\n", sizeof("\n")); //schreibt einen Zeilenumbruch
                } //Der Client hat ein QUIT gesetzt und wir lesen nicht mehr weiter was er schreibt
                if (quit != 0) { //wenn mit QUIT gequittet wurde soll nochmal manuell gequittet werden
                    puts(out); //Quit message wird auf ausgegeben
                    char *quit = "You have to manually quit!"; //die neue quit message wird definiert
                    write(connection_fd, (quit), strlen(quit)); //Die neue Quit message wird ausgegeben
                    while (read(connection_fd, in, BUFSIZE) > 0) {
                        write(connection_fd, (quit), strlen(quit)); //und das solange bis manuell gequittet wird
                    }
                } else { //Wenn von vorne rein manuell gequittet wurde, dann wird das ausgegeben
                    printf("Connection was quit manually!\n");
                }
                close(connection_fd);   //Die Verbindung zum client wird geschlossen und die Shared Memory wird freigegeben
                printf("\nSecond child connection closed, by client no.%d\n", *clientNumber);
                releaseKeyAndValueSharedMemory();
                releaseTransactionSharedMemory();
                *clientNumber -= 1; //Die client Anzahl wird verringert
                exit(0); //Der child-prozess wird geschlossen
            } //Hier würde der Prozess von vorher weiterlaufen, wenn es sich um den parent-prozess handelt
            else { // Hier der Prozess, der komplett für die Nachrichtenwarteschlange verantwortlich ist, sodass subscriber auch benachrichtigt werden
                subService(pid); // Dazu wird die Prozess Id übergeben
            }
        }
        close(connection_fd); //Die connection wird geschlossen und es wird gesagt von wem.
        printf("\nParent connection closed, by client no.%d\n", *clientNumber);
    }
    close(sock);
    return 0; //return 0 wie man das kennt
}
