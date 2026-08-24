#ifndef EMPLOYEEPUNCHWIDGET_H
#define EMPLOYEEPUNCHWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QImage>
#include <QRect>
#include <QThread>

#include "cameraframesurface.h"
#include "facerecognitionworker.h"

namespace Ui {
class EmployeePunchWidget;
}

// ============================================================
// 员工考勤打卡界面（人脸识别）
// 打开摄像头实时预览，识别到已录入人脸后自动执行打卡
// （时段判断/当天去重/写入考勤 完全复用现有逻辑）
// ============================================================
class EmployeePunchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeePunchWidget(QWidget *parent = nullptr);
    ~EmployeePunchWidget();

private slots:
    void on_startBtn_clicked();     // 开始打卡（打开摄像头）
    void on_stopBtn_clicked();      // 停止打卡（关闭摄像头）
    void on_refreshBtn_clicked();   // 刷新人脸库

    void onFrameReady(const QImage &frame);
    void onFaceDetected(const QRect &rect);
    void onMatchSuccess(const QString &card, const QString &name, double distance);
    void onMatchFailed(double distance);
    void onDatabaseUpdated(int count);

private:
    Ui::EmployeePunchWidget *ui;

    QCamera *m_camera;
    CameraFrameSurface *m_surface;
    QThread *m_workerThread;
    FaceRecognitionWorker *m_worker;

    QImage m_lastFrame;
    QRect  m_lastFaceRect;
    bool   m_hasFace = false;
    bool   m_punchDone = false;   // 本次识别周期是否已完成打卡

    void startCamera();
    void stopCamera();
    void updatePreview();
    void showStatus(const QString &text, const QString &color = "#4287d8");
    void reloadFaceDatabase();
    // 人脸识别成功后的打卡执行（复用考勤规则/去重/入库逻辑）
    void doAttendance(const QString &card, const QString &name);

};

#endif // EMPLOYEEPUNCHWIDGET_H
