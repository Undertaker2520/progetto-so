#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ticket.h"
#include <sys/socket.h>

// Percorso file con ticket
#define TICKET_FILE "tickets.db"
typedef int (*TicketFilterFn)(const Ticket *, const char *);

//definizione funzioni usate solo internamente a ticket.c
static int generateNewTicketId();
static void getCurrentDate(char *date);
static int saveTicket(Ticket *ticket);
static void ticketToString(const Ticket *t, char *dest, size_t maxlen);
static int rewriteTicket(FILE *file, const Ticket *t);
static int getTicketsByPredicate(TicketFilterFn filter, const char *value, char *buffer, size_t max_size);
static int isUserTicket(const Ticket *t, const char *user);
static int isAgentTicket(const Ticket *t, const char *agent);
static void cleanString(char *s, size_t len);

//generazione nuovo ID leggendo da file
static int generateNewTicketId() {
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
static void getCurrentDate(char *date) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

//Salva ticket su file, ritorna 0 se successo, -1 se errore
static int saveTicket(Ticket *ticket) {
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

int getTicketById(int id, Ticket *ticket) {
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
        ticketToString(&ticket, temp, sizeof(temp));
        int written = strlen(temp);

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

int createNewTicket(const char *buffer, const char *username){
    Ticket t;
    char temp[1024];
    strncpy(temp, buffer, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *titolo = strtok(temp + 11, "|");
    char *descrizione = strtok(NULL, "|");
    char *priorita = strtok(NULL, "|");

    if (titolo && descrizione && priorita) {
        t.id = generateNewTicketId();
        strncpy(t.titolo, titolo, sizeof(t.titolo) - 1);
        strncpy(t.descrizione, descrizione, sizeof(t.descrizione) - 1);
        getCurrentDate(t.data_creazione);
        t.priorita = stringToPriorita(priorita);
        t.stato = STATO_APERTO;
        strncpy(t.agente, "nessuno", sizeof(t.agente) - 1);
        strncpy(t.username, username, sizeof(t.username) - 1);
        t.username[sizeof(t.username) - 1] = '\0';//mi assicuro che user non venga salvato con caratteri extra


        return saveTicket(&t) == 0 ? t.id : -1;
    }

    return -2; // errore di sintassi
}

int getTicketsByUser(const char *username, char *buffer, size_t max_size) {
    return getTicketsByPredicate(isUserTicket, username, buffer, max_size);
}

int getTicketsByAgent(const char *agent_username, char *buffer, size_t max_size) {
    return getTicketsByPredicate(isAgentTicket, agent_username, buffer, max_size);
}


int readTicketByIdAndUser(int id, const char *username, Ticket *t) {
    FILE *fp = fopen(TICKET_FILE, "rb"); 
    if (!fp) {
        perror("Errore apertura tickets.db");
        return -1;
    }

    while (fread(t, sizeof(Ticket), 1, fp)) {
        if (t->id == id && strcmp(t->username, username) == 0) {
            fclose(fp);
            return 0;  // trovato
        }
    }

    fclose(fp);
    return -1;  // non trovato o appartiene a un altro utente
}

const char* prioritaToString(Priorita p) {
    switch (p) {
        case PRIORITA_ALTA: return "Alta";
        case PRIORITA_MEDIA: return "Media";
        case PRIORITA_BASSA: return "Bassa";
        default: return "Sconosciuta";
    }
}

const char* statoToString(Stato s) {
    switch (s) {
        case STATO_APERTO: return "Aperto";
        case STATO_IN_CORSO: return "In Corso";
        case STATO_CHIUSO: return "Chiuso";
        default: return "Sconosciuto";
    }
}

Priorita stringToPriorita(const char *s) {
    if (strcasecmp(s, "Alta") == 0) return PRIORITA_ALTA;
    if (strcasecmp(s, "Media") == 0) return PRIORITA_MEDIA;
    if (strcasecmp(s, "Bassa") == 0) return PRIORITA_BASSA;
    return PRIORITA_BASSA; // default
}

Stato stringToStato(const char *s) {
    if (strcasecmp(s, "Aperto") == 0) return STATO_APERTO;
    if (strcasecmp(s, "In Corso") == 0) return STATO_IN_CORSO;
    if (strcasecmp(s, "Chiuso") == 0) return STATO_CHIUSO;
    return STATO_APERTO; // default
}


int searchTicketsByFieldByUser(const char *username, const char *keyword, CampoRicerca campo, char *buffer, size_t max_size) {
    FILE *file = fopen(TICKET_FILE, "rb");
    if (!file) return -1;

    buffer[0] = '\0';
    Ticket t;
    int count = 0;

    while (fread(&t, sizeof(Ticket), 1, file)) {    

        if (strcasecmp(t.username, username) != 0) continue; //confronto non case sensitive


        int match = 0;
        switch (campo) {
            case FIELD_TITOLO:
                match = strcasestr(t.titolo, keyword) != NULL;
                break;
            case FIELD_DESCRIZIONE:
                match = strcasestr(t.descrizione, keyword) != NULL;
                break;
            case FIELD_STATO:
                match = strcasecmp(statoToString(t.stato), keyword) == 0;
                break;
        }


        if (match) {
            char temp[512];
            ticketToString(&t, temp, sizeof(temp));
            if (strlen(buffer) + strlen(temp) < max_size) {
                strcat(buffer, temp);
                count++;
            } else {
                break;
            }
        }
    }

    fclose(file);
    return count;
}

int updateAssignedAgent(int id, const char *assigned_agent){
    FILE *file = fopen(TICKET_FILE, "r+b"); // lettura + scrittura binaria
    if (!file) return -1;

    Ticket t;
    while (fread(&t, sizeof(Ticket), 1, file)) {
        if (t.id == id) {
            // Aggiorna i campi
            strncpy(t.agente, assigned_agent, sizeof(t.agente) - 1);
            t.agente[sizeof(t.agente) - 1] = '\0';  // assicura il terminatore di stringa
            
            if (rewriteTicket(file, &t) == 0) {
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -2; // non trovato o non autorizzato
}

int updateDescriptionAndTitle(int id, const char *username, const char *nuovo_titolo, const char *nuova_descrizione){
    FILE *file = fopen(TICKET_FILE, "r+b"); // lettura + scrittura binaria
    if (!file) return -1;

    Ticket t;
    while (fread(&t, sizeof(Ticket), 1, file)) {
        char clean_username[64];
        memcpy(clean_username, t.username, sizeof(t.username));
        //normalizzazione input utente per confronto 
        cleanString(clean_username, sizeof(clean_username));        

        if (t.id == id && strcmp(clean_username, username) == 0) {
            // Aggiorna i campi
            strncpy(t.titolo, nuovo_titolo, sizeof(t.titolo) - 1);
            strncpy(t.descrizione, nuova_descrizione, sizeof(t.descrizione) - 1);

            if (rewriteTicket(file, &t) == 0) {
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -2; // non trovato 
}

int updateStatus(int id, const char *new_status){
    FILE *file = fopen(TICKET_FILE, "r+b"); // lettura + scrittura binaria
    if (!file) return -1;

    Ticket t;
    while (fread(&t, sizeof(Ticket), 1, file)) {
        if (t.id == id) {
            // Aggiorna i campi
            t.stato = stringToStato(new_status);

            if (rewriteTicket(file, &t) == 0) {
                fclose(file);
                return 0;
            }

        }
    }

    fclose(file);
    return -2; // non trovato o non autorizzato
}

int updatePriority(int id, const char *new_priority){
    FILE *file = fopen(TICKET_FILE, "r+b"); // lettura + scrittura binaria
    if (!file) return -1;

    Ticket t;
    while (fread(&t, sizeof(Ticket), 1, file)) {
        if (t.id == id) {
            // Aggiorna i campi
            t.priorita = stringToPriorita(new_priority);
            
            if (rewriteTicket(file, &t) == 0) {
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -2; // non trovato o non autorizzato
}

static void ticketToString(const Ticket *t, char *dest, size_t maxlen) {
    snprintf(dest, maxlen,
        "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\nCreatore: %s\n",
        t->id,
        t->titolo,
        t->descrizione,
        t->data_creazione,
        prioritaToString(t->priorita),
        statoToString(t->stato),
        t->agente,
        t->username
    );
}

//normalizzazione input utente
static void cleanString(char *s, size_t len) {
    s[len - 1] = '\0';
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '\n' || s[i] == '\r') {
            s[i] = '\0';
            break;
        }
    }
}

static int rewriteTicket(FILE *file, const Ticket *t) {
    // Riporta il puntatore indietro in modo da sovrascrivere il ticket 
    fseek(file, -sizeof(Ticket), SEEK_CUR);
    return fwrite(t, sizeof(Ticket), 1, file) == 1 ? 0 : -1;
}

static int getTicketsByPredicate(TicketFilterFn filter, const char *value, char *buffer, size_t max_size) {
    FILE *fp = fopen(TICKET_FILE, "rb");
    if (!fp) return -1;

    buffer[0] = '\0';
    Ticket t;
    int count = 0;

    while (fread(&t, sizeof(Ticket), 1, fp)) {
        if (filter(&t, value)) {
            char temp[512];
            ticketToString(&t, temp, sizeof(temp));
            if (strlen(buffer) + strlen(temp) < max_size) {
                strcat(buffer, temp);
                count++;
            } else {
                break;
            }
        }
    }

    fclose(fp);
    return count;
}

static int isUserTicket(const Ticket *t, const char *user) {
    return strcmp(t->username, user) == 0;
}

static int isAgentTicket(const Ticket *t, const char *agent) {
    return strcmp(t->agente, agent) == 0;
}