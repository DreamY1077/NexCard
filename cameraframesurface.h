#ifndef CAMERAFRAMESURFACE_H
#define CAMERAFRAMESURFACE_H

#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QImage>
#include <QTimer>

// ============================================================
// 摄像头帧捕获：QCamera 输出接口
// 每收到一帧视频数据 -> 转为 QImage -> 按限频间隔发出 frameReady 信号
// 使用示例：
//   CameraFrameSurface surface;
//   QCamera camera;
//   camera.setViewfinder(&surface);
//   camera.start();
// ============================================================
class CameraFrameSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit CameraFrameSurface(QObject *parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const override;
    bool present(const QVideoFrame &frame) override;

    // 限频间隔（毫秒），默认 120ms（约 8 帧/秒，足够识别且不卡顿）
    void setFrameInterval(int msec);

signals:
    void frameReady(const QImage &image);

private:
    QTimer m_intervalTimer;
    bool   m_emitAllowed = true;
};

#endif // CAMERAFRAMESURFACE_H
