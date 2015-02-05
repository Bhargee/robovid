#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <opencv2/opencv.hpp>

using namespace cv;


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes, bytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    int img_size = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: hostname port\n");
        exit(1);
    }

    printf("Connecting to server %s at port %s\n", argv[1], argv[2]);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo))) {
        fprintf(stderr, "getaddrinfo failed with %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, 
                        p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Socket error");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Connect error");
            continue;
        }

        break;
    }

    if (!p) {
        fprintf(stderr, "client failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    VideoCapture cap(0);
    if (!cap.isOpened()) 
        return -1;
    printf("Beginning video capture...\n");

    // capture RGB frames and transmit
    Mat frame;
    while (true) {
        cap >> frame;
        frame = frame.reshape(0,1);
        int img_size = frame.total() * frame.elemSize();
        if (getchar() == 'q')
            break;
        if (!img_size) {
            img_size = frame.total() * frame.elemSize();
        }
        bytes = send(sockfd, frame.data, img_size, 0);
        if (bytes != img_size) {
            printf("Send incomplete: sent %d, server recieved %d", img_size, bytes);
        }
    }

    close(sockfd);
    return 0;
}

