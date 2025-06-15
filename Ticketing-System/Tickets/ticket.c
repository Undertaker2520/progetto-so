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

int readTicketById(int id, Ticket *ticket) {
    FILE *file = fopen(TICKET_FILE, "rb");
    if (!file) {
        perror("Errore apertura file tickets.db");
        return -1;
    }

    while (fread(ticket, sizeof(Ticket), 1, file)) {
        if (ticket->id == id) {
            fclose(file);
            return 0; // trovato
        }
    }

    fclose(file);
    return -1; // non trovato
}

int getAllTickets(char *buffer, size_t bufsize) {
    FILE *file = fopen(TICKET_FILE, "rb");
    if (!file) {
        // File non esiste
        return -1;
    }

    Ticket ticket;
    int count = 0;
    size_t used = 0;

    while (fread(&ticket, sizeof(Ticket), 1, file)) {
        // Prepariamo una rappresentazione testuale di un singolo ticket
        char temp[512];
        int written = snprintf(
            temp, sizeof(temp),
            "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\n\n",
            ticket.id,
            ticket.titolo,
            ticket.descrizione,
            ticket.data_creazione,
            ticket.priorita,
            ticket.stato,
            ticket.agente
        );

        // Controlliamo se abbiamo spazio nel buffer
        if (used + written >= bufsize) {
            // Non c'è più spazio, interrompiamo
            break;
        }

        // Copia nel buffer
        memcpy(buffer + used, temp, written);
        used += written;
        count++;
    }

    fclose(file);

    if (count == 0) {
        return 0; // file presente ma nessun ticket
    }

    buffer[used] = '\0'; // terminatore di stringa
    return count;
}
