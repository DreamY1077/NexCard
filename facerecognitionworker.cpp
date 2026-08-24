#include "facerecognitionworker.h"
#include <QDebug>

FaceRecognitionWorker::FaceRecognitionWorker(QObject *parent)
    : QObject(parent)
{
}

void FaceRecognitionWorker::processFrame(const QImage &frame)
{
    if(frame.isNull()) return;

    // DirectShow 摄像头帧为 bottom-up 存储，收到的帧上下颠倒；
    // 与界面预览一致的 180° 翻转，让检测器看到正立的人脸
    QImage f = frame.mirrored(true, true);

    // 首次处理时懒加载模型（在本线程执行，保证线程亲和）
    if(!m_modelsLoaded)
    {
        m_modelsLoaded = m_detector.loadModels("models");
        if(!m_modelsLoaded)
        {
            static bool s_logged = false;
            if(!s_logged)
            {
                s_logged = true;
                qDebug() << "[FaceWorker] 模型加载失败！请确认 models 目录（含 2 个 .dat）位于 exe 同级或项目目录";
            }
            return;
        }
    }

    // 1. 检测人脸
    QList<QRect> faces = m_detector.detectFaces(f);
    if(faces.isEmpty())
    {
        return;
    }

    // 2. 取最大的人脸（近距离多人时优先主脸）
    QRect best = faces.first();
    for(const QRect &r : faces)
    {
        if(r.width() * r.height() > best.width() * best.height())
        {
            best = r;
        }
    }
    emit faceDetected(best);

    // 3. 提取特征
    QVector<float> feature = m_detector.extractFeature(f, best);
    if(feature.size() != 128)
    {
        qDebug() << "[FaceWorker] 特征提取失败";
        return;
    }

    // 录入模式：只发特征，不比对
    if(m_registerMode)
    {
        emit faceFeatureReady(feature);
        return;
    }

    // 4. 与人脸库比对
    QList<FaceRecord> db;
    {
        QMutexLocker locker(&m_dbMutex);
        db = m_faceDB;
    }

    QString card, name;
    double dist = 0.0;
    bool ok = m_detector.matchFeature(feature, db, m_threshold, card, name, dist);
    if(ok)
    {
        emit matchSuccess(card, name, dist);
    }
    else if(!db.isEmpty())
    {
        emit matchFailed(dist);
    }
    else
    {
    }
}

void FaceRecognitionWorker::updateFaceDatabase(const QList<FaceRecord> &faces)
{
    QMutexLocker locker(&m_dbMutex);
    m_faceDB = faces;
    emit databaseUpdated(faces.size());
}

void FaceRecognitionWorker::clearFaceDatabase()
{
    QMutexLocker locker(&m_dbMutex);
    m_faceDB.clear();
    emit databaseUpdated(0);
}

void FaceRecognitionWorker::setRegisterMode(bool on)
{
    m_registerMode = on;
}
