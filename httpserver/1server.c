#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <hiredis/hiredis.h>

#define PORT 8888

char *host;
char *port;

static int click_count = 0;
static redisContext *redis_conn;

int redis_connect() {
    redis_conn = redisConnect(host, atoi(port));
    if (redis_conn == NULL || redis_conn->err) {
        if (redis_conn) {
            fprintf(stderr, "Error connecting to Redis: %s\n", redis_conn->errstr);
            redisFree(redis_conn);
        } else {
            fprintf(stderr, "Error connecting to Redis: Can't allocate redis context\n");
        }
        return 0;
    }
    return 1;
}

int redis_increment_count() {
    if (redis_connect()) {
        redisReply *reply = redisCommand(redis_conn, "INCR count");
        if (reply == NULL) {
            fprintf(stderr, "Error incrementing count: No reply from Redis server\n");
            redisFree(redis_conn);
            return 0;
        }
        // Print reply type and message
                 fprintf(stderr,"Reply type: %d\n", reply->type);
                         fprintf(stderr, "Reply message: %s\n", reply->str);
        if (reply->type != REDIS_REPLY_INTEGER) {
            fprintf(stderr, "Error incrementing count: Invalid reply type from Redis server\n");
            freeReplyObject(reply);
            redisFree(redis_conn);
            return 0;
        }
        click_count = reply->integer;
        freeReplyObject(reply);
        redisFree(redis_conn);
        return 1;
    }
    return 0;
}

int redis_get_count() {
    if (redis_connect()) {
        redisReply *reply = redisCommand(redis_conn, "GET count");
        if (reply == NULL) {
            fprintf(stderr, "Error fetching count: No reply from Redis server\n");
            redisFree(redis_conn);
            return -1;
        }
                 fprintf(stderr, "Reply type: %d\n", reply->type);
                         fprintf(stderr, "Reply message: %s\n", reply->str);
        if (reply->type != REDIS_REPLY_STRING) {
            fprintf(stderr, "Error fetching count: Invalid reply type from Redis server\n");
            freeReplyObject(reply);
            redisFree(redis_conn);
            return -1;
        }
        click_count = atoi(reply->str);
        freeReplyObject(reply);
        redisFree(redis_conn);
        return click_count;
    }
    return -1;
}

void handle_request(int client_socket, char *request) {
    char response[1024];
    int count;

    // Handle button1 click
    if (strstr(request, "POST /button1") != NULL) {
        redis_increment_count();
        count = redis_get_count();
        if (count != -1) {
            click_count = count;
        }
    }


    // HTTP response with HTML content
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<html><body>"
             "<form action=\"/button1\" method=\"post\"><button type=\"submit\">Button1</button></form>"
             "<p>Click Count: %d</p>"
             "</body></html>",
             click_count);

    // Send response to client
    send(client_socket, response, strlen(response), 0);
}

int main() {


host = getenv("REDIS_HOST");
port = getenv("REDIS_PORT");


    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char request[1024];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 10) == -1) {
        perror("Error listening on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket == -1) {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Receive request from client
        recv(client_socket, request, sizeof(request), 0);

        // Handle request
        handle_request(client_socket, request);

        // Close client socket
        close(client_socket);
    }

    // Close server socket
    close(server_socket);

    return 0;
}
