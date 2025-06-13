// include/ticket.h
#ifndef TICKET_H
#define TICKET_H

// Definizione della struttura Ticket
typedef struct {
    int id;
    char titolo[100];
    char descrizione[256];
    char data_creazione[20]; // formato "YYYY-MM-DD"
    char priorita[10];       // Alta, Media, Bassa
    char stato[20];          // Aperto, In Corso, Chiuso
    char agente[50];         // Nome agente o "nessuno"
} Ticket;

#endif
