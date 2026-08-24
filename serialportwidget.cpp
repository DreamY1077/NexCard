#include "serialportwidget.h"
#include "ui_serialportwidget.h"
#include <QComboBox>
#include "myserial.h"
#include <QMessageBox>
#include <QtDebug>
#include <QDateTime>//显示时间戳
SerialPortWidget::SerialPortWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialPortWidget)
{
    ui->setupUi(this);
    //获取串口类实例地址
    ms = MySerial::getMyserial();
    //获取串口对象
    QSerialPort *m_port = ms->getMySerialPort();
    //不再自动关闭已打开的串口（保持当前连接状态）
    //调用 自动识别串口方法
    ms->searchSerialPort(this);

    //若当前已有串口打开，下拉框默认选中该串口
    if(m_port->isOpen())
    {
        int idx = ui->portComboBox->findText(m_port->portName());
        if(idx >= 0)
        {
            ui->portComboBox->setCurrentIndex(idx);
        }
    }

    //设置日志文本框为只读
    ui->logTextEdit->setReadOnly(true);

    //连接信号槽
    connect(ui->openBtn,&QPushButton::clicked,this,&SerialPortWidget::on_openBtn_clicked);
    connect(ui->closeBtn,&QPushButton::clicked,this,&SerialPortWidget::on_closeBtn_clicked);
    //connect(ui->sendBtn,&QPushButton::clicked,this,&SerialPortWidget::on_)
    connect(ms,&MySerial::dataReceived,this,&SerialPortWidget::readSerialData);

    //初始时根据串口状态设置控件可用性（已打开则关闭按钮可用、参数不可改）
    bool portOpen = m_port->isOpen();
    ui->closeBtn->setEnabled(portOpen);
    ui->openBtn->setEnabled(!portOpen);
    //串口打开时禁止刷新（刷新会清空/重列端口列表，影响已打开连接）
    ui->refreshBtn->setEnabled(!portOpen);
    //串口打开时禁止修改参数（与 on_openBtn_clicked 打开后的禁用逻辑一致）
    ui->portComboBox->setEnabled(!portOpen);
    ui->baudRateComboBox->setEnabled(!portOpen);
    ui->dataComboBox->setEnabled(!portOpen);
    ui->stopComboBox->setEnabled(!portOpen);
    ui->parityComboBox->setEnabled(!portOpen);
    ui->flowComboBox->setEnabled(!portOpen);
}

SerialPortWidget::~SerialPortWidget()
{
    delete ui;
}

QComboBox* SerialPortWidget::getComboBox()
{
    return ui->portComboBox;
}

bool SerialPortWidget::configureSerialPort()
{
    QSerialPort *m_port = ms->getMySerialPort();
    QString portName = ui->portComboBox->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先搜索并选择串口号！");
        return false;
    }
    //波特率转换
    qint32 baudRate = ui->baudRateComboBox->currentText().toUInt();

    //数据位转换
    QSerialPort::DataBits dataBits;
    switch (ui->dataComboBox->currentText().toInt())
    {
        case 5: dataBits = QSerialPort::Data5; break;
        case 6: dataBits = QSerialPort::Data6; break;
        case 7: dataBits = QSerialPort::Data7; break;
        default: dataBits = QSerialPort::Data8; break;
    }

    //停止位转换
    QSerialPort::StopBits stopBits;
    QString stopStr = ui->stopComboBox->currentText();
    if(stopStr == "1")
        stopBits = QSerialPort::OneStop;
    else if(stopStr == "1.5")
        stopBits = QSerialPort::OneAndHalfStop;
    else if(stopStr == "2")
        stopBits = QSerialPort::TwoStop;
    else {
        stopBits = QSerialPort::OneStop;
    }

    //检验位转换
    QSerialPort::Parity parity;
    QString paritySty = ui->parityComboBox->currentText();
    if(paritySty == "NONE")
        parity = QSerialPort::NoParity;
    else if(paritySty == "ODD")
        parity = QSerialPort::OddParity;
    else if(paritySty == "EVEN")
        parity = QSerialPort::EvenParity;
    else if(paritySty == "MARK")
        parity = QSerialPort::MarkParity;
    else if(paritySty == "SPACE")
        parity = QSerialPort::SpaceParity;
    else {
        parity = QSerialPort::NoParity;
    }

    //流控制转换
    QSerialPort::FlowControl flow;
    QString flowStr = ui->flowComboBox->currentText();
    if(flowStr == "NONE")
        flow = QSerialPort::NoFlowControl;
    else if(flowStr == "XON/XOFF")
        flow = QSerialPort::SoftwareControl;
    else if(flowStr == "RTS/CTS")
        flow = QSerialPort::HardwareControl;
    else {
        flow = QSerialPort::NoFlowControl;
    }

    //调用MySerial的配置方法
    ms->configPort(portName,baudRate,dataBits,stopBits,parity,flow);

    // === 打开串口 ===
    if (!m_port->open(QIODevice::ReadWrite)) {
    QMessageBox::critical(this, "错误", "打开串口失败：" + m_port->errorString());
    return false;
    }

    return true;
}

