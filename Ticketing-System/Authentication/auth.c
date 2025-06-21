#include <stdio.h>
#include <string.h>
#include "auth.h"
#include <sys/socket.h>

#define USER_FILE "users.txt"

int authenticateUser(const char *username, const char *password, char *ruolo) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return -1;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char usr[50], pwd[50], role[20];
        sscanf(line, "%49[^,],%49[^,],%19s", usr, pwd, role);

        if (strcmp(username, usr) == 0 && strcmp(password, pwd) == 0) {
            strcpy(ruolo, role);
            fclose(file);
            return 1; // login success
        }
    }

    fclose(file);
    return 0; // user not found
}

int getUserRole(const char *username, char *ruolo, size_t maxlen) {
    FILE *fp = fopen(USER_FILE, "r");  // usa "users.txt"
    if (!fp) return -1;

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        char file_username[64], file_password[64], file_ruolo[64];

        // parsing da CSV, non pipe-separated
        if (sscanf(line, "%63[^,],%63[^,],%63[^\n]", file_username, file_password, file_ruolo) == 3) {
            if (strcasecmp(username, file_username) == 0) {
                strncpy(ruolo, file_ruolo, maxlen - 1);
                ruolo[maxlen - 1] = '\0';
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return -1;
}


