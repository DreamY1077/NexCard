#include "adminfeedbackwidget.h"
#include "ui_adminfeedbackwidget.h"
#include "mysql.h"
#include <QMessageBox>
#include <QColor>
#include <QSqlQuery>
#include <QSqlError>

AdminFeedbackWidget::AdminFeedbackWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminFeedbackWidget)
{
    ui->setupUi(this);

    QStringList headers{"异常类型","关联卡号/商品号","名称","发生时间","说明"};
    ui->feedbackTableWidget->setColumnCount(headers.size());
    ui->feedbackTableWidget->setHorizontalHeaderLabels(headers);
    //所有列平均铺满
    ui->feedbackTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->feedbackTableWidget->setAlternatingRowColors(true);

    //双击任意单元格：弹出该行完整信息（解决长文本看不完整的问题）
    connect(ui->feedbackTableWidget, &QTableWidget::cellDoubleClicked,
            this, [this](int row, int column){
        Q_UNUSED(column);
        auto itemText = [this](int r, int c) -> QString {
            QTableWidgetItem *it = ui->feedbackTableWidget->item(r, c);
            return it ? it->text() : QString();
        };
        QString type = itemText(row, 0);
        QString id   = itemText(row, 1);
        QString name = itemText(row, 2);
        QString time = itemText(row, 3);
        QString desc = itemText(row, 4);
        QString msg = QString("异常类型：%1\n关联卡号/商品号：%2\n名称：%3\n发生时间：%4\n\n详细说明：\n%5")
                          .arg(type).arg(id).arg(name).arg(time).arg(desc);
        QMessageBox::information(this, "详细信息", msg);
    });

    //打开页面自动刷新
    on_refreshBtn_clicked();
}

AdminFeedbackWidget::~AdminFeedbackWidget()
{
    delete ui;
}

void AdminFeedbackWidget::on_refreshBtn_clicked()
{
    ui->feedbackTableWidget->setRowCount(0);
    int row = 0;

    //1. 迟到/早退考勤记录
    QSqlQuery attQuery;
    attQuery.prepare("SELECT card, name, department, e_time, status "
                     "FROM attendance WHERE status = '迟到' OR status = '早退' "
                     "ORDER BY e_time DESC");
    if(!attQuery.exec())
    {
        QMessageBox::critical(this,"错误","查询考勤异常失败：" + attQuery.lastError().text());
        return;
    }
    while(attQuery.next())
    {
        ui->feedbackTableWidget->insertRow(row);
        QTableWidgetItem *typeItem = new QTableWidgetItem("考勤-" + attQuery.value("status").toString());
        typeItem->setForeground(QColor(0xf0, 0xad, 0x4e));   //橙色提示
        ui->feedbackTableWidget->setItem(row, 0, typeItem);
        ui->feedbackTableWidget->setItem(row, 1, new QTableWidgetItem(attQuery.value("card").toString()));
        ui->feedbackTableWidget->setItem(row, 2, new QTableWidgetItem(attQuery.value("name").toString()));
        ui->feedbackTableWidget->setItem(row, 3, new QTableWidgetItem(attQuery.value("e_time").toString()));
        ui->feedbackTableWidget->setItem(row, 4,
            new QTableWidgetItem("考勤异常：" + attQuery.value("status").toString() + "（" + attQuery.value("department").toString() + "）"));
        row++;
    }

    //2. 库存为 0 的商品
    QSqlQuery stockQuery;
    stockQuery.prepare("SELECT id, name, price, number FROM product WHERE CAST(number AS INTEGER) = 0 ORDER BY id");
    if(!stockQuery.exec())
    {
        QMessageBox::critical(this,"错误","查询库存异常失败：" + stockQuery.lastError().text());
        return;
    }
    while(stockQuery.next())
    {
        ui->feedbackTableWidget->insertRow(row);
        QTableWidgetItem *typeItem = new QTableWidgetItem("库存不足");
        typeItem->setForeground(QColor(0xd9, 0x53, 0x4f));   //红色警告
        ui->feedbackTableWidget->setItem(row, 0, typeItem);
        ui->feedbackTableWidget->setItem(row, 1, new QTableWidgetItem(stockQuery.value("id").toString()));
        ui->feedbackTableWidget->setItem(row, 2, new QTableWidgetItem(stockQuery.value("name").toString()));
        ui->feedbackTableWidget->setItem(row, 3, new QTableWidgetItem("-"));
        ui->feedbackTableWidget->setItem(row, 4,
            new QTableWidgetItem("库存为 0，请及时补货（单价 ¥" + stockQuery.value("price").toString() + "）"));
        row++;
    }

    //3. 员工问题反馈（含反馈人姓名与部门）
    QSqlQuery fbQuery;
    fbQuery.prepare("SELECT card, name, department, title, type, content, f_time "
                    "FROM feedback ORDER BY f_time DESC");
    if(!fbQuery.exec())
    {
        QMessageBox::critical(this,"错误","查询问题反馈失败：" + fbQuery.lastError().text());
        return;
    }
    while(fbQuery.next())
    {
        ui->feedbackTableWidget->insertRow(row);
        QTableWidgetItem *typeItem = new QTableWidgetItem("问题反馈-" + fbQuery.value("type").toString());
        typeItem->setForeground(QColor(0x34, 0x76, 0xc2));   //蓝色
        ui->feedbackTableWidget->setItem(row, 0, typeItem);
        ui->feedbackTableWidget->setItem(row, 1, new QTableWidgetItem(fbQuery.value("card").toString()));
        ui->feedbackTableWidget->setItem(row, 2, new QTableWidgetItem(fbQuery.value("name").toString()));
        ui->feedbackTableWidget->setItem(row, 3, new QTableWidgetItem(fbQuery.value("f_time").toString()));
        ui->feedbackTableWidget->setItem(row, 4,
            new QTableWidgetItem("【" + fbQuery.value("title").toString() + "】"
                + fbQuery.value("content").toString()
                + "（反馈人：" + fbQuery.value("name").toString()
                + " · " + fbQuery.value("department").toString() + "）"));
        row++;
    }

    if(row == 0)
    {
        //无异常时插入一行友好提示
        ui->feedbackTableWidget->insertRow(0);
        QTableWidgetItem *okItem = new QTableWidgetItem("暂无异常");
        okItem->setForeground(QColor(0x27, 0xae, 0x60));   //绿色
        ui->feedbackTableWidget->setItem(0, 0, okItem);
        ui->feedbackTableWidget->setItem(0, 1, new QTableWidgetItem("-"));
        ui->feedbackTableWidget->setItem(0, 2, new QTableWidgetItem("-"));
        ui->feedbackTableWidget->setItem(0, 3, new QTableWidgetItem("-"));
        ui->feedbackTableWidget->setItem(0, 4, new QTableWidgetItem("当前没有异常反馈"));
    }
}
