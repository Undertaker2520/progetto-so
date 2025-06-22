#ifndef AUTH_H
#define AUTH_H

int authenticateUser(const char *username, const char *password, char *ruolo);
int getUserRole(const char *username, char *ruolo, size_t maxlen);

#endif