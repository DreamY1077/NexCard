#ifndef EMPLOYEEPAGEWIDGET_H
#define EMPLOYEEPAGEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "employeedatawidget.h"
#include "employeeattendancewidget.h"
#include "employeeofshop.h"
#include "employeerefill.h"
#include "employeefeedbackwidget.h"
#include "employeepunchwidget.h"
namespace Ui {
class EmployeePageWidget;
}

class EmployeePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeePageWidget(QWidget *parent = nullptr);
    ~EmployeePageWidget();

    //登录成功后由 widget.cpp 调用，更新身份信息（姓名/卡号）到欢迎页与顶部欢迎语
    void setLoginInfo();

    virtual void paintEvent(QPaintEvent *event);
signals:
    void employeeExit();
private slots:
    void on_ngtTreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_pageTabWidget_tabCloseRequested(int index);

    void on_exitBtn_clicked();

private:
    Ui::EmployeePageWidget *ui;
    EmployeeDataWidget *m_dataPage;
    EmployeeAttendanceWidget *m_attendancePage;
    EmployeeOfShop *m_shopPage;
    EmployeeRefill *m_refillPage;
    EmployeeFeedbackWidget *m_feedbackPage;
    EmployeePunchWidget *m_punchPage;
};

#endif // EMPLOYEEPAGEWIDGET_H
