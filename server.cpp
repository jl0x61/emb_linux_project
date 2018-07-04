#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <map>
#include <vector>
#include <string>
#define SERV_PORT 8023
#define MAXMSG 1024
#define ID_SIZE 7

std::map<int, std::vector<int> > groups;
std::map<int, struct sockaddr_in> id_to_addr;
void debug_addr(struct sockaddr_in addr)
{
    char ip[64];
    inet_ntop(AF_INET, &addr.sin_addr, ip, 64);
    int port = htons(addr.sin_port);
    printf("ip: %s port: %d\n", ip, port);
}

void addr_to_ipport(struct sockaddr_in addr, char *ip, int &port)
{
    inet_ntop(AF_INET, &addr.sin_addr, ip, 64);
    port = htons(addr.sin_port);
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

int get_random_id()
{
    int id = rand()%99999+1;
    while(groups.count(id))
        id = rand()%99999+1;
    return id;
}
int main(int argc, char **argv)
{
    srand(time(0));
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));


    int clilen = sizeof(cliaddr);
    socklen_t len;
    char msg[MAXMSG];
    char rspmsg[MAXMSG];
    int from_id, to_id;
    char request_type[64];
    char body[MAXMSG];
    for(;;)
    {
        int n;
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, msg, MAXMSG, 0, (struct sockaddr*)&cliaddr, &len);
        msg[n] = '\0';
        char ip[64];
        int port;
        addr_to_ipport(cliaddr, ip, port);
        bool ok = parse_request(msg, from_id, to_id, request_type, body);
        printf("get a message from ip=%s, port=%d, type=%s\n", ip, port, request_type);
        if(strcmp(request_type, "LOGIN") == 0) 
        {
            if(from_id != 0 || groups.count(from_id))
                id_to_addr[from_id] = cliaddr;
            else
            {
                int id = get_random_id();
                groups[id] = std::vector<int>();
                groups[id].push_back(id);
                id_to_addr[id] = cliaddr;
                sprintf(rspmsg, "%d %d\n%s\n", 0, id, "MEMBER_ID");
                debug_addr(cliaddr);
                sendto(sockfd, rspmsg, strlen(rspmsg), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            }
        }
        else if(strcmp(request_type, "SEND") == 0)
        {
            if(!groups.count(to_id))
                continue;
            for(int id : groups[to_id])
            {
                if(id == from_id) 
                    continue;
                sprintf(rspmsg, "%d %d\n%s\n%s", from_id, id, "SEND", body);
                sendto(sockfd, rspmsg, strlen(rspmsg), 0, (struct sockaddr*)&id_to_addr[id], sizeof(cliaddr));
            }
        }
        else if(strcmp(request_type, "CREATE_GROUP") == 0)
        {
            int group_id = get_random_id();
            groups[group_id] = std::vector<int>();
            groups[group_id].push_back(from_id);
            sprintf(rspmsg, "%d %d\n%s\n", group_id, from_id, "GROUP_ID");
            sendto(sockfd, rspmsg, strlen(rspmsg), 0, (struct sockaddr*)&id_to_addr[from_id], sizeof(cliaddr));
        }
        else if(strcmp(request_type, "ENTER_GROUP") == 0)
        {
            groups[to_id].push_back(from_id);
        }
    }
}

