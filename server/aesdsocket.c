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
#include <pthread.h>
#include <time.h>

#define buf_size 65536

static int running = 0;

void handler(int arg)
{
    printf("handler arg=%d\n", arg);
    unlink("/var/tmp/aesdsocketdata");
    running = 0;
}
typedef struct thread_task_node_t thread_task_node_t;
struct thread_task_node_t
{
    pthread_t thrd;
    thread_task_node_t *next;
    int is_completed;
    int client_sock_fd;
    struct sockaddr_in cli_sockaddr;
    uint8_t *mem;

};

FILE *gFILE_HANDLER;
pthread_mutex_t gMUTEX;
size_t gCOUNT = 0;
time_t gTIME;
void* thrd_task(void *arg)
{
    thread_task_node_t *node = (thread_task_node_t*)arg;

    uint8_t *rec_buf = node->mem;

    FILE *fptr = fopen("/var/tmp/aesdsocketdata", "a+");
    
    while(running == 1)
    {    
        int end_new_line = 0;

        pthread_mutex_lock(&gMUTEX);

        ssize_t recv_count = recv(node->client_sock_fd, rec_buf + gCOUNT, buf_size, 0);
        
        if (recv_count != (ssize_t)-1)
        {
            for(int i = 0 ; i < recv_count ; i++)
            {
                if (rec_buf[i] == '\n')
                {
                    end_new_line = 1;
                    break;
                }
            }

            gCOUNT += recv_count;

            if (end_new_line == 1)
            {
                sendto(node->client_sock_fd, rec_buf, gCOUNT, 0, (struct sockaddr *)&node->cli_sockaddr, sizeof(node->cli_sockaddr));
            }

            fwrite(rec_buf, 1, recv_count, fptr);
        }

        time_t t3;
        time_t t2 = time(&t3);
        if (t2 - gTIME >= 10)
        {
            struct tm *cur_time = localtime(&t3);
            
            char date_buf[64] = {};
            strftime(date_buf, sizeof(date_buf), " timestamp:%Y %m %d %H %M %S\n", cur_time);
            int r = sendto(node->client_sock_fd, date_buf,  strlen(date_buf), 0, (struct sockaddr *)&node->cli_sockaddr, sizeof(node->cli_sockaddr));
            if (r == -1)
                printf("failed to send\n");
            memcpy(rec_buf+ gCOUNT, date_buf, strlen(date_buf));
            gCOUNT += strlen(date_buf);
            fwrite(date_buf, 1, strlen(date_buf), fptr);

            printf("data_buf = %s\n",date_buf);
            
            gTIME = t2;
        }

        fflush(fptr);
        pthread_mutex_unlock(&gMUTEX);
    }

    fclose(fptr);
   
    return NULL;
}

void run_server(int socket_fd, FILE *fptr)
{
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);  
    uint8_t* mem = malloc(buf_size);
    thread_task_node_t *phead;

    size_t count = 0;
    while (running) 
    {

        int forClientSockfd = accept(socket_fd,(struct sockaddr*) &clientAddr, &len);

        int oldSocketFlag = fcntl(forClientSockfd, F_GETFL, 0);
        int newSocketFlag = oldSocketFlag | O_NONBLOCK;
        fcntl(forClientSockfd, F_SETFL,  newSocketFlag);


        if (forClientSockfd != -1)
        {
            thread_task_node_t *node = malloc(sizeof(*node));
            node->next = phead;
            node->is_completed = 0;
            node->client_sock_fd = forClientSockfd;
            node->cli_sockaddr = clientAddr;
            node->mem = mem;
            phead = node->next;
            printf("create thrd\n");
            pthread_create(&node->thrd, NULL, thrd_task, (void*)node);
        }
    }
    while(phead != NULL)
    {
        thread_task_node_t *nxt = phead->next;

        if (phead->is_completed == 1)
        {
            free(phead);
            phead = nxt;
        }
        else
        {
            pthread_join(phead->thrd, NULL);           
            free(phead);
            phead = nxt;            
        }
    }
    free(mem);
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
    FILE *gFILE_HANDLER = fopen("/var/tmp/aesdsocketdata", "a+");
    if (gFILE_HANDLER == NULL)
    {
        close(socket_fd);
        return -1;
    }

    running = 1;

    struct sigaction act1 = {};
    act1.sa_handler = handler;
    sigaction(SIGINT, &act1, NULL);
    sigaction(SIGTERM, &act1, NULL);

    pthread_mutex_init(&gMUTEX, NULL);
    gTIME = time(NULL);

    if (argc == 2)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            printf("demon mode\n");
            pid_t pid = fork();
            if (pid == 0) // child
            {
                run_server(socket_fd, gFILE_HANDLER);
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
        run_server(socket_fd, gFILE_HANDLER);
    }
    
    fclose(gFILE_HANDLER);
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
        return -1;
    }
    unlink("/var/tmp/aesdsocketdata");
    return ret;
}