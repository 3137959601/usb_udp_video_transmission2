#include "image_processor.h"

ImageProcessor ::ImageProcessor (QObject *parent)
    : QObject{parent}
{

}

void ImageProcessor::process(ushort *data, int width, int height) {
//    cv::TickMeter tm;
//    tm.start();
    // 1. 将原始 16-bit 数据存入 gray16
    cv::Mat gray16(height, width, CV_16UC1, data);

    // 2. **先**右移 4 位，得到 12-bit 图 gray12
    cv::Mat gray12 = cv::Mat::zeros(height, width, CV_16UC1);
    for (int y = 0; y < height; ++y) {
        const ushort* src = gray16.ptr<ushort>(y);
        ushort* dst = gray12.ptr<ushort>(y);
        for (int x = 0; x < width; ++x) {
            dst[x] = src[x] >> 4;
        }
    }

    // 3. 采集参考帧（12-bit gray12），满 8 帧则关闭对应标志
    if (capturingLow && lowBuf.size() < sampleFrameNum) {
        lowBuf.emplace_back(gray12.clone());        // 使用emplace_back而不是push_back避免拷贝
        if (lowBuf.size() == sampleFrameNum) {
            capturingLow = false;
            emit captureStatus("低温参考采集完成");
        }
    }
    if (capturingHigh && highBuf.size() < sampleFrameNum) {
        highBuf.emplace_back(gray12.clone());
        if (highBuf.size() == sampleFrameNum) {
            capturingHigh = false;
            emit captureStatus("高温参考采集完成");
        }
    }

    // 4. 将 gray12 转成 float，准备校正
    cv::Mat gray12f;
    gray12.convertTo(gray12f, CV_32FC1);

    // 5. 两点校正（针对 12-bit float），结果仍是 12-bit 范围的 float
    cv::Mat calibratedF = gray12f;
    if (calibrated && twoPointEnabled) {
//        calibratedF = applyTwoPoint12(gray12f);

        calibratedF = applyMATLABStyle(gray12f);
    }
    //*******方案1：先进行12位直方图处理后再转换为8位显示
    // 6. 四舍五入得到 12-bit work12
    cv::Mat work12(height, width, CV_16UC1);
    for (int y = 0; y < height; ++y) {
        const float* sf = calibratedF.ptr<float>(y);
        ushort*       dw = work12.ptr<ushort>(y);
        for (int x = 0; x < width; ++x) {
            int v = int(std::round(sf[x]));
            dw[x] = ushort(std::clamp(v, 0, 4095));
        }
    }

    // 7. 在 work12（0–4095）上做直方图均衡
    if (histEqualizeEnabled) {
        // 7.1 计算直方图
        cv::Mat hist;
        int    channels[] = {0}, histSize[] = {4096};
        float range[] = {0.0f, 4096.0f};
        const float* ranges[] = { range };
        cv::calcHist(&work12, 1, channels, cv::Mat(), hist, 1, histSize, ranges);


        // 7.2 累积分布
        std::vector<float> cdf(4096);
        cdf[0] = hist.at<float>(0);
        for (int i = 1; i < 4096; ++i) cdf[i] = cdf[i-1] + hist.at<float>(i);
        float c0 = cdf.front(), cMax = cdf.back();

        // 7.3 重映射
        for (int y = 0; y < height; ++y) {
            ushort* row = work12.ptr<ushort>(y);
            for (int x = 0; x < width; ++x) {
                float v = (cdf[row[x]] - c0) * 4095.0f / (cMax - c0);
                row[x]   = ushort(std::clamp(int(std::round(v)), 0, 4095));
            }
        }
    }
    QImage img12(
        reinterpret_cast<const uchar*>(work12.data),
        work12.cols,
        work12.rows,
        int(work12.step),
        QImage::Format_Grayscale16
    );
    // 直接发出 12 位图（每个像素 16 bit，但真正用的是低 12 bit），后面在窗口中再进行处理
    emit processed12(img12.copy());
//     8. 最后 >>4 → 8-bit 显示
//    cv::Mat display8(height, width, CV_8UC1);
//    for (int y = 0; y < height; ++y) {
//        const ushort* sw = work12.ptr<ushort>(y);
//        uchar*         dd = display8.ptr<uchar>(y);
//        for (int x = 0; x < width; ++x)
//            dd[x] = uchar(sw[x] >> 4);
//    }
    //*******方案2：先转换为8位数据再进行8位直方图处理后直接显示
//    // 6. **再**右移 4 位并转为 8-bit，供后续直方图或显示
//    cv::Mat display8(height, width, CV_8UC1);
//    for (int y = 0; y < height; ++y) {
//        const float* srcF = calibratedF.ptr<float>(y);
//        uchar* dst8 = display8.ptr<uchar>(y);
//        for (int x = 0; x < width; ++x) {
//            int v12 = static_cast<int>(srcF[x] + 0.5f);
//            // 右移 4 位
//            dst8[x] = static_cast<uchar>( (v12 >> 4) & 0xFF );
//        }
//    }

//    // 7. 可选直方图均衡
//    if (histEqualizeEnabled) {
//        cv::equalizeHist(display8, display8);
////      }
//    //CLAHE（对比度受限自适应直方图均衡化）
////        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
////        clahe->setClipLimit(4);    // 对比度限制阈值（默认40）
////        clahe->setTilesGridSize(cv::Size(8, 8)); // 分块大小
////        clahe->apply(display8, display8);
//    }

    // 8. 转 QImage 发射
//    QImage output(display8.data,
//                  display8.cols,
//                  display8.rows,
//                  display8.step,
//                  QImage::Format_Grayscale8);
//    emit processed8(output.copy());

//    tm.stop();
//    std::cout << "Time: " << tm.getTimeMilli() << "ms" << std::endl;
}

