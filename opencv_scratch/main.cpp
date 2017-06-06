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
#include "xbot.hpp"




int main(int argc, const char * argv[])
{
    cv::Mat img = cv::imread("/Users/moults31/GoogleDrive/secret_hax/IMG_0419.png");

    cv::Mat img_resized = img.clone();
    double ratio = xbot_resize(img, &img_resized);
    
    cv::Mat img_edge = xbot_detectEdge(img_resized);
    
    xbot_findScreenFrame(img_edge);
    
    cv::destroyAllWindows();
    return 0;
}
