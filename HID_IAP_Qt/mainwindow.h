#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include "./src/comthread.h"

#include <QFile>
#include <QFileDialog>

//#include <QtGui/QWidget>
//#include <windows.h>
//#include <QString>
#include <dbt.h>

class ComThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected:
    bool winEvent(MSG *msg, long *result); // 重载windows事件
    

private slots:
    void on_pushButtonOpenFile_clicked(); // 打开文件
    void on_pushButtonSndFile_clicked(); // 发送文件
    void on_pushButtonClear_clicked(); // 清空显示框
    void on_actionStartApp_triggered(); // 启动APP程序
    void on_actionGetVer_triggered(); // 获取App程序固件版本号

    /* 线程槽函数 */
    void sigAppendDebugInfoHand(QString msg);


private:
    void setWidgetGroupEnabled(bool state);

private:
    Ui::MainWindow *ui;
    ComThread *comThread;// 通讯处理线程


};

#endif // MAINWINDOW_H
