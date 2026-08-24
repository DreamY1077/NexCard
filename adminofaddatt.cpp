#include "adminofaddatt.h"
#include "ui_adminofaddatt.h"
#include "mysql.h"
#include <QMessageBox>

AdminOfAddAtt::AdminOfAddAtt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminOfAddAtt)
{
    ui->setupUi(this);
}

AdminOfAddAtt::~AdminOfAddAtt()
{
    delete ui;
}

void AdminOfAddAtt::on_clearBtn_clicked()
{
    ui->cardLineEdit->clear();
    ui->timeLineEdit->clear();
    ui->statusLineEdit->clear();
}

void AdminOfAddAtt::on_confirmBtn_clicked()
{
    QString card = ui->cardLineEdit->text().trimmed();
    QString timeStr = ui->timeLineEdit->text().trimmed();
    QString status = ui->statusLineEdit->text().trimmed();

    // 非空校验
    if(card.isEmpty() || timeStr.isEmpty() || status.isEmpty())
    {
        QMessageBox::warning(this,"提示","卡号、时间、考勤状态不能为空！");
        return;
    }

    // 根据卡号查询员工表，获取姓名、部门
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT name, department FROM employee WHERE card = :card");
    checkQuery.bindValue(":card", card);
    if(!checkQuery.exec())
    {
        QSqlError err = checkQuery.lastError();
        QMessageBox::critical(this,"错误","数据库查询失败：" + err.text());
        return;
    }
    if(!checkQuery.next())
    {
        QMessageBox::warning(this,"提示","该卡号不存在，请先注册员工！");
        return;
    }

    QString name = checkQuery.value("name").toString();
    QString dept = checkQuery.value("department").toString();

    // 插入考勤记录
    QSqlQuery query;
    query.prepare("INSERT INTO attendance(card, name, department, e_time, status) "
                  "VALUES(:card, :name, :dept, :etime, :status)");
    query.bindValue(":card", card);
    query.bindValue(":name", name);
    query.bindValue(":dept", dept);
    query.bindValue(":etime", timeStr);
    query.bindValue(":status", status);

    bool ok = query.exec();
    if(!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","添加考勤失败："+err.text());
        return;
    }

    QMessageBox::information(this,"成功","考勤记录添加完成！");

    // 清空所有输入框
    ui->cardLineEdit->clear();
    ui->timeLineEdit->clear();
    ui->statusLineEdit->clear();
}
