#include "comthread.h"

ComThread::ComThread(QObject *parent) :
    QThread(parent), iUSBOpened(false), iUSBdev(NULL), cmdId(CMD_NON)
{
}

ComThread::~ComThread()
{
    // 释放usb资源
    if(iUSBdev)
    {
        usb_release_interface(iUSBdev, 0);
        usb_close(iUSBdev);
    }
}


void ComThread::run()
{
    int blk_cnt = 0;
    int re_send = 0;

    while(1)
    {
        QThread::msleep(100);// 避免线程占用太多CPU运行时间

        if(CMD_START_APP == cmdId)
        {
            cmdId = CMD_NON;

            /* cmd: 0x02 数据传输结束 */
            re_send = 0;
            while((0!=cmdFileSndEnd()) && (RE_SEND_TOTAL>re_send))
            {
                re_send++;
                emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
            }
        }
        else if(CMD_GET_VERSION == cmdId)
        {
            cmdId = CMD_NON;

            /* cmd: 0x80 获取IAP版本号 */
            re_send = 0;
            while((0!=cmdGetIapVer()) && (RE_SEND_TOTAL>re_send))
            {
                re_send++;
                emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
            }
        }
        else if(CMD_SND_FILE_DATA == cmdId)
        {
            cmdId = CMD_NON;

            /* cmd: 0x01 数据传输准备 */
            re_send = 0;
            while((0!=cmdFileInfo()) && (RE_SEND_TOTAL>re_send))
            {
                re_send++;
                emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
            }

            for(blk_cnt=0; blk_cnt<(fileSize/BLK_SIZE); blk_cnt++)
            {
                /* cmd: 0x10 启动数据块传输 */
                re_send = 0;
                while((0!=cmdBlkSndStart(blk_cnt, BLK_SIZE)) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }

                /* cmd: 0x20 数据帧传输 */
                re_send = 0;
                while((0!=cmdBlkSndData(fileHexBuf.mid(blk_cnt*BLK_SIZE, BLK_SIZE), BLK_SIZE)) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }

                /* cmd: 0x11 数据块传输结束 */
                re_send = 0;
                while((0!=cmdBlkSndEnd(blk_cnt, BLK_SIZE)) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }
            }
            // 剩余块余留数据...
            if(fileSize > blk_cnt*BLK_SIZE)
            {
                /* cmd: 0x10 启动数据块传输 */
                re_send = 0;
                while((0!=cmdBlkSndStart(blk_cnt, fileSize-(blk_cnt*BLK_SIZE))) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }

                /* cmd: 0x20 数据帧传输 */
                re_send = 0;
                while((0!=cmdBlkSndData(fileHexBuf.mid(blk_cnt*BLK_SIZE, fileSize-(blk_cnt*BLK_SIZE)), fileSize-(blk_cnt*BLK_SIZE))) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }

                /* cmd: 0x11 数据块传输结束 */
                re_send = 0;
                while((0!=cmdBlkSndEnd(blk_cnt, fileSize-(blk_cnt*BLK_SIZE))) && (RE_SEND_TOTAL>re_send))
                {
                    re_send++;
                    emit sigAppendDebugInfo(tr("重新发送 第%1次").arg(re_send));
                }
            }

            emit sigAppendDebugInfo(tr("\n--------------- 所有数据发送完成 ---------------\n\n\n"));
        }
    }
}

