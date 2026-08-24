#ifndef ADMINOFADDSHOP_H
#define ADMINOFADDSHOP_H

#include <QDialog>

namespace Ui {
class AdminOfAddShop;
}

class AdminOfAddShop : public QDialog
{
    Q_OBJECT

public:
    explicit AdminOfAddShop(QWidget *parent = nullptr);
    ~AdminOfAddShop();

private slots:
    void on_clearBtn_clicked();

    void on_confirmBtn_clicked();

private:
    Ui::AdminOfAddShop *ui;
};

#endif // ADMINOFADDSHOP_H
