#ifndef UTILS_CLIENT_H
#define UTILS_CLIENT_H

void buildTicketMessage(char *dest, int max_length);
void sendRequestaAndReveiveResponse(int client_fd, const char *messaggio, char *buffer, size_t bufsize);

#endif
