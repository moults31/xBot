//
//  xBot.cpp
//  opencv_scratch
//
//  Created by Zac Moulton on 2017-06-06.
//  Copyright © 2017 Xbot. All rights reserved.
//


#include "xBot.hpp"
#include <math.h>

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
 * Brief:             Resize an input image to 500px width, retaining aspect ratio.
 *
 * Param img:         Image to be resized
 * Param img_resized: Pointer to destination for resized image
 *
 * Return ratio:      Factor by which we resized the original image
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
    //Display original image for reference
    cv::Mat img_bw = img.clone(), img_blur = img.clone(), img_edge = img.clone();
    cv::namedWindow("Raw", CV_WINDOW_AUTOSIZE);
    cv::imshow("Raw", img );
    
    //Convert from RGB to grayscale
    cv::cvtColor(img, img_bw, CV_RGB2GRAY);
    cv::namedWindow("BW", CV_WINDOW_AUTOSIZE);
    cv::imshow("BW", img_bw );
    
    //Apply a Gaussian low-pass "blurring" filter
    cv::GaussianBlur(img_bw, img_blur, cv::Size(5,5), 0);
    cv::namedWindow("Blur", CV_WINDOW_AUTOSIZE);
    cv::imshow("Blur", img_blur );
    
    //Apply a Canny edge detection filter
    cv::Canny(img_blur, img_edge, 75, 200);
    cv::namedWindow("Edge", CV_WINDOW_AUTOSIZE);
    cv::imshow("Edge", img_edge );
    
    //cvWaitKey(0);
    
    return img_edge;
}

/* xbot_findScreenFrame
 *
 * Brief:          Find rectangle corresponding to tv screen frame
 *
 * Param img_edge: Edge grayscale image
 *
 * Return:         Vector that defines the tv screen corners.
 */
