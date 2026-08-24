#include <QTimer>
#include "adminofshop.h"
#include "ui_adminofshop.h"
#include "mysql.h"
#include <QMessageBox>
#include <QColor>

//库存着色：<=0 红色加粗"缺货"，<=5 橙色提示
static QTableWidgetItem *makeStockItem(int stock)
{
    QTableWidgetItem *item = new QTableWidgetItem(QString::number(stock));
    if(stock <= 0)
    {
        item->setText("缺货");
        item->setForeground(QColor(0xd9, 0x53, 0x4f));
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    else if(stock <= 5)
    {
        item->setForeground(QColor(0xf0, 0xad, 0x4e));
    }
    return item;
}


AdminOfShop::AdminOfShop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminOfShop)
{
    ui->setupUi(this);
    QStringList headers{"选择","商品号","商品名称","单价","库存","商品信息"};
    ui->shopTableWidget->setColumnCount(headers.size());
    ui->shopTableWidget->setHorizontalHeaderLabels(headers);
    //所有列平均铺满
    ui->shopTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->shopTableWidget->setAlternatingRowColors(true);

    on_refreshBtn_clicked();
}

AdminOfShop::~AdminOfShop()
{
    delete ui;
}

void AdminOfShop::on_refreshBtn_clicked()
{
    QSqlQuery query;

    query.prepare("SELECT id, name, price, number, information FROM product ORDER BY id ASC");
    bool ok = query.exec();
    if (!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","加载考勤数据失败：" + err.text());
        return;
    }

    ui->shopTableWidget->setRowCount(0);
    int row = 0;
    while(query.next())
    {
        ui->shopTableWidget->insertRow(row);


        //添加复选框
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        // 把主键id存到item，后面删除时读取
        checkItem->setData(Qt::UserRole, query.value("id"));
        ui->shopTableWidget->setItem(row, 0, checkItem);

        ui->shopTableWidget->setItem(row,1,new QTableWidgetItem(query.value("id").toString()));
        ui->shopTableWidget->setItem(row,2,new QTableWidgetItem(query.value("name").toString()));
        ui->shopTableWidget->setItem(row,3,new QTableWidgetItem(query.value("price").toString()));
        ui->shopTableWidget->setItem(row,4,makeStockItem(query.value("number").toInt()));
        ui->shopTableWidget->setItem(row,5,new QTableWidgetItem(query.value("information").toString()));
        row++;
    }
}

void AdminOfShop::on_addBtn_clicked()
{
    m_adminOfAddShop = new AdminOfAddShop;
    m_adminOfAddShop->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动释放内存
    m_adminOfAddShop->show();
}

void AdminOfShop::on_deleteBtn_clicked()
{
    QStringList idList;
    int totalRow = ui->shopTableWidget->rowCount();

    // 遍历所有行，收集勾选的id
    for(int r = 0; r < totalRow; r++)
    {
        QTableWidgetItem *item = ui->shopTableWidget->item(r, 0);
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
    QString sql = "DELETE FROM product WHERE id IN (";
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

void AdminOfShop::on_setBtn_clicked()
{
    int selectCount = 0;
    int selectRow = -1;
    QVariant targetId;
    QString card, timeStr, status;

    int total = ui->shopTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->shopTableWidget->item(r, 0);
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

    // 切换到修改页面
    m_adminOfSetShop = new AdminOfSetShop;
    m_adminOfSetShop->setData(targetId);
    m_adminOfSetShop->setAttribute(Qt::WA_DeleteOnClose); // 关闭自动释放内存
    m_adminOfSetShop->show();
}

void AdminOfShop::on_searchBtn_clicked()
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
        SELECT id, name, price, number, information
        FROM product
        WHERE id LIKE :key
           OR name LIKE :key
           OR information LIKE :key
        ORDER BY id DESC
    )");
    query.bindValue(":key", "%" + keyword + "%");

    bool ok = query.exec();
    if (!ok)
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this, "错误", "查询失败：" + err.text());
        return;
    }

    ui->shopTableWidget->setRowCount(0);
    int row = 0;
    bool hasData = false;

    while (query.next())
    {
        hasData = true;
        ui->shopTableWidget->insertRow(row);
        ui->shopTableWidget->setItem(row,1,new QTableWidgetItem(query.value("id").toString()));
        ui->shopTableWidget->setItem(row,2,new QTableWidgetItem(query.value("name").toString()));
        ui->shopTableWidget->setItem(row,3,new QTableWidgetItem(query.value("price").toString()));
        ui->shopTableWidget->setItem(row,4,makeStockItem(query.value("number").toInt()));
        ui->shopTableWidget->setItem(row,5,new QTableWidgetItem(query.value("information").toString()));
        row++;
    }

    if (!hasData)
    {
        QMessageBox::information(this, "提示", "未找到匹配的商品记录");
    }

    //清空编辑框内容
    ui->searchLineEdit->clear();
}
