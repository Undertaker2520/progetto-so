#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_client.h"
#include "utils_client.h"

void startClientMenu(int client_fd, const char *username) {
    char scelta[10], messaggio[512], response[8192];

    while (1) {
        printf("\n--- MENU CLIENT ---\n");
        printf("1. Inserisci un nuovo ticket\n");
        printf("2. Visualizza tutti i tuoi ticket\n");
        printf("3. Cerca un tuo ticket per ID\n");
        printf("4. Cerca tuoi ticket per titolo\n");
        printf("5. Cerca tuoi ticket per descrizione\n");
        printf("6. Cerca tuoi ticket per stato (Aperto/In Corso/Chiuso)\n");
        printf("7. Modifica titolo e descrizione di un tuo ticket\n");
        printf("0. Esci\nScelta: ");
        fgets(scelta, sizeof(scelta), stdin);

        switch(scelta[0]) {
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
                char id[10];
                readInput("Inserisci ID: ", id, sizeof(id));
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_ID_AND_USER|%s|%s", id, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }
            case '4': {
                char titolo[100];
                readInput("Inserisci una parola contenuta nel titolo: ", titolo, sizeof(titolo));
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_TITOLO_BY_USER|%s|%s", titolo, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }
            case '5': {
                char descrizione[200];
                readInput("Inserisci una parola nella descrizione: ", descrizione, sizeof(descrizione));
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_DESCRIZIONE_BY_USER|%s|%s", descrizione, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }
            case '6': {
                char stato[32];
                const char *validi[] = {"Aperto", "In Corso", "Chiuso"};
                do {
                    readInput("Inserisci stato (Aperto, In Corso, Chiuso): ", stato, sizeof(stato));
                    if (!isValidInput(stato, validi, 3)) {
                        printf("Stato non valido. Riprova.\n");
                    }
                } while (!isValidInput(stato, validi, 3));

                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_STATO_BY_USER|%s|%s", stato, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }
            case '7': {
                char id[10], nuovo_titolo[100], nuova_descrizione[256];

                readInput("Inserisci ID del ticket da modificare: ", id, sizeof(id));
                readInput("Nuovo titolo: ", nuovo_titolo, sizeof(nuovo_titolo));
                readInput("Nuova descrizione: ", nuova_descrizione, sizeof(nuova_descrizione));

                snprintf(messaggio, sizeof(messaggio), "UPDATE_YOUR_TICKET|%s|%s|%s", id, nuovo_titolo, nuova_descrizione);
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