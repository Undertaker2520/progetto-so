// include/ticket.h
#ifndef TICKET_H
#define TICKET_H

typedef enum { //Enum priorità
    PRIORITA_BASSA,
    PRIORITA_MEDIA,
    PRIORITA_ALTA
} Priorita;

typedef enum { //Enum stato
    STATO_APERTO,
    STATO_IN_CORSO,
    STATO_CHIUSO
} Stato;

// Definizione della struttura Ticket
typedef struct {
    int id;
    char titolo[100];
    char descrizione[256];
    char data_creazione[20]; // formato "YYYY-MM-DD"
    Priorita priorita;
    Stato stato;          // Aperto, In Corso, Chiuso
    char agente[50];        // Nome agente o "nessuno"
    char username[50];     // Nome utente del cliente che ha creato il ticket
} Ticket;

typedef enum {
    FIELD_TITOLO,
    FIELD_DESCRIZIONE,
    FIELD_STATO
} CampoRicerca;


int getTicketById(int id, Ticket *ticket);
int getAllTickets(char *buffer, size_t bufsize);
int getTicketsByUser(const char *username, char *buffer, size_t bufsize);
int readTicketByIdAndUser(int id, const char *username, Ticket *ticket);
int createNewTicket(const char *buffer, const char *username);
int searchTicketsByFieldByUser(const char *username, const char *keyword, CampoRicerca campo, char *buffer, size_t max_size);
int updateDescriptionAndTitle(int id, const char *username, const char *nuovo_titolo, const char *nuova_descrizione);
int getTicketsByAgent(const char *agent_username, char *buffer, size_t max_size);
int updateAssignedAgent(int id, const char *assigned_agent);
int updateStatus(int id, const char *new_status);
int updatePriority(int id, const char *new_priority);
const char* prioritaToString(Priorita p);
const char* statoToString(Stato s);
Priorita stringToPriorita(const char *s);
Stato stringToStato(const char *s);

#endif