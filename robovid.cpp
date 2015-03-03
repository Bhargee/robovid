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

#define PORT "3490" // the port client will be connecting to 

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

uchar *vec_to_arr(vector<uchar> compressed_data) {
    uchar *ret_buf = new uchar(compressed_data.size());
    for (int i = 0; i < compressed_data.size(); i++) {
        ret_buf[i] = compressed_data[i];
    }
    return ret_buf;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    if (argc != 2) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
    
    VideoCapture cap(0);
    if (!cap.isOpened()) 
        return -1;
    printf("Beginning video capture...\n");

    // capture RGB frames and transmit
    Mat frame, frame_gray;
    char input;
    vector<uchar> compressed_buff;
    vector<int> params = vector<int>(2);
    params[0] = IMWRITE_JPEG_QUALITY;
    params[1] = 80;

    while (true) {
        cap >> frame;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
        imencode(".jpg", frame_gray, compressed_buff, params);
        unsigned short data_len = compressed_buff.size();
        uchar lower = data_len & 0xff;
        uchar upper = (data_len >> 8) & 0xff;
        vector<uchar>::iterator it = compressed_buff.begin();
        it = compressed_buff.insert(it, lower);
        compressed_buff.insert(it, upper);
        printf("compressed buffer size - %lu\n", compressed_buff.size());
        //uchar *to_net = vec_to_arr(compressed_buff);
        if((numbytes = sendto(sockfd, compressed_buff.data(), compressed_buff.size(), 0,
                        (struct sockaddr *) p->ai_addr, p->ai_addrlen)) == -1) {
            perror("sendto ");
            exit(1);
        }
        //delete[] to_net;
    }

    close(sockfd);
    return 0;
}

