#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

static void socket_close(int sock);
static bool socket_is_valid(int sock);
static int socket_get_error(void);

int main(void) {
    printf("Configuring local address....\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* bind_address;
    getaddrinfo(NULL, "8080", &hints, &bind_address);

    printf("Creating socket...\n");
    int socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (!socket_is_valid(socket_listen)) {
        int error = socket_get_error();
        fprintf(stderr, "socket() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Configuring dual-stack...\n");
    int option = 0;
    if (setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option))) {
        int error = socket_get_error();
        fprintf(stderr, "setsockopt() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        int error = socket_get_error();
        fprintf(stderr, "bind() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        int error = socket_get_error();
        fprintf(stderr, "listen() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);
    if (!socket_is_valid(socket_client)) {
        int error = socket_get_error();
        fprintf(stderr, "accept() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Client is connected...\n");
    char address_buffer[100];
    getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), NULL, 0, NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    printf("Reading request...\n");
    char request[1024];
    int bytes_received = recv(socket_client, request, sizeof(request), 0);
    printf("Received %d bytes.\n", bytes_received);

    printf("%.*s", bytes_received, request);

    printf("Sending response...\n");
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Sent %d of %zu bytes.\n", bytes_sent, strlen(response));

    time_t timer;
    time(&timer);
    char* time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d of %zu bytes.\n", bytes_sent, strlen(time_msg));

    printf("Closing connection...\n");
    socket_close(socket_client);

    printf("Closing listening socket...\n");
    socket_close(socket_listen);

    printf("Finished\n");

    return EXIT_SUCCESS;
}

static void socket_close(int sock) {
    close(sock);
}

static bool socket_is_valid(int sock) {
    return (sock >= 0);
}

static int socket_get_error(void) {
    return errno;
}
