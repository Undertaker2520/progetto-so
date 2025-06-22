#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_agent.h"
#include "utils_client.h"

void startAgentMenu(int client_fd, const char *username) {
    char scelta[10], messaggio[512], response[8192];

    while (1) {
        printf("\n--- MENU AGENTE ---\n");
        printf("1. Visualizza tutti i ticket\n");
        printf("2. Visualizza tutti i ticket assegnati a te\n");
        printf("3. Assegna un agente ad un ticket\n");
        printf("4. Modifica stato di un ticket\n");
        printf("5. Modifica priorita' di un ticket\n");
        printf("6. Cerca un ticket per id\n");
        printf("0. Esci\nScelta: ");
        fgets(scelta, sizeof(scelta), stdin);

        char id[10];
        switch(scelta[0]) {
            case '1':
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;

            case '2':
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS_BY_AGENT|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;

            case '3': {
                char ticket_id[10], agente[64], ruolo_check[128], risposta_agente[512];

                readInput("ID del ticket da assegnare: ", ticket_id, sizeof(ticket_id));

                while (1) {
                    readInput("Mail dell'agente da assegnare: ", agente, sizeof(agente));
                    snprintf(messaggio, sizeof(messaggio), "CHECK_USER_ROLE|%s", agente);
                    sendRequestaAndReveiveResponse(client_fd, messaggio, ruolo_check, sizeof(ruolo_check));
                    if (strncmp(ruolo_check, "OK|AGENTE", 9) == 0) break;
                    printf("Utente non valido o non è un agente. Riprova.\n");
                }

                snprintf(messaggio, sizeof(messaggio), "UPDATE_ASSIGNED_AGENT|%s|%s", ticket_id, agente);
                sendRequestaAndReveiveResponse(client_fd, messaggio, risposta_agente, sizeof(risposta_agente));
                printf("%s\n", risposta_agente);
                break;
            }

            case '4': {
                char nuovo_stato[100];
                const char *stati_validi[] = {"Aperto", "In Corso", "Chiuso"};

                readInput("Inserisci ID del ticket da modificare: ", id, sizeof(id));

                do {
                    readInput("Nuovo stato (Aperto / In Corso / Chiuso): ", nuovo_stato, sizeof(nuovo_stato));
                    if (!isValidInput(nuovo_stato, stati_validi, 3)) {
                        printf("Stato non valido. Riprova.\n");
                    }
                } while (!isValidInput(nuovo_stato, stati_validi, 3));

                snprintf(messaggio, sizeof(messaggio), "UPDATE_TICKET_STATUS|%s|%s", id, nuovo_stato);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '5': {
                char nuova_priorita[100];
                const char *priorita_valide[] = {"Alta", "Media", "Bassa"};

                readInput("Inserisci ID del ticket da modificare: ", id, sizeof(id));

                do {
                    readInput("Nuova priorita' (Alta / Media / Bassa): ", nuova_priorita, sizeof(nuova_priorita));
                    if (!isValidInput(nuova_priorita, priorita_valide, 3)) {
                        printf("Priorità non valida. Riprova.\n");
                    }
                } while (!isValidInput(nuova_priorita, priorita_valide, 3));

                snprintf(messaggio, sizeof(messaggio), "UPDATE_TICKET_PRIORITY|%s|%s", id, nuova_priorita);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            }

            case '6': {
                char id_input[10];
                readInput("Inserisci ID: ", id_input, sizeof(id_input));
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_ID|%s", id_input);
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