#include "usbthread.h"
#include "cy_cpp/inc/CyAPI.h"
#include "mainwindow.h"
//#include "string.h"
//#include "qimage.h"
#include<qelapsedtimer.h>
#include <QElapsedTimer>

QElapsedTimer timer;
float elapsedMilliseconds = 0;
int usbThread::valid_pic = 0;
uchar usbThread::usbpic[2][256*256];//双缓冲区;
ushort usbThread::ROW_FPGA = 128;//分辨率初始化
ushort usbThread::COL_FPGA = 128;//分辨率初始化
QImage usbThread::image = QImage(640,512,QImage::Format_Grayscale16);//

usbThread::usbThread(QObject *parent):QObject(parent)
{

}

void usbThread::stream_save()
{

}

void usbThread::setSaveDir(const QString &path)
{
    save_path = path;
}

int usbThread::Xfer()
{
    //------------------------------------连接USB FX3--------------------------------------//
    CCyUSBDevice* pUSB = MainWindow::pUSB;
    int epCnt = pUSB->EndPointCount();
    bool bBulk, bIn;
    CCyUSBEndPoint * BulkInEpt = MainWindow::BulkInEpt;
    //CCyUSBEndPoint * BulkOutEpt;
//    if(pUSB->IsOpen())
//    {
        for (int e=0; e<epCnt; e++){
           bBulk = (pUSB->EndPoints[e]->Attributes  == 2);
           bIn   = ((pUSB->EndPoints[e]->Address & 0x80)==0x80);
          if (bBulk && bIn)  BulkInEpt =  (CCyBulkEndPoint *) pUSB->EndPoints[e];
          //if (bBulk && !bIn) BulkOutEpt = (CCyBulkEndPoint *) pUSB->EndPoints[e];
         }

    //------------------------------------开启异步传输--------------------------------------//
    //MyWidget::Acquire = true;
    //short pic[512][640];
//    ushort  col = MainWindow::COL_FPGA;
//    ushort  row = MainWindow::ROW_FPGA;

    int QueueSize = 64;
//    LONG length = 64*16384;
    LONG length = 16384;
    // Allocate the arrays needed for queueing
    UCHAR buffers[65][64*16384];
    PUCHAR			*contexts		= new PUCHAR[QueueSize];
    OVERLAPPED		inOvLap[64];

    LONG XferData_length = 10*16384;

//    BulkInEpt->XferData(buffers[1],length);
    BulkInEpt->XferData(buffers[1],XferData_length);

    // Allocate all the buffers for the queues
    int i=0;
    for (i=0; i< QueueSize; i++)
    {
        //buffers[i]        = new UCHAR[length];
        inOvLap[i].hEvent = CreateEvent(NULL, false, false, L"CYUSB_IN");
        memset(buffers[i],0,length);
    }
    memset(buffers[QueueSize],0,length);
    // Queue-up the first batch of transfer requests
    for (i=0; i< QueueSize; i++)
    {
        contexts[i] = BulkInEpt->BeginDataXfer(buffers[i], length, &inOvLap[i]);
        if (BulkInEpt->NtStatus || BulkInEpt->UsbdStatus) // BeginDataXfer failed
        {
            //dlg->SetDlgItemText(IDC_STATIC1,"USB error0!");
            //dlg->AbortXferLoop(i+1, buffers,contexts,inOvLap);
            //return true;
        }
    }
    i = 0;

    //------------------------------------数据解析和搬移--------------------------------------//
    int j = 0 ;
//    int k = 0 ;
//    int z;
//    int y;
//    int frame_num;
//    int package_num = 1;
    int pic_index = 0;
    uchar *p = buffers[0];
    uchar *_p = buffers[0];
//    long rLen = length;
    long rLen = 16384;
//    QElapsedTimer mstimer;
//    float time;
//    mstimer.start();
//    int frame_cnt = 0;
//    double frame[10];
//    int ave_num = 10;
//    double frame_add;

    //ushort usb_pic[2700][2700];
//    while(1){
//        if(MyWidget::Acquire == 0) _sleep(100);

    uchar temp1[16], temp2[16];
    uchar usbpic_temp[256*256];//双缓冲区;
    int dect_cnt = 19;

    for (;MainWindow::Acquire;)		//循环bLooping
        {

            if (!BulkInEpt->WaitForXfer(&inOvLap[i], 3000))
            {
                        BulkInEpt->Abort();
                        for (int j=0; j< QueueSize; j++)
                            {

                                CloseHandle(inOvLap[j].hEvent);
                            }
                        delete[] contexts;
                        return 0;

            }

            if (BulkInEpt->FinishDataXfer(p, rLen, &inOvLap[i], contexts[i]))
            {
                if( i == 0){
                    memcpy(p+64*rLen,p,6000);
                    _p = p+63*rLen;
                }
                else
                {
                    _p = p - rLen;

                }
//                z = 2*col + 20;
                /**************数据流保存**********************/
                if(stream_save_flag==1)
                {
                    if (!fileOpened)
                    {
                        auto path = this->save_path + '/' + QDateTime::currentDateTime().toString("hh_mm_ss-yyyy-MM-dd") + "-usb_pic.raw";
                        file.setFileName(path);
                        if (file.open(QIODevice::WriteOnly))
                        {
                            fileOpened = true;
                            qDebug() << "File opened successfully.";
                        } else {
                            qDebug() << "Failed to open file for writing!";
                            continue; // 跳过本次循环，继续处理 USB 数据
                        }
                    }
                    // 写入文件
                    if (fileOpened) {
                        QDataStream aStream(&file);
                        aStream.setByteOrder(QDataStream::LittleEndian);
                        aStream.writeRawData(reinterpret_cast<const char*>(_p), rLen);
//                        qDebug() << "Data written to file. Length:" << rLen;
                    }
                }
                else{
                    if(fileOpened){
                        file.close();
                        fileOpened = false; // 文件已关闭
                        qDebug() << "File closed because stream_save_flag is false.";
                    }
                }
                /**************数据流保存**********************/
                for(j = 0;j < rLen;j++)
                {
                    if(_p[j+3]==0x90&&_p[j+2]==0xEB&&_p[j+1]==0x00&&_p[j]==0x00)
                    {
                        if(_p[j+23]==0x90&&_p[j+22]==0xEB&&_p[j+21]==0x00&&_p[j+20]==0x00)
                        {
                            dect_cnt ++;
                            if(dect_cnt == 20)
                            {
                                dect_cnt = 0;
                                for(int pix_shift=j;pix_shift<j+128*40;pix_shift+=40)   //一行包括帧头，分为奇偶行共40个元素
                                {
                                    //一行像素重新排列
                                    // 提取 每行的偶数列个 8-bit 元素和奇数列个 8-bit 元素
                                    memcpy(temp1, _p + pix_shift + 4, 16);        // 复制4~20 个 8-bit 元素
                                    memcpy(temp2, _p + pix_shift + 24, 16);   // 复制第 24~40 个 8-bit 元素

                                    // 重新排列到 pic 数组
                                    for (int i = 0; i < 16; i++) {     //原8bit字节，共16个
                                        for (int k = 0; k < 8; k++) {   //每个字节共8bit，每个bit代表一个on或off事件
                                            int index = (pix_shift-j)/40*128 + i * 8 + k;
                                            // 提取temp1的1-bit和temp2的1-bit,拼接，移位的同时放大到0~255的像素范围
                                            usbpic[pic_index][index] = ((((temp1[i] >> (7- k))& 0x01)<<1)|((temp2[i] >> (7- k))& 0x01));
//                                            usbpic_temp[index] =((((temp1[i] >> (7- k))& 0x01)<<1)|((temp2[i] >> (8- k))& 0x01));
                                        }
                                    }
//                                    //一行像素重新排列
//                                    for(int row_group = 0;row_group<4;row_group++)  //共有四个列分组，分别对应第0~31，,3~63，,6~95,96~127列
//                                    {
//                                        // 提取 每行的偶数列个 8-bit 元素和奇数列个 8-bit 元素
//                                        memcpy(temp1, _p + pix_shift + row_group*4  + 4, 4);        // 复制前 4 个 8-bit 元素
//                                        memcpy(temp2, _p + pix_shift + row_group*4 + 24, 4);   // 复制第 17~20 个 8-bit 元素

//                                        // 重新排列到 pic 数组
//                                        for (int i = 0; i < 4; i++) {
//                                            for (int k = 0; k < 4; k++) {
//                                                int index = (pix_shift-j)/40*128 + row_group*32 + i * 8 + k * 2;
//                                                usbpic[pic_index][index] = ((temp1[3-i] >> (6- k * 2))& 0x3) * 0x55; // 提取 temp1 的 2-bit,移位的同时放大到0~255的像素范围
//                                                usbpic[pic_index][index + 1] = ((temp2[3-i] >> (6-k * 2))& 0x3) * 0x55; // 提取 temp2 的 2-bit
//                                            }
//                                        }
//                                    }
                                    if(pix_shift-j >=127*40)
                                    {
//                                        elapsedMilliseconds = (double)timer.nsecsElapsed()/(double)1000000;
//                                        timer.start();
                                         //打印运行时间
//                                        qDebug() << "Elapsed time:" << elapsedMilliseconds << "milliseconds";
                                        if(pic_index) {
                                            pic_index  = 0;
                                            usbThread::valid_pic = 1;
                                        }
                                        else {
                                            pic_index = 1;
                                            usbThread::valid_pic = 0;
                                        }
                                        emit updatapic();
                                    }
                                }
                            }
                        }
                    }

                }


//                for( j = 0; j< z;j++)
//                  {

//                    if(_p[j+3]==0xEB){
//                        if(_p[j+2]==0x90&&_p[j+3]==0xEB&&_p[j]==0x00&&_p[j+1]==0x00){//buffers[k][j]==0x00&&buffers[k][j+1]==0x00&&
//                            frame_num = _p[j+8] + _p[j+9]*256;
//                            row = (_p[j+4] + _p[j+5]*256)%2800;
//                            col = (_p[j+6] + _p[j+7]*256)%2800;
//                            if(MainWindow::b_area_array)  package_num = (_p[j+10] + _p[j+11]*256)%(row+1);

//                            if(col!=0) MainWindow::COL_FPGA = col;

//                            if(MainWindow::b_area_array && (row!=0)){
//                                MainWindow::ROW_FPGA = row;

//                            }
//                            if( col*MainWindow::ROW_FPGA > 3000000)//720*1080
//                                rLen = 64*16384;
//                            else if( col*MainWindow::ROW_FPGA > 600000)//720*1080
//                                rLen = 24*16384;
//                            else if(col*MainWindow::ROW_FPGA > 200000)
//                                rLen = 8 * 16384;
//                            else rLen = 2 * 16384;
//                            y = 2*col + 16;
//                            for( ; j<rLen;j+=y)
//                              {
//                                    memcpy(MainWindow::pic[pic_index]+(package_num-1)*col,_p+j+16,2*col);

//                                    package_num++;
//                                    if(package_num > MainWindow::ROW_FPGA)
//                                    {
//                                        time = (double)mstimer.nsecsElapsed()/(double)1000000;
//                                        mstimer.start();

//                                        frame_add = 0;
//                                        for(int m = ave_num-1;m>0;m--){
//                                            frame[m] = frame[m-1];
//                                            frame_add += frame[m];
//                                        }
//                                        frame_cnt ++;

//                                        frame[0] = 1000.0/time;
//                                        frame_add += frame[0];
//                                        MainWindow::frame_rate = frame_add/ave_num;

//                                        package_num = 1;
//                                        if(pic_index) {
//                                            pic_index  = 0;
//                                            MainWindow::valid_pic = 1;
//                                        }
//                                        else {
//                                            pic_index = 1;
//                                            MainWindow::valid_pic = 0;
//                                        }

//                                        //memcpy(MyWidget::pic,usb_pic[0],col*row*2);

////                                        qDebug() <<"Analysis:"<< time<<"ms";// 最终统计出来是ms
//                                        emit updatapic();
//                                    }
//                              }
//                            j = z;
//                            break;
//                        }
//                    }
//                }


            }
            else
            {
               // bLooping=false;
//                        dlg->bshow=false;
                //dlg->SetDlgItemText(IDC_STATIC1,"USB error2!");
            }

            //重新开始传输
            contexts[i] = BulkInEpt->BeginDataXfer(p, rLen, &inOvLap[i]);
            if (BulkInEpt->NtStatus || BulkInEpt->UsbdStatus) // BeginDataXfer failed
            {
                //dlg->SetDlgItemText(IDC_STATIC1,"USB error0!");
                //dlg->AbortXferLoop(i+1, buffers,contexts,inOvLap);
                //return true;
            }
            i++;
            p += rLen;

            if (i == QueueSize ) //Only update the display once each time through the Queue
            {
                //emit updatapic();
                i = 0;
                p = buffers[0];
                //bLooping=false;
            }
        }
    // 在循环外部关闭文件
    if (fileOpened) {
        file.close();
        fileOpened = false;
        qDebug() << "File closed successfully.";
    }
//   }
    BulkInEpt->Abort();
    for (int j=0; j< QueueSize; j++)
        {

            CloseHandle(inOvLap[j].hEvent);
        }
    delete[] contexts;
    return 1;
    //delete pUSB;
}
