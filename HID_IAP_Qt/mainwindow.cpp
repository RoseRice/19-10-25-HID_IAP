#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./src/kit.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), comThread(new(ComThread))
{
    ui->setupUi(this);

    comThread->start(); // 开启线程

    connect(comThread, SIGNAL(sigAppendDebugInfo(QString)), this, SLOT(sigAppendDebugInfoHand(QString)));

    comThread->searchUSB(0x0483, 0x5750);
    setWidgetGroupEnabled(comThread->iUSBOpened);
    ui->pushButtonSndFile->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
    comThread->terminate();// 退出应答处理线程
    delete comThread;
}

bool MainWindow::winEvent(MSG *msg, long *)
{
    if(WM_DEVICECHANGE == msg->message)
    {
        if(DBT_DEVNODES_CHANGED == msg->wParam)
        {
            comThread->searchUSB(0x6666, 0x8888); // (0x0483, 0x5750);
            setWidgetGroupEnabled(comThread->iUSBOpened);
            if(comThread->iUSBOpened)
            {
                ui->actionDebug->setChecked(true);
            }
        }
    }

    return false;
}

void MainWindow::on_pushButtonOpenFile_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("打开文件"), QDir::currentPath(), tr("bin文件(*.bin);;所有文件(*.*)"));  //添加更多的文件类型
    QFile file(file_name);
    //如果取消打开则退出函数
    if(file.fileName().isEmpty())
    {
        return;
    }
    //如果打开失败则提示并退出函数
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("打开失败"),tr("抱歉！未能打开此文件, 这可能是由于没有足够的权限造成的。"),QMessageBox::Ok);
        return;
    }
    //文件大小超过单片机Flash最大预留空间
    if(APP_MAX_SIZE < file.size())
    {
        QMessageBox::critical(this, tr("打开失败"), tr("文件大小为 %1 字节, 超过单片机Flash存储空间").arg(file.size()), QMessageBox::Ok);
        return;
    }
    ui->lineEditFilePath->setText(file_name);// 文件路径

    QFileInfo fileInfo(file_name);
    comThread->fileSize = fileInfo.size();

    QDataStream bin_stream(&file);
    bin_stream.setVersion(QDataStream::Qt_4_8);// 不同Qt版本字节序会不同

    // 二进制格式
    char bin_buf[comThread->fileSize];
    bin_stream.readRawData(bin_buf, comThread->fileSize); // 读出文件到缓存

    // 十六进制格式
    comThread->fileHexBuf = QByteArray(bin_buf, comThread->fileSize);  //格式转换
    if(true == ui->actionFileData->isChecked())
    {
        ui->statusBar->showMessage(tr("文件写入中..."), 3000);
        ui->textBrowserFileData->clear();
        ui->textBrowserFileData->setPlainText(convertHex2Str(comThread->fileHexBuf));// 此处写入很花费时间
    }
    ui->statusBar->showMessage(tr("文件读取完成，共 %1 字节").arg(comThread->fileSize));

    file.close();
    ui->pushButtonSndFile->setEnabled(true);
}

void MainWindow::on_pushButtonSndFile_clicked()
{
    comThread->cmdId = CMD_SND_FILE_DATA;
}

void MainWindow::on_pushButtonClear_clicked()
{
    ui->textBrowserDebug->clear();
}

void MainWindow::on_actionStartApp_triggered()
{
    ui->actionDebug->setChecked(false);
    comThread->cmdId = CMD_START_APP;
}

void MainWindow::on_actionGetVer_triggered()
{
    comThread->cmdId = CMD_GET_VERSION;
}

void MainWindow::sigAppendDebugInfoHand(QString msg)
{
    if(ui->actionDebug->isChecked())
    {
        ui->textBrowserDebug->append(msg);
    }
}

void MainWindow::setWidgetGroupEnabled(bool state)
{
    ui->pushButtonOpenFile->setEnabled(state);
    ui->pushButtonClear->setEnabled(state);
    ui->tabWidget->setEnabled(state);
    //ui->textBrowserFileData->setEnabled(state);
    //ui->textBrowserDebug->setEnabled(state);
    ui->lineEditFilePath->setEnabled(state);

    if(!comThread->fileHexBuf.isEmpty())
    {
        ui->pushButtonSndFile->setEnabled(state);
    }
}

