#ifndef ADMINOFADDATT_H
#define ADMINOFADDATT_H

#include <QDialog>

namespace Ui {
class AdminOfAddAtt;
}

class AdminOfAddAtt : public QDialog
{
    Q_OBJECT

public:
    explicit AdminOfAddAtt(QWidget *parent = nullptr);
    ~AdminOfAddAtt();

private slots:
    void on_clearBtn_clicked();

    void on_confirmBtn_clicked();

private:
    Ui::AdminOfAddAtt *ui;
};

#endif // ADMINOFADDATT_H
