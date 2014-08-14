#ifndef _UTILS_H_
#define _UTILS_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void gradient_energe(const cv::Mat& I,cv::Mat& Ienergy);
void saliency_map(const cv::Mat& I,cv::Mat& Is);

#endif