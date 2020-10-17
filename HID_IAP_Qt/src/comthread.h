#ifndef COMTHREAD_H
#define COMTHREAD_H

#include <QThread>

#include <QDebug>

#include "./src/lusb0_usb.h"
#include "./src/kit.h"

//////////////////////////////////////////////////////////////////////////////

#define APP_MAX_SIZE                ((512-64) * 1024)       // 烧录固件最大容量
#define BLK_SIZE                    (2*1024)                // 数据块大小
#define WRITE_WAIT_TOUT             (1000)
#define READ_WAIT_TOUT              (1000)
#define DATA_LOG_LENGTH             (6)
#define RE_SEND_TOTAL               (3)

// 运行命令
typedef enum
{
    CMD_NON = 0,
    CMD_SND_FILE_DATA,
    CMD_START_APP,
    CMD_GET_VERSION

} CMD_ENUM;

//////////////////////////////////////////////////////////////////////////////
// Device vendor and product id.
//#define MY_VID 0x0483
//#define MY_PID 0x5750

// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF 0

// Device endpoint(s)
#define EP_IN 0x81
#define EP_OUT 0x01

// Device of bytes to transfer.
#define HID_BUF_SIZE 64


//////////////////////////////////////////////////////////////////////////////




class ComThread : public QThread
{
    Q_OBJECT
public:
    explicit ComThread(QObject *parent = 0);
    ~ComThread();

protected:
    void run(void);

signals:
    void sigAppendDebugInfo(QString msg);// USB搜索结果反馈信号

public slots:


    
public:
    void searchUSB(int vid, int pid);


private:
    usb_dev_handle *open_dev(int vid, int pid);
    int cmdFileInfo(void); // 0x01 数据传输准备
    int cmdFileSndEnd(void); // 0x02 数据传输结束
    int cmdBlkSndStart(int blk_cnt, int blk_len); // 0x10 启动数据块传输
    int cmdBlkSndEnd(int blk_cnt, int blk_size); // 0x11 数据块传输结束
    int cmdBlkSndData(const QByteArray &blk_buf, int blk_size); // 0x20 数据帧传输
    int cmdGetIapVer(void); // 0x80 获取IAP版本号

public:
    bool iUSBOpened;
    usb_dev_handle *iUSBdev; /* the device handle */

    CMD_ENUM cmdId;

    QByteArray fileHexBuf;// bin文件的hex格式缓存
    int fileSize;// bin文件大小




};

#endif // COMTHREAD_H
