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