void SerialPortWidget::on_openBtn_clicked()
{
    QSerialPort *m_port = ms->getMySerialPort();
    if(m_port->isOpen())
    {
        QMessageBox::information(this,"提示","串口已打开");
        return;
    }
    //串口打开后禁止修改参数
    if (configureSerialPort()) {
        appendLog("串口打开成功：" + m_port->portName(), false);
        ui->portComboBox->setEnabled(false);
        ui->baudRateComboBox->setEnabled(false);
        ui->dataComboBox->setEnabled(false);
        ui->stopComboBox->setEnabled(false);
        ui->parityComboBox->setEnabled(false);
        ui->flowComboBox->setEnabled(false);
        ui->openBtn->setEnabled(false);
        ui->closeBtn->setEnabled(true);
        ui->refreshBtn->setEnabled(false);   //串口打开时刷新按钮不可点击
    }
}

void SerialPortWidget::on_refreshBtn_clicked()
{
    ui->portComboBox->clear();
    ms->searchSerialPort(this);
}

void SerialPortWidget::on_clearLogBtn_clicked()
{
    ui->logTextEdit->clear();
}

void SerialPortWidget::readSerialData(const QByteArray &data)
{
    if(!data.isEmpty())
    {
        QString received = QString::fromUtf8(data);
        appendLog("接收:" + received,false);
    }
}

void SerialPortWidget::appendLog(const QString &text, bool isSend)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString prefix = isSend ? "[发送]":"[接收]";
    //追加到日志文本框，自动换行
    ui->logTextEdit->append(QString("[%1] %2 %3").arg(time).arg(prefix).arg(text));
    //自动滚动到底部，始终显示最新数据
    ui->logTextEdit->moveCursor(QTextCursor::End);
}

void SerialPortWidget::on_closeBtn_clicked()
{
    QSerialPort *m_port = ms->getMySerialPort();
    if (m_port->isOpen())
    {
            m_port->close();
            appendLog("串口已关闭", false);
            // 恢复可编辑
            ui->portComboBox->setEnabled(true);
            ui->baudRateComboBox->setEnabled(true);
            ui->dataComboBox->setEnabled(true);
            ui->stopComboBox->setEnabled(true);
            ui->parityComboBox->setEnabled(true);
            ui->flowComboBox->setEnabled(true);
            ui->openBtn->setEnabled(true);
            ui->closeBtn->setEnabled(false);
            ui->refreshBtn->setEnabled(true);   //串口关闭后恢复刷新按钮
            QMessageBox::information(this, "提示", "串口已关闭");
    }
    //串口未打开时不再弹提示框（避免关闭弹窗后的第二次点击误报）
}

void SerialPortWidget::on_sendBtn_clicked()
{
    QSerialPort *serialPort = ms->getMySerialPort();
    if (!serialPort->isOpen()) {
            QMessageBox::warning(this, "警告", "请先打开串口");
            return;
        }
    //定义data获取输入框中的信息
        QString data = ui->sendTextEdit->toPlainText();
        if (data.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入要发送的数据");
            return;
        }
        QByteArray sendData = data.toUtf8();
        qint64 bytes = serialPort->write(sendData);
        if (bytes == -1) {
            QMessageBox::critical(this, "错误", "发送失败：" + serialPort->errorString());
        } else {
            appendLog("发送: " + data, true);
            ui->sendTextEdit->clear();
        }
}
