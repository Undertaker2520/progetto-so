#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_client.h"

// buildTicketMessage è definita in client.c, la referenziamo qui
extern void buildTicketMessage(char *dest, int max_length);

void avviaMenuClient(int client_fd) {
    char scelta[10], messaggio[512];

    while (1) {
        printf("\n--- MENU CLIENT ---\n");
        printf("1. Inserisci un nuovo ticket\n");
        printf("2. Visualizza tutti i ticket\n");
        printf("3. Cerca un ticket per ID\n");
        printf("0. Esci\n");
        printf("Scelta: ");
        fgets(scelta, sizeof(scelta), stdin);
    
        if (scelta[0] == '1') {
            printf("\nInserimento nuovo ticket:\n");
            memset(messaggio, 0, sizeof(messaggio));
            buildTicketMessage(messaggio, sizeof(messaggio));
            send(client_fd, messaggio, strlen(messaggio), 0);

            char risposta[2048] = {0};
            recv(client_fd, risposta, sizeof(risposta) - 1, 0);
            printf("Risposta: %s\n", risposta);

        } else if (scelta[0] == '2') {
            printf("\nVisualizzazione di tutti i ticket: \n");
            strcpy(messaggio, "GET_ALL_TICKETS");
            send(client_fd, messaggio, strlen(messaggio), 0);

            char risposta[8192] = {0};
            int bytes = recv(client_fd, risposta, sizeof(risposta) - 1, 0);
            if (bytes > 0) {
                risposta[bytes] = '\0';
                printf("%s\n", risposta);
            }

        } else if (scelta[0] == '3') {
            printf("\nCerca un ticket per ID: \n");
            char id_input[10];
            printf("Inserisci ID: ");
            fgets(id_input, sizeof(id_input), stdin);
            id_input[strcspn(id_input, "\n")] = 0;

            snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_ID|%s", id_input);
            send(client_fd, messaggio, strlen(messaggio), 0);

            char risposta[1024] = {0};
            int bytes = recv(client_fd, risposta, sizeof(risposta) - 1, 0);
            if (bytes > 0) {
                risposta[bytes] = '\0';
                printf("%s\n", risposta);
            }

        } else if (scelta[0] == '0') {
            printf("Uscita...\n");
            break;
        } else {
            printf("Scelta non valida.\n");
        }
    }

    close(client_fd);
}