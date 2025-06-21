// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Tickets/ticket.h" //includo solo questo includere il ticket.c e' cattiva pratica
#include "../Authentication/auth.h" //includo solo questo includere il auth.c e' cattiva pratica

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLIENTS 100


void createSocket(int *server_fd);
void configureAddress(struct sockaddr_in *address);
void binding(int server_fd, struct sockaddr *address, socklen_t addrlen);
void acceptConnections(int server_fd, int *new_socket);
int handleClientRequest(int socket);
void handleGetAllTickets(int socket, const char *buffer);
void handleGetTicketsByAgent(int socket, const char *buffer);
void handleGetAllTicketsByLoggedUser(int socket, const char *buffer);
void handleGetTicketByIdAndLoggedUser(int socket, const char *buffer);
void handleNewTicket(int socket, const char *buffer);
int handleLogin(int socket, const char *buffer);
int authenticateUser(const char *username, const char *password, char *ruolo);
void salvaSessione(int socket_fd, const char *username);
const char* getUsernameBySocket(int socket_fd);
void handleSearchByFieldByUser(int socket, const char *buffer, CampoRicerca campo);
void handleUpdateTicketDescriptionAndTitle(int socket, const char *buffer);
void handleUpdateAssignedAgent(int socket, const char *buffer);
void handleUpdateStatus(int socket, const char *buffer);
void handleCheckUserRole(int socket, const char *buffer);
void handleUpdatePriority(int socket, const char *buffer);
void handleGetTicketById(int socket, const char *buffer);

typedef enum {
    CMD_NEW_TICKET,
    CMD_GET_ALL_BY_USER,
    CMD_GET_BY_ID_BY_USER,
    CMD_BY_TITOLO_BY_USER,
    CMD_BY_DESCRIZIONE_BY_USER,
    CMD_BY_STATO_BY_USER,
    CMD_UNKNOWN,
    CMD_UPDATE_YOUR_TICKET,
    CMD_GET_ALL_TICKETS,
    CMD_GET_TICKETS_BY_AGENT,
    CMD_UPDATE_ASSIGNED_AGENT,
    CMD_UPDATE_STATUS,
    CMD_CHECK_USER_ROLE,
    CMD_LOGIN,
    CMD_UPDATE_TICKET_PRIORITY,
    CMD_GET_TICKET_BY_ID
} CommandType;

CommandType parseCommand(const char *buffer) {
    if (strncmp(buffer, "NEW_TICKET|", strlen("NEW_TICKET|")) == 0) return CMD_NEW_TICKET;
    if (strncmp(buffer, "GET_ALL_TICKETS_BY_USER|", strlen("GET_ALL_TICKETS_BY_USER|")) == 0) return CMD_GET_ALL_BY_USER;
    if (strncmp(buffer, "CHECK_USER_ROLE|", strlen("CHECK_USER_ROLE|")) == 0) return CMD_CHECK_USER_ROLE;
    if (strncmp(buffer, "GET_ALL_TICKETS|", strlen("GET_ALL_TICKETS|")) == 0) return CMD_GET_ALL_TICKETS;
    if (strncmp(buffer, "GET_ALL_TICKETS_BY_AGENT|", strlen("GET_ALL_TICKETS_BY_AGENT|")) == 0) return CMD_GET_TICKETS_BY_AGENT;
    if (strncmp(buffer, "GET_TICKET_BY_ID_AND_USER|", strlen("GET_TICKET_BY_ID_AND_USER|")) == 0) return CMD_GET_BY_ID_BY_USER;
    if (strncmp(buffer, "GET_TICKET_BY_TITOLO_BY_USER|", strlen("GET_TICKET_BY_TITOLO_BY_USER|")) == 0) return CMD_BY_TITOLO_BY_USER;
    if (strncmp(buffer, "GET_TICKET_BY_DESCRIZIONE_BY_USER|", strlen("GET_TICKET_BY_DESCRIZIONE_BY_USER|")) == 0) return CMD_BY_DESCRIZIONE_BY_USER;
    if (strncmp(buffer, "GET_TICKET_BY_STATO_BY_USER|", strlen("GET_TICKET_BY_STATO_BY_USER|")) == 0) return CMD_BY_STATO_BY_USER;
    if (strncmp(buffer, "UPDATE_YOUR_TICKET|", strlen("UPDATE_YOUR_TICKET|")) == 0) return CMD_UPDATE_YOUR_TICKET;
    if (strncmp(buffer, "UPDATE_TICKET_STATUS|", strlen("UPDATE_TICKET_STATUS|")) == 0) return CMD_UPDATE_STATUS;
    if (strncmp(buffer, "UPDATE_ASSIGNED_AGENT|", strlen("UPDATE_ASSIGNED_AGENT|")) == 0) return CMD_UPDATE_ASSIGNED_AGENT;
    if (strncmp(buffer, "UPDATE_TICKET_PRIORITY|", strlen("UPDATE_TICKET_PRIORITY|")) == 0) return CMD_UPDATE_TICKET_PRIORITY;
    if (strncmp(buffer, "GET_TICKET_BY_ID|", strlen("GET_TICKET_BY_ID|")) == 0) return CMD_GET_TICKET_BY_ID;
    if (strncmp(buffer, "LOGIN|", strlen("LOGIN")) == 0) return CMD_LOGIN;
    return CMD_UNKNOWN;
}

