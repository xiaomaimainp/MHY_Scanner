#include "QRCodeForStream.h"

#include <string>
#include <string_view>

#include "QRScanner.h"
#include "MhyApi.hpp"

QRCodeForStream::QRCodeForStream(QObject* parent) :
    QThread(parent),
    pAvdictionary(nullptr),
    pAVFormatContext(nullptr),
    pSwsContext(nullptr),
    pAVFrame(nullptr),
    pAVPacket(nullptr),
    pAVCodecContext(nullptr),
    m_stop(false),
    servertype(ServerType::Official)

{
    av_log_set_level(AV_LOG_FATAL);
    m_config = &(ConfigDate::getInstance());
}

QRCodeForStream::~QRCodeForStream()
{
    if (!this->isInterruptionRequested())
    {
        m_stop.store(false);
    }
    this->requestInterruption();
    this->wait();
}

void QRCodeForStream::setLoginInfo(const std::string_view uid, const std::string_view gameToken)
{
    this->uid = uid;
    this->gameToken = gameToken;
}

void QRCodeForStream::setLoginInfo(const std::string_view uid, const std::string_view gameToken, const std::string& name)
{
    this->uid = uid;
    this->gameToken = gameToken;
    this->m_name = name;
}

void QRCodeForStream::setLoginInfo1(const std::string_view uid, const std::string_view stoken, const std::string_view mid)
{
    this->uid = uid;
    this->gameToken = stoken;
    this->mid = mid;
}

void QRCodeForStream::setServerType(const ServerType servertype)
{
    this->servertype = servertype;
}

void QRCodeForStream::LoginOfficial()
{
    while (m_stop.load())
    {
        if (av_read_frame(pAVFormatContext, pAVPacket) < 0)
        {
            ret = ScanRet::LIVESTOP;
            break;
        }
        if (pAVPacket->stream_index != videoStreamIndex)
        {
            continue;
        }
        avcodec_send_packet(pAVCodecContext, pAVPacket);
        if (pAVFrame == nullptr)
        {
            std::cerr << "Error allocating frame" << std::endl;
            ret = ScanRet::LIVESTOP;
            break;
        }
        while (avcodec_receive_frame(pAVCodecContext, pAVFrame) == 0)
        {
            cv::Mat img(videoStreamHeight, videoStreamWidth, CV_8UC3);
            uint8_t* dstData[1] = { img.data };
            const int dstLinesize[1] = { static_cast<int>(img.step) };
            sws_scale(pSwsContext, pAVFrame->data, pAVFrame->linesize, 0, pAVFrame->height,
                      dstData, dstLinesize);
#ifndef SHOW
            cv::imshow("Video_Stream", img);
            cv::waitKey(1);
#endif
            // 不再用 tryStart（线程池满会丢帧）。改为投递到"最新帧"槽位，由解码线程持续解最新帧。
            offerFrame(std::move(img));
        }
        av_frame_unref(pAVFrame);
        av_packet_unref(pAVPacket);
    }
}

void QRCodeForStream::LoginBH3BiliBili()
{
    while (m_stop.load())
    {
        if (av_read_frame(pAVFormatContext, pAVPacket) < 0)
        {
            ret = ScanRet::LIVESTOP;
            break;
        }
        if (pAVPacket->stream_index != videoStreamIndex)
        {
            continue;
        }
        avcodec_send_packet(pAVCodecContext, pAVPacket);
        if (pAVFrame == nullptr)
        {
            std::cerr << "Error allocating frame" << std::endl;
            ret = ScanRet::LIVESTOP;
            break;
        }

        while (avcodec_receive_frame(pAVCodecContext, pAVFrame) == 0)
        {
            cv::Mat img(videoStreamHeight, videoStreamWidth, CV_8UC3);
            uint8_t* dstData[1] = { img.data };
            const int dstLinesize[1] = { static_cast<int>(img.step) };
            sws_scale(pSwsContext, pAVFrame->data, pAVFrame->linesize, 0, pAVFrame->height,
                      dstData, dstLinesize);
#ifndef SHOW
            cv::imshow("Video_Stream", img);
            cv::waitKey(1);
#endif
            // 同上：投递到"最新帧"槽位，由解码线程持续解最新帧（不会因读帧过快丢帧）。
            offerFrame(std::move(img));
        }
        av_frame_unref(pAVFrame);
        av_packet_unref(pAVPacket);
    }
}

void QRCodeForStream::setStreamHW()
{
    if (pAVCodecContext->width < pAVCodecContext->height ||
        pAVCodecContext->height == 480 ||
        pAVCodecContext->height == 720)
    {
        videoStreamWidth = pAVCodecContext->width;
        videoStreamHeight = pAVCodecContext->height;
    }
    else
    {
        videoStreamWidth = pAVCodecContext->width / 1.5;
        videoStreamHeight = pAVCodecContext->height / 1.5;
    }
}

