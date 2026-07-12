#include "QRScanner.h"

#include <QCoreApplication>
#include <QDir>

// 模型文件所在目录：优先放在可执行文件同级目录下（不依赖启动时的当前工作目录），
// 其次回退到当前工作目录下的 ./ScanModel。
static QString findScanModelDir()
{
    const QString nextToExe = QCoreApplication::applicationDirPath() + "/ScanModel";
    if (QDir(nextToExe).exists())
        return nextToExe;
    if (QDir("./ScanModel").exists())
        return "./ScanModel";
    return nextToExe; // 都不存在时退回默认路径，让 OpenCV 报错信息更清晰
}

QRScanner::QRScanner()
{
    const QString modelDir = findScanModelDir();
    const std::string detectPrototxt = (modelDir + "/detect.prototxt").toStdString();
    const std::string detectModel   = (modelDir + "/detect.caffemodel").toStdString();
    const std::string srPrototxt    = (modelDir + "/sr.prototxt").toStdString();
    const std::string srModel       = (modelDir + "/sr.caffemodel").toStdString();

    detector = cv::makePtr<cv::wechat_qrcode::WeChatQRCode>(detectPrototxt.c_str(),
                                                            detectModel.c_str(),
                                                            srPrototxt.c_str(),
                                                            srModel.c_str());
    detector->setScaleFactor(0.4);
}

QRScanner::~QRScanner()
{
}

void QRScanner::decodeSingle(const cv::Mat& img, std::string& qrCode)
{
#ifndef TESTSPEED
    auto startTime = std::chrono::high_resolution_clock::now();
#endif
    // 快速路径：经典 QRCodeDetector（纯 CV，无神经网络），单帧约 1~5ms。
    // 对于屏幕/直播里清晰、正向的登录二维码，绝大多数能直接解出，显著分流 WeChatQRCode 的开销。
    cv::Mat points;
    std::string fastDecoded = fastDetector.detectAndDecode(img, points);
    if (!fastDecoded.empty())
    {
        qrCode = fastDecoded;
    }
    else
    {
        // 兜底：WeChatQRCode（深度学习，更鲁棒但慢），用于经典检测器漏解的模糊/变形/小二维码。
        const std::vector<std::string>& strDecoded = detector->detectAndDecode(img);
        if (strDecoded.size() > 0)
        {
            qrCode = strDecoded[0];
        }
    }
#ifndef TESTSPEED
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << static_cast<float>(duration) / 1000000 << " decode: " << qrCode << std::endl;
#endif
}

void QRScanner::decodeMultiple(const cv::Mat& img, std::string& qrCode)
{
    const std::vector<std::string>& strDecoded = detector->detectAndDecode(img);
    for (int i = 0; i < strDecoded.size(); i++)
    {
        qrCode = strDecoded[i];
#ifdef _DEBUG
        std::cout << "decode:" << qrCode << std::endl;
#endif // DEBUG
    }
}