std::vector<cv::Point2f> xbot_findScreenFrame(const cv::Mat img)
{
    int contourIdx = 0;
    double peri;
    std::vector<cv::Point2f> approx, rect;
    std::vector<std::vector<cv::Point> > cnts;
    
    //Resize image to 500px width
    cv::Mat img_resized = img.clone();
    double ratio = xbot_resize(img, &img_resized);
    
    //Filter image with Canny edge detector
    cv::Mat img_edge = xbot_detectEdge(img_resized);
    
    //Find all contours in edge image and sort by area in descending order
    cv::findContours(img_edge, cnts, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    std::sort(cnts.begin(), cnts.end(), xbot_areaComp);
    
    //Loop through contours and stop as soon as we find one with 4 sides.
    //We assume that the largest 4-sided contour is the frame of our tv screen.
    for(size_t c = 0; c < cnts.size(); c++)
    {
        peri = cv::arcLength(cnts[c], true);
        cv::approxPolyDP(cnts[c], approx, 0.02 * peri, true);
        
        if( approx.size() == 4)
        {
            contourIdx = (int)c;
            break;
        }
    }
    
    //Resize the contour we found to match og image size
    for(int i = 0; i < approx.size(); i++)
    {
        approx[i].x /= ratio;
        approx[i].y /= ratio;
    }
    
    //Display contour superimposed on edge image
    cv::Scalar color = cv::Scalar( 255, 255, 255 );
    cv::drawContours(img_edge, cnts, contourIdx, color, 5, 8);
    cv::imshow("Outline", img_edge);
    
    return approx;
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
 *
 * Return:    Image with "tv screen" filling the image
 */
cv::Mat xbot_perspectiveXform(const cv::Mat img, std::vector<cv::Point2f> rect)
{
    //Rearrange contour to the proper order
    std::vector<cv::Point2f> rect_ordered = xbot_orderpts(rect);
    
    //Create a separate variable for each corner for clarity
    cv::Point tl = rect_ordered[0];
    cv::Point tr = rect_ordered[1];
    cv::Point br = rect_ordered[2];
    cv::Point bl = rect_ordered[3];
    
    //We need to first get the height and width of our new image.
    //Width will be the greater distance between bl->br or tl->tr
    float widthA = sqrt(pow(std::abs(br.x-bl.x),2) + pow(std::abs(br.y - bl.y),2));
    float widthB = sqrt(pow(std::abs(tr.x-tl.x),2) + pow(std::abs(tr.y - tl.y),2));
    float width = std::max(widthA,widthB);
    
    //Similarly height is greater distance between tl->bl or tr->br
    float heightA = sqrt(pow(std::abs(tl.x-bl.x),2) + pow(std::abs(tl.y - bl.y),2));
    float heightB = sqrt(pow(std::abs(tr.x-br.x),2) + pow(std::abs(tr.y - br.y),2));
    float height = std::max(heightA,heightB);
    
    //Now we define a vector with the 4 pts representing our new image corners
    std::vector<cv::Point2f> dst = rect_ordered;
    dst.reserve(4);
    
    //Set the 4 corners in the same order as before (tl->tr->br->bl)
    dst[0].x = 0;
    dst[0].y = 0;
    dst[1].y = width - 1;
    dst[1].x = 0;
    dst[2].y = width - 1;
    dst[2].x = height - 1;
    dst[3].y = 0;
    dst[3].x = height - 1;
    
    //Use our contour and the new matrix to do a perspective transform
    cv::Mat M = cv::getPerspectiveTransform(rect_ordered, dst);
    cv::Mat img_warped;
    cv::warpPerspective(img, img_warped, M, cv::Size(height,width));
    
    //Display the final product
    cv::imshow("Warped", img_warped);
    
    return img_warped;
}

/* xbot_orderpts
 *
 * Brief:      Order points in the point vector that defines the tv screen corners.
 *
 * Param img:  Raw photograph
 * Param rect: Vector that defines the tv screen corners.
 *
 * Return rect_ordered: rect but with points in the order top-left, top-right, bottom-right, bottom-left
 */
std::vector<cv::Point2f> xbot_orderpts(std::vector<cv::Point2f> rect)
{
    std::vector<cv::Point2f> rect_ordered = rect;
    rect_ordered.reserve(4);
    
    //Initialize these with data from idx 0
    float sum_max  = rect[0].x + rect[0].y;
    float sum_min  = rect[0].x + rect[0].y;
    float diff_max = rect[0].x - rect[0].y;
    float diff_min = rect[0].x - rect[0].y;
    
    int idx_sum_max = 0;
    int idx_sum_min = 0;
    int idx_diff_max = 0;
    int idx_diff_min = 0;
    
    
    for(int i = 0; i < rect.size(); i++)
    {
        //Update max/min sum/diff if appropriate on this iteration.
        sum_max = (rect[i].x + rect[i].y > sum_max) ? (rect[i].x + rect[i].y) : sum_max;
        sum_min = (rect[i].x + rect[i].y < sum_min) ? (rect[i].x + rect[i].y) : sum_min;
        diff_max = (rect[i].x - rect[i].y > diff_max) ? (rect[i].x - rect[i].y) : diff_max;
        diff_min = (rect[i].x - rect[i].y < diff_min) ? (rect[i].x - rect[i].y) : diff_min;
        
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


/**********************************************************************
 * METHODS FOR 3) PARSE TEXT FROM TV SCREEN
 **********************************************************************
 */

/* xbot_crop
 *
 * Brief:        Crop image to specified region of interest.
 *
 * Param img_in: Image to crop.
 * Param roi:    Region of image to return.
 *
 * Return img_cropped: Section of img_in specified by roi argument.
 */
cv::Mat xbot_crop(cv::Mat img_in, xbot_roi_t roi)
{
    //Initialize roi as entire img_in
    int roi_x = 0;
    int roi_y = 0;
    int roi_width = (int) img_in.size().width;
    int roi_height = (int) img_in.size().height;
    
    //set the roi x,y,width,height parameters based on the
    //section of the image we need to return
    switch(roi)
    {
        case COIN_BALANCE:
            //Limit region of interest to coin balance.
            //Use ratios of width and height from testing.
            roi_x = (int) (img_in.size().width * 2.0 / 3.0);
            roi_y = 0;
            roi_width = (int) (img_in.size().width * 1.0 / 3.0);
            roi_height = (int) (img_in.size().height / 7.0);
            break;
            
        case XBOT_ROI_MAX:
        default:
            std::cout << "\nSpecified an undefined region of interest to crop.";
            std::cout << "\nReturning img_in untouched.\n";
    }
    
    //crop the image and display the result in a window
    cv::Mat img_cropped = img_in(cv::Rect(roi_x, roi_y, roi_width, roi_height));
    cv::imshow("Cropped", img_cropped);
    
    return img_cropped;
}

/* xbot_parseText
 *
 * Brief:        Parse the text in a given image
 *
 * Param img:    Image to get text from.
 * Param ocr:    Tesseract ocr object to use.
 *
 * Return out:   Pointer to head of string containing text.
 */
char *xbot_parseText(cv::Mat img, tesseract::TessBaseAPI& ocr)
{
    ocr.SetImage((uchar*)img.data, img.size().width, img.size().height, img.channels(), (int)img.step1());
    ocr.Recognize(0);
    char* out = ocr.GetUTF8Text();
    return out;
}

/* xbot_getCoinBalance
 *
 * Brief:        Get the user's current coin balance
 *
 * Param img:    Image to get text from
 * Param ocr:    Tesseract ocr object to use.
 *
 * Return balval:   Coin balance value as an int
 */
int xbot_getCoinBalance(cv::Mat img, tesseract::TessBaseAPI& ocr)
{
    //crop the og image to get only the coin balance tile
    cv::Mat img_cropped = xbot_crop(img, COIN_BALANCE);
    cv::imshow("sub", img_cropped);

    //generate and report a string containing the text on the coin balance tile
    char* bal = xbot_parseText(img_cropped, ocr);
    std::cout << bal;
    
    //extract the coin value as an int from the string
    //@TODO
    int balval = 0;
    
    return balval;
}
