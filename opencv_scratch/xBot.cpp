//
//  xBot.cpp
//  opencv_scratch
//
//  Created by Zac Moulton on 2017-06-06.
//  Copyright © 2017 Xbot. All rights reserved.
//

#include "xBot.hpp"

/******************************************************
 * Algorithm done in 3 steps:
 * 1) Detect edge of tv screen
 * 2) Transform perspective to fill image with tv screen
 * 3) Read text from tv screen
 ******************************************************
 */


/******************************************************
 * METHODS FOR 1) DETECT EDGE OF TV SCREEN
 ******************************************************
 */

/* xbot_resize
 *
 * Brief:     Resize an input image to 500px width, retaining aspect ratio.
 *
 * Param img: Image to be resized
 * Param img_resized: Pointer to destination for resized image
 *
 * Return ratio: Factor by which we resized the original image
 */
double xbot_resize(cv::Mat img, cv::Mat *img_resized)
{
    double ratio = 500.0/img.cols;
    double height = ratio*img.rows;
    cv::Size size(500,height);
    cv::resize(img,*img_resized,size);
    
    return ratio;
}

/* xbot_detectEdge
 *
 * Brief:     Produce Canny edge detected image from raw photograph
 *
 * Param img: Raw photograph
 *
 * Return:    Edge grayscale image
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

/* xbot_findScreenFrame
 *
 * Brief:     Find rectangle corresponding to tv screen frame
 *
 * Param img_edge: Edge grayscale image
 *
 * Return: Vector that defines the tv screen corners.
 */
std::vector<cv::Point> xbot_findScreenFrame(const cv::Mat img)
{
    //Resize image to 500px width
    cv::Mat img_resized = img.clone();
    double ratio = xbot_resize(img, &img_resized);
    
    //Filter image with Canny edge detector
    cv::Mat img_edge = xbot_detectEdge(img_resized);
    
    std::vector<std::vector<cv::Point> > cnts;
    cv::findContours(img_edge, cnts, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    
    std::sort(cnts.begin(), cnts.end(), areaComp);
    
    double peri;
    std::vector<cv::Point> approx, rect;
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
    
    for(int i = 0; i < cnts[contourIdx].size(); i++)
    {
        cnts[contourIdx][i].x /= ratio;
        cnts[contourIdx][i].y /= ratio;
    }
    
    cv::Scalar color = cv::Scalar( 255, 255, 255 );
    cv::drawContours(img, cnts, contourIdx, color, 5, 8);
    cv::imshow("Outline", img);
    
    rect = approx;
    
    return rect;
}

/**********************************************************************
 * METHODS FOR 2) TRANSFORM PERSPECTIVE TO FILL IMAGE WITH TV SCREEN
 **********************************************************************
 */

/* xbot_perspectiveXform
 *
 * Brief:     Transform perspective to fill image with "tv screen"
 *
 * Param img: Raw photograph
 */
void xbot_perspectiveXform(cv::Mat img, std::vector<cv::Point> rect)
{
    
    
    
}

/* xbot_orderpts
 *
 * Brief:     Order points in the point vector that defines the tv screen corners.
 *
 * Param img: Raw photograph
 * Param rect: Vector that defines the tv screen corners.
 *
 * Return rect_ordered: rect but with points in the order top-left, top-right, bottom-right, bottom-left
 */
std::vector<cv::Point> xbot_orderpts(cv::Mat img, std::vector<cv::Point> rect)
{
    std::vector<cv::Point> rect_ordered;
    rect_ordered.reserve(4);
    
    //Initialize these with data from idx 0
    int sum_max  = rect[0].x + rect[0].y;
    int sum_min  = rect[0].x + rect[0].y;
    int diff_max = rect[0].x - rect[0].y;
    int diff_min = rect[0].x - rect[0].y;
    
    int idx_sum_max = 0;
    int idx_sum_min = 0;
    int idx_diff_max = 0;
    int idx_diff_min = 0;
    
    
    for(int i = 0; i < rect.size(); i++)
    {
        //Update max/min sum/diff if appropriate on this iteration.
        sum_max   = std::max(rect[i].x + rect[i].y, sum_max);
        sum_min   = std::min(rect[i].x + rect[i].y, sum_min);
        diff_max  = std::max(rect[i].x - rect[i].y, diff_max);
        diff_min  = std::min(rect[i].x - rect[i].y, diff_min);
        
        //If we updated one on this iteration, save which iteration we were on.
        //After all 4 iterations, these 4 values should be all different and in [0,3]
        idx_sum_max  = (sum_max == rect[i].x + rect[i].y)  ? i : idx_sum_max;
        idx_sum_min  = (sum_min == rect[i].x + rect[i].y)  ? i : idx_sum_min;
        idx_diff_max = (diff_max == rect[i].x - rect[i].y) ? i : idx_diff_max;
        idx_diff_min = (diff_min == rect[i].x - rect[i].y) ? i : idx_diff_min;
    }

    //Order the output vector appropriately:
    //Top-left has smallest sum
    //Bottom-right has largest sum
    //Top-right has smallest difference
    //Bottom-left has largest difference
    rect_ordered[0] = rect[idx_sum_min];
    rect_ordered[1] = rect[idx_diff_min];
    rect_ordered[2] = rect[idx_sum_max];
    rect_ordered[3] = rect[idx_diff_max];
    
    return rect_ordered;
}
