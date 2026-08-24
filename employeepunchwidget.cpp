#include "employeepunchwidget.h"
#include "ui_employeepunchwidget.h"
#include "mysql.h"
#include "myserial.h"
#include "facedetector.h"
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QTime>
#include <QDebug>

EmployeePunchWidget::EmployeePunchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeePunchWidget)
{
    ui->setupUi(this);

    m_camera = nullptr;
    m_surface = new CameraFrameSurface(this);
    m_workerThread = new QThread(this);
    m_worker = new FaceRecognitionWorker;
    m_worker->moveToThread(m_workerThread);
    m_workerThread->start();
    m_worker->setThreshold(0.5);

    // 帧 -> UI + 识别线程（队列连接自动跨线程）
    connect(m_surface, &CameraFrameSurface::frameReady,
            this, &EmployeePunchWidget::onFrameReady);
    connect(m_surface, &CameraFrameSurface::frameReady,
            m_worker, &FaceRecognitionWorker::processFrame, Qt::QueuedConnection);
    // 识别结果 -> UI
    connect(m_worker, &FaceRecognitionWorker::faceDetected,
            this, &EmployeePunchWidget::onFaceDetected);
    connect(m_worker, &FaceRecognitionWorker::matchSuccess,
            this, &EmployeePunchWidget::onMatchSuccess);
    connect(m_worker, &FaceRecognitionWorker::matchFailed,
            this, &EmployeePunchWidget::onMatchFailed);
    connect(m_worker, &FaceRecognitionWorker::databaseUpdated,
            this, &EmployeePunchWidget::onDatabaseUpdated);

    // videoLabel 固定尺寸（640x360），避免图像随窗口/帧变化自动放大
    ui->videoLabel->setFixedSize(560, 315);

    // 加载人脸库
    reloadFaceDatabase();
}

EmployeePunchWidget::~EmployeePunchWidget()
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

void EmployeePunchWidget::startCamera()
{
    if(m_camera) return;
    m_punchDone = false;
    m_camera = new QCamera(this);
    m_camera->setViewfinder(m_surface);
    m_camera->start();
    ui->startBtn->setEnabled(false);
    ui->stopBtn->setEnabled(true);
    showStatus("请正对摄像头，识别成功后自动打卡", "#2c65ac");
}

void EmployeePunchWidget::stopCamera()
{
    if(m_camera)
    {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    m_hasFace = false;
    ui->startBtn->setEnabled(true);
    ui->stopBtn->setEnabled(false);
}

void EmployeePunchWidget::on_startBtn_clicked()
{
    startCamera();
}

void EmployeePunchWidget::on_stopBtn_clicked()
{
    stopCamera();
    showStatus("已停止打卡", "#666666");
}

void EmployeePunchWidget::on_refreshBtn_clicked()
{
    reloadFaceDatabase();
}

void EmployeePunchWidget::onFrameReady(const QImage &frame)
{
    m_lastFrame = frame;
    updatePreview();
}

void EmployeePunchWidget::onFaceDetected(const QRect &rect)
{
    m_hasFace = true;
    m_lastFaceRect = rect;
    updatePreview();
}

void EmployeePunchWidget::onMatchSuccess(const QString &card, const QString &name, double distance)
{
    if(m_punchDone) return;  // 本次周期已打卡，忽略后续帧
    m_punchDone = true;

    ui->recognizedLabel->setText(QString("识别成功：%1（卡号 %2，距离 %3）")
                                 .arg(name).arg(card).arg(distance, 0, 'f', 3));
    showStatus("识别成功，正在打卡……", "#27ae60");
    doAttendance(card, name);
}

void EmployeePunchWidget::onMatchFailed(double distance)
{
    if(m_punchDone) return;
    showStatus(QString("未识别（最近距离 %1），请正对摄像头").arg(distance, 0, 'f', 3), "#f0ad4e");
}

void EmployeePunchWidget::onDatabaseUpdated(int count)
{
    ui->dbInfoLabel->setText(QString("已录入人脸：%1 人").arg(count));
    if(count == 0)
    {
        showStatus("人脸库为空：请先由管理员在「员工信息管理 → 人脸录入」录入人脸", "#f0ad4e");
    }
}

void EmployeePunchWidget::updatePreview()
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

void EmployeePunchWidget::showStatus(const QString &text, const QString &color)
{
    ui->statusLabel->setText(text);
    ui->statusLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(color));
}


void EmployeePunchWidget::reloadFaceDatabase()
{
    QList<FaceRecord> faces = MySql::getMySql()->getAllFaceInfo();
    m_worker->updateFaceDatabase(faces);
    ui->dbInfoLabel->setText(QString("已录入人脸：%1 人").arg(faces.size()));
}

void EmployeePunchWidget::doAttendance(const QString &card, const QString &name)
{
    MySql *db = MySql::getMySql();
    QString department = db->findEmployeeDepartment(card);

    // 根据打卡规则判断当前时段
    AttendanceRule rule = db->getAttendanceRule();
    QTime now = QTime::currentTime();
    QTime signInS  = QTime::fromString(rule.signInStart, "HH:mm");
    QTime signInE  = QTime::fromString(rule.signInEnd, "HH:mm");
    QTime lateS    = QTime::fromString(rule.lateStart, "HH:mm");
    QTime lateE    = QTime::fromString(rule.lateEnd, "HH:mm");
    QTime earlyS   = QTime::fromString(rule.earlyStart, "HH:mm");
    QTime earlyE   = QTime::fromString(rule.earlyEnd, "HH:mm");
    QTime signOutS = QTime::fromString(rule.signOutStart, "HH:mm");
    QTime signOutE = QTime::fromString(rule.signOutEnd, "HH:mm");

    QString newStatus;
    if(now >= signInS && now < signInE)
        newStatus = "签到";
    else if(now >= lateS && now < lateE)
        newStatus = "迟到";
    else if(now >= earlyS && now < earlyE)
        newStatus = "早退";
    else if(now >= signOutS && now < signOutE)
        newStatus = "签退";
    else
    {
        showStatus("该时段考勤系统未开放！", "#d9534f");
        m_punchDone = false;   // 未打卡成功，允许继续识别（或用户点停止）
        return;
    }

    // 当天打卡去重
    QStringList todayStatus = db->getTodayAttendanceStatus(card);
    bool isWorkIn  = (newStatus == "签到" || newStatus == "迟到");
    bool isWorkOut = (newStatus == "早退" || newStatus == "签退");
    if(isWorkIn && (todayStatus.contains("签到") || todayStatus.contains("迟到")))
    {
        showStatus("今天已完成上班打卡（签到/迟到），请勿重复打卡！", "#f0ad4e");
        m_punchDone = false;
        return;
    }
    if(isWorkOut && (todayStatus.contains("早退") || todayStatus.contains("签退")))
    {
        showStatus("今天已完成下班打卡（早退/签退），请勿重复打卡！", "#f0ad4e");
        m_punchDone = false;
        return;
    }

    bool ok = db->insertAttendance(card, name, department, newStatus);
    if(ok)
    {
        showStatus(QString("✅ 打卡成功（%1）· %2").arg(newStatus).arg(name), "#27ae60");
        ui->recognizedLabel->setText(QString("%1 打卡成功（%2 %3）")
                                     .arg(name).arg(newStatus)
                                     .arg(QTime::currentTime().toString("HH:mm:ss")));
        stopCamera();   // 打卡完成自动关闭摄像头
    }
    else
    {
        showStatus("打卡失败，请重试", "#d9534f");
        m_punchDone = false;
    }
}
