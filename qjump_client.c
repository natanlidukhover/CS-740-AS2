#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE_CHECK 1
#define BUFFER_SIZE_LOW 1
#define BUFFER_SIZE_HIGH 65536
#define LOCALHOST "127.0.0.1"
#define PORT 6969

int main(int argc, char *argv[]) {
    int min = INT_MAX;
    int max = INT_MIN;

    int client_socket;
    struct sockaddr_in server_address;
    char buffer_check[BUFFER_SIZE_CHECK];
    char buffer_low[BUFFER_SIZE_LOW];
    char buffer_high[BUFFER_SIZE_HIGH];
    struct timespec start, stop;
    time_t t;
    
    srand((unsigned) time(&t));

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed\n");
    }
    printf("Socket creation succeeded\n");

    char* ip = LOCALHOST;
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(ip);

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Connecting failed\n");
    }
    printf("Connecting succeeded\n");

    for (;;) {
        if ((rand() % 3) == 0) {
            memset(buffer_check, 0, BUFFER_SIZE_CHECK);
            memset(buffer_low, 0, BUFFER_SIZE_LOW);

            if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
                perror("Starting the clock failed");
            }

            if (send(client_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                printf("Sending check data failed\n");
            }
            if (recv(client_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                printf("Receiving check confirmation failed\n");
            }
            if (send(client_socket, buffer_low, BUFFER_SIZE_LOW, 0) < 0) {
                printf("Sending low latency data failed\n");
            }
            if (recv(client_socket, buffer_low, BUFFER_SIZE_LOW, 0) < 0) {
                printf("Receiving low latency confirmation failed\n");
            }

            if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
                perror("Stopping the clock failed");
            }

            long int current_latency = stop.tv_nsec - start.tv_nsec;
            if (current_latency < min && current_latency > 0) {
                min = current_latency;
                printf("New minimum latency in nanoseconds: %lf\n", (float) min / 2.0);
            }
            if (current_latency > max && current_latency > 0) {
                max = current_latency;
                printf("New maximum latency in nanoseconds: %lf\n", (float) max / 2.0);
            }
        } else {
            memset(buffer_check, 1, BUFFER_SIZE_CHECK);
            memset(buffer_high, 1, BUFFER_SIZE_HIGH);

            /*if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
                perror("Starting the clock failed");
            }*/

            if (send(client_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                printf("Sending check data failed\n");
            }
            if (recv(client_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                printf("Receiving check confirmation failed\n");
            }
            if (send(client_socket, buffer_high, BUFFER_SIZE_HIGH, 0) < 0) {
                printf("Sending high throughput data failed\n");
            }
            if (recv(client_socket, buffer_high, BUFFER_SIZE_HIGH, 0) < 0) {
                printf("Receiving high throughput confirmation failed\n");
            }

            /*if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
                perror("Stopping the clock failed");
            }

            long int current_latency = stop.tv_nsec - start.tv_nsec;
            if (current_latency < min && current_latency > 0) {
                min = current_latency;
                printf("New minimum latency in nanoseconds: %lf\n", (float) min / 2.0);
            }
            if (current_latency > max && current_latency > 0) {
                max = current_latency;
                printf("New maximum latency in nanoseconds: %lf\n", (float) max / 2.0);
            }*/
        }
    }

    close(client_socket);
    return 0;
}