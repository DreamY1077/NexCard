#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <QImage>
#include <QRect>
#include <QVector>
#include <QString>
#include <QList>
#include <QByteArray>
#include <memory>

// FaceRecord 定义在 mysql.h（card/name/feature 字节序列）
#include "mysql.h"

// ============================================================
// 人脸识别核心封装（dlib 实现隐藏在 cpp，头文件不暴露 dlib 类型）
// 职责：加载模型 / 人脸检测 / 128 维特征提取 / 特征比对
// 注意：本类内部对象（dlib 模型）与线程绑定，建议只在单线程中使用
// ============================================================
class FaceDetector
{
public:
    FaceDetector();
    ~FaceDetector();
    FaceDetector(const FaceDetector&) = delete;
    FaceDetector& operator=(const FaceDetector&) = delete;

    // 加载 3 个 dlib 模型（人脸检测器/关键点/特征网络），成功返回 true
    // modelsDir 指向存放 .dat 的目录（如 "./models/"）
    bool loadModels(const QString &modelsDir);
    bool isReady() const;

    // 检测图像中的人脸，返回人脸矩形列表（图像坐标系，原点左上）
    QList<QRect> detectFaces(const QImage &image);

    // 提取指定人脸矩形区域的特征（128 维 float，失败返回空向量）
    QVector<float> extractFeature(const QImage &image, const QRect &faceRect);

    // 与已知人脸特征库比对：
    // feature 为待识别特征，knownFaces 为库中全部记录（feature 为字节序列）
    // threshold 为欧氏距离阈值（建议 0.45~0.55）
    // 找到最接近且距离 < threshold 的记录时返回 true，输出匹配的 card/name/距离
    bool matchFeature(const QVector<float> &feature,
                      const QList<FaceRecord> &knownFaces,
                      double threshold,
                      QString &outCard, QString &outName, double &outDistance);

    // 静态工具：128 维 float 向量 <-> 字节序列（数据库 BLOB 存取）
    static QByteArray featureToBytes(const QVector<float> &feature);
    static QVector<float> bytesToFeature(const QByteArray &bytes);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

#endif // FACEDETECTOR_H
