#include "loginadminwidget.h"
#include "ui_loginadminwidget.h"
#include "mysql.h"
#include <QDebug>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>

//把内联 SVG 字符串渲染到指定 QLabel
static void setSvgIcon(QLabel *label, const QString &svg)
{
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pix(22, 22);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    renderer.render(&painter);
    painter.end();
    label->setPixmap(pix);
}

LoginAdminWidget::LoginAdminWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginAdminWidget)
{
    ui->setupUi(this);

    //用户图标
    setSvgIcon(ui->userIconLabel,
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='#4287d8'>"
        "<circle cx='12' cy='8' r='4'/><path d='M4 20c1.5-3.5 4.5-5 8-5s5.5 1.5 8 5'/></svg>");
    //密码锁图标
    setSvgIcon(ui->pwdIconLabel,
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='#4287d8'>"
        "<path d='M17 10V8a5 5 0 0 0-10 0v2H5v12h14V10h-2zm-8-2a3 3 0 0 1 6 0v2H9V8z'/></svg>");
}

LoginAdminWidget::~LoginAdminWidget()
{
    delete ui;
}

void LoginAdminWidget::on_loginBtn_clicked()
{
    //获取输入框的用户名与密码
    QString user = ui->userLineEdit->text();
    QString pwd = ui->pwdLineEdit->text();

    //获取数据库地址
    MySql *db = MySql::getMySql();

    if(db->checkAdmin(user,pwd))
    {
        qDebug()<<"登录成功"<<endl;
        //查询当前登录管理员的权限等级
        int level = db->findAdminLevel(user);
        ui->userLineEdit->setText("");
        ui->pwdLineEdit->setText("");
        emit loginOK(user, level);
    }
    else {
        QMessageBox::information(this,"提示","用户名或密码错误！");
        ui->userLineEdit->setText("");
        ui->pwdLineEdit->setText("");
    }
}
