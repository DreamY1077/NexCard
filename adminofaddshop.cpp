#include "adminofaddshop.h"
#include "ui_adminofaddshop.h"
#include <QMessageBox>
#include "mysql.h"
AdminOfAddShop::AdminOfAddShop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminOfAddShop)
{
    ui->setupUi(this);
}

AdminOfAddShop::~AdminOfAddShop()
{
    delete ui;
}

void AdminOfAddShop::on_clearBtn_clicked()
{
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->numberLineEdit->clear();
    ui->informationLineEdit->clear();
}

void AdminOfAddShop::on_confirmBtn_clicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    QString price = ui->priceLineEdit->text().trimmed();
    QString number = ui->numberLineEdit->text().trimmed();
    QString information = ui->informationLineEdit->text().trimmed();

    //非空检验
    if(name.isEmpty() || price.isEmpty() || number.isEmpty() || information.isEmpty())
    {
        QMessageBox::warning(this,"提示","请填写信息");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO product(name, price, number, information) "
                  "VALUES(:name ,:price ,:number ,:information)");
    query.bindValue(":name",name);
    query.bindValue(":price",price);
    query.bindValue(":number",number);
    query.bindValue(":information",information);

    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","添加商品失败："+query.lastError().text());
        return;
    }

    QMessageBox::information(this,"成功","商品添加成功");

    on_clearBtn_clicked();
}
