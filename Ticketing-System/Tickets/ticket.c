

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ticket.h"

// Percorso file con ticket
#define TICKET_FILE "tickets.db"

//generazione nuovo ID leggendo da file
int generateNewTicketId() {
    FILE *file = fopen(TICKET_FILE, "r");
    if (!file) {
        return 1; // Se il file non esiste, inizia da 1
    }

    int id = 0;
    Ticket ticket;
    while (fread(&ticket, sizeof(Ticket), 1, file)) {
        if (ticket.id > id) {
            id = ticket.id;
        }
    }
    fclose(file);
    return id + 1; // Incrementa l'ID massimo trovato
}

// Ottiene la data corrente in formato "YYYY-MM-DD"
void getCurrentDate(char *date) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

//Salva ticket su file, ritorna 0 se successo, -1 se errore
int saveTicket(Ticket *ticket) {
    FILE *file = fopen(TICKET_FILE, "ab");
    if (!file) {
        perror("Errore nell'apertura del file");
        return -1;
    }

    // Scrive il ticket nel file
    if (fwrite(ticket, sizeof(Ticket), 1, file) != 1) {
        perror("Errore nella scrittura del ticket");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

// Converte da stringa (pipe-separated) a `Ticket`
int parse_ticket_string(const char *s, Ticket *t) {
    char tmp[512];
    strncpy(tmp, s, 511);
    tmp[511] = '\0';
    char *tok = strtok(tmp, "|");
    if (!tok) return -1;
    t->id = atoi(tok);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->titolo, tok, sizeof(t->titolo)-1);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->descrizione, tok, sizeof(t->descrizione)-1);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->data_creazione, tok, sizeof(t->data_creazione)-1);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->priorita, tok, sizeof(t->priorita)-1);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->stato, tok, sizeof(t->stato)-1);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->agente, tok, sizeof(t->agente)-1);

    return 0;
}