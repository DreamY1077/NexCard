#include "attendanceruledialog.h"
#include "ui_attendanceruledialog.h"
#include <QMessageBox>
#include <QTime>

AttendanceRuleDialog::AttendanceRuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttendanceRuleDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("打卡规则设置");
    this->setWindowIcon(QIcon(":/res/app.ico"));

    //加载当前规则到控件
    AttendanceRule rule = MySql::getMySql()->getAttendanceRule();
    ui->signInStartEdit->setTime(QTime::fromString(rule.signInStart, "HH:mm"));
    ui->signInEndEdit->setTime(QTime::fromString(rule.signInEnd, "HH:mm"));
    ui->lateStartEdit->setTime(QTime::fromString(rule.lateStart, "HH:mm"));
    ui->lateEndEdit->setTime(QTime::fromString(rule.lateEnd, "HH:mm"));
    ui->earlyStartEdit->setTime(QTime::fromString(rule.earlyStart, "HH:mm"));
    ui->earlyEndEdit->setTime(QTime::fromString(rule.earlyEnd, "HH:mm"));
    ui->signOutStartEdit->setTime(QTime::fromString(rule.signOutStart, "HH:mm"));
    ui->signOutEndEdit->setTime(QTime::fromString(rule.signOutEnd, "HH:mm"));
}

AttendanceRuleDialog::~AttendanceRuleDialog()
{
    delete ui;
}

bool AttendanceRuleDialog::loadRuleFromUi(AttendanceRule &rule)
{
    rule.signInStart  = ui->signInStartEdit->time().toString("HH:mm");
    rule.signInEnd    = ui->signInEndEdit->time().toString("HH:mm");
    rule.lateStart    = ui->lateStartEdit->time().toString("HH:mm");
    rule.lateEnd      = ui->lateEndEdit->time().toString("HH:mm");
    rule.earlyStart   = ui->earlyStartEdit->time().toString("HH:mm");
    rule.earlyEnd     = ui->earlyEndEdit->time().toString("HH:mm");
    rule.signOutStart = ui->signOutStartEdit->time().toString("HH:mm");
    rule.signOutEnd   = ui->signOutEndEdit->time().toString("HH:mm");

    //合法性检查：各时段必须首尾相接，且签退结束不能早于签到开始
    QTime s1 = QTime::fromString(rule.signInStart, "HH:mm");
    QTime s2 = QTime::fromString(rule.signInEnd, "HH:mm");
    QTime l1 = QTime::fromString(rule.lateStart, "HH:mm");
    QTime l2 = QTime::fromString(rule.lateEnd, "HH:mm");
    QTime e1 = QTime::fromString(rule.earlyStart, "HH:mm");
    QTime e2 = QTime::fromString(rule.earlyEnd, "HH:mm");
    QTime o1 = QTime::fromString(rule.signOutStart, "HH:mm");
    QTime o2 = QTime::fromString(rule.signOutEnd, "HH:mm");

    if(s1 >= s2) return false;
    if(s2 != l1) return false;   //签到结束必须等于迟到开始
    if(l1 >= l2) return false;
    if(l2 != e1) return false;   //迟到结束必须等于早退开始
    if(e1 >= e2) return false;
    if(e2 != o1) return false;   //早退结束必须等于签退开始
    if(o1 >= o2) return false;
    return true;
}

void AttendanceRuleDialog::on_saveBtn_clicked()
{
    AttendanceRule rule;
    if(!loadRuleFromUi(rule))
    {
        QMessageBox::warning(this, "提示",
            "规则不合法！\n4 个时段必须首尾相接且时间递增，例如：\n"
            "签到 08:00-09:00 / 迟到 09:00-12:00 / 早退 12:00-18:00 / 签退 18:00-23:00");
        return;
    }

    if(MySql::getMySql()->updateAttendanceRule(rule))
    {
        QMessageBox::information(this, "成功", "打卡规则已保存！");
        accept();
    }
    else {
        QMessageBox::critical(this, "错误", "保存打卡规则失败！");
    }
}

void AttendanceRuleDialog::on_cancelBtn_clicked()
{
    reject();
}
