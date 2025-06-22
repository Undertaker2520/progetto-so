#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_agent.h"
#include "utils_client.h"

extern void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

void promptAndSend(int client_fd, const char *format, const char *id_prompt, const char *value_prompt, const char *command_name) {
    char id[10], value[64], message[256], response[8192];
    printf("%s", id_prompt);
    fgets(id, sizeof(id), stdin); id[strcspn(id, "\n")] = 0;

    do {
        printf("%s", value_prompt);
        fgets(value, sizeof(value), stdin); value[strcspn(value, "\n")] = 0;
    } while (strlen(value) == 0);

    snprintf(message, sizeof(message), format, id, value);
    sendRequestaAndReveiveResponse(client_fd, message, response, sizeof(response));
}

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

                printf("ID del ticket da assegnare: ");
                fgets(ticket_id, sizeof(ticket_id), stdin); ticket_id[strcspn(ticket_id, "\n")] = 0;

                while (1) {
                    printf("Nome dell'agente da assegnare: ");
                    fgets(agente, sizeof(agente), stdin); agente[strcspn(agente, "\n")] = 0;

                    char check_msg[128];
                    snprintf(check_msg, sizeof(check_msg), "CHECK_USER_ROLE|%s", agente);
                    sendRequestaAndReveiveResponse(client_fd, check_msg, ruolo_check, sizeof(ruolo_check));

                    if (strncmp(ruolo_check, "OK|AGENTE", 9) == 0) break;
                    printf("Utente non valido o non è un agente. Riprova.\n");
                }

                while (1) {
                    snprintf(messaggio, sizeof(messaggio), "UPDATE_ASSIGNED_AGENT|%s|%s", ticket_id, agente);
                    sendRequestaAndReveiveResponse(client_fd, messaggio, risposta_agente, sizeof(risposta_agente));

                    if (strncmp(risposta_agente, "OK|Modifica completata", 23) == 0) {
                        printf("Assegnazione completata con successo.\n");
                        break;
                    } else if (strncmp(risposta_agente, "OK|Ticket non trovato", 22) == 0) {
                        printf("ID non valido. Riprova.\nID del ticket da assegnare: ");
                        fgets(ticket_id, sizeof(ticket_id), stdin);
                        ticket_id[strcspn(ticket_id, "\n")] = 0;
                    } else {
                        printf("Errore: %s\n", risposta_agente);
                        break;
                    }
                }
                break;
            }
            case '4':
                promptAndSend(client_fd, "UPDATE_TICKET_STATUS|%s|%s", "Inserisci ID del ticket: ", "Nuovo stato: ", "UPDATE_TICKET_STATUS");
                break;
            case '5':
                promptAndSend(client_fd, "UPDATE_TICKET_PRIORITY|%s|%s", "Inserisci ID del ticket: ", "Nuova priorità: ", "UPDATE_TICKET_PRIORITY");
                break;
            case '6': {
                char id_input[10];
                printf("Inserisci ID: ");
                fgets(id_input, sizeof(id_input), stdin); id_input[strcspn(id_input, "\n")] = 0;
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