#ifndef EMPLOYEEREGISTERWIDGET_H
#define EMPLOYEEREGISTERWIDGET_H

#include <QWidget>
#include "faceregisterdialog.h"

namespace Ui {
class EmployeeRegisterWidget;
}

class EmployeeRegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeRegisterWidget(QWidget *parent = nullptr);
    ~EmployeeRegisterWidget();

private slots:
    void on_confirmBtn_clicked();
    void onGetCard(const QString &cardId);
    void on_faceBtn_clicked();   // 注册成功后人脸录入

private:
    Ui::EmployeeRegisterWidget *ui;
};

#endif // EMPLOYEEREGISTERWIDGET_H