void ComThread::searchUSB(int vid, int pid)
{
    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */

    /* 遍历设备 */
    if (!(iUSBdev = open_dev(vid, pid)))
    {
        if(false == iUSBOpened)
        {
            emit sigAppendDebugInfo(tr("错误：设备打开失败 - '%1'").arg(usb_strerror()));
        }
        else
        {
            iUSBOpened = false;
            emit sigAppendDebugInfo(tr("关闭USB设备\n"));

            if(iUSBdev)
            {
                emit sigAppendDebugInfo(tr("关闭USB设备，释放资源\n"));
                usb_release_interface(iUSBdev, 0);
                usb_close(iUSBdev);
            }
        }
        return;
    }
    else
    {
        emit sigAppendDebugInfo(tr("设备打开成功 VID:PID = %1:%2").arg(vid, 4, 16, (QChar)'0').arg(pid, 4, 16, (QChar)'0'));
    }
    /* 配置 */
    if (usb_set_configuration(iUSBdev, MY_CONFIG) < 0)
    {
        emit sigAppendDebugInfo(tr("错误：配置设置错误 - '%1'").arg(usb_strerror()));
        usb_close(iUSBdev);
        return;
    }
    else
    {
        emit sigAppendDebugInfo(tr("配置成功"));
    }
    /* 注册 */
    if(usb_claim_interface(iUSBdev, 0) < 0)
    {
        emit sigAppendDebugInfo(tr("错误：注册失败 - '%1'").arg(usb_strerror()));
        usb_close(iUSBdev);
        return;
    }
    else
    {
        emit sigAppendDebugInfo(tr("注册成功\n"));
    }

    iUSBOpened = true;
}

usb_dev_handle *ComThread::open_dev(int vid, int pid)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if((vid == dev->descriptor.idVendor)
            && (pid == dev->descriptor.idProduct))
            {
                emit sigAppendDebugInfo(tr("搜索到设备"));
                return usb_open(dev);
            }
        }
    }
    return NULL;
}

int ComThread::cmdFileInfo()
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    QByteArray w_frame;
    char r_buf[HID_BUF_SIZE];

    // 数据传输准备(发送)
    w_frame.clear();
    w_frame.append(0x01);
    w_frame.append(fileSize&0xFF);
    w_frame.append((fileSize>>8)&0xFF);
    w_frame.append((fileSize>>16)&0xFF);
    w_frame.append((fileSize>>24)&0xFF);
    while(HID_BUF_SIZE > w_frame.size())
    {
        w_frame.append((char)0x00);
    }
    if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
    {
       emit sigAppendDebugInfo(tr("数据传输准备(发送)-成功"));
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据传输准备(发送)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x10;
    }
    //emit sigAppendDebugInfo(tr("w_data: %1").arg(convertHexChar2Str(w_frame.data(), DATA_LOG_LENGTH)));

    // 数据传输准备(接收)
    memset(r_buf, 0x00, HID_BUF_SIZE);
    int ret = usb_bulk_read(iUSBdev, EP_IN, r_buf, HID_BUF_SIZE, 8000+READ_WAIT_TOUT);
    emit sigAppendDebugInfo(tr("r_data: %1").arg(convertHexChar2Str(r_buf, DATA_LOG_LENGTH)));
    if(HID_BUF_SIZE == ret)
    {
        QString info = convertHexChar2Str(&r_buf[2], 4);
        emit sigAppendDebugInfo(tr("数据传输准备(接收)-成功\t  结果:%1 参数:%2").arg(convertHexChar2Str(&r_buf[1], 1)).arg(info));
        if(0 != r_buf[1])   return 0x41;
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据传输准备(接收)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x80;
    }

    return 0;
}

int ComThread::cmdFileSndEnd()
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    QByteArray w_frame;
    char r_buf[HID_BUF_SIZE];

    emit sigAppendDebugInfo(tr("\n"));

    // 数据传输准备(发送)
    w_frame.clear();
    w_frame.append(0x02);
    w_frame.append(0x01); // 启动APP程序
    while(HID_BUF_SIZE > w_frame.size())
    {
        w_frame.append((char)0x00);
    }
    if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
    {
        //emit sigAppendDebugInfo(tr("数据传输结束(发送)-成功"));
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据传输结束(发送)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x10;
    }
    //emit sigAppendDebugInfo(tr("w_data: %1").arg(convertHexChar2Str(w_frame.data(), DATA_LOG_LENGTH)));

    // 数据传输准备(接收)
    memset(r_buf, 0x00, HID_BUF_SIZE);
    int ret = usb_bulk_read(iUSBdev, EP_IN, r_buf, HID_BUF_SIZE, READ_WAIT_TOUT);
    emit sigAppendDebugInfo(tr("r_data: %1").arg(convertHexChar2Str(r_buf, DATA_LOG_LENGTH)));
    if(HID_BUF_SIZE == ret)
    {
        emit sigAppendDebugInfo(tr("数据传输结束(接收)-成功\t  结果:%1").arg(convertHexChar2Str(&r_buf[1], 1)));
        if(0 != r_buf[1])   return 0x41;
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据传输结束(接收)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x80;
    }

    return 0;
}

