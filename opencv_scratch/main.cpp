//
//  main.cpp
//  opencv_scratch
//
//  Created by Zac Moulton on 2017-06-05.
//  Copyright © 2017 Xbot. All rights reserved.
// Algorithm adapted from:
// http://www.pyimagesearch.com/2014/09/01/build-kick-ass-mobile-document-scanner-just-5-minutes/
// and
// http://www.pyimagesearch.com/2014/08/25/4-point-opencv-getperspective-transform-example/
//
//
//
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "xbot.hpp"

int main(int argc, const char * argv[])
{
    //Load image to process
    cv::Mat img = cv::imread("/Users/moults31/GoogleDrive/secret_hax/IMG_0419.png");
    
    //Find the frame of the tv screen
    std::vector<cv::Point2f> rect = xbot_findScreenFrame(img);
    
    //Transform perspective
    cv::Mat img_warped = xbot_perspectiveXform(img, rect);
    
    cvWaitKey(0);
    cv::destroyAllWindows();
    return 0;
}
