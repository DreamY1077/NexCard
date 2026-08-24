#include "facedetector.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <cstring>

// ---------- dlib 相关（仅本文件可见） ----------
#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_transforms.h>   // extract_image_chip / get_face_chip_details

using namespace dlib;

// ============================================================
// ResNet 人脸识别网络定义（dlib 19.24 官方 face_recognition_ex 示例代码）
// ============================================================
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;

// ============================================================
// MMOD 人脸检测器网络定义（dlib 19.24 官方 mmod_face_detection_ex 示例）
// 对戴眼镜/侧脸/各种角度都比 HOG 检测器更准
// ============================================================
template <long num_filters, typename SUBNET> using con5d = con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = con<num_filters,5,5,1,1,SUBNET>;
template <typename SUBNET> using downsampler  = relu<affine<con5d<32, relu<affine<con5d<32, relu<affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5  = relu<affine<con<45,5,5,1,1,SUBNET>>>;

using mmod_net_type = loss_mmod<con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

struct FaceDetector::Impl
{
    frontal_face_detector hogDetector;
    mmod_net_type       mmodNet;
    shape_predictor     shapePredictor;
    anet_type           net;
    bool                ready = false;
    bool                mmodReady = false;   // MMOD 模型加载状态（MMOD 文件可选）

    // 将 QImage 转为 dlib 图像矩阵
    static matrix<rgb_pixel> toDlibImage(const QImage &image)
    {
        QImage rgb = image.convertToFormat(QImage::Format_RGB32);
        matrix<rgb_pixel> mat(rgb.height(), rgb.width());
        for(int y = 0; y < rgb.height(); y++)
        {
            const QRgb *line = reinterpret_cast<const QRgb*>(rgb.constScanLine(y));
            for(int x = 0; x < rgb.width(); x++)
            {
                QRgb px = line[x];
                rgb_pixel p;
                p.red   = qRed(px);
                p.green = qGreen(px);
                p.blue  = qBlue(px);
                mat(y, x) = p;
            }
        }
        return mat;
    }
};

FaceDetector::FaceDetector() : d(new Impl) {}

FaceDetector::~FaceDetector() = default;

bool FaceDetector::loadModels(const QString &modelsDir)
{
    // 多路径搜索模型目录（兼容 QtCreator 运行目录与发布目录）
    QStringList candidates;
    if(!modelsDir.isEmpty()) candidates << modelsDir;
    candidates << QCoreApplication::applicationDirPath() + "/models"
               << QCoreApplication::applicationDirPath() + "/../models"
               << QCoreApplication::applicationDirPath()
               << QDir::currentPath() + "/models"
               << QDir::currentPath();

    QString dir;
    for(const QString &c : candidates)
    {
        if(QFile::exists(c + "/shape_predictor_68_face_landmarks.dat")
           && QFile::exists(c + "/dlib_face_recognition_resnet_model_v1.dat"))
        {
            dir = c;
            break;
        }
    }
    if(dir.isEmpty())
    {
        qDebug() << "[FaceDetector] 模型文件缺失，请将模型放入 models 目录";
        qDebug() << "[FaceDetector] 搜索路径:" << candidates;
        return false;
    }
    if(!dir.endsWith('/')) dir += '/';

    const QString spPath   = dir + "shape_predictor_68_face_landmarks.dat";
    const QString netPath  = dir + "dlib_face_recognition_resnet_model_v1.dat";
    const QString mmodPath = dir + "mmod_human_face_detector.dat";   // 可选，优先使用

    try {
        dlib::deserialize(spPath.toStdString()) >> d->shapePredictor;
        dlib::deserialize(netPath.toStdString()) >> d->net;
        d->hogDetector = dlib::get_frontal_face_detector();

        // 尝试加载 MMOD（CNN，精度高）
        d->mmodReady = false;
        if(QFile::exists(mmodPath))
        {
            try
            {
                dlib::deserialize(mmodPath.toStdString()) >> d->mmodNet;
                d->mmodReady = true;
            }
            catch(const std::exception &e)
            {
            }
        }
        else
        {
            }

        d->ready = true;
        return true;
    }
    catch(const std::exception &e)
    {
        qDebug() << "[FaceDetector] 模型加载失败:" << e.what();
        d->ready = false;
        return false;
    }
}

bool FaceDetector::isReady() const
{
    return d->ready;
}

