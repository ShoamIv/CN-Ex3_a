
    #include <stdio.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <string.h>
    #include <errno.h>
    #include <signal.h>
    #include <unistd.h>
    #include <fcntl.h>

    #define INVALID_SOCKET (-1)

    // global var
    char * file_name="sample-2mb-text-file.txt";
    int socketfd = INVALID_SOCKET;

    int socketSetup (struct sockaddr_in *serverAddress,int SERVER_PORT,char *algo, char *ip) {
        int socketfd = INVALID_SOCKET;
        memset(serverAddress, 0, sizeof(*serverAddress));
        serverAddress->sin_family = AF_INET;
        serverAddress->sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET, (const char *) ip, &serverAddress->sin_addr) == -1) {
            perror("inet_pton()");
            exit(1);
        }

        if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            perror("socket()");
            exit(1);
        }

        printf("Socket successfully created.\n");

        return socketfd;
    }

    char* read_file_data(int *size) {
     FILE *point=NULL;
     char * content;
     point= fopen(file_name,"r");
             if(point==NULL){
                 perror(("fopen"));
                 exit(1);
             }
        fseek(point,0L,SEEK_END);

        *size=(int) ftell(point);
        content= (char *) malloc(*size * sizeof(char));
        fseek(point,0L,SEEK_SET);

        fread(content,sizeof (char),*size,point);
        fclose(point);
        return content;
    }

    int sendDataToReceiver(int receiverSocket, void* buffer,int len) {
        int isSent = send(receiverSocket, buffer, len, 0);
        if (isSent == -1) {
            perror("send");
        }
        return isSent;
    }

    int main(int argc, char *argv[]) {

        // check command line args count
        if (argc != 7) {
            fprintf(stderr, "Usage: %s <ip> <port> <algo>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        // parse command line var
        char *ip_address=argv[2];
        int port=atoi(argv[4]);
        char *congestion_algorithm=argv[6];

        // file var
        char* content=NULL;
        int fileSize=0;

        // read the file content, init the size
        content= read_file_data(&fileSize);

        // create the receiver socket
        struct sockaddr_in serverAddress;
        printf("Setting up the socket...\n");
        socketfd=socketSetup(&serverAddress,port,congestion_algorithm,ip_address);

        // establish connection with the receiver
        if (connect(socketfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
        {
            perror("connect");
            exit(1);
        }
        printf("Connected successfully to %s:%d!\n", ip_address, port);


        // send the file size init the buffer size
        printf("Sending file size to receiver...\n");
        sendDataToReceiver(socketfd,&fileSize,sizeof(int));

        //send the data for the first time
        sendDataToReceiver(socketfd,content,fileSize);


        while(true){
            // get the user decision
            int decision=-1;
            printf("send the file again? 1 for yes, 0 for no\n");
            while (scanf("%d",&decision)!=1 || (decision !=0 && decision!=1)){
                scanf("%*s");
                printf("wrong choise!\n");
            }

            // handle user decision
            if(decision == 0){
                // send exit message to the receiver
                char exit='N';
                sendDataToReceiver(socketfd,&exit, sizeof(char ));
                printf("Exit");
                break;
            }
            else if(decision ==1) {
                  // send resend message to the receiver
                  char resend='Y';
                  sendDataToReceiver(socketfd,&resend, sizeof(char ));
            }
            // resend the actual data
            sendDataToReceiver(socketfd,content,fileSize);
        }

        // close connection, reals memory.
        printf("Closing connection...\n");
        close(socketfd);
        printf("Sender exit.\n");

        return 0;
    }
