#include <QTimer>
#include "adminofattendance.h"
#include "ui_adminofattendance.h"
#include "attendanceruledialog.h"
#include <mysql.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QColor>

//考勤状态着色：签到绿 / 签退蓝
static QTableWidgetItem *makeStatusItem(const QString &status)
{
    QTableWidgetItem *item = new QTableWidgetItem(status);
    if(status == "签到")
    {
        item->setForeground(QColor(0x27, 0xae, 0x60));
        item->setFont(item->font());
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
AdminOfAttendance::AdminOfAttendance(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminOfAttendance)
{
    ui->setupUi(this);

    m_adminOfAddAtt = nullptr;
    m_adminOfSetAtt = nullptr;
    m_loginLevel = 3;   //默认按最低权限初始化，登录后由 setLoginLevel 调整

    QStringList headers{"选择","卡号","姓名","部门","时间","考勤状态"};
    ui->attendanceTableWidget->setColumnCount(headers.size());
    ui->attendanceTableWidget->setHorizontalHeaderLabels(headers);
    //所有列平均铺满
    ui->attendanceTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->attendanceTableWidget->setAlternatingRowColors(true);


    //打开页面自动刷新
    on_refreshBtn_clicked();
}

void AdminOfAttendance::setLoginLevel(int level)
{
    m_loginLevel = level;
    //一级：全部按钮可用（含规则修改）；二级：可增删改考勤，不可改规则；三级：仅可查看
    bool canEdit = (level != 3);          //一二级可增删改
    bool canRule = (level == 1);          //仅一级可改规则
    ui->addBtn->setVisible(canEdit);
    ui->setBtn->setVisible(canEdit);
    ui->deleteBtn->setVisible(canEdit);
    ui->ruleBtn->setVisible(canRule);
}

void AdminOfAttendance::on_ruleBtn_clicked()
{
    //仅一级管理员可以修改打卡规则（按钮已按权限隐藏，此处再校验一次）
    if(m_loginLevel != 1)
    {
        QMessageBox::warning(this, "提示", "仅一级管理员可以修改打卡规则！");
        return;
    }
    AttendanceRuleDialog dlg(this);
    dlg.exec();
}

AdminOfAttendance::~AdminOfAttendance()
{
    delete ui;
}

void AdminOfAttendance::on_refreshBtn_clicked()
{
    QSqlQuery query;

    query.prepare("SELECT id, card, name, department, e_time, status FROM attendance ORDER BY e_time DESC");
    bool ok = query.exec();
    if (!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","加载考勤数据失败：" + err.text());
        return;
    }

    ui->attendanceTableWidget->setRowCount(0);
    int row = 0;
    while(query.next())
    {
        ui->attendanceTableWidget->insertRow(row);


        //添加复选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        // 把主键id存到item，后面删除时读取
        checkItem->setData(Qt::UserRole, query.value("id"));
        ui->attendanceTableWidget->setItem(row, 0, checkItem);

        ui->attendanceTableWidget->setItem(row,1,new QTableWidgetItem(query.value("card").toString()));
        ui->attendanceTableWidget->setItem(row,2,new QTableWidgetItem(query.value("name").toString()));
        ui->attendanceTableWidget->setItem(row,3,new QTableWidgetItem(query.value("department").toString()));
        ui->attendanceTableWidget->setItem(row,4,new QTableWidgetItem(query.value("e_time").toString()));
        ui->attendanceTableWidget->setItem(row,5,makeStatusItem(query.value("status").toString()));
        row++;
    }
}

void AdminOfAttendance::on_searchBtn_clicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    QSqlQuery query;

    // 搜索框为空，刷新全部
    if (keyword.isEmpty())
    {
        on_refreshBtn_clicked();
        return;
    }

    query.prepare(R"(
        SELECT card, name, department, e_time, status
        FROM attendance
        WHERE card LIKE :key
           OR name LIKE :key
           OR department LIKE :key
           OR e_time LIKE :key
           OR status LIKE :key
        ORDER BY e_time DESC
    )");
    query.bindValue(":key", "%" + keyword + "%");

    bool ok = query.exec();
    if (!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this, "错误", "查询失败：" + err.text());
        return;
    }

    ui->attendanceTableWidget->setRowCount(0);
    int row = 0;
    bool hasData = false;

    while (query.next())
    {
        hasData = true;
        ui->attendanceTableWidget->insertRow(row);
        ui->attendanceTableWidget->setItem(row,1,new QTableWidgetItem(query.value("card").toString()));
        ui->attendanceTableWidget->setItem(row,2,new QTableWidgetItem(query.value("name").toString()));
        ui->attendanceTableWidget->setItem(row,3,new QTableWidgetItem(query.value("department").toString()));
        ui->attendanceTableWidget->setItem(row,4,new QTableWidgetItem(query.value("e_time").toString()));
        ui->attendanceTableWidget->setItem(row,5,makeStatusItem(query.value("status").toString()));
        row++;
    }

    if (!hasData)
    {
        QMessageBox::information(this, "提示", "未找到匹配的考勤记录");
    }

    //清空编辑框内容
    ui->searchLineEdit->clear();
}

void AdminOfAttendance::on_deleteBtn_clicked()
{
    QStringList idList;
    int totalRow = ui->attendanceTableWidget->rowCount();

    // 遍历所有行，收集勾选的id
    for(int r = 0; r < totalRow; r++)
    {
        QTableWidgetItem *item = ui->attendanceTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            QString id = item->data(Qt::UserRole).toString();
            idList << id;
        }
    }

    if(idList.isEmpty())
    {
        QMessageBox::information(this,"提示","请先勾选要删除的记录！");
        return;
    }

    // 弹窗确认，防止误删
    int ret = QMessageBox::question(this,"确认删除",
        QString("你选中了%1条记录，删除后无法恢复，确定继续？").arg(idList.size()),
        QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes)
        return;

    QSqlQuery query;
    // 拼接批量删除SQL
    QString sql = "DELETE FROM attendance WHERE id IN (";
    sql += idList.join(",");
    sql += ")";
    query.prepare(sql);

    bool ok = query.exec();
    if(!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","删除失败：" + err.text());
        return;
    }

    QMessageBox::information(this,"成功","选中记录已删除！");
    // 删除完成刷新表格
    on_refreshBtn_clicked();
}

void AdminOfAttendance::on_addBtn_clicked()
{
    m_adminOfAddAtt = new AdminOfAddAtt;
    m_adminOfAddAtt->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动释放内存
    m_adminOfAddAtt->show();
}

void AdminOfAttendance::on_setBtn_clicked()
{
    int selectCount = 0;
    int selectRow = -1;
    QVariant targetId;
    QString card, timeStr, status;

    int total = ui->attendanceTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->attendanceTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            selectCount++;
            selectRow = r;
            targetId = item->data(Qt::UserRole);
        }
    }

    if(selectCount == 0)
    {
        QMessageBox::information(this,"提示","请勾选一条考勤记录");
        return;
    }
    if(selectCount > 1)
    {
        QMessageBox::warning(this,"提示","只能选中一条记录修改！");
        return;
    }

    // 读取该行数据
    card = ui->attendanceTableWidget->item(selectRow,1)->text();
    timeStr = ui->attendanceTableWidget->item(selectRow,4)->text();
    status = ui->attendanceTableWidget->item(selectRow,5)->text();

    // 切换到修改页面
    m_adminOfSetAtt = new AdminOfSetAdd;
    m_adminOfSetAtt->setData(targetId, card, timeStr, status);
    m_adminOfSetAtt->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动释放内存
    m_adminOfSetAtt->show();
}
