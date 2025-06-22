#include <stdio.h>
#include <string.h>
#include "auth.h"

#define USER_FILE "users.txt"

typedef struct {
    char username[64];
    char password[64];
    char ruolo[64];
} User;

static int parseUserLine(const char *line, User *user);
static int findUser(const char *username, User *found, int case_sensitive);

// Autenticazione utente con username e password
int authenticateUser(const char *username, const char *password, char *ruolo) {
    User user;
    if (findUser(username, &user, 1) && strcmp(password, user.password) == 0) {
        strcpy(ruolo, user.ruolo);
        return 1;  // login OK
    }
    return 0;  // utente non trovato o password errata
}

// Ottiene il ruolo dato un nome utente (case-insensitive)
int getUserRole(const char *username, char *ruolo, size_t maxlen) {
    User user;
    if (findUser(username, &user, 0)) {
        strncpy(ruolo, user.ruolo, maxlen - 1);
        ruolo[maxlen - 1] = '\0';
        return 0;
    }
    return -1;
}

// Funzione interna per caricare una riga dal file utenti
static int parseUserLine(const char *line, User *user) {
    return sscanf(line, "%63[^,],%63[^,],%63[^\n]", user->username, user->password, user->ruolo) == 3;
}

// Funzione interna per trovare un utente (confronto case-sensitive o insensitive)
static int findUser(const char *username, User *found, int case_sensitive) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return 0;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        User u;
        if (!parseUserLine(line, &u)) continue;

        int match = case_sensitive ? strcmp(username, u.username) == 0
                                   : strcasecmp(username, u.username) == 0;

        if (match) {
            if (found) *found = u;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}