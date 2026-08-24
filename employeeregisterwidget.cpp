#include "employeeregisterwidget.h"
#include "ui_employeeregisterwidget.h"
#include <QtDebug>
#include <QSqlQuery>
#include "mysql.h"
#include "myserial.h"
#include <QMessageBox>

EmployeeRegisterWidget::EmployeeRegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeRegisterWidget)
{
    ui->setupUi(this);
    //设置卡号编辑框只读 只能刷卡
    ui->cardLineEdit->setReadOnly(true);

    //绑定信号 获取卡号
    connect(MySerial::getMyserial(),&MySerial::cardIdReceived,this,&EmployeeRegisterWidget::onGetCard);
}

EmployeeRegisterWidget::~EmployeeRegisterWidget()
{
    delete ui;
}

void EmployeeRegisterWidget::on_confirmBtn_clicked()
{
    //获取数据
    QString card = ui->cardLineEdit->text();
    QString name = ui->nameLineEdit->text();
    QString age = ui->ageLineEdit->text();
    QString sex;
    if(ui->manRadioBtn->isChecked())
    {
        sex = "男";
    }
    else {
        sex = "女";
    }
    QString department = ui->departmentLineEdit->text();

    QSqlQuery query;
    query.prepare("SELECT * FROM employee WHERE card=:card");
    query.bindValue(":card",card);
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
        QMessageBox::information(this,"提示","该卡已注册！");
        ui->cardLineEdit->setText("");
        return;
    }

    if(card.isEmpty())
    {
        QMessageBox::information(this,"提示","卡号不能为空！");
        return;
    }
    if(name.isEmpty())
    {
        QMessageBox::information(this,"提示","姓名不能为空！");
        return;
    }
    if(age.isEmpty())
    {
        QMessageBox::information(this,"提示","年龄不能为空！");
        return;
    }
    if(sex.isEmpty())
    {
        QMessageBox::information(this,"提示","性别不能为空！");
        return;
    }
    if(department.isEmpty())
    {
        QMessageBox::information(this,"提示","部门不能为空！");
        return;
    }

    //获取数据库地址
    MySql *db = MySql::getMySql();

    //人脸识别录入为必填项：注册前必须已完成人脸录入
    if(!db->hasFaceInfo(card))
    {
        QMessageBox::information(this,"提示","请先完成人脸识别录入（点击「人脸录入」按钮）！");
        return;
    }

    //执行插入
    db->insertEmployeeData(card,name,age,sex,department);

    ui->faceBtn->setEnabled(true);
    QMessageBox::information(this,"提示","注册成功！可正常使用人脸考勤打卡。");

    //清空输入框
    ui->cardLineEdit->clear();
    ui->nameLineEdit->clear();
    ui->ageLineEdit->clear();

    //单选框取消互斥后设空
    ui->manRadioBtn->setAutoExclusive(false);
    ui->manRadioBtn->setChecked(false);
    ui->manRadioBtn->setAutoExclusive(true);
    ui->womanRadioBtn->setAutoExclusive(false);
    ui->womanRadioBtn->setChecked(false);
    ui->womanRadioBtn->setAutoExclusive(true);

    ui->departmentLineEdit->clear();
}

//获取卡号
void EmployeeRegisterWidget::onGetCard(const QString &cardId)
{
    ui->cardLineEdit->setText(cardId);
    //刷卡识别到卡号后即可点击"人脸录入"
    ui->faceBtn->setEnabled(true);
}

void EmployeeRegisterWidget::on_faceBtn_clicked()
{
    //使用输入框当前卡号（刷卡获得）
    QString card = ui->cardLineEdit->text().trimmed();
    if(card.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先刷卡获取卡号");
        return;
    }

    QString name = ui->nameLineEdit->text().trimmed();
    if(name.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先填写姓名，再进行人脸录入");
        return;
    }

    // 打开人脸录入对话框（复用管理员端录入界面）
    FaceRegisterDialog *dlg = new FaceRegisterDialog(card, name, this);
    dlg->show();
}
