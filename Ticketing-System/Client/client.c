#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "menu_client.h"
#include "menu_agent.h"
#include "utils_client.h"

#define MAX_INPUT 256

void createSocket(int *client_fd);
void configureAddress(struct sockaddr_in *address);
void connectToServer(int client_fd, struct sockaddr_in *address);
int recv_until(int sockfd, char *buffer, size_t maxlen, char delim);

int main() {
    int client_fd;
    struct sockaddr_in address;

    createSocket(&client_fd);
    configureAddress(&address);
    connectToServer(client_fd, &address);

    char username[64], password[64], ruolo[16] = {0};
    int tentativi = 0;

    while (tentativi < 3) {
        readInput("Login\nUsername(mail): ", username, sizeof(username));
        readInput("Password: ", password, sizeof(password));

        char login_msg[256];
        snprintf(login_msg, sizeof(login_msg), "LOGIN|%s|%s", username, password);
        send(client_fd, login_msg, strlen(login_msg), 0);

        char login_resp[128];
        int bytes = recv(client_fd, login_resp, sizeof(login_resp) - 1, 0);
        if (bytes <= 0) {
            perror("Errore nella comunicazione con il server");
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