void ImageProcessor::startLowCapture()
{
    lowBuf.clear();
    capturingLow = true;
    calibrated = false;
    emit captureStatus("开始采集低温参考帧");
}

void ImageProcessor::startHighCapture()
{
    highBuf.clear();
    capturingHigh = true;
    calibrated = false;
    emit captureStatus("开始采集高温参考帧");
}
//两点校正公式：
//V_low_i = K_i * R_low_i + B_i
//V_high_i = K_i * R_high_i + B_i
//解得：K_i = (V_low - V_high) / (R_low_i - R_high_i)
//     B_i = (R_low_i * V_high - R_high_i * V_low) / (R_low_i - R_high_i)
//其中V_low和V_high是期望的输出值(通常取两幅参考图像的平均值)。
//它们通常取两幅参考图像（低温参考帧和高温参考帧）的全局平均值。
//代表了传感器在两种温度下的​​整体响应水平​​。通过让所有像素的校正结果向全局平均值对齐，可以消除像素间的非均匀性。

void ImageProcessor::doTwoPointCalibration()
{
    if (lowBuf.size() == sampleFrameNum && highBuf.size() == sampleFrameNum) {
        computeMean(lowBuf,  lowMean);
        computeMean(highBuf, highMean);

        // 2) 计算全局平均 DN（标量）
        T0_DN = static_cast<float>(cv::mean(lowMean)[0]);
        T1_DN = static_cast<float>(cv::mean(highMean)[0]);

        // 计算 diff
        cv::Mat diff = lowMean - highMean;
        // 2) 補小值 eps 保留符号
        const float eps = 1e-3f;
        cv::Mat absdiff;
        cv::absdiff(diff, cv::Scalar(0), absdiff);
        cv::Mat mask = absdiff < eps;
        diff.setTo(cv::Scalar(diff.at<float>(0,0) > 0 ? eps : -eps), mask);

        // 元素级计算 Kmat、Bmat
        cv::Mat numK(diff.size(), CV_32FC1, cv::Scalar(T0_DN - T1_DN));
        cv::divide(numK, diff, Kmat);

        cv::Mat numB = lowMean * T1_DN - highMean * T0_DN;
        cv::divide(numB, diff, Bmat);

        calibrated = true;
        emit captureStatus("两点校正参数已计算");
    } else {
        emit captureStatus("参考帧不足，无法校正");
    }
}

