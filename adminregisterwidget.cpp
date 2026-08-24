#include "adminregisterwidget.h"
#include "ui_adminregisterwidget.h"
#include "mysql.h"
#include <QtDebug>
#include <QMessageBox>
AdminRegisterWidget::AdminRegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminRegisterWidget)
{
    ui->setupUi(this);
}

AdminRegisterWidget::~AdminRegisterWidget()
{
    delete ui;
}

void AdminRegisterWidget::on_confirmBtn_clicked()
{
    //获取账号 密码
    QString user = ui->userLineEdit->text();
    QString pwd = ui->pwdLineEdit->text();
    QString pwdCheck = ui->pwdCheckLineEdit->text();
    //判断用户名是否已经存在
    QSqlQuery query;
    query.prepare("SELECT * FROM admin WHERE name=:name");
    query.bindValue(":name",user);
    if(query.exec())
    {
        qDebug()<<"执行查询语句成功"<<endl;
    }
    else {
        qDebug()<<"执行查询语句失败："<<query.lastError().text();
    }
    //遍历查询结果
    while(query.next())
    {
        QMessageBox::information(this,"提示","用户名已存在！");
        return;
    }

    //判断密码是否一致
    if(pwd!=pwdCheck)
    {
        QMessageBox::information(this,"提示","输入的密码不相同,请重新输入！");
        ui->userLineEdit->setText("");
        ui->pwdLineEdit->setText("");
        ui->pwdCheckLineEdit->setText("");
        return;
    }
    //得到数据库的实例地址
    MySql *db = MySql::getMySql();
    db->insertAdminData(user,pwd);
    ui->userLineEdit->setText("");
    ui->pwdLineEdit->setText("");
    ui->pwdCheckLineEdit->setText("");
}
