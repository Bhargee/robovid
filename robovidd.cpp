#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <opencv2/opencv.hpp>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     
#define MAX_BUFF_LEN 65536

using namespace cv;
using namespace std;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    printf("server: waiting for connections...\n");

    addr_len = sizeof their_addr;
    uchar buf[MAX_BUFF_LEN];
    Mat img;
    unsigned short data_len;
    namedWindow("display");
    while (true) {
        if (recvfrom(sockfd, buf, MAX_BUFF_LEN-1, 0,
                    (struct sockaddr *) &their_addr, &addr_len) == -1) {
            perror("recvfrom");
            exit(1);
        }
        printf("Got some bytes boi\n");
        data_len = (((unsigned short)buf[0]) << 8) | buf[1];
        vector<uchar> data(&buf[2], &buf[2] + data_len);    
        img = imdecode(data, 0);
        imshow("display", img);
        waitKey(0);
    }

    close(sockfd);
    return 0;
}
