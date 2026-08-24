#ifndef ADMINREGISTERWIDGET_H
#define ADMINREGISTERWIDGET_H

#include <QWidget>

namespace Ui {
class AdminRegisterWidget;
}

class AdminRegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminRegisterWidget(QWidget *parent = nullptr);
    ~AdminRegisterWidget();

private slots:
    void on_confirmBtn_clicked();

private:
    Ui::AdminRegisterWidget *ui;
};

#endif // ADMINREGISTERWIDGET_H
