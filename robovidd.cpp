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
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define PORT "6666"
// hardcoded for now
#define IMGSIZE 921600
#define ROWS 1
#define COLS 307200

#define BACKLOG 10


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

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

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
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

    freeaddrinfo(servinfo);
    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof client_addr;
    Mat img = Mat(ROWS, COLS, CV_8UC3, 0.0);
    uchar buf[IMGSIZE];
    int bytes = 0;

    for (int i = 0; i < IMGSIZE; i += bytes) {
        if ((bytes = recvfrom(sockfd, buf + i, IMGSIZE - i, 0,
                    (struct sockaddr *)&client_addr, &addr_len)) == -1) {
            perror("recvfrom:");
            exit(1);
        }
    }

   int ptr=0;
   for (int i = 0;  i < img.rows; i++) {
       for (int j = 0; j < img.cols; j++) {                                     
           img.at<Vec3b>(i,j) = Vec3b(buf[ptr+ 0],buf[ptr+1],buf[ptr+2]);
           ptr=ptr+3;
       }
   }

   namedWindow("Display", WINDOW_AUTOSIZE);
   imshow("Display", img);


   close(sockfd);

   return 0;
    
}
