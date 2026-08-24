#ifndef ADMINOFSETSHOP_H
#define ADMINOFSETSHOP_H

#include <QDialog>
#include <QVariant>

namespace Ui {
class AdminOfSetShop;
}

class AdminOfSetShop : public QDialog
{
    Q_OBJECT

public:
    explicit AdminOfSetShop(QWidget *parent = nullptr);
    ~AdminOfSetShop();

    void setData(QVariant id);

private slots:
    void on_clearBtn_clicked();

    void on_confirmBtn_clicked();

private:
    Ui::AdminOfSetShop *ui;

    QVariant m_attId;
};

#endif // ADMINOFSETSHOP_H
