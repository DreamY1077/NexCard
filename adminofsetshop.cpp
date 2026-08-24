#include "adminofsetshop.h"
#include "ui_adminofsetshop.h"
#include <QMessageBox>
#include "mysql.h"
AdminOfSetShop::AdminOfSetShop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminOfSetShop)
{
    ui->setupUi(this);
}

AdminOfSetShop::~AdminOfSetShop()
{
    delete ui;
}

void AdminOfSetShop::setData(QVariant id)
{
    m_attId = id;

    //根据商品 id 查询原始数据并回显到编辑框
    QSqlQuery query;
    query.prepare("SELECT name, price, number, information FROM product WHERE id = :id");
    query.bindValue(":id", id);
    if(query.exec() && query.next())
    {
        ui->nameLineEdit->setText(query.value("name").toString());
        ui->priceLineEdit->setText(query.value("price").toString());
        ui->numberLineEdit->setText(query.value("number").toString());
        ui->informationLineEdit->setText(query.value("information").toString());
    }
}

void AdminOfSetShop::on_clearBtn_clicked()
{
    ui->nameLineEdit->clear();
    ui->priceLineEdit->clear();
    ui->numberLineEdit->clear();
    ui->informationLineEdit->clear();
}

void AdminOfSetShop::on_confirmBtn_clicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    QString price = ui->priceLineEdit->text().trimmed();
    QString number = ui->numberLineEdit->text().trimmed();
    QString information = ui->informationLineEdit->text().trimmed();
    // 判断是否存在待修改记录id
    if(m_attId.isNull())
    {
        QMessageBox::warning(this,"提示","没有要修改的记录！");
        return;
    }
    if(name.isEmpty() || price.isEmpty() || number.isEmpty() || information.isEmpty())
    {
        QMessageBox::warning(this,"提示","请填写信息！");
        return;
    }

    // 执行更新
    QSqlQuery query;
    query.prepare(R"(
        UPDATE product
        SET name = :name, price = :price, number = :number, information = :information
        WHERE id = :id
    )");
    query.bindValue(":name", name);
    query.bindValue(":price", price);
    query.bindValue(":number", number);
    query.bindValue(":information", information);
    query.bindValue(":id", m_attId);

    bool ok = query.exec();
    if(!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","更新商品记录失败：" + err.text());
        return;
    }

    QMessageBox::information(this,"成功","商品记录更新完成！");
    this->close();
}
