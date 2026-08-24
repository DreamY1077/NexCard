#include "widget.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QMetaType>
#include <QVector>

int main(int argc, char *argv[])
{
    //高分屏适配（必须在 QApplication 创建前设置）
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    //注册跨线程信号参数类型（避免 QVector<float> 等类型的 connect 警告）
    qRegisterMetaType<QVector<float>>("QVector<float>");

    QApplication a(argc, argv);

    //全局字体
    QFont font("Microsoft YaHei UI", 10);
    font.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(font);

    //加载全局样式表
    QFile qssFile(":/res/app.qss");
    if(qssFile.open(QFile::ReadOnly | QFile::Text))
    {
        a.setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }

    Widget w;
    w.show();

    return a.exec();
}
