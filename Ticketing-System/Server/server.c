// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Tickets/ticket.h" //includo solo questo includere il ticket.c e' cattiva pratica

#define PORT 8080
#define BUF_SIZE 1024

void createSocket(int *server_fd);
void configureAddress(struct sockaddr_in *address);
void binding(int server_fd, struct sockaddr *address, socklen_t addrlen);
void acceptConnections(int server_fd, int *new_socket);
void handleClientRequest(int socket);
void createNewTicket(int socket, const char *buffer, Ticket *t);

int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // creazione socket ipv4 e socket TCP
    createSocket(&server_fd);

    // configurazione dell'indirizzo
    configureAddress(&address);

    // Binding del socket ad un indirizzo e porta specifici
    binding(server_fd, (struct sockaddr *)&address, sizeof(address));
    /* === FINE CODICE ORIGINALE === */

    // Ascolta le connessioni in arrivo
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);
    
    while (1) {
        // Accetta una connessione in arrivo
        acceptConnections(server_fd, &new_socket);

        // Gestisce la richiesta del client e invia risposta
        handleClientRequest(new_socket);

        // Chiude la connessione con il client
        close(new_socket);

        printf("🔁 In attesa di una nuova connessione...\n");
    }

    return 0;
}

void createSocket(int *server_fd){
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // per evitare errori di "Address already in use" se riavvii il server rapidamente
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("Set socket options failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
}

void configureAddress(struct sockaddr_in *address) {
    int opt = 1;
    address->sin_family = AF_INET; // IPv4
    address->sin_addr.s_addr = INADDR_ANY; // Accetta connessioni da qualsiasi indirizzo
    address->sin_port = htons(PORT); // Imposta la porta
}

void binding(int server_fd, struct sockaddr *address, socklen_t addrlen) {
    if (bind(server_fd, address, addrlen) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void acceptConnections(int server_fd, int *new_socket) {
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    *new_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
    if (*new_socket < 0) {
        perror("Errore in accept");
        exit(EXIT_FAILURE);
    }

    printf("Connessione accettata da %s:%d\n",
        inet_ntoa(client_address.sin_addr),
        ntohs(client_address.sin_port));
}

void handleClientRequest(int socket) {
    char buffer[1024] = {0};

    int bytes_read = read(socket, buffer, BUF_SIZE);
    if (bytes_read < 0) {
        perror("Read failed");
        close(socket);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0'; // Assicura fine stringa
    printf("Messaggio ricevuto: %s\n", buffer);

    // Gestione creazione nuovo ticket
    if (strncmp(buffer, "NEW_TICKET|", 11) == 0) {
        Ticket t;
        createNewTicket(socket, buffer, &t );

    // Gestione richiesta all ticket - EMA
    } else if (strncmp(buffer, "GET_ALL_TICKETS", 15) == 0) {
        char ticket_buffer[8192];
        int num_tickets = getAllTickets(ticket_buffer, sizeof(ticket_buffer));

        printf("Numero di ticket letti: %d\n", num_tickets);
        if (num_tickets < 0) {
            send(socket, "ERR|Errore lettura tickets", 27, 0);
        } else if (num_tickets == 0) {
            send(socket, "OK|Nessun ticket presente", 26, 0);
        } else {
            send(socket, ticket_buffer, strlen(ticket_buffer), 0);
        }

    } else {
        send(socket, "ERR|Comando sconosciuto", 24, 0);
    }
}