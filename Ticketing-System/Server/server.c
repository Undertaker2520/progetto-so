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
void hendleClientRequest(int socket, int bytes_read);

int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUF_SIZE] = {0};

    // creazione socket ipv4 e socket TCP
    createSocket(&server_fd);

    // configurazione dell'indirizzo
    configureAddress(&address);

    // Binding del socket ad un indirizzo e porta specifici
    binding(server_fd, (struct sockaddr *)&address, sizeof(address));

    // Ascolta le connessioni in arrivo
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    // Accetta una connessione in arrivo
    acceptConnections(server_fd, &new_socket);
    
    // Riceve dati dal client
    int bytes_read = read(new_socket, buffer, BUF_SIZE);
    if (bytes_read < 0) {
        perror("Read failed");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Messaggio ricevuto: %s\n", buffer);

    // Invia una risposta al client
    hendleClientRequest(new_socket, bytes_read);

    // Chiude il socket del client
    close(new_socket);
    // Chiude il socket del server
    close(server_fd);
    printf("Server chiuso.\n");

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

void hendleClientRequest(int socket, int bytes_read) {
    char buffer[1024] = {0};
    bytes_read = read(socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        perror("Errore nella ricezione");
        return;
    }

    buffer[bytes_read] = '\0'; // Termina la stringa
    printf("Messaggio ricevuto: %s\n", buffer);

    if (strncmp(buffer, "NEW_TICKET|", 11) == 0) {
        char *titolo = strtok(buffer + 11, "|");
        char *descrizione = strtok(NULL, "|");
        char *priorita = strtok(NULL, "|");

        if (titolo && descrizione && priorita) {
            Ticket t;
            t.id = generateNewTicketId();
            strncpy(t.titolo, titolo, sizeof(t.titolo) - 1);
            strncpy(t.descrizione, descrizione, sizeof(t.descrizione) - 1);
            getCurrentDate(t.data_creazione);
            strncpy(t.priorita, priorita, sizeof(t.priorita) - 1);
            strncpy(t.stato, "Aperto", sizeof(t.stato) - 1);
            strncpy(t.agente, "nessuno", sizeof(t.agente) - 1);

            if (saveTicket(&t) == 0) {
                char risposta[128];
                snprintf(risposta, sizeof(risposta), "OK|Ticket salvato con ID %d", t.id);
                send(socket, risposta, strlen(risposta), 0);
                printf("✅ Ticket salvato (ID: %d)\n", t.id);
            } else {
                send(socket, "ERR|Errore salvataggio", 23, 0);
            }
        } else {
            send(socket, "ERR|Sintassi: NEW_TICKET|Titolo|Descrizione|Priorita", 55, 0);
        }
    } else {
        send(socket, "ERR|Comando sconosciuto", 24, 0);
    }
}

