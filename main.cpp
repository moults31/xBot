//
//  main.cpp
//  opencv_scratch
//
//  Created by Zac Moulton on 2017-06-05.
//  Copyright © 2017 Xbot. All rights reserved.
// Algorithm adapted from:
// http://www.pyimagesearch.com/2014/09/01/build-kick-ass-mobile-document-scanner-just-5-minutes/
//and
//http://www.pyimagesearch.com/2014/08/25/4-point-opencv-getperspective-transform-example/
//
//
//
//

#include <iostream>
#include <opencv2/opencv.hpp>

cv::Mat xbot_detectEdge(cv::Mat img);
void    xbot_findScreenFrame(cv::Mat img_edge);
void    xbot_sortByArea(std::vector<std::vector<cv::Point> > arr, int lo, int hi);
int     xbot_partition(std::vector<std::vector<cv::Point> > arr, int lo, int hi);
cv::Mat xbot_resize(cv::Mat img);

int main(int argc, const char * argv[])
{
    cv::Mat img = cv::imread("/Users/moults31/GoogleDrive/secret_hax/IMG_0419.png");

    cv::Mat img_resized = xbot_resize(img);
    
    cv::Mat img_edge = xbot_detectEdge(img_resized);
    
    xbot_findScreenFrame(img_edge);
    
    cv::destroyAllWindows();
    return 0;
}

/* xbot_resize
 *
 * Brief:     Resize an input image to 500px width, retaining aspect ratio.
 *
 * Param img: Image to be resized
 *
 * Return:    Resized image
 */
cv::Mat xbot_resize(cv::Mat img)
{
    double ratio = 500.0/img.cols;
    double height = ratio*img.rows;
    cv::Size size(500,height);
    cv::resize(img,img,size);
    
    return img;
}

/* xbot_detectEdge
 * 
 * Brief:     Resize an input image to 500px width, retaining aspect ratio.
 *
 * Param img: Image to be resized
 *
 * Return:    Resized image
 */
cv::Mat xbot_detectEdge(cv::Mat img)
{
    cv::Mat img_bw = img.clone(), img_blur = img.clone(), img_edge = img.clone();
    cv::namedWindow("Raw", CV_WINDOW_AUTOSIZE);
    cv::imshow("Raw", img );
    
    cv::cvtColor(img, img_bw, CV_RGB2GRAY);
    cv::namedWindow("BW", CV_WINDOW_AUTOSIZE);
    cv::imshow("BW", img_bw );
    
    cv::GaussianBlur(img_bw, img_blur, cv::Size(5,5), 0);
    cv::namedWindow("Blur", CV_WINDOW_AUTOSIZE);
    cv::imshow("Blur", img_blur );
    
    cv::Canny(img_blur, img_edge, 75, 200);
    cv::namedWindow("Edge", CV_WINDOW_AUTOSIZE);
    cv::imshow("Edge", img_edge );
    
    //cvWaitKey(0);
    
    return img_edge;
}

struct
{
    bool operator()(std::vector<cv::Point> a, std::vector<cv::Point> b) const
    {
        return cv::contourArea(a) > cv::contourArea(b);
    }
} areaComp;

void xbot_findScreenFrame(const cv::Mat img_edge)
{
    cv::Mat img = img_edge.clone();
    
    std::vector<std::vector<cv::Point> > cnts;
    cv::findContours(img_edge, cnts, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    
    std::sort(cnts.begin(), cnts.end(), areaComp);
    
    double peri;
    std::vector<cv::Point> approx, screenCnt;
    int contourIdx;
    
    for(size_t c = 0; c < cnts.size(); c++)
    {
        peri = cv::arcLength(cnts[c], true);
        cv::approxPolyDP(cnts[c], approx, 0.02 * peri, true);
        
        if( approx.size() == 4)
        {
            contourIdx = (int)c;
            std::cout << "\n";
            std::cout << contourIdx;
            break;
        }
    }
    cv::Scalar color = cv::Scalar( 255, 255, 255 );
    cv::drawContours(img, cnts, contourIdx, color, 5, 8);
    cv::imshow("Outline", img);
    
    cvWaitKey(0);
}
