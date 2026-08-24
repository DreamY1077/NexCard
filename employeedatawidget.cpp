#include "employeedatawidget.h"
#include "ui_employeedatawidget.h"
#include "mysql.h"
#include "myserial.h"
#include <QMessageBox>

EmployeeDataWidget::EmployeeDataWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeDataWidget)
{
    ui->setupUi(this);

    //设置编辑框只读(只显示信息)
    ui->nameLineEdit->setReadOnly(true);
    ui->ageLineEdit->setReadOnly(true);
    ui->sexLineEdit->setReadOnly(true);
    ui->departmentLabelEdit->setReadOnly(true);
    ui->cardLineEdit->setReadOnly(true);
    ui->balanceLineEdit->setReadOnly(true);

    //获取数据库地址
    MySql *db = MySql::getMySql();
    //获取串口类地址
    MySerial *ms = MySerial::getMyserial();
    //获取卡号
    QString card = ms->getCardId();
    //从数据库中获取数据
    QString sql = QString("SELECT name,age,sex,department,balance FROM employee WHERE card='%1'").arg(card);
    QSqlQuery query;
        if(query.exec(sql))
        {
            if(query.next()) // 找到记录
            {
                QString name = query.value("name").toString();
                QString age = query.value("age").toString();
                QString sex = query.value("sex").toString();
                QString department = query.value("department").toString();
                QString balance = query.value("balance").toString();

                ui->cardLineEdit->setText(card);
                ui->nameLineEdit->setText(name);
                ui->ageLineEdit->setText(age);
                ui->sexLineEdit->setText(sex);
                ui->departmentLabelEdit->setText(department);
                ui->balanceLineEdit->setText(balance);
            }
            else
            {
                QMessageBox::information(this,"提示","该卡号没有匹配员工信息！");
            }
        }
        else
        {
            QMessageBox::critical(this,"数据库错误",query.lastError().text());
        }

}

EmployeeDataWidget::~EmployeeDataWidget()
{
    delete ui;
}

void EmployeeDataWidget::on_refreshBtn_clicked()
{
    //获取串口类地址
    MySerial *ms = MySerial::getMyserial();
    //获取卡号
    QString globalLoginCard = ms->getCardId();

    QSqlQuery query;
    query.prepare("SELECT name, age, sex, department, card, balance FROM employee WHERE card = :card");
    query.bindValue(":card", globalLoginCard);

    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","查询员工信息失败：" + query.lastError().text());
        return;
    }
    if(!query.next())
    {
        QMessageBox::warning(this,"提示","未找到当前员工信息！");
        // 清空所有输入框
        ui->nameLineEdit->clear();
        ui->ageLineEdit->clear();
        ui->sexLineEdit->clear();
        ui->departmentLabelEdit->clear();
        ui->cardLineEdit->clear();
        ui->balanceLineEdit->clear();
        return;
    }

    // 将数据库数据填充到界面编辑框
    ui->nameLineEdit->setText(query.value("name").toString());
    ui->ageLineEdit->setText(query.value("age").toString());
    ui->sexLineEdit->setText(query.value("sex").toString());
    ui->departmentLabelEdit->setText(query.value("department").toString());
    ui->cardLineEdit->setText(query.value("card").toString());
    ui->balanceLineEdit->setText(query.value("balance").toString());
}