int ComThread::cmdBlkSndStart(int blk_cnt, int blk_size)
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    emit sigAppendDebugInfo(tr("\n------- 块序号：%1 -------").arg(blk_cnt));

    QByteArray w_frame;
    char r_buf[HID_BUF_SIZE];

    // 启动数据块传输(发送)
    w_frame.clear();
    w_frame.append(0x10);
    w_frame.append((char)blk_cnt & 0xFF);
    w_frame.append((char)(blk_cnt>>8) & 0xFF);
    w_frame.append((char)(blk_size & 0xFF));
    w_frame.append((char)((blk_size>>8) & 0xFF));
    while(HID_BUF_SIZE > w_frame.size())
    {
        w_frame.append((char)0x00);
    }
    if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
    {
        //emit sigAppendDebugInfo(tr("启动数据块传输(发送)-成功"));
    }
    else
    {
        emit sigAppendDebugInfo(tr("启动数据块传输(发送)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x10;
    }
    //emit sigAppendDebugInfo(tr("w_data: %1").arg(convertHexChar2Str(w_frame.data(), DATA_LOG_LENGTH)));

    // 启动数据块传输(接收)
    memset(r_buf, 0x00, HID_BUF_SIZE);
    int ret = usb_bulk_read(iUSBdev, EP_IN, r_buf, HID_BUF_SIZE, READ_WAIT_TOUT);
    emit sigAppendDebugInfo(tr("r_data: %1").arg(convertHexChar2Str(r_buf, DATA_LOG_LENGTH)));
    if(HID_BUF_SIZE == ret)
    {
        emit sigAppendDebugInfo(tr("启动数据块传输(接收)-成功\t  结果:%1").arg(convertHexChar2Str(&r_buf[1], 1)));
        if(0 != r_buf[1])   return 0x41;
    }
    else
    {
        emit sigAppendDebugInfo(tr("启动数据块传输(接收)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x80;
    }

    return 0;
}

int ComThread::cmdBlkSndEnd(int blk_cnt, int blk_size)
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    QByteArray w_frame;
    char r_buf[HID_BUF_SIZE];

    // 数据块传输结束(发送)
    QByteArray blk_buf;
    blk_buf.clear();
    blk_buf = fileHexBuf.mid(blk_cnt*BLK_SIZE, blk_size);
    unsigned short crc = CRC16(blk_buf, blk_size);
    w_frame.clear();
    w_frame.append(0x11);
    w_frame.append(crc&0xFF);
    w_frame.append((crc>>8)&0xFF);
    while(HID_BUF_SIZE > w_frame.size())
    {
        w_frame.append((char)0x00);
    }
    if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
    {
        //emit sigAppendDebugInfo(tr("数据块传输结束(发送)-成功"));
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据块传输结束(发送)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x10;
    }
    //emit sigAppendDebugInfo(tr("w_data: %1").arg(convertHexChar2Str(w_frame.data(), DATA_LOG_LENGTH)));

    // 数据块传输结束(接收)
    memset(r_buf, 0x00, HID_BUF_SIZE);
    int ret = usb_bulk_read(iUSBdev, EP_IN, r_buf, HID_BUF_SIZE, READ_WAIT_TOUT);
    emit sigAppendDebugInfo(tr("r_data: %1").arg(convertHexChar2Str(r_buf, DATA_LOG_LENGTH)));
    if(HID_BUF_SIZE == ret)
    {
        emit sigAppendDebugInfo(tr("数据块传输结束(接收)-成功\t  块:%1 参数:%2").arg(convertHexChar2Str(&r_buf[1], 2)).arg(convertHexChar2Str(&r_buf[3], 1)));
        if(0 != r_buf[3])   return 0x41;
    }
    else
    {
        emit sigAppendDebugInfo(tr("数据块传输结束(接收)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x80;
    }

    return 0;
}

int ComThread::cmdBlkSndData(const QByteArray &blk_buf, int blk_size)
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    QByteArray w_frame;

    int frame_cnt;
    for(frame_cnt=0; frame_cnt<(blk_size/(HID_BUF_SIZE-2)); frame_cnt++)
    {
        w_frame.clear();
        w_frame.append(0x20);
        w_frame.append(HID_BUF_SIZE-2);
        /* iNote ：写入数据 */
        w_frame += blk_buf.mid(frame_cnt*(HID_BUF_SIZE-2), (HID_BUF_SIZE-2));
        while(HID_BUF_SIZE > w_frame.size())
        {
            w_frame.append((char)0x00);
        }
        if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
        {
            //emit sigAppendDebugInfo(tr("%1 数据帧传输-成功").arg(frame_cnt));
        }
        else
        {
            emit sigAppendDebugInfo(tr("%2 数据帧传输-失败\t  %1").arg(tr(usb_strerror())).arg(frame_cnt));
            return 0x10;
        }
        //QThread::msleep(1);
    }
    // 剩余数据
    if(blk_size > frame_cnt*(HID_BUF_SIZE-2))
    {
        w_frame.clear();
        w_frame.append(0x20);
        w_frame.append(blk_size - (frame_cnt*(HID_BUF_SIZE-2)));
        /* iNote ：写入数据 */
        w_frame += blk_buf.mid(frame_cnt*(HID_BUF_SIZE-2), blk_size - (frame_cnt*(HID_BUF_SIZE-2)));

        if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
        {
            //emit sigAppendDebugInfo(tr("%1 数据帧传输-成功").arg(frame_cnt));
        }
        else
        {
            emit sigAppendDebugInfo(tr("%2 数据帧传输-失败\t  %1").arg(tr(usb_strerror())).arg(frame_cnt));
            return 0x80;
        }
        //QThread::msleep(1);
    }

    return 0;
}

int ComThread::cmdGetIapVer()
{
    if(!iUSBOpened)
    {
        emit sigAppendDebugInfo(tr("\nUSB设备不存在，请插入设备"));
        return 0;
    }

    QByteArray w_frame;
    char r_buf[HID_BUF_SIZE];

    // 获取IAP版本号(发送)
    w_frame.clear();
    w_frame.append(0x80);
    w_frame.append(0x01);
    while(HID_BUF_SIZE > w_frame.size())
    {
        w_frame.append((char)0x00);
    }
    if(HID_BUF_SIZE == usb_bulk_write(iUSBdev, EP_OUT, w_frame.data(), HID_BUF_SIZE, WRITE_WAIT_TOUT))
    {
        //emit sigAppendDebugInfo(tr("获取IAP版本号(发送)-成功"));
    }
    else
    {
        emit sigAppendDebugInfo(tr("获取IAP版本号(发送)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x10;
    }
    //emit sigAppendDebugInfo(tr("w_data: %1").arg(convertHexChar2Str(w_frame.data(), DATA_LOG_LENGTH)));

    // 获取IAP版本号(接收)
    memset(r_buf, 0x00, HID_BUF_SIZE);
    int ret = usb_bulk_read(iUSBdev, EP_IN, r_buf, HID_BUF_SIZE, READ_WAIT_TOUT);
    emit sigAppendDebugInfo(tr("r_data: %1").arg(convertHexChar2Str(r_buf, DATA_LOG_LENGTH)));
    if(HID_BUF_SIZE == ret)
    {
        QString ver(&r_buf[2]);
        emit sigAppendDebugInfo(tr("获取IAP版本号(接收)-成功\n  结果:%1 参数: %2").arg(convertHexChar2Str(&r_buf[1], 1)).arg(ver));
        if(0 != r_buf[1])   return 0x41;
    }
    else
    {
        emit sigAppendDebugInfo(tr("获取IAP版本号(接收)-失败\t  %1").arg(tr(usb_strerror())));
        return 0x80;
    }

    return 0;
}
