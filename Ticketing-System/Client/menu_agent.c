#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "menu_agent.h"

extern void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

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
        printf("0. Esci\n");
        printf("Scelta: ");
        fgets(scelta, sizeof(scelta), stdin);

        char id[10];
        switch(scelta[0]){
            case '1':
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            break;
            case '2':
                snprintf(messaggio, sizeof(messaggio), "GET_ALL_TICKETS_BY_AGENT|%s", username);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            break;
            case '3': 
                char agente[64];
                char ruolo_check[128];

                printf("ID del ticket da assegnare: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = 0; // rimuove newline

                while (1) {
                    printf("Nome dell'agente da assegnare: ");
                    fgets(agente, sizeof(agente), stdin);
                    agente[strcspn(agente, "\n")] = 0;

                    // Verifica che l'agente esista ed abbia il ruolo corretto
                    char check_msg[128];
                    snprintf(check_msg, sizeof(check_msg), "CHECK_USER_ROLE|%s", agente);
                    sendRequestaAndReveiveResponse(client_fd, check_msg, ruolo_check, sizeof(ruolo_check));

                    if (strncmp(ruolo_check, "OK|AGENTE", 9) == 0) {
                        break;  // Agente valido
                    } else {
                        printf("Utente non valido o non è un agente. Riprova.\n");
                    }
                }

                snprintf(messaggio, sizeof(messaggio), "UPDATE_ASSIGNED_AGENT|%s|%s", id, agente);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            case '4': 
                char nuovo_stato[100];

                printf("Inserisci ID del ticket da modificare: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = 0;

                char *stati_validi[] = {"Aperto", "In Corso", "Chiuso"};
                int stato_valido = 0;

                do {
                    printf("Nuovo stato (Aperto / In Corso / Chiuso): ");
                    fgets(nuovo_stato, sizeof(nuovo_stato), stdin);
                    nuovo_stato[strcspn(nuovo_stato, "\n")] = 0;

                    stato_valido = 0;
                    for (int i = 0; i < 3; i++) {
                        if (strcasecmp(nuovo_stato, stati_validi[i]) == 0) {
                            stato_valido = 1;
                            break;
                        }
                    }

                    if (!stato_valido) {
                        printf("Stato non valido. Riprova.\n");
                    }

                } while (!stato_valido);

                snprintf(messaggio, sizeof(messaggio), "UPDATE_TICKET_STATUS|%s|%s", id, nuovo_stato);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            case '5': 
                char nuova_priorita[100];

                printf("Inserisci ID del ticket da modificare: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = 0;

                char *priorita_valide[] = {"Alta", "Media", "Bassa"};
                int priorita_valida = 0;

                do {
                    printf("Nuova priorita' (Alta / Media / Bassa): ");
                    fgets(nuova_priorita, sizeof(nuova_priorita), stdin);
                    nuova_priorita[strcspn(nuova_priorita, "\n")] = 0;

                    priorita_valida = 0;
                    for (int i = 0; i < 3; i++) {
                        if (strcasecmp(nuova_priorita, priorita_valide[i]) == 0) {
                            priorita_valida = 1;
                            break;
                        }
                    }

                    if (!priorita_valida) {
                        printf("Priotia' non valida. Riprova.\n");
                    }

                } while (!stato_valido);

                snprintf(messaggio, sizeof(messaggio), "UPDATE_TICKET_PRIORITY|%s|%s", id, nuova_priorita);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;
            case '6': 
                char id_input[10];
                printf("Inserisci ID: ");
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = 0;
                snprintf(messaggio, sizeof(messaggio), "GET_TICKET_BY_ID|%s", id_input);
                sendRequestaAndReveiveResponse(client_fd, messaggio, response, sizeof(response));
                break;

            case '0':
                printf("Uscita...\n");
                close(client_fd);
                return;
            default:
                printf("Scelta non valida.\n");
                break;
        }
    }

    close(client_fd);
}
