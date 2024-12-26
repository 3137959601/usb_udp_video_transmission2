#include "drawthread.h"
#include "qimage.h"
#include "mainwindow.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include<qelapsedtimer.h>
using namespace cv;
drawThread::drawThread(QObject *parent):QObject(parent)
{

}
void drawThread::drawimage()
{
   QElapsedTimer mstimer;
   mstimer.start();
    Mat mat(MainWindow::ROW_FPGA,MainWindow::COL_FPGA,CV_16U,MainWindow::pic[MainWindow::valid_pic]);
    Mat mat2 = Mat::zeros(MainWindow::ROW_FPGA,MainWindow::COL_FPGA,CV_16U);//
    UMat umat;
    UMat umat2;

    //th_medianBlur(mat1,mat,3);25ms


    //blur(umat,umat,Size(3,3));
    //normalize(umat, umat, 65536, 0, NORM_MINMAX, CV_16U);

    if(MainWindow::b_equalizehist)
     EqualizeHist(mat,mat,mat2,65536,16);

    if(MainWindow::b_medianblur){

        mat.copyTo(umat);
        mat2.copyTo(umat2);
        medianBlur(umat,umat2,3);
        umat2.copyTo(mat2);
        umat.copyTo(mat);
        //mat = mat2;
        comp_medianBlur(mat,mat2,mat);
    }

    QImage im( mat.data,mat.cols, mat.rows,static_cast<int>(mat.step),QImage::Format_Grayscale16 );
    MainWindow::image = im;

    //float time = (double)mstimer.nsecsElapsed()/(double)1000000;
    //qDebug() <<"DrawImage TimeCost:"<< time<<"ms";// ms

    emit updataimage();

}
void drawThread::comp_medianBlur(Mat& src,Mat& src2, Mat& dst )
{

    for (int i = 0; i < src.rows; i++)
    {
        ushort* pSrc = src.ptr<ushort>(i);
        ushort* pSrc2 = src2.ptr<ushort>(i);
        ushort* pDst = dst.ptr<ushort>(i);
        for (int j = 0; j < src.cols; j++)
        {
            if((pSrc[j]<=pSrc2[j]/1.2)||(pSrc[j]>=pSrc2[j]*1.2))//60000
                pDst[j] = pSrc2[j];
                ;
        }
    }
}
void drawThread::th_medianBlur(Mat& src, Mat& dst, int size)
{
    //copy
//    for (size_t i = 0; i < src.rows; i++)
//    {
//        uchar* pSrc = src.ptr<uchar>(i);
//        uchar* pDst = dst.ptr<uchar>(i);
//        for (size_t j = 0; j < src.cols; j++)
//        {
//            pDst[j] = pSrc[j];
//        }
//    }

    int _size = (size-1)/2;
    int i = 0;
    int half = (size*size+1)/2;
    int num = size*size;
    ushort res;

    for (int m = _size; m < src.rows-_size; ++m){

        ushort* ptr[9] ;
        for( i = 0;i<size;i++){
            ptr[i] = src.ptr<ushort>(m - _size + i);
        }
        unsigned short* pDst = dst.ptr<unsigned short>(i);
        for (int n = _size; n < src.cols-_size; ++n)
          {
             //   Pick up window elements
             int k = 0;
             ushort window[81];
             for ( i = 0; i < size; i++)
                for (int j = n - _size; j < n - _size +i; j++)
                   window[k++] = ptr[i][j];
             //   Order elements (only half of them)
             for (int j = 0; j < half; ++j)
             {
                //   Find position of minimum element
                int min = j;
                for (int l = j + 1; l < num; ++l){
                    if (window[l] < window[min])
                     min = l;
                }
                //   Put found minimum element in its place
                ushort temp = window[j];
                window[j] = window[min];
                window[min] = temp;
             }
             //   Get result - the middle element
             res = window[half - 1];
             if((res<=ptr[_size][n]/1.5)&&(res>=ptr[_size][n]*1.5))
                 pDst[n] = window[half - 1];
          }
    }
}

