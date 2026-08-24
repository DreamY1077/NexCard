#ifndef ADMINOFSETADD_H
#define ADMINOFSETADD_H

#include <QDialog>
#include <QVariant>
namespace Ui {
class AdminOfSetAdd;
}

class AdminOfSetAdd : public QDialog
{
    Q_OBJECT

public:
    explicit AdminOfSetAdd(QWidget *parent = nullptr);
    ~AdminOfSetAdd();

    void setData(QVariant id, QString card, QString timeStr, QString status);
private slots:
    void on_clearBtn_clicked();

    void on_confirmBtn_clicked();

private:
    Ui::AdminOfSetAdd *ui;
    QVariant m_attId;
};

#endif // ADMINOFSETADD_H
