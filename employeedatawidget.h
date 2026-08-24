#ifndef EMPLOYEEDATAWIDGET_H
#define EMPLOYEEDATAWIDGET_H

#include <QWidget>

namespace Ui {
class EmployeeDataWidget;
}

class EmployeeDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeDataWidget(QWidget *parent = nullptr);
    ~EmployeeDataWidget();

private slots:
    void on_refreshBtn_clicked();

private:
    Ui::EmployeeDataWidget *ui;
};

#endif // EMPLOYEEDATAWIDGET_H
