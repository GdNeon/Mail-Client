#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

char buf[MAX_SIZE+1];

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "*****"; // TODO: Specify the user
    const char* pass = "*****"; // TODO: Specify the password
    const char* from = "*****"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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

    // Send EHLO command and print server response
    const char* EHLO = "EHLO qq.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    // TODO: Print server response to EHLO command
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    
    char USER[50];
    strcpy(USER, user);
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
    strcpy(PASS, pass);
    strcat(PASS, "\r\n");
    send(s_fd, PASS, strlen(PASS), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    
    // TODO: Send MAIL FROM command and print server response
    char MAIL[100]; 
    strcpy(MAIL, "MAIL FROM:<");
    strcat(MAIL, from);
    strcat(MAIL, ">\r\n");
    send(s_fd, MAIL, strlen(MAIL), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send RCPT TO command and print server response
    char RCPT[100];
    strcpy(RCPT, "RCPT TO:<");
    strcat(RCPT, receiver);
    strcat(RCPT, ">\r\n");
    send(s_fd, RCPT, strlen(RCPT), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send DATA command and print server response
    const char* data = "data\r\n";
    send(s_fd, data, strlen(data), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send message data
    char message[65536];
    strcpy(message, "from:");
    strcat(message, from);
    strcat(message, "\r\n");
    strcat(message, "to:");
    strcat(message, receiver);
    strcat(message, "\r\n");
    strcat(message, "MIME-Version: 1.0\r\n");
    strcat(message, "Content-Type:multipart/mixed; boundary=abcd1234\r\n");
    strcat(message, "subject:");
    strcat(message, subject);
    strcat(message, "\r\n");
    
    strcat(message, "--abcd1234\r\n");
    strcat(message, "Content-Type: text/plain\r\n\r\n");

    FILE *f = fopen(msg, "r");
    char str[100];
    fgets(str, 100, f);
    strcat(message, str);
    strcat(message, "\r\n\r\n");
    fclose(f);

    strcat(message, "--abcd1234\r\n");
    strcat(message, "Content-Type: application/octet-stream; name=");
    strcat(message, att_path);
    strcat(message, "\r\n");
    strcat(message, "Content-Transfer-Encoding: base64\r\n");

    FILE *att = fopen(att_path, "r");
    FILE *att64 = fopen("file64", "w+");
    encode_file(att, att64);
    fseek(att64, 0, SEEK_SET);
    char file[32768];
    fread(file, 1, 32768, att64);
    strcat(message, file);
    fclose(att);
    fclose(att64);
    remove("file64");
    strcat(message, end_msg);
    send(s_fd, message, strlen(message), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Message ends with a single period

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
    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
