#include <QTimer>
#include "adminofemployee.h"
#include "ui_adminofemployee.h"
#include "mysql.h"
#include "myserial.h"
#include <QMessageBox>

AdminOfEmployee::AdminOfEmployee(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminOfEmployee)
{
    ui->setupUi(this);

    //所有列平均铺满
    ui->informationTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->informationTableWidget->setAlternatingRowColors(true);

    //打开页面自动刷新
    on_refreshBtn_clicked();
}

AdminOfEmployee::~AdminOfEmployee()
{
    delete ui;
}

void AdminOfEmployee::setLoginLevel(int level)
{
    //三级管理员无"人脸录入"权限（人脸数据属敏感信息，仅一级/二级可操作）
    ui->faceBtn->setVisible(level != 3);
}

void AdminOfEmployee::on_refreshBtn_clicked()
{
    QSqlQuery query;
    query.prepare("SELECT card, name, age, sex, department, balance FROM employee ORDER BY card");

    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","加载员工信息失败:"+query.lastError().text());
        return;
    }

    ui->informationTableWidget->setRowCount(0);
    int row = 0;
    while(query.next())
    {
        ui->informationTableWidget->insertRow(row);

        QString cardVal = query.value("card").toString();
        QString name = query.value("name").toString();
        QString age = query.value("age").toString();
        QString sex = query.value("sex").toString();
        QString department = query.value("department").toString();
        QString balance = query.value("balance").toString();

        //列 0：勾选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        ui->informationTableWidget->setItem(row,0,checkItem);
        ui->informationTableWidget->setItem(row,1,new QTableWidgetItem(cardVal));
        ui->informationTableWidget->setItem(row,2,new QTableWidgetItem(name));
        ui->informationTableWidget->setItem(row,3,new QTableWidgetItem(age));
        ui->informationTableWidget->setItem(row,4,new QTableWidgetItem(sex));
        ui->informationTableWidget->setItem(row,5,new QTableWidgetItem(department));
        ui->informationTableWidget->setItem(row,6,new QTableWidgetItem(balance));

        row++;
    }
    if(row == 0)
    {
        QMessageBox::information(this,"提示","数据库暂无员工信息记录");
    }
}

void AdminOfEmployee::on_searchBtn_clicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    QSqlQuery query;
    if(keyword.isEmpty())
    {
        on_refreshBtn_clicked();
        return;
    }
    query.prepare(R"(
                  SELECT card, name, age, sex, department, balance
                  FROM employee
                  WHERE card LIKE :key
                     OR name LIKE :key
                     OR age LIKE :key
                     OR sex LIKE :key
                     OR department LIKE :key
                  ORDER BY card
              )");
    query.bindValue(":key", "%" + keyword + "%");

    if (!query.exec())
    {
        QMessageBox::critical(this,"错误","查询失败："+query.lastError().text());
        return;
    }

    ui->informationTableWidget->setRowCount(0);
    int row = 0;
    bool find = false;
    while(query.next())
    {
        find = true;
        ui->informationTableWidget->insertRow(row);
        //列 0：勾选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        ui->informationTableWidget->setItem(row,0, checkItem);
        ui->informationTableWidget->setItem(row,1, new QTableWidgetItem(query.value("card").toString()));
        ui->informationTableWidget->setItem(row,2, new QTableWidgetItem(query.value("name").toString()));
        ui->informationTableWidget->setItem(row,3, new QTableWidgetItem(query.value("age").toString()));
        ui->informationTableWidget->setItem(row,4, new QTableWidgetItem(query.value("sex").toString()));
        ui->informationTableWidget->setItem(row,5, new QTableWidgetItem(query.value("department").toString()));
        ui->informationTableWidget->setItem(row,6, new QTableWidgetItem(query.value("balance").toString()));
        row++;
    }
    if(!find)
    {
        QMessageBox::information(this,"提示","未找到匹配员工");
    }

    ui->searchLineEdit->clear();
}

void AdminOfEmployee::on_setBtn_clicked()
{
    //检查是否勾选了一条记录
    int selectCount = 0;
    int selectRow = -1;
    int total = ui->informationTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->informationTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            selectCount++;
            selectRow = r;
        }
    }
    if(selectCount == 0)
    {
        QMessageBox::information(this,"提示","请勾选一条员工记录");
        return;
    }
    if(selectCount > 1)
    {
        QMessageBox::warning(this,"提示","只能勾选一条记录修改！");
        return;
    }

    //读取选中行原始数据（列：1卡号 2姓名 3年龄 4性别 5部门 6余额）
    QString card = ui->informationTableWidget->item(selectRow, 1)->text();
    QString name = ui->informationTableWidget->item(selectRow, 2)->text();
    QString age  = ui->informationTableWidget->item(selectRow, 3)->text();
    QString sex  = ui->informationTableWidget->item(selectRow, 4)->text();
    QString dept = ui->informationTableWidget->item(selectRow, 5)->text();

    m_adminOfSetEmp = new AdminOFSetEmp;
    m_adminOfSetEmp->setData(card, name, age, sex, dept);   //回显原数据
    m_adminOfSetEmp->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动释放内存
    m_adminOfSetEmp->show();
}

void AdminOfEmployee::on_faceBtn_clicked()
{
    //检查是否勾选了一条记录（列 0 为卡号，列 1 为姓名）
    int selectCount = 0;
    int selectRow = -1;
    int total = ui->informationTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->informationTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            selectCount++;
            selectRow = r;
        }
    }
    if(selectCount == 0)
    {
        QMessageBox::information(this,"提示","请勾选一条员工记录");
        return;
    }
    if(selectCount > 1)
    {
        QMessageBox::warning(this,"提示","只能勾选一条记录进行人脸录入！");
        return;
    }

    QString card = ui->informationTableWidget->item(selectRow, 1)->text();
    QString name = ui->informationTableWidget->item(selectRow, 2)->text();

    // 确认对话框：已录入则提示修改，未录入则提示录入
    bool hasFace = MySql::getMySql()->hasFaceInfo(card);
    QString msg;
    if(hasFace)
    {
        msg = QString("该员工已录入人脸，是否确认修改 [%1] 的人脸信息？").arg(name);
    }
    else
    {
        msg = QString("是否确认录入员工 [%1] 的人脸信息？").arg(name);
    }
    int ret = QMessageBox::question(this, "请确认", msg,
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if(ret != QMessageBox::Yes)
    {
        return;   // 取消，不打开人脸界面
    }

    FaceRegisterDialog *dlg = new FaceRegisterDialog(card, name, this);
    dlg->show();
}
