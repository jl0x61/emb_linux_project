#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXMSG 1024
#define SERV_PORT 8023
#define ID_SIZE 7
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
    }
}
