#ifndef FACEREGISTERDIALOG_H
#define FACEREGISTERDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QImage>
#include <QRect>
#include <QThread>

#include "cameraframesurface.h"
#include "facerecognitionworker.h"

namespace Ui { class FaceRegisterDialog; }

// ============================================================
// 人脸录入对话框（管理员端）
// 打开摄像头实时预览，检测到人脸后点击"录入人脸"，
// 提取 128 维特征写入 face_info 表（关联员工卡号）
// ============================================================
class FaceRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaceRegisterDialog(const QString &card, const QString &name,
                                QWidget *parent = nullptr);
    ~FaceRegisterDialog();

private slots:
    void on_startBtn_clicked();     // 打开摄像头
    void on_captureBtn_clicked();   // 录入当前检测到的人脸
    void on_closeBtn_clicked();     // 关闭摄像头/关闭窗口

    void onFrameReady(const QImage &frame);
    void onFaceDetected(const QRect &rect);
    void onFaceFeatureReady(const QVector<float> &feature);

private:
    Ui::FaceRegisterDialog *ui;
    QString m_card;
    QString m_name;

    QCamera *m_camera;
    CameraFrameSurface *m_surface;
    QThread *m_workerThread;
    FaceRecognitionWorker *m_worker;

    QImage m_lastFrame;
    QRect  m_lastFaceRect;
    QVector<float> m_lastFeature;   // 最近一次提取的特征（点击录入时存库）
    bool   m_hasFace = false;

    void startCamera();
    void stopCamera();
    void updatePreview();
    void showStatus(const QString &text, const QString &color = "#4287d8");

};

#endif // FACEREGISTERDIALOG_H
