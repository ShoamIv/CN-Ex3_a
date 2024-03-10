#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#define INVALID_SOCKET (-1)


int socketSetup(struct sockaddr_in *serverAddress, int SERVER_PORT, char *algo) {
    int socketfd=INVALID_SOCKET;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    // Create a TCP socket
    if ( socketfd == INVALID_SOCKET) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(socketfd, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo))) {
        perror("failed to set congestion control algorithm");
        exit(EXIT_FAILURE);
    }
    // Setup server address
    serverAddress->sin_family = AF_INET;                 // Set address family to IPv4
    serverAddress->sin_addr.s_addr = INADDR_ANY;         // Set IP address to localhost
    serverAddress->sin_port = htons(SERVER_PORT);        // Set port number

    // Bind socket to port
    if (bind(socketfd, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for TCP connection...\n");
    // Listen for incoming connections
    if (listen(socketfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return socketfd;
}

int getDataFromSender(int Sendersocket,void * buffer, int len){

    int rec= recv(Sendersocket,buffer,len,0);
    if (!rec){
        perror("recv");
        exit(1);
    }
    return rec;
}

void print_ness(int run,double time,double kilo){
    printf("Run=%d Time=%.2f ms Speed=%.2f MB/s\n", run, time, kilo);
    printf("data transfer completed \n");
}

// Declare a variable of type Statics
struct Statistics {
    int run;
    double time;
    double kilo;
};

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <ip> <port> <algo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // command line args
    int port = atoi(argv[2]);
    char *congestion_algorithm = argv[4];
    int run_number = 0, num_stats = 0;
    // measuring time vars
    double total_time = 0;
    //init struct for statistics.
    struct Statistics *stats = NULL;
    // file vars
    int fileSize,
            Received = 0;
    char *buffer = NULL;

    // socket vars
    int senderSocket = INVALID_SOCKET;
    struct sockaddr_in server_address, clientAddr;
    socklen_t clientAddrlen = sizeof(senderSocket);
    char clientAddress[INET_ADDRSTRLEN];


    // establish connection with the sender,
    int receiverSocket = socketSetup(&server_address, port, congestion_algorithm);
    memset(&clientAddress, 0, sizeof(clientAddress));
    senderSocket = accept(receiverSocket, (struct sockaddr *) &clientAddr, &clientAddrlen);
    if (senderSocket == -1) {
        perror("accept");
        exit(1);
    }

    printf("sender connected, beginning to receive file");
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientAddress, INET_ADDRSTRLEN);


    // get the file size and init the buffer size
    getDataFromSender(senderSocket, &fileSize, sizeof(int));
    buffer = malloc(fileSize * sizeof(char));

    while (true) {
        clock_t start, end;
        int byteRec = 0;
        if (!Received) {
            memset(buffer, 0, fileSize);
        }
        // start clock
        start = clock();
        // get data from sender
        byteRec = getDataFromSender(senderSocket, buffer + Received, fileSize - Received);
        Received += byteRec;

        if (byteRec == -1) {
            break;
        } else if (Received == fileSize) {
            //close time
            end = clock();
            double time_per = ((double) (end - start) * 1000.0) / CLOCKS_PER_SEC;
            total_time += time_per;

            // Increase the size of the stats array
            stats = realloc(stats, (num_stats + 1) * sizeof(struct Statistics));
            if (stats == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }

            //update data at struct.
            stats[num_stats].run = num_stats + 1;
            stats[num_stats].time = time_per;
            stats[num_stats].kilo = byteRec / time_per * 1000.0 / (1024.0 * 1024.0); // Convert bytes/ms to MB/s;

            num_stats++;
            run_number++;

            printf("waiting for sender\n");

            //get the sender response
            char exit;
            getDataFromSender(senderSocket, &exit, sizeof(char));

            // handle sender response
            if (exit == 'N') {
                printf("sender wants to exit\n");
                break;
            } else if (exit == 'Y') {
                printf("sender wants to send another time \n");
                Received = 0;
                start = 0, end = 0;
            }
        }
    }
    printf("* Statistics *  \n ");

    for (int i = 0; i < run_number; i++) {
        print_ness(stats[i].run, stats[i].time, stats[i].kilo);
    }

    printf("Average time=%.2f ms\n", total_time / run_number);
    if (total_time > 0){
        //avoid /0.
        printf("Average bandwidth=%.2f MB/s\n", (Received / total_time) * 1000.0 / (1024.0 * 1024.0));
    }

    //close sockets, clean memory
    close(senderSocket);
    close(receiverSocket);
    free(buffer);
    printf("Receiver exit");
    return 0;
}
