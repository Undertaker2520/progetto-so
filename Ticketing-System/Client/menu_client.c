#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_client.h"
#include "utils_client.h"


extern void buildTicketMessage(char *dest, int max_length);
extern void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

static void searchAndSend(int client_fd, const char *prompt, const char *command, const char *username) {
    char input[128], message[512], response[8192];
    printf("%s", prompt);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    snprintf(message, sizeof(message), "%s|%s|%s", command, input, username);
    sendRequestaAndReveiveResponse(client_fd, message, response, sizeof(response));
}

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
            case '1':
                printf("\nInserimento nuovo ticket:\n");
                memset(messaggio, 0, sizeof(messaggio));
                buildTicketMessage(messaggio, sizeof(messaggio));
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            case '2':
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS_BY_USER|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            case '3':
                searchAndSend(client_fd, "Inserisci ID: ", "GET_TICKET_BY_ID_AND_USER", username);
                break;
            case '4':
                searchAndSend(client_fd, "Inserisci una parola contenuta nel titolo: ", "GET_TICKET_BY_TITOLO_BY_USER", username);
                break;
            case '5':
                searchAndSend(client_fd, "Inserisci una parola nella descrizione: ", "GET_TICKET_BY_DESCRIZIONE_BY_USER", username);
                break;
            case '6': {
                char stato[32];
                char *stati_validi[] = {"Aperto", "In Corso", "Chiuso"};
                int stato_valido = 0;
                do {
                    printf("Inserisci stato (Aperto, In Corso, Chiuso): ");
                    fgets(stato, sizeof(stato), stdin);
                    stato[strcspn(stato, "\n")] = 0;
                    for (int i = 0; i < 3; i++) {
                        if (strcasecmp(stato, stati_validi[i]) == 0) {
                            stato_valido = 1;
                            break;
                        }
                    }
                    if (!stato_valido) printf("Stato non valido. Riprova.\n");
                } while (!stato_valido);

                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_STATO_BY_USER|%s|%s", stato, username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }
            case '7': {
                char id[10], nuovo_titolo[100], nuova_descrizione[256];
                printf("Inserisci ID del ticket da modificare: ");
                fgets(id, sizeof(id), stdin); id[strcspn(id, "\n")] = 0;
                printf("Nuovo titolo: ");
                fgets(nuovo_titolo, sizeof(nuovo_titolo), stdin); nuovo_titolo[strcspn(nuovo_titolo, "\n")] = 0;
                printf("Nuova descrizione: ");
                fgets(nuova_descrizione, sizeof(nuova_descrizione), stdin); nuova_descrizione[strcspn(nuova_descrizione, "\n")] = 0;
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
