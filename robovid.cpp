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

#define SERVERPORT "3490"	// the port users will be connecting to

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
    //opencv specific vars
    Mat frame, frame_gray;

	if (argc != 2) {
		fprintf(stderr,"usage: robovidc hostname \n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "failed to bind socket\n");
		return 2;
	}
   
    printf("Waiting for video capture...\n");

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        return -1;
    }

    printf("Starting video capture\n");
    while (true) {
        cap >> frame;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
        resize(frame_gray,frame_gray, Size(0,0), .45 * 480, .45 * 640);
        frame.reshape(0, 1);
        int data_len = frame_gray.total() * frame_gray.elemSize();
        if ((numbytes = sendto(sockfd, frame_gray.data, data_len, 0,
                 p->ai_addr, p->ai_addrlen)) == -1) {
            perror("robovidc: sendto");
            exit(1);
        }
    }

	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}
