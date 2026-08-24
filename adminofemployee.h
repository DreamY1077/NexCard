#ifndef ADMINOFEMPLOYEE_H
#define ADMINOFEMPLOYEE_H

#include <QWidget>
#include "adminofsetemp.h"
#include "faceregisterdialog.h"
namespace Ui {
class AdminOfEmployee;
}

class AdminOfEmployee : public QWidget
{
    Q_OBJECT

public:
    explicit AdminOfEmployee(QWidget *parent = nullptr);
    ~AdminOfEmployee();

    //设置当前登录管理员权限等级（1一级 2二级 3三级），三级无"人脸录入"权限
    void setLoginLevel(int level);

private slots:
    void on_refreshBtn_clicked();

    void on_searchBtn_clicked();

    void on_setBtn_clicked();

    void on_faceBtn_clicked();

private:
    Ui::AdminOfEmployee *ui;
    AdminOFSetEmp *m_adminOfSetEmp;
};

#endif // ADMINOFEMPLOYEE_H
