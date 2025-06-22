#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ticket.h"
#include <sys/socket.h>

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
    t->priorita = stringToPriorita(tok);

    tok = strtok(NULL, "|"); if (!tok) return -1;
     t->stato = stringToStato(tok);

    tok = strtok(NULL, "|"); if (!tok) return -1;
    strncpy(t->agente, tok, sizeof(t->agente)-1);

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
        int written = snprintf(
            temp, sizeof(temp),
            "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\nCreatore: %s\n",
            ticket.id,
            ticket.titolo,
            ticket.descrizione,
            ticket.data_creazione,
            prioritaToString(ticket.priorita),
            statoToString(ticket.stato),
            ticket.agente,
            ticket.username
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
    FILE *fp = fopen(TICKET_FILE, "r");
    if (!fp) return -1;

    buffer[0] = '\0';  // pulisci il buffer
    Ticket t;
    int count = 0;

    while (fread(&t, sizeof(Ticket), 1, fp)) {
        if (strcmp(t.username, username) == 0) {
            char temp[1024];
            snprintf(temp, sizeof(temp),
                "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\nCreatore: %s\n\n",
                t.id, t.titolo, t.descrizione, t.data_creazione, prioritaToString(t.priorita), statoToString(t.stato), t.agente, t.username);

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

int getTicketsByAgent(const char *agent_username, char *buffer, size_t max_size) {
    FILE *fp = fopen(TICKET_FILE, "r");
    if (!fp) return -1;

    buffer[0] = '\0';  // pulisci il buffer
    Ticket t;
    int count = 0;

    while (fread(&t, sizeof(Ticket), 1, fp)) {
        if (strcmp(t.agente, agent_username) == 0) {
            char temp[1024];
            snprintf(temp, sizeof(temp),
                "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\nCreatore: %s\n",
                t.id, t.titolo, t.descrizione, t.data_creazione, prioritaToString(t.priorita), statoToString(t.stato), t.agente, t.username);

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


int readTicketByIdAndUser(int id, const char *username, Ticket *t) {
    FILE *fp = fopen(TICKET_FILE, "rb");  // ✅ modalità binaria
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
        if (strcmp(t.username, username) != 0) continue;

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
            formatTicket(&t, temp, sizeof(temp));
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

void formatTicket(const Ticket *t, char *dest, size_t maxlen) {
    snprintf(dest, maxlen,
        "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\n\n",
        t->id,
        t->titolo,
        t->descrizione,
        t->data_creazione,
        prioritaToString(t->priorita),
        statoToString(t->stato),
        t->agente
    );
}

//Aggiornamento agente
static const char *_stored_agent;
static int updateAgentFn(Ticket *t) {
    strncpy(t->agente, _stored_agent, sizeof(t->agente) - 1);
    t->agente[sizeof(t->agente) - 1] = '\0';
    return 0;
}
int updateAssignedAgent(int id, const char *assigned_agent) {
    _stored_agent = assigned_agent;
    return updateTicketField(id, updateAgentFn);
}

//Aggiornamento Titolo e Descrizione
static const char *_stored_title;
static const char *_stored_descr;
static const char *_stored_user;
static int updateTitleDescrFn(Ticket *t) {
    if (strcmp(t->username, _stored_user) != 0)
        return -1;

    strncpy(t->titolo, _stored_title, sizeof(t->titolo) - 1);
    strncpy(t->descrizione, _stored_descr, sizeof(t->descrizione) - 1);
    return 0;
}
int updateDescriptionAndTitle(int id, const char *username, const char *titolo, const char *descrizione) {
    _stored_user = username;
    _stored_title = titolo;
    _stored_descr = descrizione;
    return updateTicketField(id, updateTitleDescrFn);
}


//Aggiornamento status
static const char *_stored_status;
static int updateStatusFn(Ticket *t) {
    t->stato = stringToStato(_stored_status);
    return 0;
}
int updateStatus(int id, const char *new_status) {
    _stored_status = new_status;
    return updateTicketField(id, updateStatusFn);
}

//Aggiornamento priorita'
static const char *_stored_priority;
static int updatePriorityFn(Ticket *t) {
    t->priorita = stringToPriorita(_stored_priority);
    return 0;
}
int updatePriority(int id, const char *priority) {
    _stored_priority = priority;
    return updateTicketField(id, updatePriorityFn);
}


int updateTicketField(int id, int(*updateFn)(Ticket *)){
    FILE *file = fopen(TICKET_FILE, "r+b");
    if(!file) return -1;

    Ticket t;
    while(fread(&t, sizeof(Ticket), 1, file)){
        if(t.id == id){
            if (!updateFn(&t)) {
                fseek(file, -sizeof(Ticket), SEEK_CUR);
                fwrite(&t, sizeof(Ticket), 1, file);
                fclose(file);
                return 0;
            } else {
                fclose(file);
                return -1; // fallimento callback
            }
        }
    }

    fclose(file);
    return -2; // non trovato
}