void QRCodeForStream::offerFrame(cv::Mat img)
{
    // 单槽位：只保留最新一帧，旧帧直接被覆盖（对标 Python 的 queue(maxsize=1)）
    {
        std::lock_guard<std::mutex> lk(m_frameMtx);
        m_latestFrame = std::move(img);
        m_hasNewFrame = true;
    }
    m_frameCv.notify_one();
}

void QRCodeForStream::drainStartupBuffer()
{
    // 连上直播的瞬间，CDN/服务器通常会把积压的旧帧一次性灌下来。
    // 若直接交给解码线程，会从"几秒前的旧画面"开始扫，反而拖慢真实二维码出现。
    // 这里用独立的 packet/frame 快速读并丢弃，追上直播点后再启动解码线程，
    // 保证 decodeWorker 第一次取到的就是接近实时的画面。
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frm = av_frame_alloc();
    const auto start = std::chrono::steady_clock::now();
    while (m_stop.load())
    {
        // 最多排空 2s 内的积压帧（之后基本已追上直播点，避免长时间空转）
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 2000)
        {
            break;
        }
        const int r = av_read_frame(pAVFormatContext, pkt);
        if (r < 0)
        {
            break; // 流异常/结束：留待后续 Login 循环统一处理
        }
        if (pkt->stream_index == videoStreamIndex)
        {
            avcodec_send_packet(pAVCodecContext, pkt);
            while (avcodec_receive_frame(pAVCodecContext, frm) == 0)
            {
                // 仅解码、丢弃，不送入解码线程
            }
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    av_frame_free(&frm);
}

void QRCodeForStream::decodeWorker()
{
    // 持续解码"最新帧"：读帧循环再快，二维码所在的那一帧也一定会被解到（不会丢帧）
    thread_local QRScanner qrScanners;
    while (m_decodeActive.load() && m_stop.load())
    {
        cv::Mat img;
        {
            std::unique_lock<std::mutex> lk(m_frameMtx);
            m_frameCv.wait(lk, [this]() {
                return m_hasNewFrame || !m_decodeActive.load() || !m_stop.load();
            });
            if (!m_decodeActive.load() || !m_stop.load())
            {
                break;
            }
            img = std::move(m_latestFrame);
            m_hasNewFrame = false;
        }

        std::string str;
        qrScanners.decodeSingle(img, str);
        if (str.empty())
        {
            continue;
        }

        std::string ticket;
        if (!parseOfficialQRCode(str, ticket))
        {
            continue;
        }

        if (servertype == ServerType::Official)
        {
            if (lastTicket == ticket)
            {
                continue;
            }
            if (mtx.try_lock())
            {
                if (!m_stop.load())
                {
                    mtx.unlock();
                    continue;
                }
                const std::string passportQrUrl = PandaScanQRCode(scanUrl, ticket, gameType);
                if (!passportQrUrl.empty())
                {
                    lastTicket = ticket;
                    lastQrCode = passportQrUrl;
                    nlohmann::json config = nlohmann::json::parse(m_config->getConfig());
                    if (config["auto_login"])
                    {
                        continueLastLogin();
                    }
                    else
                    {
                        Q_EMIT loginConfirm(gameType, false);
                    }
                }
                else
                {
                    Q_EMIT loginResults(ScanRet::FAILURE_1);
                }
                stop();
                mtx.unlock();
            }
        }
        else if (servertype == ServerType::BH3_BiliBili)
        {
            if (gameType != GameType::Honkai3)
            {
                continue;
            }
            if (lastTicket == ticket)
            {
                continue;
            }
            if (mtx.try_lock())
            {
                if (!m_stop.load())
                {
                    mtx.unlock();
                    continue;
                }
                if (ret = scanCheck(ticket); ret == ScanRet::SUCCESS)
                {
                    lastTicket = ticket;
                    nlohmann::json config = nlohmann::json::parse(m_config->getConfig());
                    if (config["auto_login"])
                    {
                        continueLastLogin();
                    }
                    else
                    {
                        Q_EMIT loginConfirm(GameType::Honkai3_BiliBili, false);
                    }
                }
                else
                {
                    Q_EMIT loginResults(ret);
                }
                stop();
                mtx.unlock();
            }
        }
    }
}

void QRCodeForStream::stop()
{
    m_stop.store(false);
    m_frameCv.notify_all();
}

void QRCodeForStream::setUrl(const std::string& url, const std::map<std::string, std::string> heard)
{
    streamUrl = url;
    for (const auto& it : heard)
    {
        av_dict_set(&pAvdictionary, it.first.c_str(), it.second.c_str(), 0);
    }
    // 低延迟拉流参数：尽量减少 CDN/服务器给的启动缓冲与内部缓冲
    av_dict_set(&pAvdictionary, "fflags", "nobuffer", 0);          // 禁用输入缓存（AVFMT_FLAG_NOBUFFER）
    av_dict_set(&pAvdictionary, "analyzeduration", "500000", 0);   // 流探测上限 0.5s，减少初始缓冲积压
    av_dict_set(&pAvdictionary, "tcp_nodelay", "1", 0);            // 关闭 Nagle，降低 TCP 传输延迟
    av_dict_set(&pAvdictionary, "max_delay", "0", 0);
    av_dict_set(&pAvdictionary, "probesize", "1024", 0);
    av_dict_set(&pAvdictionary, "packetsize", "128", 0);
    av_dict_set(&pAvdictionary, "rtbufsize", "0", 0);
    av_dict_set(&pAvdictionary, "delay", "0", 0);
    av_dict_set(&pAvdictionary, "buffer_size", "1000", 0);
    av_dict_set(&pAvdictionary, "rw_timeout", "5000000", 0);
}

auto QRCodeForStream::init() -> bool
{
    pAVFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pAVFormatContext, streamUrl.c_str(), NULL, &pAvdictionary) != 0)
    {
        std::cerr << "Error opening input file" << std::endl;
        return false;
    }
    if (avformat_find_stream_info(pAVFormatContext, NULL) < 0)
    {
        std::cerr << "Error finding stream information" << std::endl;
        return false;
    }
    AVStream* videoStream = nullptr;
    for (int i = 0; i < pAVFormatContext->nb_streams; i++)
    {
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = pAVFormatContext->streams[i];
            break;
        }
    }
    if (videoStream == nullptr)
    {
        std::cerr << "No video stream found" << std::endl;
        return false;
    }
    videoStreamIndex = videoStream->index;
    const AVCodec* decoder{ avcodec_find_decoder(videoStream->codecpar->codec_id) };
    if (decoder == nullptr)
    {
        std::cerr << "Codec not found" << std::endl;
        return false;
    }
    pAVCodecContext = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(pAVCodecContext, videoStream->codecpar);
    pAVCodecContext->flags |= AV_CODEC_FLAG_LOW_DELAY; // 解码器低延迟：减少内部 reorder/缓冲
    if (avcodec_open2(pAVCodecContext, decoder, NULL) < 0)
    {
        std::cerr << "Error opening codec" << std::endl;
        return false;
    }
    setStreamHW();
    pSwsContext = sws_getContext(
        pAVCodecContext->width, pAVCodecContext->height, pAVCodecContext->pix_fmt,
        videoStreamWidth, videoStreamHeight, AV_PIX_FMT_BGR24, SWS_BILINEAR, NULL, NULL, NULL);
    pAVPacket = av_packet_alloc();
    pAVFrame = av_frame_alloc();
    return true;
}

