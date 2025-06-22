#include <stdio.h>
#include <string.h>
#include "auth.h"

#define USER_FILE "users.txt"

// Funzione interna per leggere ruolo/password dato uno username (case-insensitive)
static int lookupUser(const char *username, char *out_pwd, char *out_role) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return -1;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char usr[50], pwd[50], role[20];
        if (sscanf(line, "%49[^,],%49[^,],%19s", usr, pwd, role) == 3) {
            if (strcasecmp(username, usr) == 0) {
                if (out_pwd) strcpy(out_pwd, pwd);
                if (out_role) strcpy(out_role, role);
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1; // not found
}

int authenticateUser(const char *username, const char *password, char *ruolo) {
    char stored_pwd[50], stored_role[20];
    if (lookupUser(username, stored_pwd, stored_role) == 0) {
        if (strcmp(password, stored_pwd) == 0) {
            if (ruolo) strcpy(ruolo, stored_role);
            return 1; // login ok
        } else {
            return 0; // password errata
        }
    }

    return 0; // utente non trovato
}

int getUserRole(const char *username, char *ruolo, size_t maxlen) {
    char dummy_pwd[50], role[64];
    if (lookupUser(username, dummy_pwd, role) == 0) {
        strncpy(ruolo, role, maxlen - 1);
        ruolo[maxlen - 1] = '\0';
        return 0;
    }
    return -1;
}
