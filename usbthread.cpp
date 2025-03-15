#include "usbthread.h"
#include "cy_cpp/inc/CyAPI.h"
#include "mainwindow.h"
//#include "string.h"
//#include "qimage.h"
#include<qelapsedtimer.h>
usbThread::usbThread(QObject *parent):QObject(parent)
{

}

void usbThread::stream_save()
{

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
    ushort  col = MainWindow::COL_FPGA;
    ushort  row = MainWindow::ROW_FPGA;

    int QueueSize = 64;
    LONG length = 64*16384;
    // Allocate the arrays needed for queueing
    UCHAR buffers[65][64*16384];
    PUCHAR			*contexts		= new PUCHAR[QueueSize];
    OVERLAPPED		inOvLap[64];

    LONG XferData_length = 64*16384;
    BulkInEpt->XferData(buffers[1],length);
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
    int k = 0 ;
    int z,y;
    int frame_num;
    int package_num = 1;
    int pic_index = 0;
    uchar *p = buffers[0];
    uchar *_p = buffers[0];
    long rLen = length;
    QElapsedTimer mstimer;
    float time;
    mstimer.start();
    int frame_cnt = 0;
    double frame[10];
    int ave_num = 10;
    double frame_add;
    //ushort usb_pic[2700][2700];
//    while(1){
//        if(MyWidget::Acquire == 0) _sleep(100);
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

                z = 2*col + 20;
                for( j = 0; j< z;j++)
                  {
                    if(_p[j+3]==0xEB){
                        if(_p[j+2]==0x90&&_p[j+3]==0xEB&&_p[j]==0x00&&_p[j+1]==0x00){//buffers[k][j]==0x00&&buffers[k][j+1]==0x00&&
                            frame_num = _p[j+8] + _p[j+9]*256;
                            row = (_p[j+4] + _p[j+5]*256)%2800;
                            col = (_p[j+6] + _p[j+7]*256)%2800;
                            if(MainWindow::b_area_array)  package_num = (_p[j+10] + _p[j+11]*256)%(row+1);

                            if(col!=0) MainWindow::COL_FPGA = col;

                            if(MainWindow::b_area_array && (row!=0)){
                                MainWindow::ROW_FPGA = row;

                            }
                            if( col*MainWindow::ROW_FPGA > 3000000)//720*1080
                                rLen = 64*16384;
                            else if( col*MainWindow::ROW_FPGA > 600000)//720*1080
                                rLen = 24*16384;
                            else if(col*MainWindow::ROW_FPGA > 200000)
                                rLen = 8 * 16384;
                            else rLen = 2 * 16384;
                            y = 2*col + 16;
                            for( ; j<rLen;j+=y)
                              {
                                    memcpy(MainWindow::pic[pic_index]+(package_num-1)*col,_p+j+16,2*col);

                                    package_num++;
                                    if(package_num > MainWindow::ROW_FPGA)
                                    {
                                        time = (double)mstimer.nsecsElapsed()/(double)1000000;
                                        mstimer.start();

                                        frame_add = 0;
                                        for(int m = ave_num-1;m>0;m--){
                                            frame[m] = frame[m-1];
                                            frame_add += frame[m];
                                        }
                                        frame_cnt ++;

                                        frame[0] = 1000.0/time;
                                        frame_add += frame[0];
                                        MainWindow::frame_rate = frame_add/ave_num;

                                        package_num = 1;
                                        if(pic_index) {
                                            pic_index  = 0;
                                            MainWindow::valid_pic = 1;
                                        }
                                        else {
                                            pic_index = 1;
                                            MainWindow::valid_pic = 0;
                                        }

                                        //memcpy(MyWidget::pic,usb_pic[0],col*row*2);

//                                        qDebug() <<"Analysis:"<< time<<"ms";// 最终统计出来是ms
                                        emit updatapic();
                                    }
                              }
                            j = z;
                            break;
                        }
                    }
                }


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
