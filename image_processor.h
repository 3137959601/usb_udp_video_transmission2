#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QObject>
#include <QImage>
#include <vector>
#include "opencv2/opencv.hpp"
#include <opencv2/imgproc.hpp>
#include "opencv2/highgui.hpp"
class ImageProcessor: public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor (QObject *parent = nullptr);
    void equalizeHist(cv::Mat& src, cv::Mat& dst, int graylevel, int dataBit);
public slots:
    // 接收原始 12-bit 数据缓冲，宽高信息
    void process(ushort *data, int width, int height);
    void startLowCapture();          // 采低温
    void startHighCapture();         // 采高温
    void doTwoPointCalibration();    // 开始校正
    void enableTwoPoint(bool enable);    // 新增：切换校正开关
    void enableHistEqualize(bool enable);  // 直方图控制函数
signals:
    // 输出处理后的 8-bit QImage
    void processed8(const QImage &img);
    // 输出处理后的 12-bit QImage
    void processed12(const QImage &img);
    void captureStatus(const QString &msg);  // 用于通知 UI
private:
    // --- 两点校正相关 ---
    bool capturingLow = false;
    bool capturingHigh = false;
    bool calibrated    = false;
    bool twoPointEnabled = false;    // 新增：控制开／关两点校正
    bool histEqualizeEnabled = false;  // 是否启用直方图均衡

    int sampleFrameNum = 8;          //两点校正采集低温和高温的帧数
    std::vector<cv::Mat> lowBuf, highBuf;
    cv::Mat lowMean, highMean;

    // 全局平均灰度 DN（标量）
    float T0_DN = 0.0f;
    float T1_DN = 0.0f;
    // MATLAB 风格的 per-pixel K/B 矩阵
    cv::Mat Kmat, Bmat;

    void computeMean(const std::vector<cv::Mat>& buf, cv::Mat& meanMat);
    cv::Mat applyTwoPoint(const cv::Mat& src);
    cv::Mat applyTwoPoint12(const cv::Mat& src);
    cv::Mat applyCustomStretch(const cv::Mat& src);
    cv::Mat applyMATLABStyle(const cv::Mat& src12f);

    // 直方图均衡
    uint*   m_ehMp = nullptr;
    double* m_valuePro = nullptr;

    std::atomic_int  m_equalizehistFirst = 75;
    std::atomic_int  m_equalizehistSecond = 15;
};

#endif // IMAGE_PROCESSOR_H
