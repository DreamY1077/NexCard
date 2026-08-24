#ifndef ADMINOFATTENDANCE_H
#define ADMINOFATTENDANCE_H

#include <QWidget>
#include "adminofaddatt.h"
#include "adminofsetadd.h"

namespace Ui {
class AdminOfAttendance;
}

class AdminOfAttendance : public QWidget
{
    Q_OBJECT

public:
    explicit AdminOfAttendance(QWidget *parent = nullptr);
    ~AdminOfAttendance();

    //设置登录管理员权限等级，控制按钮显隐：
    //一级：全部可用（含规则修改）；二级：可增删改考勤，不可改规则；三级：仅可查看
    void setLoginLevel(int level);

private slots:
    void on_refreshBtn_clicked();

    void on_searchBtn_clicked();

    void on_deleteBtn_clicked();

    void on_addBtn_clicked();

    void on_setBtn_clicked();

    void on_ruleBtn_clicked();

private:
    Ui::AdminOfAttendance *ui;

    AdminOfAddAtt *m_adminOfAddAtt;
    AdminOfSetAdd *m_adminOfSetAtt;

    int m_loginLevel;   //当前登录管理员权限等级
};

#endif // ADMINOFATTENDANCE_H
