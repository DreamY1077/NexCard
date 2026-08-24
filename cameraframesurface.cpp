#include "cameraframesurface.h"
#include <QVideoFrame>
#include <QDebug>

CameraFrameSurface::CameraFrameSurface(QObject *parent)
    : QAbstractVideoSurface(parent)
{
    // 限频定时器：每个周期只允许发一帧
    m_intervalTimer.setSingleShot(true);
    m_intervalTimer.setInterval(200);   // 5 帧/秒，匹配识别处理能力，避免队列积压
    connect(&m_intervalTimer, &QTimer::timeout, this, [this](){
        m_emitAllowed = true;
    });
}

QList<QVideoFrame::PixelFormat> CameraFrameSurface::supportedPixelFormats(
    QAbstractVideoBuffer::HandleType handleType) const
{
    // 支持常见 RGB 与 YUV 格式
    if(handleType == QAbstractVideoBuffer::NoHandle)
    {
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_YUYV
            << QVideoFrame::Format_YUV420P
            << QVideoFrame::Format_NV12;
    }
    return QList<QVideoFrame::PixelFormat>();
}

bool CameraFrameSurface::present(const QVideoFrame &frame)
{
    if(!frame.isValid()) return true;

    QVideoFrame f = frame;
    if(!f.map(QAbstractVideoBuffer::ReadOnly))
    {
        return false;
    }

    QImage image;
    const int w = f.width();
    const int h = f.height();
    switch(f.pixelFormat())
    {
    case QVideoFrame::Format_RGB32:
    case QVideoFrame::Format_ARGB32:
        image = QImage(f.bits(), w, h,
                       f.bytesPerLine(), QImage::Format_RGB32).copy();
        break;
    case QVideoFrame::Format_RGB24:
        image = QImage(f.bits(), w, h,
                       f.bytesPerLine(), QImage::Format_RGB888).copy();
        break;
    case QVideoFrame::Format_YUYV:
        image = QImage(w, h, QImage::Format_RGB32);
        {
            const uchar *src = f.bits();
            QRgb *dst = reinterpret_cast<QRgb*>(image.bits());
            int total = w * h;
            for(int i = 0; i < total; i += 2)
            {
                int y0 = src[0], u = src[1], y1 = src[2], v = src[3];
                src += 4;
                dst[i]     = qRgb(qBound(0, int(y0 + 1.402*(v-128)), 255),
                                  qBound(0, int(y0 - 0.344*(u-128) - 0.714*(v-128)), 255),
                                  qBound(0, int(y0 + 1.772*(u-128)), 255));
                dst[i + 1] = qRgb(qBound(0, int(y1 + 1.402*(v-128)), 255),
                                  qBound(0, int(y1 - 0.344*(u-128) - 0.714*(v-128)), 255),
                                  qBound(0, int(y1 + 1.772*(u-128)), 255));
            }
        }
        break;
    case QVideoFrame::Format_YUV420P:
        image = QImage(w, h, QImage::Format_RGB32);
        {
            const uchar *yPlane = f.bits(0);
            const uchar *uPlane = f.bits(1);
            const uchar *vPlane = f.bits(2);
            const int yStride = f.bytesPerLine(0);
            const int uvStride = f.bytesPerLine(1);
            QRgb *dst = reinterpret_cast<QRgb*>(image.bits());
            for(int yy = 0; yy < h; yy++)
            {
                for(int xx = 0; xx < w; xx++)
                {
                    int Y = yPlane[yy * yStride + xx];
                    int U = uPlane[(yy/2) * uvStride + xx/2];
                    int V = vPlane[(yy/2) * uvStride + xx/2];
                    int C = Y - 16, D = U - 128, E = V - 128;
                    int R = (298*C + 409*E + 128) >> 8;
                    int G = (298*C - 100*D - 208*E + 128) >> 8;
                    int B = (298*C + 516*D + 128) >> 8;
                    dst[yy * w + xx] = qRgb(qBound(0,R,255), qBound(0,G,255), qBound(0,B,255));
                }
            }
        }
        break;
    case QVideoFrame::Format_NV12:
        image = QImage(w, h, QImage::Format_RGB32);
        {
            const uchar *yPlane = f.bits(0);
            const uchar *uvPlane = f.bits(1);
            const int yStride = f.bytesPerLine(0);
            const int uvStride = f.bytesPerLine(1);
            QRgb *dst = reinterpret_cast<QRgb*>(image.bits());
            for(int yy = 0; yy < h; yy++)
            {
                for(int xx = 0; xx < w; xx++)
                {
                    int Y = yPlane[yy * yStride + xx];
                    int uvIndex = (yy/2) * uvStride + (xx/2) * 2;
                    int U = uvPlane[uvIndex];
                    int V = uvPlane[uvIndex + 1];
                    int C = Y - 16, D = U - 128, E = V - 128;
                    int R = (298*C + 409*E + 128) >> 8;
                    int G = (298*C - 100*D - 208*E + 128) >> 8;
                    int B = (298*C + 516*D + 128) >> 8;
                    dst[yy * w + xx] = qRgb(qBound(0,R,255), qBound(0,G,255), qBound(0,B,255));
                }
            }
        }
        break;
    default:
        // 不支持的格式：静默丢弃该帧
        f.unmap();
        return true;
    }

    f.unmap();

    if(image.isNull())
    {
        return true;
    }

    // 限频发射
    if(m_emitAllowed)
    {
        m_emitAllowed = false;
        m_intervalTimer.start();
        emit frameReady(image);
    }
    return true;
}

void CameraFrameSurface::setFrameInterval(int msec)
{
    m_intervalTimer.setInterval(msec);
}
