#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "utils_client.h"

typedef enum {
    TITOLO,
    DESCRIZIONE,
    PRIORITA
} CampoTicket;

static void ticketComponentWriter(CampoTicket campo, char *dest, int max_length) {
    char input[256];
    const char *nome_campo = (campo == TITOLO) ? "titolo" :
                              (campo == DESCRIZIONE) ? "descrizione" : "priorità";
    printf("Inserisci il %s del ticket: ", nome_campo);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if ((int)strlen(input) > max_length) {
        fprintf(stderr, "Errore: Il %s supera la lunghezza massima di %d caratteri.\n", nome_campo, max_length);
        exit(EXIT_FAILURE);
    }

    strncpy(dest, input, max_length);
}

void buildTicketMessage(char *dest, int max_length) {
    char titolo[100], descrizione[256], priorita[10];
    ticketComponentWriter(TITOLO, titolo, sizeof(titolo));
    ticketComponentWriter(DESCRIZIONE, descrizione, sizeof(descrizione));

    char *priorita_valide[] = {"Alta", "Media", "Bassa"};
    int priorita_valida = 0;

    do {
        printf("Inserisci la priorità del ticket (Alta / Media / Bassa): ");
        fgets(priorita, sizeof(priorita), stdin);
        priorita[strcspn(priorita, "\n")] = 0;

        for (int i = 0; i < 3; i++) {
            if (strcasecmp(priorita, priorita_valide[i]) == 0) {
                priorita_valida = 1;
                break;
            }
        }

        if (!priorita_valida)
            printf("Priorità non valida. Riprova.\n");

    } while (!priorita_valida);

    snprintf(dest, max_length, "NEW_TICKET|%s|%s|%s", titolo, descrizione, priorita);
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
