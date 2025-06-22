#ifndef UTILS_CLIENT_H
#define UTILS_CLIENT_H

#include <stddef.h>

typedef enum {
    TITOLO,
    DESCRIZIONE,
    PRIORITA
} CampoTicket;

void readInput(const char *prompt, char *dest, size_t size);
int isValidInput(const char *input, const char *validi[], int count);
void buildTicketMessage(char *dest, int max_length);
void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

#endif // UTILS_CLIENT_H