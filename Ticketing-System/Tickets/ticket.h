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
    char username[50];     // Nome utente del cliente che ha creato il ticket
} Ticket;

int generateNewTicketId();
void getCurrentDate(char *date);
int saveTicket(Ticket *ticket);
int parse_ticket_string(const char *ticket_string, Ticket *ticket);
int readTicketById(int id, Ticket *ticket);
int getAllTickets(char *buffer, size_t bufsize);
int getTicketsByUser(const char *username, char *buffer, size_t bufsize);
int readTicketByIdAndUser(int id, const char *username, Ticket *ticket);
int createNewTicket(const char *buffer, const char *username);
#endif
