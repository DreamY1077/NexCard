#ifndef LOGINADMINWIDGET_H
#define LOGINADMINWIDGET_H

#include <QWidget>

namespace Ui {
class LoginAdminWidget;
}

class LoginAdminWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginAdminWidget(QWidget *parent = nullptr);
    ~LoginAdminWidget();

private slots:
    void on_loginBtn_clicked();

signals:
    //登录成功，携带登录管理员用户名和权限等级（1一级 2二级 3三级）
    void loginOK(const QString &name, int level);

private:
    Ui::LoginAdminWidget *ui;
};

#endif // LOGINADMINWIDGET_H
