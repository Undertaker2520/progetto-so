#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_client.h"

extern void buildTicketMessage(char *dest, int max_length);

void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

void avviaMenuClient(int client_fd, const char *username) {
    char scelta[10], messaggio[512], response[8192];

    while (1) {
        printf("\n--- MENU CLIENT ---\n");
        printf("1. Inserisci un nuovo ticket\n");
        printf("2. Visualizza tutti i tuoi ticket\n");
        printf("3. Cerca un tuo ticket per ID\n");
        printf("4. Cerca tuoi ticket per titolo\n");
        printf("5. Cerca tuoi ticket per descrizione\n");
        printf("6. Cerca tuoi ticket per stato (Aperto/In Corso/Chiuso)\n");
        printf("0. Esci\n");
        printf("Scelta: ");
        fgets(scelta, sizeof(scelta), stdin);

        switch(scelta[0]){
            case '1': {
                printf("\nInserimento nuovo ticket:\n");
                memset(messaggio, 0, sizeof(messaggio));
                buildTicketMessage(messaggio, sizeof(messaggio));
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '2': {
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS_BY_USER|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '3': {
                char id_input[10];
                printf("Inserisci ID: ");
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = 0;
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_ID_AND_USER|%s|%s", id_input, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '4': {
                char titolo[100];
                printf("Inserisci una parola contenuta nel titolo: ");
                fgets(titolo, sizeof(titolo), stdin);
                titolo[strcspn(titolo, "\n")] = 0;
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_TITOLO_BY_USER|%s|%s", titolo, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '5': {
                char descrizione[200];
                printf("Inserisci una parola nella descrizione: ");
                fgets(descrizione, sizeof(descrizione), stdin);
                descrizione[strcspn(descrizione, "\n")] = 0;
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_DESCRIZIONE_BY_USER|%s|%s", descrizione, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '6': {
                char stato[32];
                printf("Inserisci stato (Aperto, In Corso, Chiuso): ");
                fgets(stato, sizeof(stato), stdin);
                stato[strcspn(stato, "\n")] = 0;
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_STATO_BY_USER|%s|%s", stato, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '0':
                printf("Uscita...\n");
                close(client_fd);
                return;

            default:
                printf("Scelta non valida.\n");
                break;
        }
    }
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