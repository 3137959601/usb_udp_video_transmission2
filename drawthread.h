#ifndef DRAWTHREAD_H
#define DRAWTHREAD_H

#include <QObject>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

class drawThread : public QObject
{
    Q_OBJECT
public:
    drawThread(QObject *parent = 0);
    void EqualizeHist_Array(cv::Mat& src, cv::Mat& dst, int graylevel,int dataBit);//直方图均衡
    void EqualizeHist(cv::Mat& src, cv::Mat& dst, cv::Mat& dst2,int graylevel, int dataBit);
void comp_medianBlur(cv::Mat& src,cv::Mat& src2, cv::Mat& dst );

void th_medianBlur(cv::Mat& src, cv::Mat& dst, int N);
signals:
    void updataimage( );

public slots:
    void drawimage();//数据传输和解析

};

#endif // DRAWTHREAD_H
