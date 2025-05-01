#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

static int running = 0;

void handler(int arg)
{
    printf("handler arg=%d\n", arg);
    unlink("/var/tmp/aesdsocketdata");
    running = 0;
}

void run_server(int socket_fd, FILE *fptr)
{
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    size_t buf_size = 65536;    
    uint8_t *rec_buf = (uint8_t*)malloc(buf_size), *word_buf = (uint8_t*)malloc(buf_size);



    size_t count = 0;
    while (running) {

        int forClientSockfd = accept(socket_fd,(struct sockaddr*) &clientAddr, &len);
 
         //if (recvfrom(socket_fd, buf, buf_size, 0, (struct sockaddr *)&clientAddr, &len) < 0) {
         //    break;
         //}
 
        recv(forClientSockfd, rec_buf, buf_size, 0);
        int end_new_line = 0;
        for(int i = 0 ; i < buf_size ; i++)
        {
            word_buf[count++] = rec_buf[i];
            if (rec_buf[i] == '\n')
            {
                end_new_line = 1;
                break;
            }
                
        }

        //buf[str_len] = '\n';
        // buf[str_len+1] = '\0';
        int f = fwrite(word_buf, 1, count, fptr);
        //printf("last %d\n",word_buf[count]);
        if (end_new_line == 1)
        {
            sendto(forClientSockfd, word_buf, count, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
            //memset(word_buf, 0, count);
            //count = 0;
        }

         //printf("buf:%s\n", buf);
         
        // printf("write %d words\n", f);
         
         //printf("running\n");
     }

     free(rec_buf);
     free(word_buf);
}

int main(int argc, char *argv[])
{
    #define serverPort 9000
    int ret = 0;

    // 建立 socket
    int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }
    
    // server address
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,           
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(serverPort)
    };

    // bind socket to the port
    if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    if (listen(socket_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_DEBUG, "Accepted connection from %d", serverAddr.sin_port);
    FILE *fptr = fopen("/var/tmp/aesdsocketdata", "a+");
    if (fptr == NULL)
    {
        close(socket_fd);
        return -1;
    }

    running = 1;

    struct sigaction act1 = {};
    act1.sa_handler = handler;
    sigaction(SIGINT, &act1, NULL);
    sigaction(SIGTERM, &act1, NULL);

    if (argc == 2)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            printf("demon mode\n");
            pid_t pid = fork();
            if (pid == 0) // child
            {
                run_server(socket_fd, fptr);
            }
            else if (pid > 0) // parent
            {

            }
            else
            {
                ret = -1;
            }
        }
    }
    else
    {
        run_server(socket_fd, fptr);
    }
    
    fclose(fptr);
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
        return -1;
    }

    return ret;
}