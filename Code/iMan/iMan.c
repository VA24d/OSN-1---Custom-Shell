#include "iMan.h"

#define BUFFER_SIZE 4096
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define BOLD "\x1b[1m"

void replace_substring(char *str, const char *old_sub, const char *new_sub)
{
    char buffer[BUFFER_SIZE];
    char *pos, *temp = str;
    int old_len = strlen(old_sub);

    buffer[0] = '\0'; // Initialize buffer to an empty string

    // Iterate over the string and replace occurrences of old_sub
    while ((pos = strstr(temp, old_sub)) != NULL)
    {
        // Copy part before the match
        strncat(buffer, temp, pos - temp);

        // Append the new substring
        strcat(buffer, new_sub);

        // Move temp to continue searching after the old substring
        temp = pos + old_len;
    }

    // Append the remainder of the string after the last occurrence of old_sub
    strcat(buffer, temp);

    // Ensure the resulting string fits into the original string buffer
    strcpy(str, buffer);
}

// Function to parse and print the response body (removing HTML tags)
void parse_and_print_output(char *buffer) {
    int in_tag = 0;  // Flag to track whether we're inside an HTML tag

    replace_substring(buffer, "<br>", "\n");
    replace_substring(buffer, "<strong>", BOLD );
    replace_substring(buffer, "</strong>", RESET );


    for (const char *p = buffer; *p != '\0'; p++) {
        if (*p == '<') {
            in_tag = 1;  // Entering an HTML tag
        } else if (*p == '>') {
            in_tag = 0;  // Exiting an HTML tag
        } else if (!in_tag) {
            putchar(*p);  // Print character only if it's outside of a tag
        }
    }
}

// Function to send GET request and print response (without headers and HTML tags)
void fetch_man_page(const char *command) {
    if (strlen(command) == 0) {
        printf(RED"Usage: iMan command\n"RESET);
        return;
    }

    char cmd_copy[256];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';  // Ensure null termination
    char *first_token = strtok(cmd_copy, " ");
    if (first_token == NULL) {
        printf(RED"Usage: iMan command\n"RESET);
        return;
    }

    printf("Fetching man pages for %s \n", first_token);

    const char *host = "man.he.net";
    const char *path_format = "/?topic=%s&section=all";
    char path[256];
    char request[512];
    struct sockaddr_in server_addr;
    struct hostent *server;
    int sockfd;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int in_body = 0;  // Flag to check if we're in the body part of the response

    // Format the path with the command
    snprintf(path, sizeof(path), path_format, first_token);

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        // exit(EXIT_FAILURE);
        return;
    }

    // Get the server's DNS entry
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        // exit(EXIT_FAILURE);
        return;
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        // exit(EXIT_FAILURE);
        return;
    }

    // Create the GET request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, host);

    // Send the GET request
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("Error sending request");
        close(sockfd);
        // exit(EXIT_FAILURE);
        return;
    }

    // Read the response, skip headers, and process the body
    while ((bytes_read = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        // If not in the body, search for the end of the headers
        if (!in_body) {
            char *body_start = strstr(buffer, "<html>");
            if (body_start != NULL) {
                body_start += 4;  // Skip the "\r\n\r\n" part
                parse_and_print_output(body_start);  // Parse and print body content
                in_body = 1;  // Mark that we're now in the body
            }
        } else {
            // Already in the body, parse and print content
            parse_and_print_output(buffer);
        }
    }

    // Clean up
    close(sockfd);
}

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <command_name>\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     fetch_man_page(argv[1]);

//     return 0;
// }
