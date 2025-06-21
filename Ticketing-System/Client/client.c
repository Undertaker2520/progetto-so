// client/client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "menu_client.h"   
#include "menu_agent.h"    

typedef enum {
    TITOLO,
    DESCRIZIONE,
    PRIORITA
} CampoTicket;

void createSocket(int *client_fd);
void configureAddress(struct sockaddr_in *address);
void connectToServer(int client_fd, struct sockaddr_in *address);
void ticketComponentWriter(CampoTicket campo, char *dest, int max_length);
int recv_until(int sockfd, char *buffer, size_t maxlen, char delim);
void buildTicketMessage(char *dest, int max_length);

int main() {
    int client_fd;
    struct sockaddr_in address;

    createSocket(&client_fd);
    configureAddress(&address);
    connectToServer(client_fd, &address);

    // First Login
    char username[64], password[64], ruolo[16] = {0};

    int tentativi = 0;
    while (tentativi < 3) {
        printf("Login\nUsername: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;

        printf("Password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;

        char login_msg[256];
        snprintf(login_msg, sizeof(login_msg), "LOGIN|%s|%s", username, password);
        send(client_fd, login_msg, strlen(login_msg), 0);

        char login_resp[128];
        int bytes = recv(client_fd, login_resp, sizeof(login_resp) - 1, 0);
        if (bytes <= 0) {
            printf("Errore nella comunicazione con il server.\n");
            close(client_fd);
            return 1;
        }
        login_resp[bytes] = '\0';

        if (strncmp(login_resp, "OK|Login riuscito|CLIENT", 24) == 0) {
            strcpy(ruolo, "CLIENT");
            break;
        } else if (strncmp(login_resp, "OK|Login riuscito|AGENTE", 24) == 0) {
            strcpy(ruolo, "AGENTE");
            break;
        } else if (strncmp(login_resp, "ERR|Login fallito dopo 3 tentativi", 34) == 0) {
            printf("Hai esaurito i tentativi. Connessione terminata.\n");
            close(client_fd);
            return 1;
        } else {
            printf("Login fallito: %s\n", login_resp);
            tentativi++;
        }
    }

    if (tentativi >= 3) {
        printf("Numero massimo di tentativi raggiunto. Uscita.\n");
        close(client_fd);
        return 1;
    }

    // Menu Post Login
    if (strcmp(ruolo, "CLIENT") == 0) {
        startClientMenu(client_fd, username);
    } else {
        startAgentMenu(client_fd, username);
    }
    return 0;
}

void createSocket(int *client_fd) {
    *client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void configureAddress(struct sockaddr_in *address) {
    address->sin_family = AF_INET;
    address->sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &address->sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
}

void connectToServer(int client_fd, struct sockaddr_in *address) {
    if (connect(client_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connessione al server avvenuta con successo.\n");
}

void ticketComponentWriter(CampoTicket campo, char *dest, int max_length) {
    char input[256];
    const char *nome_campo = (campo == TITOLO) ? "titolo" :
                              (campo == DESCRIZIONE) ? "descrizione" : "priorità";
    printf("Inserisci il %s del ticket: ", nome_campo);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if (strlen(input) > (size_t)max_length) {
        fprintf(stderr, "Errore: Il %s supera la lunghezza massima di %d caratteri.\n", nome_campo, max_length);
        exit(EXIT_FAILURE);
    }

    strncpy(dest, input, max_length);
}

void buildTicketMessage(char *dest, int max_length) {
    char titolo[100], descrizione[256], priorita[10];

    ticketComponentWriter(TITOLO, titolo, sizeof(titolo));
    ticketComponentWriter(DESCRIZIONE, descrizione, sizeof(descrizione));

    char *priorita_valide[] = {"Alta", "Media", "Bassa"};
    int priorita_valida = 0;

    do {
        printf("Inserisci la priorità del ticket (Alta / Media / Bassa): ");
        fgets(priorita, sizeof(priorita), stdin);
        priorita[strcspn(priorita, "\n")] = 0;

        priorita_valida = 0;
        for (int i = 0; i < 3; i++) {
            if (strcasecmp(priorita, priorita_valide[i]) == 0) {
                priorita_valida = 1;
                break;
            }
        }

        if (!priorita_valida) {
            printf("Priorità non valida. Riprova.\n");
        }
    } while (!priorita_valida);

    snprintf(dest, max_length, "NEW_TICKET|%s|%s|%s", titolo, descrizione, priorita);
}

void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize) {
    send(client_fd, messaggio, strlen(messaggio), 0);
    memset(buffer, 0, bufsize);
    int bytes = recv(client_fd, buffer, bufsize - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("%s\n", buffer);
    } else {
        printf("Errore durante la ricezione della risposta dal server.\n");
    }
}