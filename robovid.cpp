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

#define SERVERPORT "4950"	// the port users will be connecting to

using namespace cv;
using namespace std;

void fmt_data(uchar* data, uchar *prefix, uchar *res, int size) {
    res[0] = prefix[0];
    res[1] = prefix[1];
    for (int i = 0; i < size; i++) {
        res[i+2] = data[i];
    }
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
    //opencv specific vars
    Mat frame, frame_gray;
    char input;
    vector<uchar> compressed_buff;
    vector<int> params = vector<int>(2);
    params[0] = IMWRITE_JPEG_QUALITY;
    params[1] = 20;
 
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
        imencode(".jpg", frame_gray, compressed_buff, params);
#ifdef DEBUG
        cout << "compressed size - " << compressed_buff.size() << endl;
#endif
        unsigned short data_len = compressed_buff.size();
        if ((numbytes = sendto(sockfd, compressed_buff.data(), data_len, 0,
                 p->ai_addr, p->ai_addrlen)) == -1) {
            perror("robovidc: sendto");
            exit(1);
        }
    }

	freeaddrinfo(servinfo);

	printf("robovidc: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);
	return 0;
}
