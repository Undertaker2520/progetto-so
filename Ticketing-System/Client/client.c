// client/client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef enum {
    TITOLO,
    DESCRIZIONE,
    PRIORITA
} CampoTicket;


void createSocket(int *client_fd);
void clientRoutine(int client_fd);
void configureAddress(struct sockaddr_in *address);
void connectToServer(int client_fd, struct sockaddr_in *address);
void ticketComponentWriter(CampoTicket campo, char *dest, int max_length);
void buildTicketMessage(char *dest, int max_length);

int main(){
    int client_fd;
    struct sockaddr_in address;

    createSocket(&client_fd);
    configureAddress(&address);
    connectToServer(client_fd, &address);

    // Avvio della routine client con menu
    clientRoutine(client_fd);

    return 0;
}

void createSocket(int *client_fd){
    // Creazione socket
    *client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(*client_fd < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void configureAddress(struct sockaddr_in *address) {
    address -> sin_family = AF_INET;
    address -> sin_port = htons(8080);

    //Conversione stringa indirizzo IP -> formato binario per socket
    if(inet_pton(AF_INET, "127.0.0.1",&address -> sin_addr) <= 0){
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }
}

void connectToServer(int client_fd, struct sockaddr_in *address) {
    // Connessione al server
    if (connect(client_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connessione al server avvenuta con successo.\n");
}

void ticketComponentWriter(CampoTicket campo, char *dest, int max_length) {
    printf("Debug: sto leggendo il campo %d\n", campo);
    char input[256];
    printf("Inserisci il %s del ticket: ", (campo == TITOLO) ? "titolo" : (campo == DESCRIZIONE) ? "descrizione" : "priorità");
    
    // Lettura dell'input dell'utente
    fgets(input, sizeof(input), stdin);
    
    // Rimozione del carattere di nuova linea
    input[strcspn(input, "\n")] = 0;

    // Controllo della lunghezza massima (> stretto perchè fgets può restituire max_length -1 + '\0'!)
    if (strlen(input) > max_length) {
        fprintf(stderr, "Errore: Il %s supera la lunghezza massima di %d caratteri.\n", 
                (campo == TITOLO) ? "titolo" : (campo == DESCRIZIONE) ? "descrizione" : "priorità", max_length);
        exit(EXIT_FAILURE);
    }

    // Copia dell'input nella destinazione
    strncpy(dest, input, max_length);
}

void buildTicketMessage(char *dest, int max_length) {
    char titolo[100], descrizione[256], priorita[10];
    
    // Scrittura dei campi del ticket
    ticketComponentWriter(TITOLO, titolo, sizeof(titolo));
    ticketComponentWriter(DESCRIZIONE, descrizione, sizeof(descrizione));
    ticketComponentWriter(PRIORITA, priorita, sizeof(priorita));

    // Formattazione del messaggio del ticket
    
    snprintf(dest, max_length, "NEW_TICKET|%s|%s|%s", titolo, descrizione, priorita);
}

void clientRoutine(int client_fd) {
    char scelta[10];
    char messaggio[512];

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Inserisci un nuovo ticket\n");
        printf("2. Visualizza tutti i ticket\n");
        printf("3. Esci\n");
        printf("Seleziona un'opzione: ");

        fgets(scelta, sizeof(scelta), stdin);

        if (scelta[0] == '1') {
            // Costruzione ticket
            memset(messaggio, 0, sizeof(messaggio));
            buildTicketMessage(messaggio, sizeof(messaggio));
            send(client_fd, messaggio, strlen(messaggio), 0);

            char risposta[2048];
            memset(risposta, 0, sizeof(risposta));
            recv(client_fd, risposta, sizeof(risposta) - 1, 0);
            printf("Risposta del server: %s\n", risposta);

        } else if (scelta[0] == '2') {
            strcpy(messaggio, "GET_ALL_TICKETS");
            send(client_fd, messaggio, strlen(messaggio), 0);

            printf("Risposta del server:\n");
            char buffer[512];
            int bytes_received;
            while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[bytes_received] = '\0';
                printf("%s", buffer);
            }
            printf("\n");

        } else if (scelta[0] == '3') {
            printf("Chiusura connessione e uscita...\n");
            break;

        } else {
            printf("Opzione non valida. Riprova.\n");
        }
    }

    close(client_fd);
}
