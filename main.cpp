#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

double getDistance (Point pointO,Point pointA )

{

    double distance;

    distance = powf((pointO.x - pointA.x),2) + powf((pointO.y - pointA.y),2);

    distance = sqrtf(distance);



    return distance;

}
int main() {
    Mat image = imread("../brake.bmp");
    imshow("origin",image);
    Mat image_gray;
    cvtColor(image,image_gray,COLOR_BGR2GRAY);
    GaussianBlur(image_gray,image_gray,Size(9,9),2,2);

    //检测圆
    vector<Vec3f> circles;
    double dp = 2;
    double minDist = 10;
    double param1 = 100;
    double param2 = 100;
    int min_radius = 20;
    int max_radius = 100;

    HoughCircles(image_gray,circles,HOUGH_GRADIENT,dp,minDist,param1,param2,min_radius,max_radius);

    Canny(image_gray, image_gray, 10, 30);
    imshow("result", image_gray);

    waitKey(0);
//    vector<Vec4f>plines;
//    HoughLinesP(image_gray, plines, 1, CV_PI / 180.0, 10, 0, 10);  //直线不连续，调最后一个参数（10）；
//    Scalar color = Scalar(0, 0, 255);
//    for (size_t i = 0; i < plines.size(); i++) {    //size_t理解为int
//        Vec4f hline = plines[i];
//        line(image, Point(hline[0], hline[1]), Point(hline[2], hline[3]), color, 3, LINE_AA);
//    }




    Vec3f circle_max;
    int maxradius_index = 0;
    int radius_max = 0;
    for (int i = 0; i < circles.size(); ++i) {
        if(radius_max <= circles[i][2])
        {
            maxradius_index = i;
            circle_max[0] = circles[i][0];
            circle_max[1] = circles[i][1];
            radius_max = circle_max[2] = circles[i][2];
        }
    }
    //去掉最大圆
    circles.erase(circles.begin() + maxradius_index);

    vector<Vec4f> circles_rank(circles.size());
    for (int i = 0; i < circles.size(); ++i) {

        for (int j = 0; j < 3; ++j) {
            circles_rank[i][j] = circles[i][j];
        }
        circles_rank[i][3] = atan2f(circles[i][0] - circle_max[0],circles[i][1] - circle_max[1]);
        //cout<< circles_rank[i][3] * (180.0 / 3.14) <<endl;
    }

    /* 冒泡排序 */
    {
        for (int i = 0; i < circles.size(); i++)
        {
            for (int j = 0; j < circles.size() -  i - 1; j++)
            {
                if (circles_rank[j][3] < circles_rank[j + 1][3])
                {
                    Vec4f temp;
                    temp = circles_rank[j + 1];
                    circles_rank[j + 1] = circles_rank[j];
                    circles_rank[j] = temp;
                }
            }
        }
    }

    //加入最后一个大圆
//    circles_rank.resize(circles_rank.size() + 1);
//    circles_rank.push_back(Vec4f(circle_max[0],circle_max[1],circle_max[2],0));

    cout<<circles_rank.size()<<endl;

    //依次画圆
    for (int i = 0; i < circles_rank.size(); ++i) {
        Point center = Point(cvRound(circles_rank[i][0]),cvRound(circles_rank[i][1]));

        float radius = (circles_rank[i][2]);

        circle(image,center,3,Scalar(0,255,0),-1,8,0);
        circle(image,center,radius,Scalar(0,0,255),3,8,0);

        float meter = (50.0 / (circles_rank[0][0] - circles_rank[5][0]));

        putText(image,to_string(i+1),center,FONT_HERSHEY_SIMPLEX,3,Scalar(255,0,0));
        putText(image,to_string(radius*2.0 * meter),center + Point(radius,radius),FONT_HERSHEY_SIMPLEX,1,Scalar(122,255,0));

        if(i < circles_rank.size() - 1)
        {
            Point p1 = Point(circles_rank[i][0],circles_rank[i][1]);
            Point p2 = Point(circles_rank[i+1][0],circles_rank[i+1][1]);
            line(image,p1,p2,Scalar(0,255,0));

            putText(image,to_string(sqrt(pow((p1.x - p2.x),2) + pow((p1.y - p2.y),2)) * meter),(p1 + p2) / 2.0,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0));

        }
        else
        {
            Point p1 = Point(circles_rank[i][0],circles_rank[i][1]);
            Point p2 = Point(circles_rank[0][0],circles_rank[0][1]);
            line(image,p1,p2,Scalar(0,255,0));
            putText(image,to_string(sqrt(pow((p1.x - p2.x),2) + pow((p1.y - p2.y),2)) * meter),(p1 + p2) / 2.0,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0));
        }

    }

    resize(image,image,Size(),0.8,0.8);
    imshow("result",image);
    waitKey(0);
    return 0;
}
