#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_agent.h"

void startAgentMenu(int client_fd) {
    char scelta[10], messaggio[512];

    while (1) {
        printf("\n--- MENU AGENTE ---\n");
        printf("1. Visualizza tutti i ticket\n");
        printf("2. Modifica un ticket\n");
        printf("3. Assegna un agente ad un ticket\n");
        printf("0. Esci\n");
        printf("Scelta: ");
        fgets(scelta, sizeof(scelta), stdin);

        if (scelta[0] == '1') {
            strcpy(messaggio, "GET_ALL_TICKETS|");
            send(client_fd, messaggio, strlen(messaggio), 0);

            char risposta[8192] = {0};
            int bytes = recv(client_fd, risposta, sizeof(risposta) - 1, 0);
            if (bytes > 0) {
                risposta[bytes] = '\0';
                printf("%s\n", risposta);
            }

        } else if (scelta[0] == '2') {
            // TODO: Modifica di un ticket
            printf("Funzionalità di modifica ticket ancora non implementata.\n");

        } else if (scelta[0] == '3') {
            // TODO: Assegnazione di un agente
            printf("Funzionalità di assegnazione agente ancora non implementata.\n");

        } else if (scelta[0] == '0') {
            printf("Uscita...\n");
            break;
        } else {
            printf("Scelta non valida.\n");
        }
    }

    close(client_fd);
}
