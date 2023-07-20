#include <string.h>
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

static void socket_close(int sock);
static bool socket_is_valid(int sock);
static int socket_get_error(void);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: tcp-client hostname port\n");
        return EXIT_FAILURE;
    }

    printf("Configuring remote address...\n");
    struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
    struct addrinfo* peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        int error = socket_get_error();
        fprintf(stderr, "getaddrinfo() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Remote address for %s:%s is: ", argv[1], argv[2]);
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);
    printf("%s %s \n", address_buffer, service_buffer);

    printf("Creating socket...\n");
    int socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!socket_is_valid(socket_peer)) {
        int error = socket_get_error();
        fprintf(stderr, "socket() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }

    printf("Connecting...\n");
    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        int error = socket_get_error();
        fprintf(stderr, "connect() failed: %s (%d)\n", strerror(error), error);
        return EXIT_FAILURE;
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");

    while (1) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(fileno(stdin), &reads);

        struct timeval timeout = { .tv_sec = 0, .tv_usec = 100000 };

        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            int error = socket_get_error();
            fprintf(stderr, "select() failed: %s (%d)\n", strerror(error), error);
            return EXIT_FAILURE;
        }

        if (FD_ISSET(socket_peer, &reads)) {
            char read[4096];
            int bytes_received = recv(socket_peer, read, sizeof(read), 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
        }

        if (FD_ISSET(fileno(stdin), &reads)) {
            char read[4096];
            if (!fgets(read, sizeof(read), stdin)) {
                break;
            }
            printf("Sending: %s", read);
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
        }
    }

    printf("Closing socket...\n");
    socket_close(socket_peer);

    printf("Finished.\n");
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
