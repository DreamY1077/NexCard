#ifndef ATTENDANCERULEDIALOG_H
#define ATTENDANCERULEDIALOG_H

#include <QDialog>
#include "mysql.h"

namespace Ui {
class AttendanceRuleDialog;
}

//打卡规则修改对话框（仅一级管理员可打开）
class AttendanceRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttendanceRuleDialog(QWidget *parent = nullptr);
    ~AttendanceRuleDialog();

private slots:
    void on_saveBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::AttendanceRuleDialog *ui;
    //从控件加载规则到结构体并保存
    bool loadRuleFromUi(AttendanceRule &rule);
};

#endif // ATTENDANCERULEDIALOG_H
