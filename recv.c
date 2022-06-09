#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

void recv_mail()
{
    const char* host_name = "pop.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "*****@qq.com"; // TODO: Specify the user
    const char* pass = "*****"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    s_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = ((((port)&0xFF) << 8) | (((port) >> 8) & 0xFF));
    bzero(servaddr.sin_zero, 8);
    servaddr.sin_addr.s_addr = inet_addr(dest_ip);
    connect(s_fd, (const struct sockaddr *)(&servaddr), sizeof(struct sockaddr));

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    char USER[50];
    strcpy(USER, "user ");
    strcat(USER, user);
    strcat(USER, "\r\n");
    send(s_fd, USER, strlen(USER), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);

    char PASS[50];
    strcpy(PASS, "pass ");
    strcat(PASS, pass);
    strcat(PASS, "\r\n");
    send(s_fd, PASS, strlen(PASS), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);    
    // TODO: Send STAT command and print server response
    const char* STAT = "stat\r\n";
    send(s_fd, STAT, strlen(STAT), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);    
    // TODO: Send LIST command and print server response
    const char* LIST = "list\r\n";
    send(s_fd, LIST, strlen(LIST), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);    

    // TODO: Retrieve the first mail and print its content
    const char* RETR = "retr 1\r\n";
    send(s_fd, RETR, strlen(RETR), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);

    // TODO: Send QUIT command and print server response
    const char* quit = "quit\r\n";
    send(s_fd, quit, strlen(quit), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    printf("\r\n");
    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
