#include "myserial.h"
#include <QtDebug>
#include <regex>

//单例初始化
MySerial * const MySerial::m_serial = new MySerial();

MySerial::MySerial(QObject *parent) : QObject(parent)
{
    m_port = new QSerialPort(this);
    //绑定串口可读信号,打开串口后自动触发
    connect(m_port,&QSerialPort::readyRead,this,&MySerial::onReadyRead);
}

MySerial::~MySerial()
{
    if(m_port->isOpen())
    {
        m_port->close();
    }
    delete m_port;
}

MySerial *MySerial::getMyserial()
{
    return m_serial;
}

QString MySerial::getCardId()
{
    return cardId;
}

QSerialPort *MySerial::getMySerialPort()
{
    return  m_port;
}

void MySerial::searchSerialPort(SerialPortWidget *p)
{
    QComboBox *cb = p->getComboBox();
    //直接列出系统枚举到的所有串口，不再逐个 open 测试。
    //原因：逐个 open 在部分设备（虚拟串口/蓝牙串口/无响应设备）上会阻塞较长时间，
    //导致点击"刷新"时界面卡顿。被占用端口仍会列出，打开时由 open() 报错提示。
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        cb->addItem(info.portName());
    }
}

void MySerial::configPort(const QString &portName, qint32 baudRate, QSerialPort::DataBits dataBIts, QSerialPort::StopBits stopBits, QSerialPort::Parity parity, QSerialPort::FlowControl flowControl)
{
    //如果串口已打开，先关闭再重新配置
    if(m_port->isOpen())
    {
        m_port->close();
    }
    //依次配置串口参数
    m_port->setPortName(portName);
    m_port->setBaudRate(baudRate);
    m_port->setDataBits(dataBIts);
    m_port->setStopBits(stopBits);
    m_port->setParity(parity);
    m_port->setFlowControl(flowControl);

}

void MySerial::initSerialProt()
{
    m_port->setPortName("COM7");
    m_port->setBaudRate(115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
}

void MySerial::getCard(const QByteArray &data)
{
    QString text = QString::fromUtf8(data);

    // 匹配卡号格式：如 c1-70-74-6（十六进制字符，用短横线分隔）
    // 正则：1-2位十六进制数开头，后面跟1个或多个 "-xx" 格式片段
    QRegularExpression regex("([0-9a-fA-F]{1,2}(?:-[0-9a-fA-F]{1,2})+)");
    QRegularExpressionMatch match = regex.match(text);

    if (match.hasMatch()) {
        QString cardId = match.captured(1);
        // 确保至少有一个分隔符（即至少两段）
        if (cardId.contains('-')) {
            emit cardIdReceived(cardId);
            qDebug() << "识别到卡号：" << cardId;
            this->cardId = cardId;
        }
    }
}

void MySerial::onReadyRead()
{
    QByteArray data = m_port->readAll();

    // 发出原始数据接收信号（供日志显示等使用）
    emit dataReceived(data);

    m_buffer.append(data);

    // 缓冲区包含换行时进行解析（按行处理）
    while (m_buffer.contains('\n')) {
        int newlineIndex = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(newlineIndex).trimmed();
        m_buffer = m_buffer.mid(newlineIndex + 1);

        if (!line.isEmpty()) {
            getCard(line);
        }
    }

    // 也尝试直接对当前缓冲区进行匹配（处理没有换行的情况）
    if (!m_buffer.isEmpty()) {
        getCard(m_buffer);
    }
}