typedef struct {
    int socket_fd;
    char username[64];
} Session;

Session sessions[MAX_CLIENTS];
int session_count = 0;

int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // creazione socket ipv4 e socket TCP
    createSocket(&server_fd);

    // configurazione dell'indirizzo
    configureAddress(&address);

    // Binding del socket ad un indirizzo e porta specifici
    binding(server_fd, (struct sockaddr *)&address, sizeof(address));
    /* === FINE CODICE ORIGINALE === */

    // Ascolta le connessioni in arrivoF
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);
    
    while (1) {
        // Accetta una connessione in arrivo
        acceptConnections(server_fd, &new_socket);

        pid_t pid = fork(); // Crea un processo figlio per gestire il client
        if (pid < 0) {
            perror("Fork failed");
            close(new_socket); 
            continue; // Continua ad accettare nuove connessioni
        } else if (pid == 0) {  
            // Processo figlio: gestisce la connessione del client
            close(server_fd); // Il figlio non ha bisogno del socket del server
            while (1) {
            if (handleClientRequest(new_socket) == 0) {
                    break; // client ha chiuso la connessione o ha inviato un comando di uscita
                }
            }
            close(new_socket); // Chiude il socket del client
            printf("Connessione con il client chiusa.\n");
            exit(0); // Termina il processo figlio
        } else {
            // Processo padre: continua ad accettare nuove connessioni
            close(new_socket); // Il padre non ha bisogno del socket del client
            continue;
        }
        
        // Chiude la connessione con il client
        close(new_socket);

        printf("In attesa di una nuova connessione...\n");
    }

    return 0;
}

