// client/client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "menu_client.h"   // contiene la dichiarazione avviaMenuClient
#include "menu_agent.h"    // contiene la dichiarazione avviaMenuAgente

// Invia tutto il buffer di dimensione len
int send_all(int sockfd, const void *buffer, size_t len) {
    size_t total_sent = 0;
    const char *ptr = buffer;
    while (total_sent < len) {
        ssize_t sent = send(sockfd, ptr + total_sent, len - total_sent, 0);
        if (sent <= 0) {
            return -1;  // errore o connessione chiusa
        }
        total_sent += sent;
    }
    return total_sent;
}

// Riceve dati da sockfd finché non trova il carattere delim, o buffer è pieno
// Restituisce il numero di byte ricevuti, o -1 in caso di errore
int recv_until(int sockfd, char *buffer, size_t maxlen, char delim) {
    size_t received = 0;
    while (received < maxlen - 1) {
        char c;
        ssize_t ret = recv(sockfd, &c, 1, 0);
        if (ret <= 0) {
            return -1;  // Errore o connessione chiusa
        }
        buffer[received++] = c;
        if (c == delim) {
            break;
        }
    }
    buffer[received] = '\0';
    return received;
}

void buildTicketMessage(char *dest, int max_length) {
    char titolo[100], descrizione[256], priorita[10];

    printf("Inserisci titolo: ");
    fgets(titolo, sizeof(titolo), stdin);
    titolo[strcspn(titolo, "\n")] = 0;

    printf("Inserisci descrizione: ");
    fgets(descrizione, sizeof(descrizione), stdin);
    descrizione[strcspn(descrizione, "\n")] = 0;

    printf("Inserisci priorità (Bassa, Media, Alta): ");
    fgets(priorita, sizeof(priorita), stdin);
    priorita[strcspn(priorita, "\n")] = 0;

    snprintf(dest, max_length, "NEW_TICKET|%s|%s|%s", titolo, descrizione, priorita);
}

int main() {
    int client_fd;
    struct sockaddr_in address;

    // Creazione socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configurazione indirizzo server
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Connessione al server
    if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connessione al server avvenuta con successo.\n");

    // Login
    char username[64], password[64], ruolo[16] = {0};

    printf("Login\nUsername: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    char login_msg[256];
    snprintf(login_msg, sizeof(login_msg), "LOGIN|%s|%s\n", username, password);

    // Invio login
    if (send_all(client_fd, login_msg, strlen(login_msg)) < 0) {
        perror("Errore invio login");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Ricezione risposta login (terminata da '\n')
    char login_resp[64];
    int bytes = recv_until(client_fd, login_resp, sizeof(login_resp), '\n');
    if (bytes <= 0) {
        printf("Errore nella comunicazione con il server.\n");
        close(client_fd);
        return 1;
    }

    // Controllo risposta
    if (strncmp(login_resp, "OK|Login riuscito|CLIENT", 24) == 0) {
        strcpy(ruolo, "CLIENT");
    } else if (strncmp(login_resp, "OK|Login riuscito|AGENTE", 24) == 0) {
        strcpy(ruolo, "AGENTE");
    } else {
        printf("Login fallito: %s\n", login_resp);
        close(client_fd);
        return 1;
    }

    // Chiamata ai menu esterni
    if (strcmp(ruolo, "CLIENT") == 0) {
        avviaMenuClient(client_fd);
    } else if (strcmp(ruolo, "AGENTE") == 0) {
        avviaMenuAgente(client_fd);
    }

    return 0;
}
