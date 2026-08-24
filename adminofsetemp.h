#ifndef ADMINOFSETEMP_H
#define ADMINOFSETEMP_H

#include <QDialog>

namespace Ui {
class AdminOFSetEmp;
}

class AdminOFSetEmp : public QDialog
{
    Q_OBJECT

public:
    explicit AdminOFSetEmp(QWidget *parent = nullptr);
    ~AdminOFSetEmp();

    //回显选中员工的原始数据到编辑框
    void setData(const QString &card, const QString &name,
                 const QString &age, const QString &sex, const QString &department);

private slots:
    void on_confirmBtn_clicked();

    void on_clearBtn_clicked();

private:
    Ui::AdminOFSetEmp *ui;
};

#endif // ADMINOFSETEMP_H
