#ifndef EMPLOYEEATTENDANCEWIDGET_H
#define EMPLOYEEATTENDANCEWIDGET_H

#include <QWidget>

namespace Ui {
class EmployeeAttendanceWidget;
}

class EmployeeAttendanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeAttendanceWidget(QWidget *parent = nullptr);
    ~EmployeeAttendanceWidget();

private slots:
    void on_pushButton_clicked();

    void on_refreshBtn_clicked();

    void on_searchBtn_clicked();

private:
    Ui::EmployeeAttendanceWidget *ui;
};

#endif // EMPLOYEEATTENDANCEWIDGET_H
