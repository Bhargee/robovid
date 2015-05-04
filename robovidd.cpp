#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <opencv2/opencv.hpp>

#define MYPORT "3490"	// the port users will be connecting to

#define MAXBUFLEN 62208

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
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	uchar buf[MAXBUFLEN] = {0};
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("robovidd: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("robovidd: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "robovidd: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("robovidd: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
    Mat frame = Mat::zeros(.45 * 480, .45 * 640, CV_8U);
    while (true) {
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        /*int ptr = 0;
        for (int i = 0; i < .45 * 480; i++) {
            for (int j = 0; j < .45 * 640; j++) {
                frame.at<uchar>(i,j) = buf[ptr];
                ptr++;
            }
        }*/
        namedWindow("display");
        imshow("display", frame);
        waitKey(0);
    }
    close(sockfd);
    return 0;
}
