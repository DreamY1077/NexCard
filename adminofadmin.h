#ifndef ADMINOFADMIN_H
#define ADMINOFADMIN_H

#include <QWidget>
namespace Ui {
class AdminOfAdmin;
}

class AdminOfAdmin : public QWidget
{
    Q_OBJECT

public:
    explicit AdminOfAdmin(QWidget *parent = nullptr);
    ~AdminOfAdmin();

    //设置当前登录管理员的用户名与权限等级（1一级 2二级 3三级），控制"修改权限"按钮显隐
    void setLoginUser(const QString &name, int level);

private slots:
    void on_refreshBtn_clicked();

    void on_searchBtn_clicked();

    void on_setLevelBtn_clicked();

    void on_deleteBtn_clicked();

private:
    Ui::AdminOfAdmin *ui;

    //当前登录管理员用户名与权限等级（1一级 2二级 3三级）
    QString m_loginName;
    int m_loginLevel;
};

#endif // ADMINOFADMIN_H
