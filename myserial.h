#ifndef MYSERIAL_H
#define MYSERIAL_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include "serialportwidget.h"
#include "ui_serialportwidget.h"

class MySerial : public QObject
{
    Q_OBJECT
private:
    //单例模式
    explicit MySerial(QObject *parent = nullptr);
    ~MySerial();
    static MySerial * const m_serial;

    QSerialPort *m_port; //唯一串口对象
    QByteArray m_buffer; //数据接受缓冲区

    //保存卡号
    QString cardId;
public:
    //提供方法，获取唯一实例
    static MySerial * getMyserial();

    //提供方法 获取card值
    QString getCardId();
    //获取串口对象指针
    QSerialPort * getMySerialPort();

    //自动识别串口号
    void searchSerialPort(SerialPortWidget *p);

    //配置串口
    void configPort(const QString &portName,
                  qint32 baudRate,
                  QSerialPort::DataBits dataBIts,
                  QSerialPort::StopBits stopBits,
                  QSerialPort::Parity parity,
                  QSerialPort::FlowControl flowControl);

    //初始化串口
    void initSerialProt();
    //从串口数据中获取卡号
    void getCard(const QByteArray &data);
signals:
    //获取串口数据信号
    void dataReceived(const QByteArray &data);
    //接收到卡号时发出信号
    void cardIdReceived(const QString &cardId);
public slots:
    void onReadyRead();
};

#endif // MYSERIAL_H
