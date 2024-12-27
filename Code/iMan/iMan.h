#ifndef IMAN_H
#define IMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>


void fetch_man_page(const char *command);
void replace_substring(char *str, const char *old_sub, const char *new_sub);

#endif