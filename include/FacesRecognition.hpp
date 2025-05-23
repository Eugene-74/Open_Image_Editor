#pragma once

#include <opencv2/opencv.hpp>

std::vector<std::pair<cv::Rect, double>> getFaceRect(cv::Mat img);
cv::Mat detectEmbedding(cv::Mat face);
double cosineSimilarity(const cv::Mat& vec1, const cv::Mat& vec2);
