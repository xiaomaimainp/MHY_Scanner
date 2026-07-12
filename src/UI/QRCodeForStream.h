#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string_view>
#include <thread>

#include <opencv2/opencv.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
};

#include <QThread>
#include <QMutex>

#include "ApiDefs.hpp"
#include "ConfigDate.h"
#include "ScannerBase.hpp"

class QRCodeForStream final :
    public QThread,
    public ScannerBase
{
    Q_OBJECT
public:
    QRCodeForStream(QObject* parent = nullptr);
    ~QRCodeForStream();
    Q_DISABLE_COPY_MOVE(QRCodeForStream)

    void setLoginInfo(const std::string_view uid, const std::string_view gameToken);
    void setLoginInfo(const std::string_view uid, const std::string_view gameToken, const std::string& name);
    void setLoginInfo1(const std::string_view uid, const std::string_view stoken, const std::string_view mid);
    void setServerType(const ServerType servertype);
    void setUrl(const std::string& url, const std::map<std::string, std::string> heard = {});
    auto init() -> bool;
    void run();
    void stop();
    void continueLastLogin();

Q_SIGNALS:
    void loginResults(const ScanRet ret);
    void loginConfirm(const GameType gameType, bool b);

private:
    std::mutex mtx;                 // 保护登录/网络回调
    std::mutex m_frameMtx;          // 保护最新帧槽位
    std::condition_variable m_frameCv;
    cv::Mat m_latestFrame;          // 单槽位：只保留最新一帧，旧帧直接被覆盖
    bool m_hasNewFrame{ false };
    std::atomic<bool> m_decodeActive{ false }; // 解码线程是否应继续
    std::thread m_decodeThread;

    void LoginOfficial();
    void LoginBH3BiliBili();
    void setStreamHW();
    void decodeWorker();            // 独立解码线程：持续解"最新帧"
    void offerFrame(cv::Mat img);   // 把一帧交给解码线程（只保留最新帧）
    void drainStartupBuffer();      // 连上后快速丢弃 CDN 积压的旧帧，追上直播点再从最新帧解
    std::string streamUrl{};
    std::string m_name;
    ConfigDate* m_config;
    ServerType servertype;
    ScanRet ret = ScanRet::UNKNOW;
    AVDictionary* pAvdictionary;
    AVFormatContext* pAVFormatContext;
    AVCodecContext* pAVCodecContext;
    SwsContext* pSwsContext;
    AVFrame* pAVFrame;
    AVPacket* pAVPacket;
    int videoStreamIndex{ 0 };
    int videoStreamWidth{};
    int videoStreamHeight{};
    std::atomic<bool> m_stop;
};
