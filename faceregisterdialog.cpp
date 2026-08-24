#include "faceregisterdialog.h"
#include "ui_faceregisterdialog.h"
#include "mysql.h"
#include "facedetector.h"
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

FaceRegisterDialog::FaceRegisterDialog(const QString &card, const QString &name,
                                       QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FaceRegisterDialog),
      m_card(card), m_name(name)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("人脸信息 · " + name);

    // 显示员工信息
    ui->cardLabel->setText("卡号：" + card);
    ui->nameLabel->setText("姓名：" + name);

    // 创建摄像头与识别线程
    m_camera = nullptr;
    m_surface = new CameraFrameSurface(this);
    m_workerThread = new QThread(this);
    m_worker = new FaceRecognitionWorker;
    m_worker->moveToThread(m_workerThread);
    m_workerThread->start();

    // 帧 -> 识别线程（队列连接自动跨线程）
    connect(m_surface, &CameraFrameSurface::frameReady,
            this, &FaceRegisterDialog::onFrameReady);
    connect(m_surface, &CameraFrameSurface::frameReady,
            m_worker, &FaceRecognitionWorker::processFrame, Qt::QueuedConnection);
    // 识别线程结果 -> UI
    connect(m_worker, &FaceRecognitionWorker::faceDetected,
            this, &FaceRegisterDialog::onFaceDetected);
    connect(m_worker, &FaceRecognitionWorker::faceFeatureReady,
            this, &FaceRegisterDialog::onFaceFeatureReady);

    // 录入模式（提取特征，不比对）
    m_worker->setRegisterMode(true);

    // videoLabel 固定尺寸（640x360），避免图像随窗口/帧变化自动放大
    ui->videoLabel->setFixedSize(400, 225);

    showStatus("点击「开始识别」打开摄像头");
}

FaceRegisterDialog::~FaceRegisterDialog()
{
    // 显式清理：停止摄像头、结束识别线程、释放 worker
    stopCamera();
    if(m_workerThread->isRunning())
    {
        m_workerThread->quit();
        m_workerThread->wait();   // 等待识别线程完全退出（processFrame 无阻塞，很快返回）
    }
    delete m_worker;
    m_worker = nullptr;
    delete ui;
}

void FaceRegisterDialog::startCamera()
{
    if(m_camera) return;
    m_camera = new QCamera(this);
    m_camera->setViewfinder(m_surface);
    m_camera->start();
    showStatus("请正对摄像头，保持光线充足", "#2c65ac");
}

void FaceRegisterDialog::stopCamera()
{
    if(m_camera)
    {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    m_hasFace = false;
}

void FaceRegisterDialog::on_startBtn_clicked()
{
    startCamera();
    ui->captureBtn->setEnabled(true);
    ui->startBtn->setEnabled(false);
}

void FaceRegisterDialog::on_captureBtn_clicked()
{
    if(m_lastFeature.size() != 128)
    {
        QMessageBox::information(this, "提示", "尚未提取到有效人脸特征，请正对摄像头稍候");
        return;
    }

    // 先停止摄像头，释放识别线程 CPU，避免保存/弹窗期间界面卡顿
    stopCamera();

    // 用最近一次提取的特征写入数据库
    QByteArray bytes = FaceDetector::featureToBytes(m_lastFeature);
    bool ok = MySql::getMySql()->saveFaceInfo(m_card, m_name, bytes);
    if(ok)
    {
        QMessageBox::information(this, "成功", QString("人脸录入成功！\n卡号：%1\n姓名：%2").arg(m_card).arg(m_name));
        this->close();
    }
    else
    {
        showStatus("保存失败，请重试", "#d9534f");
    }
}

void FaceRegisterDialog::on_closeBtn_clicked()
{
    stopCamera();
    this->close();
}

void FaceRegisterDialog::onFrameReady(const QImage &frame)
{
    m_lastFrame = frame;
    updatePreview();
}

void FaceRegisterDialog::onFaceDetected(const QRect &rect)
{
    m_hasFace = true;
    m_lastFaceRect = rect;
    updatePreview();
    showStatus("检测到人脸，点击「录入人脸」", "#27ae60");
}

void FaceRegisterDialog::onFaceFeatureReady(const QVector<float> &feature)
{
    if(feature.size() != 128)
    {
        showStatus("特征提取失败，请调整位置重试", "#d9534f");
        return;
    }
    // 暂存特征，等待用户点击「录入人脸」
    m_lastFeature = feature;
    showStatus("检测到人脸，点击「录入人脸」", "#27ae60");
}

void FaceRegisterDialog::updatePreview()
{
    if(m_lastFrame.isNull()) return;
    QImage display = m_lastFrame;
    if(m_hasFace && !m_lastFaceRect.isEmpty())
    {
        QPainter p(&display);
        QPen pen(QColor(0x27, 0xae, 0x60));
        pen.setWidth(3);
        p.setPen(pen);
        p.drawRect(m_lastFaceRect);
    }
        // 摄像头预览旋转 180°（与摄像头安装方向匹配）；识别仍用原始帧
    display = display.mirrored(true, true);
ui->videoLabel->setPixmap(QPixmap::fromImage(display).scaled(
        ui->videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void FaceRegisterDialog::showStatus(const QString &text, const QString &color)
{
    ui->statusLabel->setText(text);
    ui->statusLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(color));
}

