#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "menu_client.h"
#include "menu_agent.h"

void createSocket(int *client_fd);
void configureAddress(struct sockaddr_in *address);
void connectToServer(int client_fd, struct sockaddr_in *address);
int login(int client_fd, char *username, char *ruolo);

int main() {
    int client_fd;
    struct sockaddr_in address;
    char username[64], ruolo[16] = {0};

    createSocket(&client_fd);
    configureAddress(&address);
    connectToServer(client_fd, &address);

    if (!login(client_fd, username, ruolo)) {
        close(client_fd);
        return 1;
    }

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

int login(int client_fd, char *username, char *ruolo) {
    char password[64];
    int tentativi = 0;

    while (tentativi < 3) {
        printf("Login\nUsername: ");
        fgets(username, 64, stdin);
        username[strcspn(username, "\n")] = 0;

        printf("Password: ");
        fgets(password, 64, stdin);
        password[strcspn(password, "\n")] = 0;

        char login_msg[256];
        snprintf(login_msg, sizeof(login_msg), "LOGIN|%s|%s", username, password);
        send(client_fd, login_msg, strlen(login_msg), 0);

        char login_resp[128];
        int bytes = recv(client_fd, login_resp, sizeof(login_resp) - 1, 0);
        if (bytes <= 0) {
            printf("Errore nella comunicazione con il server.\n");
            return 0;
        }
        login_resp[bytes] = '\0';

        if (strncmp(login_resp, "OK|Login riuscito|CLIENT", 24) == 0) {
            strcpy(ruolo, "CLIENT");
            return 1;
        } else if (strncmp(login_resp, "OK|Login riuscito|AGENTE", 24) == 0) {
            strcpy(ruolo, "AGENTE");
            return 1;
        } else if (strncmp(login_resp, "ERR|Login fallito dopo 3 tentativi", 34) == 0) {
            printf("Hai esaurito i tentativi. Connessione terminata.\n");
            return 0;
        } else {
            printf("Login fallito: %s\n", login_resp);
            tentativi++;
        }
    }

    printf("Numero massimo di tentativi raggiunto. Uscita.\n");
    return 0;
}
