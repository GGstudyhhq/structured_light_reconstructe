#ifndef __CALPHASE_H
#define __CALPHASE_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <time.h>
#include "CameraCalib.h"

const int gHeight = 1600;
const int gWidth = 1200;

cv::Mat CalWrappedPhase(const std::string &Rect_images);

void UnwrappedPhaseClassicMethod(cv::Mat& src, cv::Mat& dst);
void UnwrappedPhaseGraycodeMethod(cv::Mat& src, cv::Mat& dst, const std::string &Rect_images);

#endif