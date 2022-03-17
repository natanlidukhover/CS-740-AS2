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

#define BACKLOG 10
#define BUFFER_SIZE_CHECK 1
#define BUFFER_SIZE_LOW 1
#define BUFFER_SIZE_HIGH 65536
#define EPOCH_LENGTH 1
#define LOCALHOST "127.0.0.1"
#define NO_JUMP_ARGUMENT "--no-jump"
#define PORT 6969

int min = INT_MAX;
int max = INT_MIN;

struct Node {
    char* bytes;
    long start_ns;
    struct Node* next;
};

struct Queue {
    struct Node *front, *rear;
};

struct Node* new_node(char* b, long ns) {
    struct Node* n = (struct Node*) malloc(sizeof(struct Node));
    n->bytes = b;
    n->start_ns = ns;
    n->next = NULL;

    return n;
}

struct Queue* create_queue() {
    struct Queue* q = (struct Queue*) malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;

    return q;
}

void enqueue(struct Queue* q, char* b) {
    struct timespec start, stop;
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("Starting the clock failed");
    }
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("Stopping the clock failed");
    }
    struct Node* n = new_node(b, stop.tv_nsec);

    if (q->rear == NULL) {
        q->front = q->rear = n;
        return;
    }
    
    q->rear->next = n;
    q->rear = n;
}

void enqueue_jump(struct Queue* q, char* b) {
    struct timespec start, stop;
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("Starting the clock failed");
    }
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("Stopping the clock failed");
    }
    struct Node* n = new_node(b, stop.tv_nsec);

    if (q->rear == NULL) {
        q->front = q->rear = n;
        return;
    }
    
    n->next = q->front;
    q->front = n;
}

struct Node* dequeue(struct Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }

    struct Node* n = q->front;

    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    return n;
}

void empty_queue(struct Queue* q) {
    while(1) {
        struct Node* n = dequeue(q);
        if (n == NULL) {
            return;
        }

        struct timespec start, stop;
        if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
            perror("Starting the clock failed");
        }
        if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
            perror("Stopping the clock failed");
        }

        long int current_latency = stop.tv_nsec - n->start_ns;
        if (current_latency < min && current_latency > 0) {
            min = current_latency;
            printf("New minimum latency in nanoseconds: %lf\n", (float) min);
        }
        if (current_latency > max && current_latency > 0) {
            max = current_latency;
            printf("New maximum latency in nanoseconds: %lf\n", (float) max);
        }
    }
}

int main(int argc, char* argv[]) {
    int new_socket, nojump, sockfd;
    struct sockaddr_in new_address, server_address;
    socklen_t address_size;
    char buffer_check[BUFFER_SIZE_CHECK];
    char buffer_low[BUFFER_SIZE_LOW];
    char buffer_high[BUFFER_SIZE_HIGH];
    pid_t child_pid;
    time_t start, stop;

    time_t elapsed_time = 0;

    struct Queue* q = create_queue();

    if (argc > 1 && strcmp(argv[1], NO_JUMP_ARGUMENT)) {
        nojump = 1;
    } else {
        nojump = 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed\n");
    }
    printf("Socket creation succeeded\n");

    char* ip = LOCALHOST;
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(ip);

    if (bind(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Binding failed\n");
    }
    printf("Binding succeeded\n");

    if (listen(sockfd, BACKLOG) == 0){
        printf("Listening succeeded\n");
    } else {
        printf("Listening failed\n");
    }

    start = time(NULL);
    for (;;) {
        new_socket = accept(sockfd, (struct sockaddr*) &new_address, &address_size);
        if (new_socket < 0) {
            perror("Accepting failed\n");
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(new_address.sin_addr), ntohs(new_address.sin_port));

        if ((child_pid = fork()) == 0) {
            close(sockfd);

            for (;;) {
                if (recv(new_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                    printf("Receiving check data failed\n");
                }

                if (buffer_check[0] == 0) {
                    //printf("Received low latency transmission from %s:%d\n", inet_ntoa(new_address.sin_addr), ntohs(new_address.sin_port));
                    
                    if (send(new_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                        printf("Sending check confirmation failed\n");
                    }
                    if (recv(new_socket, buffer_low, BUFFER_SIZE_LOW, 0) < 0) {
                        printf("Receiving low latency data failed\n");
                    }
                    if (nojump) {
                        enqueue(q, buffer_low);
                    } else {
                        enqueue_jump(q, buffer_low);
                    }
                    if (send(new_socket, buffer_low, BUFFER_SIZE_LOW, 0) < 0) {
                        printf("Sending low latency confirmation failed\n");
                    }
                    
                    bzero(buffer_check, BUFFER_SIZE_CHECK);
                    bzero(buffer_low, BUFFER_SIZE_LOW);
                } else if (buffer_check[0] == 1) {
                    //printf("Received high throughput transmission from %s:%d\n", inet_ntoa(new_address.sin_addr), ntohs(new_address.sin_port));

                    if (send(new_socket, buffer_check, BUFFER_SIZE_CHECK, 0) < 0) {
                        printf("Sending check confirmation failed\n");
                    }
                    if (recv(new_socket, buffer_high, BUFFER_SIZE_HIGH, 0) < 0) {
                        printf("Receiving low latency data failed\n");
                    }
                    enqueue(q, buffer_high);
                    if (send(new_socket, buffer_high, BUFFER_SIZE_HIGH, 0) < 0) {
                        printf("Sending low latency confirmation failed\n");
                    }

                    bzero(buffer_check, BUFFER_SIZE_CHECK);
                    bzero(buffer_high, BUFFER_SIZE_HIGH);
                } else {
                    printf("Closed %s:%d\n", inet_ntoa(new_address.sin_addr), ntohs(new_address.sin_port));
                    break;
                }

                stop = time(NULL);
                elapsed_time = difftime(stop, start);
                if (elapsed_time >= (double) EPOCH_LENGTH) {
                    empty_queue(q);
                    start = time(NULL);
                }
            }
        }   
    }

    close(new_socket);
    return 0;
}