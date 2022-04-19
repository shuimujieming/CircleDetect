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

    Mat canny,dst,src;
    Rect ROI(400,300,500,60);
    src = imread("../brake.bmp");
    src = src(ROI);
    imshow("src",src);

    Canny(src,canny,10,50);
    imshow("canny",canny);
    cvtColor(canny,dst,COLOR_GRAY2BGR);

    vector<Vec4f> plines;//保存霍夫变换检测到的直线
    HoughLinesP(canny, plines, 1, CV_PI / 180, 2, 10, 3);//提取边缘时，会造成有些点不连续，所以maxLineGap设大点

    cout<<plines.size()<<endl;

    float max_x,min_x;
    int max_x_index,min_x_index;

    max_x = (plines[0][0] + plines[0][3]) / 2.0f;
    min_x = (plines[0][0] + plines[0][3]) / 2.0f;

    //5. 显示检测到的直线
    for (size_t i = 0; i < plines.size(); i++)
    {
    Vec4f hline = plines[i];
    if(((hline[0] + hline[2]) / 2.0f ) > max_x)
    {
        max_x = (hline[0] + hline[2]) / 2.0f;
        max_x_index = i;
    }
    if(((hline[0] + hline[2]) / 2.0f ) < min_x)
    {
        min_x = (hline[0] + hline[2]) / 2.0f;
        min_x_index = i;
    }
    //line(dst, Point(hline[0], hline[1]), Point(hline[2], hline[3]), Scalar(0, 0, 255), 1, LINE_AA);//绘制直线
    }
    Vec4f hline = plines[min_x_index];

    line(dst, Point(hline[0], hline[1]), Point(hline[2], hline[3]), Scalar(0, 0, 255), 1, LINE_AA);//绘制直线

    hline = plines[max_x_index];

    line(dst, Point(hline[0], hline[1]), Point(hline[2], hline[3]), Scalar(0, 0, 255), 1, LINE_AA);//绘制直线

    Point2f p1 ((plines[min_x_index][0] + plines[min_x_index][2]) / 2.0f,(plines[min_x_index][1] + plines[min_x_index][3]) / 2.0f);
    Point2f p2 ((plines[max_x_index][0] + plines[max_x_index][2]) / 2.0f,(plines[max_x_index][1] + plines[max_x_index][3]) / 2.0f);

    //像素当量
    float meter = (50.0 / getDistance(p1,p2));

    imshow("plines", dst);

    waitKey(0);


    //检测圆
    vector<Vec3f> circles;
    double dp = 2;
    double minDist = 10;
    double param1 = 100;
    double param2 = 100;
    int min_radius = 20;
    int max_radius = 100;

    //hough circle detect
    HoughCircles(image_gray,circles,HOUGH_GRADIENT,dp,minDist,param1,param2,min_radius,max_radius);

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

    //加入最后一??大圆
//    circles_rank.resize(circles_rank.size() + 1);
//    circles_rank.push_back(Vec4f(circle_max[0],circle_max[1],circle_max[2],0));

    cout<<circles_rank.size()<<endl;

    //依???画??
    for (int i = 0; i < circles_rank.size(); ++i) {
        Point center = Point(cvRound(circles_rank[i][0]),cvRound(circles_rank[i][1]));

        float radius = (circles_rank[i][2]);

        circle(image,center,3,Scalar(0,255,0),-1,8,0);
        circle(image,center,radius,Scalar(0,0,255),3,8,0);

//        float meter = (50.0 / (circles_rank[0][0] - circles_rank[5][0]));

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

    int pt_len = circles_rank.size();
    vector<Point2f> pts;
    for (int i = 0; i < circles_rank.size(); ++i) {
        Point2f p(circles_rank[i][0],circles_rank[i][1]);
        pts.push_back(p);
    }

    Mat A(pt_len, 3, CV_32FC1);
    Mat b(pt_len, 1, CV_32FC1);

// 下面的两个 for 循环初始化 A 和 b
    for (int i = 0; i < pt_len; i++)
    {
        float *pData = A.ptr<float>(i);

        pData[0] = pts[i].x * 2.0f;
        pData[1] = pts[i].y * 2.0f;

        pData[2] = 1.0f;
    }

    float *pb = (float *)b.data;

    for (int i = 0; i < pt_len; i++)
    {
        pb[i] = pts[i].x * pts[i].x + pts[i].y * pts[i].y;
    }

// 下面的几行代码就是解超定方程的最小二乘解
    Mat A_Trans;
    transpose(A, A_Trans);

    Mat Inv_A;
    invert(A_Trans * A, Inv_A);

    Mat res = Inv_A * A_Trans * b;

// 取出圆心和半径
    float x = res.at<float>(0, 0);
    float y = res.at<float>(1, 0);
    float r = (float)sqrt(x * x + y * y + res.at<float>(2, 0));

    circle(image,Point2f (x,y),r,Scalar(0,255,0),1,8,0);


    cout<<"x = "<<x<<" y = "<<y<<" r = "<<r*meter<<endl;

    resize(image,image,Size(),0.8,0.8);
    imshow("result",image);
    waitKey(0);
    return 0;
}
