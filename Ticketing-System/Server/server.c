// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Tickets/ticket.h" //includo solo questo includere il ticket.c e' cattiva pratica
#include "../Authentication/auth.h" //includo solo questo includere il auth.c e' cattiva pratica

#define PORT 8080
#define BUF_SIZE 1024

void createSocket(int *server_fd);
void configureAddress(struct sockaddr_in *address);
void binding(int server_fd, struct sockaddr *address, socklen_t addrlen);
void acceptConnections(int server_fd, int *new_socket);
int handleClientRequest(int socket);
int createNewTicket(const char *buffer);
void handleGetAllTickets(int socket);
void handleGetTicketById(int socket, const char *buffer);
void handleNewTicket(int socket, const char *buffer);
void handleLogin(int socket, const char *buffer);
int authenticateUser(const char *username, const char *password, char *ruolo);


typedef enum {
    CMD_NEW_TICKET,
    CMD_GET_ALL,
    CMD_GET_BY_ID,
    CMD_UNKNOWN,
    CMD_LOGIN
} CommandType;

CommandType parseCommand(const char *buffer) {
    if (strncmp(buffer, "NEW_TICKET|", 11) == 0) return CMD_NEW_TICKET;
    if (strncmp(buffer, "GET_ALL_TICKETS", 15) == 0) return CMD_GET_ALL;
    if (strncmp(buffer, "GET_TICKET_BY_ID|", 17) == 0) return CMD_GET_BY_ID;
    if (strncmp(buffer, "LOGIN|", 6) == 0) return CMD_LOGIN;
    return CMD_UNKNOWN;
}


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
        while (1) {
            if (handleClientRequest(new_socket) == 0) {
                break; // client ha chiuso la connessione o ha inviato un comando di uscita
            }
        }

        // Chiude la connessione con il client
        close(new_socket);

        printf("In attesa di una nuova connessione...\n");
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


int handleClientRequest(int socket) {
    char buffer[1024] = {0};

    int bytes_read = read(socket, buffer, BUF_SIZE);
    if (bytes_read <= 0) {
        perror("Read failed o connessione chiusa");
        return 0; // interrompe la connessione
    }

    buffer[bytes_read] = '\0'; // Assicura fine stringa
    printf("Messaggio ricevuto: %s\n", buffer);

    int response;
    switch (parseCommand(buffer)) {
        case CMD_NEW_TICKET:
            handleNewTicket(socket, buffer);
            break;
        case CMD_GET_ALL:
            handleGetAllTickets(socket);
            break;
        case CMD_GET_BY_ID:
            handleGetTicketById(socket, buffer);
            break;
        case CMD_LOGIN:
            handleLogin(socket, buffer);
        break;
        default:
            send(socket, "ERR|Comando sconosciuto", 24, 0);
            break;
    }
    return 1;
}

void handleGetAllTickets(int socket) {
    char ticket_buffer[8192];
    int num_tickets = getAllTickets(ticket_buffer, sizeof(ticket_buffer));

    printf("Numero di ticket letti: %d\n", num_tickets);

    switch (num_tickets) {
        case -1:
            send(socket, "ERR|Errore lettura tickets", 27, 0);
            break;
        case 0:
            send(socket, "OK|Nessun ticket presente", 26, 0);
            break;
        default:
            send(socket, ticket_buffer, strlen(ticket_buffer), 0);
            break;
    }
}

void handleGetTicketById(int socket, const char *buffer) {
    int id = atoi(buffer + strlen("GET_TICKET_BY_ID|")); // Estrai ID
    Ticket t;

    if (readTicketById(id, &t) == 0) {
        char risposta[1024];
        snprintf(
            risposta, sizeof(risposta),
            "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\n",
            t.id, t.titolo, t.descrizione, t.data_creazione,
            t.priorita, t.stato, t.agente
        );
        send(socket, risposta, strlen(risposta), 0);
    } else {
        send(socket, "ERR|Ticket non trovato", 23, 0);
    }
}

void handleNewTicket(int socket, const char *buffer) {
    int result = createNewTicket(buffer);

    if (result > 0) {
        char risposta[128];
        snprintf(risposta, sizeof(risposta), "OK|Ticket salvato con ID %d", result);
        send(socket, risposta, strlen(risposta), 0);
        printf("Ticket salvato (ID: %d)\n", result);
    } else if (result == -2) {
        send(socket, "ERR|Sintassi: NEW_TICKET|Titolo|Descrizione|Priorita", 55, 0);
    } else {
        send(socket, "ERR|Errore salvataggio", 23, 0);
    }
}


void handleLogin(int socket, const char *buffer) {
    char temp[128];
    strncpy(temp, buffer, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    char *username = strtok(temp + 6, "|");
    char *password = strtok(NULL, "|");

    if (!username || !password) {
        send(socket, "ERR|Formato: LOGIN|username|password", 37, 0);
        return;
    }

    char ruolo[20];
    int result = authenticateUser(username, password, ruolo);

    if (result == 1) {
        char risposta[64];
        snprintf(risposta, sizeof(risposta), "OK|Login riuscito|%s", ruolo);
        send(socket, risposta, strlen(risposta), 0);
    } else if (result == 0) {
        send(socket, "ERR|Credenziali non valide", 27, 0);
    } else {
        send(socket, "ERR|Errore accesso file utenti", 30, 0);
    }
}