QList<QRect> FaceDetector::detectFaces(const QImage &image)
{
    QList<QRect> result;
    if(!d->ready || image.isNull()) return result;

    // 缩小检测输入（长边 640，速度提升约 4 倍），检测后矩形映射回原图坐标
    QImage work = image;
    double scale = 1.0;
    int longSide = qMax(image.width(), image.height());
    if(longSide > 640)
    {
        scale = 640.0 / longSide;
        work = image.scaled(qMax(1, int(image.width() * scale)),
                            qMax(1, int(image.height() * scale)),
                            Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    try {
        dlib::matrix<dlib::rgb_pixel> mat = Impl::toDlibImage(work);

        // 1) 先用 HOG 检测（带 1 次上采样）
        std::vector<dlib::rectangle> dets = d->hogDetector(mat, 1);
        if(dets.empty() && d->mmodReady)
        {
            // 2) HOG 检测不到时（戴眼镜/侧脸等场景），用 MMOD CNN 检测器兜底
            auto mdets = d->mmodNet(mat);
            for(const auto &mr : mdets)
            {
                dets.push_back(mr.rect);
            }
        }

        for(const auto &rect : dets)
        {
            int x = int(rect.left() / scale);
            int y = int(rect.top() / scale);
            int w = int(rect.width() / scale);
            int h = int(rect.height() / scale);
            result << QRect(x, y, w, h);
        }
    }
    catch(const std::exception &e)
    {
        qDebug() << "[FaceDetector] 检测异常:" << e.what();
    }
    return result;
}

QVector<float> FaceDetector::extractFeature(const QImage &image, const QRect &faceRect)
{
    QVector<float> empty;
    if(!d->ready || image.isNull() || faceRect.isEmpty()) return empty;

    try {
        dlib::matrix<dlib::rgb_pixel> mat = Impl::toDlibImage(image);

        // 使用传入的人脸矩形（防止 detector 二次检测结果不一致）
        dlib::rectangle det(faceRect.left(), faceRect.top(),
                            faceRect.left() + faceRect.width() - 1,
                            faceRect.top() + faceRect.height() - 1);
        // 边界保护
        det = det.intersect(dlib::rectangle(0, 0, mat.nc() - 1, mat.nr() - 1));
        if(det.is_empty()) return empty;

        dlib::full_object_detection shape = d->shapePredictor(mat, det);
        dlib::matrix<dlib::rgb_pixel> chip;
        dlib::extract_image_chip(mat,
            dlib::get_face_chip_details(shape, 150, 0.25), chip);

        dlib::matrix<float, 0, 1> descriptor = d->net(chip);

        QVector<float> out(128);
        for(int i = 0; i < 128 && i < (int)descriptor.size(); i++)
        {
            out[i] = descriptor(i);
        }
        return out;
    }
    catch(const std::exception &e)
    {
        qDebug() << "[FaceDetector] 特征提取异常:" << e.what();
        return empty;
    }
}

bool FaceDetector::matchFeature(const QVector<float> &feature,
                                const QList<FaceRecord> &knownFaces,
                                double threshold,
                                QString &outCard, QString &outName, double &outDistance)
{
    if(feature.size() != 128) return false;

    dlib::matrix<float, 0, 1> query(128);
    for(int i = 0; i < 128; i++) query(i) = feature[i];

    double bestDist = 1e9;
    QString bestCard, bestName;

    for(const FaceRecord &r : knownFaces)
    {
        QVector<float> known = bytesToFeature(r.feature);
        if(known.size() != 128) continue;

        dlib::matrix<float, 0, 1> k(128);
        for(int i = 0; i < 128; i++) k(i) = known[i];

        double dist = dlib::length(query - k);
        if(dist < bestDist)
        {
            bestDist = dist;
            bestCard = r.card;
            bestName = r.name;
        }
    }

    if(bestDist < threshold)
    {
        outCard = bestCard;
        outName = bestName;
        outDistance = bestDist;
        return true;
    }
    outDistance = bestDist;
    return false;
}

QByteArray FaceDetector::featureToBytes(const QVector<float> &feature)
{
    QByteArray bytes;
    if(feature.size() != 128) return bytes;
    bytes.resize(128 * (int)sizeof(float));
    std::memcpy(bytes.data(), feature.constData(), 128 * sizeof(float));
    return bytes;
}

QVector<float> FaceDetector::bytesToFeature(const QByteArray &bytes)
{
    QVector<float> feature;
    if(bytes.size() < 128 * (int)sizeof(float)) return feature;
    feature.resize(128);
    std::memcpy(feature.data(), bytes.constData(), 128 * sizeof(float));
    return feature;
}
