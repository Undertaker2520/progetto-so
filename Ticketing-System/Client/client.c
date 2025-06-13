// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void createSocket(int *client_fd);
void configureAddress(struct sockaddr_in *address);
void connectToServer(int client_fd, struct sockaddr_in *address);


int main(){
    int client_fd;
    struct sockaddr_in address;

    createSocket(&client_fd);

    configureAddress(&address);
    // Connessione al server
    connectToServer(client_fd, &address);
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

