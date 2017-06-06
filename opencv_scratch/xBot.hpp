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

cv::Mat xbot_resize(cv::Mat img);
cv::Mat xbot_detectEdge(cv::Mat img);
void    xbot_findScreenFrame(cv::Mat img_edge);
void    xbot_sortByArea(std::vector<std::vector<cv::Point> > arr, int lo, int hi);
int     xbot_partition(std::vector<std::vector<cv::Point> > arr, int lo, int hi);

#endif /* xBot_hpp */