void createSocket(int *server_fd){
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // per evitare errori di "Address already in use" se riavvii il server rapidamente
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("Set socket options failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
}

void configureAddress(struct sockaddr_in *address) {
    int opt = 1;
    address->sin_family = AF_INET; // IPv4
    address->sin_addr.s_addr = INADDR_ANY; // Accetta connessioni da qualsiasi indirizzo
    address->sin_port = htons(PORT); // Imposta la porta
}

void binding(int server_fd, struct sockaddr *address, socklen_t addrlen) {
    if (bind(server_fd, address, addrlen) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void acceptConnections(int server_fd, int *new_socket) {
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    *new_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
    if (*new_socket < 0) {
        perror("Errore in accept");
        exit(EXIT_FAILURE);
    }

    printf("Connessione accettata da %s:%d\n",
        inet_ntoa(client_address.sin_addr),
        ntohs(client_address.sin_port));
}


int handleClientRequest(int socket) {
    char buffer[1024] = {0};
    int bytes_read = read(socket, buffer, BUF_SIZE);
    if (bytes_read <= 0) {
        perror("Read failed o connessione chiusa");
        return 0;
    }

    buffer[bytes_read] = '\0';
    printf("Messaggio ricevuto: %s\n", buffer);

    CommandType cmd = parseCommand(buffer);
    switch (parseCommand(buffer)) {
        case CMD_NEW_TICKET:
            handleNewTicket(socket, buffer);
            break;
        case CMD_GET_ALL_BY_USER:
            handleGetAllTicketsByLoggedUser(socket, buffer);
            break;
        case CMD_GET_BY_ID_BY_USER:
            handleGetTicketByIdAndLoggedUser(socket, buffer);
            break;
        case CMD_BY_TITOLO_BY_USER:
            handleSearchByFieldByUser(socket, buffer, FIELD_TITOLO);
            break;
        case CMD_BY_DESCRIZIONE_BY_USER:
            handleSearchByFieldByUser(socket, buffer, FIELD_DESCRIZIONE);
            break;
        case CMD_BY_STATO_BY_USER:
            handleSearchByFieldByUser(socket, buffer, FIELD_STATO);
            break;
        case CMD_LOGIN:
            if (handleLogin(socket, buffer) == 0)
                return 0;
            break;
        case CMD_GET_ALL_TICKETS:
            handleGetAllTickets(socket, buffer);
            break;
        case CMD_CHECK_USER_ROLE:
            handleCheckUserRole(socket, buffer);
            break;
        case CMD_GET_TICKETS_BY_AGENT:
            handleGetTicketsByAgent(socket, buffer);
            break;
        case CMD_UPDATE_YOUR_TICKET:
            handleUpdateTicketDescriptionAndTitle(socket, buffer);
            break;
        case CMD_UPDATE_STATUS:
            handleUpdateStatus(socket, buffer);
            break;
        case CMD_UPDATE_ASSIGNED_AGENT:
            handleUpdateAssignedAgent(socket, buffer);
            break;
        case CMD_UPDATE_TICKET_PRIORITY:
            handleUpdatePriority(socket, buffer);
            break;
        case CMD_GET_TICKET_BY_ID:
            handleGetTicketById(socket, buffer);
            break;
        default:
            send(socket, "ERR|Comando sconosciuto", 24, 0);
            break;
    }

    return 1;
}

void handleGetAllTicketsByLoggedUser(int socket, const char *buffer) {
    const char *username = buffer + strlen("GET_ALL_TICKETS_BY_USER|");
    char ticket_buffer[8192]; 
    int num_tickets = getTicketsByUser(username, ticket_buffer, sizeof(ticket_buffer));

    printf("Numero di ticket letti: %d\n", num_tickets);

    switch (num_tickets) {
        case -1:
            send(socket, "ERR|Errore lettura tickets", 27, 0);
            break;
        case 0:
            send(socket, "OK|Nessun ticket presente", 26, 0);
            break;
        default:
            send(socket, ticket_buffer, strlen(ticket_buffer), 0);
            break;
    }
}

void handleGetTicketByIdAndLoggedUser(int socket, const char *buffer) {
    // Estrai ID e username
    char copy[128];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy)-1] = '\0';

    char *token = strtok(copy + strlen("GET_TICKET_BY_ID_AND_USER|"), "|");
    char *username = strtok(NULL, "|");
    if (!token || !username) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(token);
    Ticket t;

    if (readTicketByIdAndUser(id, username, &t) == 0) {
        char risposta[1024];
        snprintf(risposta, sizeof(risposta),
            "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\n",
            t.id, t.titolo, t.descrizione, t.data_creazione,prioritaToString(t.priorita), statoToString(t.stato), t.agente
        );
        send(socket, risposta, strlen(risposta), 0);
    } else {
        send(socket, "ERR|Ticket non trovato", 23, 0);
    }
}

void handleNewTicket(int socket, const char *buffer) {
    
    const char *username = getUsernameBySocket(socket);

    if (!username) {
        send(socket, "ERR|Sessione non trovata", 24, 0);
        return;
    }
    int result = createNewTicket(buffer, username); // "nessuno" come username di default, da modificare in base al contesto

    if (result > 0) {
        char risposta[128];
        snprintf(risposta, sizeof(risposta), "OK|Ticket salvato con ID %d", result);
        send(socket, risposta, strlen(risposta), 0);
        printf("Ticket salvato (ID: %d)\n", result);
    } else if (result == -2) {
        send(socket, "ERR|Sintassi: NEW_TICKET|Titolo|Descrizione|Priorita", 55, 0);
    } else {
        send(socket, "ERR|Errore salvataggio", 23, 0);
    }
}


int handleLogin(int socket, const char *buffer) {
    char temp[128];
    strncpy(temp, buffer, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';

    char *username = strtok(temp + 6, "|");
    char *password = strtok(NULL, "|");

    if (!username || !password) {
        send(socket, "ERR|Formato: LOGIN|username|password", 37, 0);
        return -1;
    }

    char ruolo[20];
    int attempts = 0;
    while (attempts < 3) {
        int result = authenticateUser(username, password, ruolo);
        switch(result){
            case 0:
                attempts++;
                send(socket, "ERR|Credenziali non valide", 27, 0);
                return 1; // errore ma non grave, client può riprovare
            case -1:
                send(socket, "ERR|Errore accesso file utenti", 30, 0);
                return 0; // errore lettura file, chiudi connessione
            default:
                char risposta[64];
                snprintf(risposta, sizeof(risposta), "OK|Login riuscito|%s", ruolo);
                send(socket, risposta, strlen(risposta), 0);
                salvaSessione(socket, username); // salva la sessione
                return 1;  // login riuscito
        }
    }

    // se piu di 3 tentativi chiudo la connessione
    send(socket, "ERR|Login fallito dopo 3 tentativi", 34, 0);
    return 0;
}

void salvaSessione(int socket_fd, const char *username) {
    for (int i = 0; i < session_count; ++i) {
        if (sessions[i].socket_fd == socket_fd) {
            strncpy(sessions[i].username, username, sizeof(sessions[i].username) - 1);
            return;
        }
    }
    if (session_count < MAX_CLIENTS) {
        sessions[session_count].socket_fd = socket_fd;
        strncpy(sessions[session_count].username, username, sizeof(sessions[session_count].username) - 1);
        session_count++;
    }
}
const char* getUsernameBySocket(int socket_fd) {
    for (int i = 0; i < session_count; ++i) {
        if (sessions[i].socket_fd == socket_fd) {
            return sessions[i].username;
        }
    }
    return NULL;
}

void handleSearchByFieldByUser(int socket, const char *buffer, CampoRicerca campo) {
    char keyword[128], username[64];

    // la lunghezza del prefisso cambia in base al campo
    const char *prefixes[] = {
        "GET_TICKET_BY_TITOLO_BY_USER|",
        "GET_TICKET_BY_DESCRIZIONE_BY_USER|",
        "GET_TICKET_BY_STATO_BY_USER|"
    };

    sscanf(buffer + strlen(prefixes[campo]), "%127[^|]|%63s", keyword, username);

    char out[4096];
    int result = searchTicketsByFieldByUser(username, keyword, campo, out, sizeof(out));

    if (result <= 0)
        send(socket, "ERR|Nessun risultato trovato", 28, 0);
    else
        send(socket, out, strlen(out), 0);
}

void handleUpdateTicketDescriptionAndTitle(int socket, const char *buffer) {
    char copy[512];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *id_str = strtok(copy + strlen("UPDATE_YOUR_TICKET|"), "|");
    char *nuovo_titolo = strtok(NULL, "|");
    char *nuova_descrizione = strtok(NULL, "|");
    const char *username = getUsernameBySocket(socket);

    if (!id_str || !nuovo_titolo || !nuova_descrizione || !username) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(id_str);
    int result = updateDescriptionAndTitle(id, username, nuovo_titolo, nuova_descrizione);
    if (result == 0)
        send(socket, "OK|Modifica completata", 23, 0);
    else if (result == -2)
        send(socket, "ERR|Non autorizzato: il ticket non ti appartiene", 48, 0);
    else
        send(socket, "ERR|Modifica fallita", 21, 0);
}

void handleGetAllTickets(int socket, const char *buffer){
    char ticket_buffer[8192];
    int num_tickets = getAllTickets(ticket_buffer, sizeof(ticket_buffer));

    printf("Numero di ticket letti: %d\n", num_tickets);

    switch (num_tickets) {
        case -1:
            send(socket, "ERR|Errore lettura tickets", 27, 0);
            break;
        case 0:
            send(socket, "OK|Nessun ticket presente", 26, 0);
            break;
        default:
            send(socket, ticket_buffer, strlen(ticket_buffer), 0);
            break;
    }
}

void handleGetTicketsByAgent(int socket, const char *buffer) {
    const char *agent_username = buffer + strlen("GET_ALL_TICKETS_BY_AGENT|");
    char ticket_buffer[8192]; 
    int num_tickets = getTicketsByAgent(agent_username, ticket_buffer, sizeof(ticket_buffer));

    printf("Numero di ticket letti: %d\n", num_tickets);

    switch (num_tickets) {
        case -1:
            send(socket, "ERR|Errore lettura tickets", 27, 0);
            break;
        case 0:
            send(socket, "Nessun ticket a te assegnato", 26, 0);
            break;
        default:
            send(socket, ticket_buffer, strlen(ticket_buffer), 0);
            break;
    }
}

void handleUpdateAssignedAgent(int socket, const char *buffer) {
    char copy[512];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *id_str = strtok(copy + strlen("UPDATE_ASSIGNED_AGENT|"), "|");
    char *nuovo_agente = strtok(NULL, "|");

    if (!id_str || !nuovo_agente) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(id_str);
    int result = updateAssignedAgent(id, nuovo_agente);
    switch(result){
        case 0:
            send(socket, "OK|Modifica completata", 23, 0);
            break;
        case -2:
            send(socket, "OK|Ticket non trovato", 23, 0);
            break;
        default:
            send(socket, "ERR|Modifica fallita", 21, 0);
            break;
    }
}

void handleUpdateStatus(int socket, const char *buffer){
    char copy[512];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *id_str = strtok(copy + strlen("UPDATE_TICKET_STATUS|"), "|");
    char *nuovo_status = strtok(NULL, "|");

    if (!id_str || !nuovo_status) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(id_str);
    int result = updateStatus(id, nuovo_status);
    switch(result){
        case 0:
            send(socket, "OK|Modifica completata", 23, 0);
            break;
        case -2:
            send(socket, "OK|Ticket non trovato", 23, 0);
            break;
        default:
            send(socket, "ERR|Modifica fallita", 21, 0);
            break;
    }
}
void handleCheckUserRole(int socket, const char *buffer) {
    const char *username = buffer + strlen("CHECK_USER_ROLE|");
    char ruolo[32];

    if (getUserRole(username, ruolo, sizeof(ruolo)) == 0) {
        char reply[64];
        snprintf(reply, sizeof(reply), "OK|%s", ruolo);
        send(socket, reply, strlen(reply), 0);
    } else {
        send(socket, "ERR|Utente non trovato", 23, 0);
    }
}

void handleUpdatePriority(int socket, const char *buffer){
    char copy[512];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *id_str = strtok(copy + strlen("UPDATE_TICKET_PRIORITY|"), "|");
    char *nuova_priorita = strtok(NULL, "|");

    if (!id_str || !nuova_priorita) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(id_str);
    int result = updatePriority(id, nuova_priorita);
    switch(result){
        case 0:
            send(socket, "OK|Modifica completata", 23, 0);
            break;
        case -2:
            send(socket, "OK|Ticket non trovato", 23, 0);
            break;
        default:
            send(socket, "ERR|Modifica fallita", 21, 0);
            break;
    }
}

void handleGetTicketById(int socket, const char *buffer){
    // Estrai ID e username
    char copy[128];
    strncpy(copy, buffer, sizeof(copy));
    copy[sizeof(copy)-1] = '\0';

    char *token = strtok(copy + strlen("GET_TICKET_BY_ID|"), "|");
    if (!token) {
        send(socket, "ERR|Formato comando non valido", 30, 0);
        return;
    }

    int id = atoi(token);
    Ticket t;

    if (getTicketById(id, &t) == 0) {
        char risposta[1024];
        snprintf(risposta, sizeof(risposta),
            "ID: %d\nTitolo: %s\nDescrizione: %s\nData: %s\nPriorità: %s\nStato: %s\nAgente: %s\nCreatore: %s\n\n",
            t.id, t.titolo, t.descrizione, t.data_creazione,prioritaToString(t.priorita), statoToString(t.stato), t.agente, t.username
        );
        send(socket, risposta, strlen(risposta), 0);
    } else {
        send(socket, "ERR|Ticket non trovato", 23, 0);
    }
}