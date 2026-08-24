#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "adminpagewidget.h"
#include "employeepagewidget.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_cardBtn_clicked();

    void on_adminBtn_clicked();

    void on_registerBtn_clicked();

    //设置槽函数 接收管理员主控界面的退出信号
    void adminPageExit();
    //槽函数：接收员工主控界面的退出信号
    void employeeExit();

    virtual void paintEvent(QPaintEvent *event);//画图



private:
    Ui::Widget *ui;

    //定义一个指针变量 保存管理员主控页面的地址
    AdminPageWidget * aw;
    EmployeePageWidget * ew;
};

#endif // WIDGET_H
