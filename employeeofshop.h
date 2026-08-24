#ifndef EMPLOYEEOFSHOP_H
#define EMPLOYEEOFSHOP_H

#include <QWidget>

namespace Ui {
class EmployeeOfShop;
}

class EmployeeOfShop : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeOfShop(QWidget *parent = nullptr);
    ~EmployeeOfShop();

private slots:
    void on_refreshBtn_clicked();

    void on_searchBtn_clicked();

    void on_buyBtn_clicked();

private:
    Ui::EmployeeOfShop *ui;
};

#endif // EMPLOYEEOFSHOP_H
