#ifndef EMPLOYEEREFILL_H
#define EMPLOYEEREFILL_H

#include <QWidget>

namespace Ui {
class EmployeeRefill;
}

class EmployeeRefill : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeRefill(QWidget *parent = nullptr);
    ~EmployeeRefill();

private slots:
    void on_confirmBtn_clicked();

private:
    Ui::EmployeeRefill *ui;
};

#endif // EMPLOYEEREFILL_H
