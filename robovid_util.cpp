#include <opencv2/opencv.hpp>
 
// This file exists to screw around with images for the purposes of
// making the app better

using namespace cv;
using namespace std;

uchar *vec_to_arr(vector<uchar> compressed_data) {
    unsigned char *ret_buf = new unsigned char[compressed_data.size()];
    for (int i = 0; i < compressed_data.size(); i++) {
        ret_buf[i] = compressed_data[i];
    }
    return ret_buf;
}
int main(int argc, char *argv[]) {
    Mat frame, frame_gray;
    char input;
    vector<uchar> compressed_buff;
    vector<int> params = vector<int>(2);
    params[0] = IMWRITE_JPEG_QUALITY;
    params[1] = 90;
    namedWindow("display");
    
    printf("Waiting for video capture...\n");

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        return -1;
    }

    printf("Starting video capture\n");
    while ((input = getchar()) != 'q') {
        cap >> frame;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
        imencode(".jpg", frame_gray, compressed_buff, params);
        cout << "compressed size - " << compressed_buff.size() << endl;
        unsigned short data_len = compressed_buff.size();
        uchar buf[2];
        buf[0] = data_len & 0xff;
        buf[1] = (data_len >> 8) & 0xff;
        unsigned short recomb = (((unsigned short)buf[1]) << 8) | buf[0];
        printf("split data recombines to: %u\n", recomb);
        uchar *to_net = vec_to_arr(compressed_buff);
        printf("array!");
        delete[] to_net;

    }

    return 0;
}
