#pragma once

#include <string>

#include <opencv2/wechat_qrcode.hpp>
#include <opencv2/opencv.hpp>

class QRScanner
{
public:
	QRScanner();
	~QRScanner();
	void decodeSingle(const cv::Mat& img, std::string& qrCode);
	void decodeMultiple(const cv::Mat& img, std::string& qrCode);
private:
	cv::Ptr<cv::wechat_qrcode::WeChatQRCode> detector;
	cv::QRCodeDetector fastDetector; // 经典 QRCodeDetector：纯 CV、无神经网络，单帧约 1~5ms，作为快速优先路径
};