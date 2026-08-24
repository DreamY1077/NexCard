#include "adminofsetemp.h"
#include "ui_adminofsetemp.h"
#include "mysql.h"
#include <QMessageBox>
#include <QSqlQuery>

AdminOFSetEmp::AdminOFSetEmp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminOFSetEmp)
{
    ui->setupUi(this);
}

AdminOFSetEmp::~AdminOFSetEmp()
{
    delete ui;
}

//回显选中员工的原始数据到编辑框
void AdminOFSetEmp::setData(const QString &card, const QString &name,
                            const QString &age, const QString &sex, const QString &department)
{
    ui->cardLineEdit->setText(card);
    ui->nameLineEdit->setText(name);
    ui->ageLineEdit->setText(age);
    ui->sexLineEdit->setText(sex);
    ui->departmentLineEdit->setText(department);
}

void AdminOFSetEmp::on_confirmBtn_clicked()
{
    MySql *db = MySql::getMySql();

    QString card = ui->cardLineEdit->text().trimmed();
    QString newName = ui->nameLineEdit->text().trimmed();
    QString newAge = ui->ageLineEdit->text().trimmed();
    QString newSex = ui->sexLineEdit->text().trimmed();
    QString newDept = ui->departmentLineEdit->text().trimmed();

    // 卡号不能为空
    if(card.isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入卡号！");
        return;
    }

    QSqlQuery query;
    // 以card作为匹配条件
    query.prepare(R"(
        UPDATE employee
        SET name = :name, age = :age, sex = :sex, department = :dept
        WHERE card = :card
    )");
    query.bindValue(":card", card);
    query.bindValue(":name", newName);
    query.bindValue(":age", newAge);
    query.bindValue(":sex", newSex);
    query.bindValue(":dept", newDept);

    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","更新失败：" + query.lastError().text());
        return;
    }

    // numRowsAffected() 获取受影响行数
    if(query.numRowsAffected() <= 0)
    {
        QMessageBox::information(this,"提示","数据库中不存在该卡号，更新无效！");
    }
    else
    {
        QMessageBox::information(this,"提示","员工信息更新成功！");
        // 更新成功后可选清空输入框
        ui->cardLineEdit->clear();
        ui->nameLineEdit->clear();
        ui->ageLineEdit->clear();
        ui->sexLineEdit->clear();
        ui->departmentLineEdit->clear();
    }
}

void AdminOFSetEmp::on_clearBtn_clicked()
{
    ui->cardLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->ageLineEdit->clear();
    ui->sexLineEdit->clear();
    ui->departmentLineEdit->clear();
}
