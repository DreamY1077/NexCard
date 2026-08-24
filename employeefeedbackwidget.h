#ifndef EMPLOYEEFEEDBACKWIDGET_H
#define EMPLOYEEFEEDBACKWIDGET_H

#include <QWidget>

namespace Ui {
class EmployeeFeedbackWidget;
}

//员工问题反馈：填写标题、类型、内容提交，管理员在异常反馈箱中查看
class EmployeeFeedbackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeFeedbackWidget(QWidget *parent = nullptr);
    ~EmployeeFeedbackWidget();

private slots:
    void on_submitBtn_clicked();

    void on_clearBtn_clicked();

private:
    Ui::EmployeeFeedbackWidget *ui;
};

#endif // EMPLOYEEFEEDBACKWIDGET_H
