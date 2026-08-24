#include "employeerefill.h"
#include "ui_employeerefill.h"
#include <QMessageBox>
#include "mysql.h"
#include "myserial.h"
EmployeeRefill::EmployeeRefill(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeRefill)
{
    ui->setupUi(this);
}

EmployeeRefill::~EmployeeRefill()
{
    delete ui;
}

void EmployeeRefill::on_confirmBtn_clicked()
{
    // 获取输入内容
    QString moneyText = ui->moneyLineEdit->text().trimmed();
    QString payWay = ui->wayComboBox->currentText();

    // 非空校验
    if(moneyText.isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入充值金额！");
        return;
    }
    bool ok;
    double addMoney = moneyText.toDouble(&ok);
    // 判断是否为合法数字，并且金额大于0
    if(!ok || addMoney <= 0)
    {
        QMessageBox::warning(this,"提示","请输入有效的充值金额（金额必须大于0）");
        return;
    }

    if(payWay.isEmpty())
    {
        QMessageBox::warning(this,"提示","请选择充值方式！");
        return;
    }

    // 二次确认弹窗
    int ret = QMessageBox::question(this,"确认充值",
        QString("充值金额：%1 元\n充值方式：%2\n确认提交？").arg(addMoney).arg(payWay),
        QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes)
        return;

    //获取登录员工卡号
    MySerial *ms = MySerial::getMyserial();
    QString globalLoginCard = ms->getCardId();

    // 数据库更新余额
    QSqlQuery query;
    query.prepare("UPDATE employee SET balance = balance + :addMoney WHERE card = :card");
    query.bindValue(":addMoney", addMoney);
    query.bindValue(":card", globalLoginCard); // 当前登录员工卡号

    if(!query.exec())
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"充值失败","数据库错误：" + err.text());
        return;
    }

    QMessageBox::information(this,"充值成功",QString("成功充值 %1 元！").arg(addMoney));

    // 清空输入框
    ui->moneyLineEdit->clear();
}
