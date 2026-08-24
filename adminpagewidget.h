#ifndef ADMINPAGEWIDGET_H
#define ADMINPAGEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "adminregisterwidget.h"//管理员注册页面
#include "serialportwidget.h"//串口配置页面
#include "adminofemployee.h"//员工信息管理页面
#include "adminofattendance.h"//考勤管理页面
#include "adminofshop.h"//商品管理界面
#include "adminofadmin.h"//管理员列表界面
#include "adminfeedbackwidget.h"//异常反馈箱界面
namespace Ui {
class AdminPageWidget;
}

class AdminPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminPageWidget(QWidget *parent = nullptr);
    ~AdminPageWidget();

    //登录成功后设置当前管理员身份与权限等级（1一级 2二级 3三级）
    void setLoginAdmin(const QString &name, int level);

    virtual void paintEvent(QPaintEvent *event);
private slots:

    void on_ngtTreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_pageTabWidget_tabCloseRequested(int index);

    void on_exitBtn_clicked();
signals:
    void adminExit();
private:
    Ui::AdminPageWidget *ui;

    AdminRegisterWidget *m_adminRegPage;
    SerialPortWidget *m_serialpage;
    AdminOfEmployee *m_adminOfEmployee;
    AdminOfAttendance *m_adminOfAttendance;
    AdminOfShop *m_adminOfShop;
    AdminOfAdmin *m_adminOfAdmin;
    AdminFeedbackWidget *m_adminFeedback;

    //当前登录管理员身份与权限（1一级 2二级 3三级）
    QString m_loginAdmin;
    int m_loginLevel;
};

#endif // ADMINPAGEWIDGET_H
