//
//  xBot.hpp
//  opencv_scratch
//
//  Created by Zac Moulton on 2017-06-06.
//  Copyright © 2017 Xbot. All rights reserved.
//

#ifndef xBot_hpp
#define xBot_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

//structs and enums
typedef enum
{
    COIN_BALANCE = 0,
    XBOT_ROI_MAX
} xbot_roi_t;

struct
{
    bool operator()(std::vector<cv::Point> a, std::vector<cv::Point> b) const
    {
        return cv::contourArea(a) > cv::contourArea(b);
    }
} xbot_areaComp;

//Methods for Step 1: Detecting frame of tv
double  xbot_resize(cv::Mat img, cv::Mat *img_resized);
cv::Mat xbot_detectEdge(cv::Mat img);
std::vector<cv::Point2f>    xbot_findScreenFrame(cv::Mat img_edge);
void    xbot_sortByArea(std::vector<std::vector<cv::Point2f> > arr, int lo, int hi);
int     xbot_partition(std::vector<std::vector<cv::Point2f> > arr, int lo, int hi);

//Methods for Step 2: Perspective Transform
std::vector<cv::Point2f> xbot_orderpts(std::vector<cv::Point2f> rect);
cv::Mat xbot_perspectiveXform(const cv::Mat img, std::vector<cv::Point2f> rect);

//Methods for Step 3: Parsing text
cv::Mat xbot_crop(cv::Mat img_in, xbot_roi_t roi);
char *xbot_parseText(cv::Mat img, tesseract::TessBaseAPI& ocr);
int xbot_getCoinBalance(cv::Mat img, tesseract::TessBaseAPI& ocr);

#endif /* xBot_hpp */