void drawThread::EqualizeHist(Mat& src, Mat& dst,Mat& dst2, int graylevel, int dataBit)
{

    //第1步：计算原始图像的像素总个数
    int ss = src.cols * src.rows;
    if (!src.data)
    {
        return;
    }
    int T1 = MainWindow::T_equalizehist[0];//高阈值
    int T2 = MainWindow::T_equalizehist[1];//低阈值
    if(T1<=T2) T1 = T2 = 30;

    //第2步：计算图像的直方图，即计算出每一取值范围内的像素值个数
    uint* mp = new uint[graylevel];
    memset(mp, 0, sizeof(uint) * graylevel);//初始化
    for (size_t i = 0; i < src.rows; i++)
    {
        ushort* ptr = src.ptr<ushort>(i);
        for (size_t j = 0; j < src.cols; j++)
        {
            int value = ptr[j];
            mp[value]++;
        }
    }

    int number = 0;
    for (size_t i = 0; i < graylevel; i++)//平台阈值
    {
        if (mp[i]>T1)
        {
            mp[i] = T1;//上限平台阈值
        }
        else if((mp[i]<T2)&&(mp[i]>0))
        {
            mp[i] = T2;//下限平台阈值
        }
        number +=  mp[i];
    }


    //第3步：计算灰度分布频率+灰度累加分布频率+重新计算均衡化后的灰度值，四舍五入//第四步：计算灰度累计分布频率//第五步:重新计算均衡化后的灰度值，四舍五入。参考公式：(N-1)*T+0.5
    double* valuePro = new double[graylevel];
    memset(valuePro, 0, sizeof(double) * graylevel);//初始化  一定要记住

    valuePro[0] = ((double)mp[0] / number);
    mp[0] = (ushort)(255 * valuePro[0]);
    for (size_t i = 1; i < graylevel; i++)
    {
        valuePro[i] = ((double)mp[i] / number) + valuePro[i - 1] ;
        mp[i] = (ushort)(255 * valuePro[i]);
    }


    //第6步：灰度变换
    if (dataBit == 8)
    {
        for (size_t i = 0; i < src.rows; i++)
        {
            uchar* pSrc = src.ptr<uchar>(i);
            uchar* pDst = dst.ptr<uchar>(i);
            for (size_t j = 0; j < src.cols; j++)
            {
                pDst[j] = mp[pSrc[j]];
            }
        }
    }
    else if(dataBit==16){
        //第四步：灰度变换
        for (size_t i = 0; i < src.rows; i++)
        {
            unsigned short* pSrc = src.ptr<unsigned short>(i);
            unsigned short* pDst = dst.ptr<unsigned short>(i);
            for (size_t j = 0; j < src.cols; j++)
            {
                pDst[j] = mp[pSrc[j]]* 256;
            }
        }

        // for (size_t i = 0; i < src.rows; i++)
        // {
        //     unsigned short* pSrc = src.ptr<unsigned short>(i);
        //     unsigned char* pDst2 = dst2.ptr<unsigned char>(i);
        //     for (size_t j = 0; j < src.cols; j++)
        //     {
        //         pDst2[j] = (uchar)mp[pSrc[j]] ;
        //     }
        // }

    }
    else
    {

    }

    //


    delete[]mp;
    delete[]valuePro;
}
void drawThread::EqualizeHist_Array(Mat& src, Mat& dst, int graylevel,int dataBit)
{

    //第1步：计算原始图像的像素总个数
    int ss = src.cols * src.rows;
    if (!src.data)
    {
        return;
    }

    //第2步：计算图像的直方图，即计算出每一取值范围内的像素值个数
    ushort *mp = new ushort[graylevel];
    memset(mp, 0, sizeof(ushort) * graylevel);//初始化
    for (size_t i = 0; i < src.rows; i++)
    {
        ushort* ptr = src.ptr<ushort>(i);
        for (size_t j = 0; j < src.cols; j++)
        {
            int value = ptr[j];
            mp[value]++;
        }
    }

    //第3步：计算灰度分布频率+灰度累加分布频率+重新计算均衡化后的灰度值，四舍五入
    double *valuePro= new double[graylevel];
    memset(valuePro, 0, sizeof(double) * graylevel);//初始化
    //单独处理第一个数据;
    valuePro[0] = ((double)mp[0] / ss);
    mp[0] = (ushort)(65535 * valuePro[0]);
    for (size_t i = 1; i < graylevel; i++)
    {
        valuePro[i] = ((double)mp[i] / ss) + valuePro[i-1];
        mp[i] = (ushort)(65535 * valuePro[i] );
    }

    //第4步：灰度变换
    if (dataBit==8)
    {
        for (size_t i = 0; i < src.rows; i++)
        {
            uchar* pSrc = src.ptr<uchar>(i);
            uchar* pDst = dst.ptr<uchar>(i);
            for (size_t j = 0; j < src.cols; j++)
            {
                pDst[j] = mp[pSrc[j]];
            }
        }
    }
    else if(dataBit==16){
        //第四步：灰度变换
        for (size_t i = 0; i < src.rows; i++)
        {
            ushort* pSrc = src.ptr<ushort>(i);
            ushort* pDst = dst.ptr<ushort>(i);
            for (size_t j = 0; j < src.cols; j++)
            {
                pDst[j] = mp[pSrc[j]];
            }
        }
    }
    else
    {

    }
    delete[]mp;
    delete[]valuePro;
}