void ImageProcessor::enableTwoPoint(bool enable) {
    twoPointEnabled = enable;
    QString msg = twoPointEnabled ? "两点校正已开启" : "两点校正已关闭";
    if(twoPointEnabled){
        doTwoPointCalibration();
    }
    emit captureStatus(msg);
}

void ImageProcessor::enableHistEqualize(bool enable)
{
    histEqualizeEnabled = enable;
    emit captureStatus(enable ? "直方图均衡已开启" : "直方图均衡已关闭");
}

// 计算平均图
void ImageProcessor::computeMean(const std::vector<cv::Mat>& buf, cv::Mat& meanMat) {
    meanMat = cv::Mat::zeros(buf[0].size(), CV_32FC1);
    for (auto& m : buf) {
        cv::Mat tmp;
        m.convertTo(tmp, CV_32FC1);
        meanMat += tmp;
    }
    meanMat /= static_cast<float>(buf.size());
}

// 两点校正：线性拉伸到 [0,255]
cv::Mat ImageProcessor::applyTwoPoint(const cv::Mat& src) {
    cv::Mat srcF; src.convertTo(srcF, CV_32FC1);
    cv::Mat outF = (srcF - lowMean).mul(255.0f / (highMean - lowMean));
    cv::Mat clipped;
    cv::threshold(outF, clipped, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(clipped, clipped, 0,   0,   cv::THRESH_TOZERO);
    cv::Mat out8;
    clipped.convertTo(out8, CV_8UC1);
    return out8;
}
// 对 12-bit float 图做两点线性校正，输出也是 float（范围 [0,4095]）
cv::Mat ImageProcessor::applyTwoPoint12(const cv::Mat& src12f) {
    cv::Mat outF = (src12f - lowMean).mul(4095.0f / (highMean - lowMean));
    // 裁剪到 [0,4095]
    cv::threshold(outF, outF, 4095.0, 4095.0, cv::THRESH_TRUNC);
    cv::threshold(outF, outF,    0.0,    0.0, cv::THRESH_TOZERO);
    return outF;
}

// ---------- MATLAB 风格的两点校正 ----------
cv::Mat ImageProcessor::applyMATLABStyle(const cv::Mat& src12f) {
    // out = Kmat .* src12f + Bmat
    cv::Mat outF = Kmat.mul(src12f) + Bmat;
    // 裁剪到 [0,4095]
    cv::threshold(outF, outF, 4095.0f, 4095.0f, cv::THRESH_TRUNC);
    cv::threshold(outF, outF,    0.0f,    0.0f, cv::THRESH_TOZERO);
    return outF;
}

//12bit直方图均衡化转换为8bit
//void ImageProcessor::process(ushort *data, int width, int height, bool histEqualize) {
//    cv::Mat gray12(height, width, CV_16UC1, data);
//    cv::Mat img8;

//    if (histEqualize) {
//        // 1. 提取高12位数据（右移4位）
//        cv::Mat gray12_shifted = cv::Mat::zeros(gray12.size(), CV_16UC1);
//        for (int y = 0; y < gray12.rows; y++) {
//            const ushort* src_row = gray12.ptr<ushort>(y);
//            ushort* dst_row = gray12_shifted.ptr<ushort>(y);
//            for (int x = 0; x < gray12.cols; x++) {
//                dst_row[x] = src_row[x] >> 4; // 右移4位，保留高12位
//            }
//        }

//        // 2. 计算12-bit直方图
//        cv::Mat hist;
//        int channels[] = {0};
//        int histSize[] = {4096};
//        float range[] = {0, 4096};
//        const float* ranges[] = {range};
//        cv::calcHist(&gray12_shifted, 1, channels, cv::Mat(), hist, 1, histSize, ranges);

//        // 3. 计算CDF
//        float cdf[4096] = {0};
//        cdf[0] = hist.at<float>(0);
//        for (int i = 1; i < 4096; i++) {
//            cdf[i] = cdf[i-1] + hist.at<float>(i);
//        }

//        // 4. 归一化CDF到0-4095
//        float cdf_min = cdf[0];
//        float cdf_max = cdf[4095];
//        for (int y = 0; y < gray12_shifted.rows; y++) {
//            ushort* row = gray12_shifted.ptr<ushort>(y);
//            for (int x = 0; x < gray12_shifted.cols; x++) {
//                row[x] = static_cast<ushort>((cdf[row[x]] - cdf_min) * 4095 / (cdf_max - cdf_min));
//            }
//        }

//        // 5. 缩放到8-bit
//        gray12_shifted.convertTo(img8, CV_8UC1, 1.0 / 16.0);
//    } else {
//        // 不均衡化时，直接右移4位
//        gray12.convertTo(img8, CV_8UC1, 1.0 / 256.0);
//    }

//    // 6. 转为QImage
//    QImage output(img8.data, width, height, img8.step, QImage::Format_Grayscale8);
//    emit processed(output.copy());
//}
void ImageProcessor::equalizeHist(cv::Mat& src, cv::Mat& dst, int graylevel, int dataBit)
{

    // 第1步：计算原始图像的像素总个数
    if (!src.data) {
        return;
    }
    int T1 = m_equalizehistFirst.load(std::memory_order_acquire);  // 高阈值
    int T2 = m_equalizehistSecond.load(std::memory_order_acquire);
    ;  // 低阈值

    if (T1 <= T2) T1 = T2 = 30;

    // 第2步：计算图像的直方图，即计算出每一取值范围内的像素值个数
    memset(m_ehMp, 0, sizeof(uint) * graylevel);  // 初始化
    for (size_t i = 0; i < src.rows; i++) {
        ushort* ptr = src.ptr<ushort>(i);
        for (size_t j = 0; j < src.cols; j++) {
            int value = ptr[j];
            m_ehMp[value]++;
        }
    }

    int number = 0;
    for (size_t i = 0; i < graylevel; i++)  // 平台阈值
    {
        if (m_ehMp[i] > T1) {
            m_ehMp[i] = T1;  // 上限平台阈值
        }
        else if ((m_ehMp[i] < T2) && (m_ehMp[i] > 0)) {
            m_ehMp[i] = T2;  // 下限平台阈值
        }
        number += m_ehMp[i];
    }

    // 第3步：计算灰度分布频率+灰度累加分布频率+重新计算均衡化后的灰度值，四舍五入//第四步：计算灰度累计分布频率//第五步:重新计算均衡化后的灰度值，四舍五入。参考公式：(N-1)*T+0.5
    memset(m_valuePro, 0, sizeof(double) * graylevel);  // 初始化  一定要记住

    m_valuePro[0] = ((double)m_ehMp[0] / number);
    m_ehMp[0] = (ushort)(65535 * m_valuePro[0]);
    for (size_t i = 1; i < graylevel; i++) {
        m_valuePro[i] = ((double)m_ehMp[i] / number) + m_valuePro[i - 1];
        m_ehMp[i] = (ushort)(65535 * m_valuePro[i]);
    }

    // 第6步：灰度变换
    if (dataBit == 8) {
        for (size_t i = 0; i < src.rows; i++) {
            uchar* pSrc = src.ptr<uchar>(i);
            uchar* pDst = dst.ptr<uchar>(i);
            for (size_t j = 0; j < src.cols; j++) {
                pDst[j] = m_ehMp[pSrc[j]];
            }
        }
    }
    else if (dataBit == 16) {
        // 第四步：灰度变换
        for (size_t i = 0; i < src.rows; i++) {
            unsigned short* pSrc = src.ptr<unsigned short>(i);
            unsigned short* pDst = dst.ptr<unsigned short>(i);
            for (size_t j = 0; j < src.cols; j++) {
                pDst[j] = m_ehMp[pSrc[j]];
            }
        }
    }
    else {
    }
}
