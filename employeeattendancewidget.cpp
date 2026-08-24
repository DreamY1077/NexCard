#include <QTimer>
#include "employeeattendancewidget.h"
#include "ui_employeeattendancewidget.h"
#include "myserial.h"
#include "mysql.h"
#include <QMessageBox>
#include <QDebug>
#include <QColor>
#include <QTime>

//考勤状态着色：签到绿 / 签退蓝
static QTableWidgetItem *makeStatusItem(const QString &status)
{
    QTableWidgetItem *item = new QTableWidgetItem(status);
    if(status == "签到")
    {
        item->setForeground(QColor(0x27, 0xae, 0x60));
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    else if(status == "签退")
    {
        item->setForeground(QColor(0x5b, 0x9b, 0xd5));
    }
    return item;
}

EmployeeAttendanceWidget::EmployeeAttendanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeAttendanceWidget)
{
    ui->setupUi(this);
    //所有列平均铺满
    ui->attendanceTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->attendanceTableWidget->setAlternatingRowColors(true);
    //跳转页面 直接刷新
    on_refreshBtn_clicked();
}

EmployeeAttendanceWidget::~EmployeeAttendanceWidget()
{
    delete ui;
}

void EmployeeAttendanceWidget::on_pushButton_clicked()
{
    //获取卡号
    MySerial *ms = MySerial::getMyserial();
    QString card = ms->getCardId();

    //获取数据库地址
    MySql *db = MySql::getMySql();
    QString name = db->findEmployeeName(card);
    QString department = db->findEmployeeDepartment(card);

    //根据打卡规则判断当前时段
    AttendanceRule rule = db->getAttendanceRule();
    QTime now = QTime::currentTime();
    QTime signInS  = QTime::fromString(rule.signInStart, "HH:mm");
    QTime signInE  = QTime::fromString(rule.signInEnd, "HH:mm");
    QTime lateS    = QTime::fromString(rule.lateStart, "HH:mm");
    QTime lateE    = QTime::fromString(rule.lateEnd, "HH:mm");
    QTime earlyS   = QTime::fromString(rule.earlyStart, "HH:mm");
    QTime earlyE   = QTime::fromString(rule.earlyEnd, "HH:mm");
    QTime signOutS = QTime::fromString(rule.signOutStart, "HH:mm");
    QTime signOutE = QTime::fromString(rule.signOutEnd, "HH:mm");

    QString newStatus;
    if(now >= signInS && now < signInE)
    {
        newStatus = "签到";
    }
    else if(now >= lateS && now < lateE)
    {
        newStatus = "迟到";
    }
    else if(now >= earlyS && now < earlyE)
    {
        newStatus = "早退";
    }
    else if(now >= signOutS && now < signOutE)
    {
        newStatus = "签退";
    }
    else {
        //非开放时段
        QMessageBox::information(this,"提示","该时段考勤系统未开放！");
        return;
    }

    //当天打卡去重：
    //签到/迟到同属"上班打卡"，当天只能存在一种；早退/签退同属"下班打卡"，当天只能存在一种
    QStringList todayStatus = db->getTodayAttendanceStatus(card);
    bool isWorkIn  = (newStatus == "签到" || newStatus == "迟到");
    bool isWorkOut = (newStatus == "早退" || newStatus == "签退");
    if(isWorkIn && (todayStatus.contains("签到") || todayStatus.contains("迟到")))
    {
        QMessageBox::information(this,"提示","今天已完成上班打卡（签到/迟到），请勿重复打卡！");
        return;
    }
    if(isWorkOut && (todayStatus.contains("早退") || todayStatus.contains("签退")))
    {
        QMessageBox::information(this,"提示","今天已完成下班打卡（早退/签退），请勿重复打卡！");
        return;
    }

    bool isAttendance = db->insertAttendance(card,name,department,newStatus);
    if(isAttendance)
    {
        QMessageBox::information(this,"提示",QString("打卡成功（%1）").arg(newStatus));
    }
    else {
        QMessageBox::critical(this,"错误","打卡失败");
    }
}

void EmployeeAttendanceWidget::on_refreshBtn_clicked()
{
    MySql *db =MySql::getMySql();

    MySerial *ms = MySerial::getMyserial();
    QString cardId = ms->getCardId();

    QSqlQuery query;
    query.prepare("SELECT card, name, department, e_time, status "
                  "FROM attendance "
                  "WHERE card = :card "
                  "ORDER BY e_time DESC");
    query.bindValue(":card", cardId);

    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","查询考勤失败:"+query.lastError().text());
        return;
    }

    ui->attendanceTableWidget->setRowCount(0);
    int row = 0;
    while(query.next())
    {
        ui->attendanceTableWidget->insertRow(row);

        QString cardVal = query.value("card").toString();
        QString name = query.value("name").toString();
        QString department = query.value("department").toString();
        QString e_time = query.value("e_time").toString();
        QString status = query.value("status").toString();

        ui->attendanceTableWidget->setItem(row,0,new QTableWidgetItem(cardVal));
        ui->attendanceTableWidget->setItem(row,1,new QTableWidgetItem(name));
        ui->attendanceTableWidget->setItem(row,2,new QTableWidgetItem(department));
        ui->attendanceTableWidget->setItem(row,3,new QTableWidgetItem(e_time));
        ui->attendanceTableWidget->setItem(row,4,makeStatusItem(status));

        row++;
    }
    if(row == 0)
    {
        QMessageBox::information(this,"提示","该卡号暂无考勤记录");
    }
}

void EmployeeAttendanceWidget::on_searchBtn_clicked()
{
    QString key = ui->searchLineEdit->text().trimmed();
    MySql *db = MySql::getMySql();

    MySerial *ms = MySerial::getMyserial();
    QString cardId = ms->getCardId();

    QSqlQuery query;

    if(key.isEmpty())
    {
        // 输入为空，等价刷新当前卡号全部记录
        on_refreshBtn_clicked();
        return;
    }
    // 模糊匹配时间字段
    query.prepare("SELECT card, name, department, e_time, status "
                      "FROM attendance "
                      "WHERE card = :card AND e_time LIKE :keyword "
                      "ORDER BY e_time DESC");
    query.bindValue(":card", cardId);
    query.bindValue(":keyword", "%" + key + "%");

    if (!query.exec())
    {
        QMessageBox::critical(this,"错误","查询失败：" + query.lastError().text());
        return;
    }

    ui->attendanceTableWidget->setRowCount(0);
    int row = 0;
    bool hasData = false;
    while(query.next())
    {
        hasData = true;
        ui->attendanceTableWidget->insertRow(row);
        ui->attendanceTableWidget->setItem(row,0,new QTableWidgetItem(query.value("card").toString()));
        ui->attendanceTableWidget->setItem(row,1,new QTableWidgetItem(query.value("name").toString()));
        ui->attendanceTableWidget->setItem(row,2,new QTableWidgetItem(query.value("department").toString()));
        ui->attendanceTableWidget->setItem(row,3,new QTableWidgetItem(query.value("e_time").toString()));
        ui->attendanceTableWidget->setItem(row,4,makeStatusItem(query.value("status").toString()));
        row++;
    }
    if(!hasData)
    {
        QMessageBox::information(this,"提示","未找到该时间段考勤记录");
    }

    ui->searchLineEdit->clear();
}
