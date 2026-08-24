#include "adminofsetadd.h"
#include "ui_adminofsetadd.h"
#include <QMessageBox>
#include "mysql.h"
AdminOfSetAdd::AdminOfSetAdd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminOfSetAdd)
{
    ui->setupUi(this);

    //设置卡号编辑框只读
    ui->cardLineEdit->setReadOnly(true);
}

AdminOfSetAdd::~AdminOfSetAdd()
{
    delete ui;
}

void AdminOfSetAdd::setData(QVariant id, QString card, QString timeStr, QString status)
{
    m_attId = id;
    ui->cardLineEdit->setText(card);
    ui->timeLineEdit->setText(timeStr);
    ui->statusLineEdit->setText(status);
}

void AdminOfSetAdd::on_clearBtn_clicked()
{
    ui->timeLineEdit->clear();
    ui->statusLineEdit->clear();
}

void AdminOfSetAdd::on_confirmBtn_clicked()
{
    QString card = ui->cardLineEdit->text().trimmed();
    QString timeStr = ui->timeLineEdit->text().trimmed();
    QString status = ui->statusLineEdit->text().trimmed();

    // 判断是否存在待修改记录id
    if(m_attId.isNull())
    {
        QMessageBox::warning(this,"提示","未加载需要修改的考勤记录！");
        return;
    }
    if(timeStr.isEmpty() || status.isEmpty())
    {
        QMessageBox::warning(this,"提示","更新时间、考勤状态不能为空！");
        return;
    }

    // 根据卡号查询员工姓名、部门
    QSqlQuery empQuery;
    empQuery.prepare("SELECT name, department FROM employee WHERE card = :card");
    empQuery.bindValue(":card", card);
    if(!empQuery.exec())
    {
        QSqlError err = empQuery.lastError();
        QMessageBox::critical(this,"错误","查询员工信息失败：" + err.text());
        return;
    }
    if(!empQuery.next())
    {
        QMessageBox::warning(this,"提示","该卡号不存在！");
        return;
    }
    QString name = empQuery.value("name").toString();
    QString dept = empQuery.value("department").toString();

    // 执行更新
    QSqlQuery query;
    query.prepare(R"(
        UPDATE attendance
        SET card = :c, name = :n, department = :d, e_time = :t, status = :s
        WHERE id = :id
    )");
    query.bindValue(":c", card);
    query.bindValue(":n", name);
    query.bindValue(":d", dept);
    query.bindValue(":t", timeStr);
    query.bindValue(":s", status);
    query.bindValue(":id", m_attId);

    bool ok = query.exec();
    if(!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","更新考勤记录失败：" + err.text());
        return;
    }

    QMessageBox::information(this,"成功","考勤记录更新完成！");
    this->close();
}
