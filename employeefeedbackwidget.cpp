#include "employeefeedbackwidget.h"
#include "ui_employeefeedbackwidget.h"
#include "mysql.h"
#include "myserial.h"
#include <QMessageBox>

EmployeeFeedbackWidget::EmployeeFeedbackWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeFeedbackWidget)
{
    ui->setupUi(this);
}

EmployeeFeedbackWidget::~EmployeeFeedbackWidget()
{
    delete ui;
}

void EmployeeFeedbackWidget::on_submitBtn_clicked()
{
    QString title = ui->titleLineEdit->text().trimmed();
    QString type = ui->typeComboBox->currentText();
    QString content = ui->contentTextEdit->toPlainText().trimmed();

    if(title.isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入反馈标题！");
        return;
    }
    if(content.isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入反馈内容！");
        return;
    }

    //当前登录员工身份
    MySerial *ms = MySerial::getMyserial();
    QString card = ms->getCardId();
    MySql *db = MySql::getMySql();
    QString name = db->findEmployeeName(card);
    QString department = db->findEmployeeDepartment(card);

    if(db->insertFeedback(card, name, department, title, type, content))
    {
        QMessageBox::information(this,"成功","反馈已提交，管理员将在异常反馈箱中查看！");
        ui->titleLineEdit->clear();
        ui->contentTextEdit->clear();
    }
    else {
        QMessageBox::critical(this,"错误","反馈提交失败！");
    }
}

void EmployeeFeedbackWidget::on_clearBtn_clicked()
{
    ui->titleLineEdit->clear();
    ui->contentTextEdit->clear();
}
