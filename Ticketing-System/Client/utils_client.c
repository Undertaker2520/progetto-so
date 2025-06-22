// utils_client.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils_client.h"
#include <sys/socket.h>

static void ticketComponentWriter(CampoTicket campo, char *dest, int max_length); //usata solo internamente ad utils non visibile da header e quindi da altri importatori

void readInput(const char *prompt, char *dest, size_t size) {
    printf("%s", prompt);
    if (fgets(dest, size, stdin)) {
        dest[strcspn(dest, "\n")] = '\0';  // Rimuove newline
    }
}

int isValidInput(const char *input, const char *validi[], int count) {
    for (int i = 0; i < count; i++) {
        if (strcasecmp(input, validi[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void buildTicketMessage(char *dest, int max_length) {
    char titolo[100], descrizione[256], priorita[20];
    const char *priorita_valide[] = {"Alta", "Media", "Bassa"};

    ticketComponentWriter(TITOLO, titolo, sizeof(titolo));
    ticketComponentWriter(DESCRIZIONE, descrizione, sizeof(descrizione));

    do {
        readInput("Inserisci la priorità del ticket (Alta / Media / Bassa): ", priorita, sizeof(priorita));
        if (!isValidInput(priorita, priorita_valide, 3)) {
            printf("Priorità non valida. Riprova.\n");
        }
    } while (!isValidInput(priorita, priorita_valide, 3));

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

static void ticketComponentWriter(CampoTicket campo, char *dest, int max_length) {
    const char *nome_campo = (campo == TITOLO) ? "titolo" : (campo == DESCRIZIONE) ? "descrizione" : "priorit\u00e0";
    char prompt[100];
    snprintf(prompt, sizeof(prompt), "Inserisci il %s del ticket: ", nome_campo);
    readInput(prompt, dest, max_length);
    if (strlen(dest) > (size_t)max_length) {
        fprintf(stderr, "Errore: Il %s supera la lunghezza massima di %d caratteri.\n", nome_campo, max_length);
        exit(EXIT_FAILURE);
    }
}