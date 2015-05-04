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
#include <opencv2/highgui/highgui.hpp>

#include <lcm/lcm-cpp.hpp>
#include "LCM_Image_Type.hpp"

#define SERVERPORT "3490"

using namespace std;
using namespace JMD;
using namespace cv;

class Handler
{
    public:
        ~Handler() {}
        Handler(const char *server_ip) { 
           int rv;
           struct addrinfo hints, *servinfo;
           memset(&hints, 0, sizeof hints);
           hints.ai_family = AF_UNSPEC;
           hints.ai_socktype = SOCK_DGRAM;
           if ((rv = getaddrinfo(server_ip, SERVERPORT, &hints, &servinfo)) != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
           }
           for(p = servinfo; p != NULL; p = p->ai_next) {
               if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                   perror("talker: socket");
                   continue;
               }
               break;
           }

           if (p == NULL) {
               fprintf(stderr, "failed to bind socket\n");
           }
        }
        void handleMessage(const lcm::ReceiveBuffer* rbuf,
                const string& chan,
                const LCM_Image_Type* msg)
        {
            int numbytes;
            Mat orig, gray, gray_resized;
            vector<uint8_t> data = msg->Data;
            orig = Mat(msg->Height, msg->Width, CV_8UC3, &data[0]);
            if (orig.empty())
                printf("empty!\n");
            cvtColor(orig, gray, COLOR_BGR2GRAY);
            resize(gray, gray_resized, Size(.45*480, .45*640));
            int data_len = gray_resized.total() * gray_resized.elemSize();
            if ((numbytes = sendto(sockfd, gray_resized.data, data_len, 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
                perror("robovidc: sendto");
                exit(1);
            }
            printf("num bytes sent - %d\n", numbytes);
        }
    private:
        int sockfd;
        struct addrinfo *p;


};

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "robovidc called without hostname, exiting\n");
        return -1;
    }
    // lcm 
    lcm::LCM lcm;
    if (!lcm.good()) {
        fprintf(stderr, "LCM initialization failed, returning \n");
        return -1;
    }
    Handler h(argv[1]);
    lcm.subscribe("EXAMPLE", &Handler::handleMessage, &h);
    printf("subscribed to EXAMPLE\n");
    while (0 == lcm.handle());
    printf("done\n");
	return 0;
}
