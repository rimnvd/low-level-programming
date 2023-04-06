#include "../server.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "../protos/msg.proto"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include "init.h"

#define PORT 3939

int main(int argc, char **argv) {
    setbuf(stdout, 0);
    FILE* file = initializer(argc, argv);
    int listenfd, connfd;
    struct sockaddr_in server_address;
    int reuse = 1;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr*) &server_address, sizeof(server_address)) != 0) {
        perror("bind");
        return 1;
    }
    if (listen(listenfd, 5) != 0) {
        perror("listen");
        return 1;
    }
    printf("waiting for connection\n");
    connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
        perror("accept");
        return 1;
    }
    printf("got connection\n");
    pb_istream_t istream = pb_istream_from_socket(connfd);
    pb_ostream_t ostream = pb_ostream_from_socket(connfd);
    char* response = "";
    while (1) {
        Query_tree tree = {};
        if (!pb_decode_delimited(&istream, Query_tree_fields, &tree)) {
            printf("decode failed: %s\n", PB_GET_ERROR(&istream));
            return 2;
        }
        handle_query(file, tree, &response);
        Response resp = {};
        while (strlen(response) > 1023) {
            strncpy(resp.r_string, response, 1023);
            resp.r_string[1023] = 0;
            resp.last = 0;
            if (!pb_encode_delimited(&ostream, Response_fields, &resp)) {
                fprintf(stderr, "encoding failed: %s\n", PB_GET_ERROR(&ostream));
            }
            response += 1023;
        }
        strcpy(resp.r_string, response);
        resp.last = 1;
        if (!pb_encode_delimited(&ostream, Response_fields, &resp)) {
            fprintf(stderr, "encoding failed: %s\n", PB_GET_ERROR(&ostream));
        }
    }
}
