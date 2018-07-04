#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXMSG 1024
#define SERV_PORT 8023
#define FILE_RECV_PORT 3208
#define ID_SIZE 7
#define MAXLINE 1024
// start a thread to send msg
// main thread send msg 
//
// send <msg>
// switch <group_id>
// login
// create_group
// enter_group <group_id>
//

int current_group;
int my_id = 0;
int sockfd;
struct sockaddr_in servaddr;
char next_send_filename[128];

void debug_addr(struct sockaddr_in addr)
{
    char ip[64];
    inet_ntop(AF_INET, &addr.sin_addr, ip, 64);
    int port = htons(addr.sin_port);
    printf("ip: %s port: %d\n", ip, port);
}

void *file_recv(void *fk)
{
    struct sockaddr_in recvaddr, sendaddr;
    char buf[MAXLINE];
    int recvfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&recvaddr, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    recvaddr.sin_port = htons(FILE_RECV_PORT);
    int on = 1;
    setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bind(recvfd, (struct sockaddr*)&recvaddr, sizeof(recvaddr));
    perror("bind");
    listen(recvfd, 10);
    socklen_t send_addr_len = sizeof(sendaddr);
    int sendfd = accept(recvfd, (struct sockaddr*)&sendaddr, &send_addr_len);
    FILE *fp=fopen("recv.file", "wb");
    bzero(buf, sizeof(buf));
    int recv_len;
    while(recv_len = recv(sendfd, buf, MAXLINE, 0))
    {
        if(recv_len<0)
            break;
        int write_len = fwrite(buf, sizeof(char), recv_len, fp);
        if(write_len < recv_len)
        {
            printf("Write failed\n");
            break;
        }
        bzero(buf, MAXLINE);
    }
    printf("Recv sucess\n");
    fclose(fp);
    close(recvfd);
    close(sendfd);
}


void *file_send(void *arg)
{
    char *info = (char*)arg;
    char ip[128], filename[128];
    sscanf(info, "%s %s", ip, filename);
    struct sockaddr_in recvaddr;
    bzero(&recvaddr, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(FILE_RECV_PORT);
    inet_pton(AF_INET, ip, &recvaddr.sin_addr);
    int recvfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(recvfd, (struct sockaddr*)&recvaddr, sizeof(recvaddr));
    char buf[MAXLINE];
    bzero(buf, MAXLINE);
    FILE *fp = fopen(filename, "rb");
    int read_len;
    while((read_len = fread(buf, sizeof(char), MAXLINE, fp)) > 0)
    {
        int send_len = send(recvfd,buf, read_len, 0);
        perror("send");
        if(send_len < 0)
        {
            perror("send file failed\n");
            break;
        }
        bzero(buf, MAXLINE);
    }
    fclose(fp);
    close(recvfd);
    printf("send finish\n");
}

void parse_input(char *msg, char *type, char *arg)
{
    sscanf(msg, "%s", type);
    int len = strlen(msg);
    int arg_start;
    msg[--len] = '\0';
    for(arg_start=0; arg_start < len; ++arg_start)
    {
        if(msg[arg_start] == ' ')
        {
            ++arg_start;
            break;
        }
    }
    sprintf(arg, "%s", msg + arg_start);
}

bool parse_request(char *msg, int &from_id, int &to_id, char *request_type, char *body)
{
    int msg_len = strlen(msg);
    sscanf(msg, "%d %d %s", &from_id, &to_id, request_type);
    int body_start = 0;
    int nc = 0;
    for(body_start = 0; body_start< msg_len; ++body_start)
    {
        if(msg[body_start] == '\n')
        nc++;
        if(nc == 2)
        {
            ++body_start;
            break;
                                                                                          
        }
                                                        
    }
    sprintf(body, "%s", msg + body_start);
    return true;                                    
}


void *send_to_server(void *fk)
{
    char msg[MAXMSG];
    char sendmsg[MAXMSG];
    char type[64];
    char arg[MAXMSG];
    while(fgets(msg, MAXMSG, stdin))
    {
        parse_input(msg, type, arg);
        if(strcmp(type, "send") == 0)
        {
            sprintf(sendmsg, "%d %d\n%s\n%s", my_id, current_group, "SEND", arg);
            sendto(sockfd, sendmsg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
        else if(strcmp(type, "switch") == 0)
        {
            int group_id = 0;
            sscanf(arg, "%d", &group_id);
            
            current_group = group_id;
            printf("cuurent_group: %d\n", group_id);
        }   
        else if(strcmp(type, "login") == 0)
        {
            sprintf(sendmsg, "%d %d\n%s\n", 0, 0, "LOGIN");
            sendto(sockfd, sendmsg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
        else if(strcmp(type, "create_group") == 0)
        {
            sprintf(sendmsg, "%d %d\n%s\n", my_id, 0, "CREATE_GROUP");
            sendto(sockfd, sendmsg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
        else if(strcmp(type, "enter_group") == 0)
        {
            int group_id;
            sscanf(arg, "%d", &group_id);
            sprintf(sendmsg, "%d %d\n%s\n", my_id, group_id, "ENTER_GROUP");
            sendto(sockfd, sendmsg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
        else if(strcmp(type, "sendfile") == 0)
        {
            int to_id;
            char filename[128];
            sscanf(arg, "%d %s", &to_id, filename);
            strcpy(next_send_filename, filename);
            sprintf(sendmsg, "%d %d\n%s\n", my_id, to_id, "TRANS_FILE");
            sendto(sockfd, sendmsg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        }
    
    }
}

void* send_heartbeat(void *fk)
{
    char msg[1024];
    sprintf(msg, "%d %d\n%s\n", 0, 0, "HEART_BEAT");
    while(1)
    {
        sendto(sockfd, msg, sizeof(sendmsg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        sleep(10);
    }
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        fprintf(stderr, "usage: %s <server_ip> <listen_port>", argv[0]);
        exit(1);                       
    }
    int listen_port;
    sscanf(argv[2], "%d", &listen_port);
    pthread_t tid;
    pthread_create(&tid, NULL, send_to_server, NULL);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    
    struct sockaddr_in cliaddr; 
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr.sin_port = htons(listen_port);
    bind(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    
    char msg[MAXMSG];
    socklen_t len;
    int from_id, to_id;
    char type[64];
    char body[MAXMSG];
    char output[MAXMSG];
    char file_send_info[MAXMSG];
    for(;;)
    {
        len = sizeof(servaddr);
        int n = recvfrom(sockfd, msg, MAXMSG, 0, NULL, NULL);
        msg[n] = '\0';
        parse_request(msg, from_id, to_id, type, body);
        if(strcmp(type, "SEND") == 0)
        {
            printf("%d: %s\n", from_id, body);
        }
        else if(strcmp(type, "GROUP_ID") == 0)
        {
            printf("created group_id is %d\n", from_id);
        }
        else if(strcmp(type, "MEMBER_ID") == 0)
        {
            printf("my id is %d\n", to_id);
            my_id = to_id;
        }
        else if(strcmp(type, "SEND_FILE") == 0)
        {
            char ip[128];
            int port;
            sscanf(body, "%s", ip);
            sprintf(file_send_info, "%s %s", ip, next_send_filename);
            pthread_t tid;
            pthread_create(&tid, NULL, file_send, (void*)file_send_info);
        }
        else if(strcmp(type, "RECV_FILE") == 0)
        {
            pthread_t tid;
            pthread_create(&tid, NULL, file_recv, NULL);
        }
    }
}
