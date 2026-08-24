#ifndef ADMINOFSHOP_H
#define ADMINOFSHOP_H

#include <QWidget>
#include "adminofaddshop.h"
#include "adminofsetshop.h"
namespace Ui {
class AdminOfShop;
}

class AdminOfShop : public QWidget
{
    Q_OBJECT

public:
    explicit AdminOfShop(QWidget *parent = nullptr);
    ~AdminOfShop();

private slots:
    void on_refreshBtn_clicked();

    void on_addBtn_clicked();

    void on_deleteBtn_clicked();

    void on_setBtn_clicked();

    void on_searchBtn_clicked();

private:
    Ui::AdminOfShop *ui;

    AdminOfAddShop *m_adminOfAddShop;
    AdminOfSetShop *m_adminOfSetShop;
};

#endif // ADMINOFSHOP_H
