#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSerialPort>
class MySerial;

namespace Ui {
class SerialPortWidget;
}

class SerialPortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortWidget(QWidget *parent = nullptr);
    ~SerialPortWidget();

    QComboBox* getComboBox();
private slots:
    //打开串口
    void on_openBtn_clicked();
    //关闭串口
    void on_closeBtn_clicked();
    //刷新串口号
    void on_refreshBtn_clicked();

    //清空日志框
    void on_clearLogBtn_clicked();

    //读取接收数据
    void readSerialData(const QByteArray &data);


    void on_sendBtn_clicked();

private:
    Ui::SerialPortWidget *ui;
    MySerial *ms;
    bool configureSerialPort();//根据当前UI设置串口参数并打开
    void appendLog(const QString &text,bool isSend = true); //添加日志：标记发送/接收
};

#endif // SERIALPORTWIDGET_H