void QRCodeForStream::continueLastLogin()
{
    switch (servertype)
    {
        using enum ServerType;
    case Official:
    {
        bool b = ScanPassportQRLogin(lastQrCode, gameToken, mid) &&
                 ConfirmPassportQRLogin(lastQrCode, gameToken, mid);
        if (b)
        {
            Q_EMIT loginResults(ScanRet::SUCCESS);
        }
        else
        {
            Q_EMIT loginResults(ScanRet::FAILURE_2);
        }
    }
    break;
    case BH3_BiliBili:
    {
        ret = scanConfirm(lastTicket, uid, gameToken, m_name);
        Q_EMIT loginResults(ret);
    }
    break;
    default:
        break;
    }
}

void QRCodeForStream::run()
{
    m_stop.store(true);
    ret = ScanRet::UNKNOW;
    //TODO 获取直播流地址放在这里
    if (init())
    {
#ifndef SHOW
        cv::namedWindow("Video_Stream", cv::WINDOW_AUTOSIZE);
        cv::resizeWindow("Video_Stream", videoStreamWidth / 2, videoStreamHeight / 2);
#endif
        // 连上后先快速排空 CDN 积压的旧帧，追上直播点，只从最新帧开始解
        drainStartupBuffer();
        // 启动独立解码线程：只解"最新帧"，不因读帧过快而丢帧（背压 + 最新帧策略，对标 Python 版）
        m_decodeActive.store(true);
        m_decodeThread = std::thread(&QRCodeForStream::decodeWorker, this);
        switch (servertype)
        {
            using enum ServerType;
        case Official:
            LoginOfficial();
            break;
        case BH3_BiliBili:
            LoginBH3BiliBili();
            break;
        default:
            break;
        }
        // 流结束/被停止：退出解码线程并回收
        m_decodeActive.store(false);
        m_frameCv.notify_all();
        if (m_decodeThread.joinable())
        {
            m_decodeThread.join();
        }
    }
    else
    {
        ret = ScanRet::STREAMERROR;
    }
    if (ret == ScanRet::LIVESTOP || ret == ScanRet::STREAMERROR)
    {
        emit loginResults(ret);
    }
#ifndef SHOW
    cv::destroyWindow("Video_Stream");
#endif
    avformat_close_input(&pAVFormatContext);
    avcodec_free_context(&pAVCodecContext);
    sws_freeContext(pSwsContext);
    av_dict_free(&pAvdictionary);
    av_frame_free(&pAVFrame);
    av_packet_free(&pAVPacket);
    pAVFormatContext = nullptr;
    pAVCodecContext = nullptr;
    pSwsContext = nullptr;
    pAvdictionary = nullptr;
    pAVFrame = nullptr;
    pAVPacket = nullptr;
}
