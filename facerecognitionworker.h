#ifndef FACERECOGNITIONWORKER_H
#define FACERECOGNITIONWORKER_H

#include <QObject>
#include <QImage>
#include <QRect>
#include <QList>
#include <QString>
#include <QVector>
#include <QMutex>

#include "mysql.h"          // FaceRecord
#include "facedetector.h"   // FaceDetector

// ============================================================
// 人脸识别工作线程（QObject + moveToThread 模式）
// 职责：接收摄像头帧 -> 人脸检测 -> 特征提取 -> 与库比对 -> 发出结果信号
// 线程安全注意：
//   - FaceDetector 实例只在本对象所在线程使用（模型懒加载于首次 processFrame）
//   - 人脸库数据通过 updateFaceDatabase 从主线程传入（跨线程信号会自动拷贝）
// ============================================================
class FaceRecognitionWorker : public QObject
{
    Q_OBJECT
public:
    explicit FaceRecognitionWorker(QObject *parent = nullptr);

    // 识别阈值（欧氏距离，越小越严格）
    double threshold() const { return m_threshold; }
    void setThreshold(double t) { m_threshold = t; }

public slots:
    // 处理一帧图像（由线程内事件循环调用）
    void processFrame(const QImage &frame);

    // 更新人脸库（主线程查询数据库后调用，跨线程自动拷贝）
    void updateFaceDatabase(const QList<FaceRecord> &faces);

    // 清空人脸库
    void clearFaceDatabase();

    // 录入模式：提取特征后发 faceFeatureReady（不比对），用于管理员人脸录入
    void setRegisterMode(bool on);

signals:
    // 检测到人脸（矩形为图像坐标，供 UI 画框）
    void faceDetected(const QRect &rect);
    // 匹配成功（输出卡号/姓名/距离）
    void matchSuccess(const QString &card, const QString &name, double distance);
    // 检测到人脸但未匹配（distance 为最近距离）
    void matchFailed(double distance);
    // 录入模式下提取到的 128 维特征
    void faceFeatureReady(const QVector<float> &feature);
    // 人脸库更新完成
    void databaseUpdated(int count);

private:
    FaceDetector m_detector;
    QList<FaceRecord> m_faceDB;
    QMutex m_dbMutex;
    double m_threshold = 0.5;
    bool m_modelsLoaded = false;
    bool m_registerMode = false;
};

#endif // FACERECOGNITIONWORKER_H
