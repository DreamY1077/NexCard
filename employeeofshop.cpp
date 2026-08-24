#include "employeeofshop.h"
#include "ui_employeeofshop.h"
#include "mysql.h"
#include <QMessageBox>
#include <QColor>
#include "myserial.h"


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


EmployeeOfShop::EmployeeOfShop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeOfShop)
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

EmployeeOfShop::~EmployeeOfShop()
{
    delete ui;
}

void EmployeeOfShop::on_refreshBtn_clicked()
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

void EmployeeOfShop::on_searchBtn_clicked()
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

void EmployeeOfShop::on_buyBtn_clicked()
{
    QStringList idList;
    int totalRow = ui->shopTableWidget->rowCount();

    // 遍历表格收集勾选商品id
    for(int r = 0; r < totalRow; r++)
    {
        QTableWidgetItem *item = ui->shopTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            QString goodsId = item->data(Qt::UserRole).toString();
            idList << goodsId;
        }
    }

    if(idList.isEmpty())
    {
        QMessageBox::information(this,"提示","请先勾选要购买的商品！");
        return;
    }

    // 确认弹窗
    int ret = QMessageBox::question(this,"确认购买",
        QString("你选中了%1个商品，确定继续？").arg(idList.size()),
        QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.exec("BEGIN TRANSACTION"); //开启事务，保证数据一致性

    // 查询选中商品：单价、库存
    QString inSql = idList.join(",");
    query.prepare(QString("SELECT price, number FROM product WHERE id IN (%1)").arg(inSql));
    if(!query.exec())
    {
        QMessageBox::critical(this,"错误","查询商品失败：" + query.lastError().text());
        query.exec("ROLLBACK");
        return;
    }

    double totalCost = 0.0;
    bool stockNotEnough = false;
    while(query.next())
    {
        int stock = query.value("number").toInt();
        double price = query.value("price").toDouble();
        if(stock <= 0)
        {
            stockNotEnough = true;
            break;
        }
        totalCost += price;
    }
    if(stockNotEnough)
    {
        QMessageBox::warning(this,"提示","部分商品库存不足，无法购买！");
        query.exec("ROLLBACK");
        return;
    }

    // 获取登录员工卡号
    MySerial *ms = MySerial::getMyserial();
    QString loginCard = ms->getCardId();

    // 查询员工余额
    QSqlQuery balQuery;
    balQuery.prepare("SELECT balance FROM employee WHERE card = :card");
    balQuery.bindValue(":card", loginCard);
    if(!balQuery.exec() || !balQuery.next())
    {
        QMessageBox::critical(this,"错误","读取员工余额失败！");
        query.exec("ROLLBACK");
        return;
    }

    double userBalance = balQuery.value("balance").toDouble();
    // 判断余额
    if(userBalance < totalCost)
    {
        QMessageBox::warning(this,"余额不足",
            QString("当前余额：%1 元\n所需金额：%2 元\n请先进行充值！").arg(userBalance).arg(totalCost));
        query.exec("ROLLBACK");
        return;
    }

    // 扣除余额
    QSqlQuery updateBalance;
    updateBalance.prepare("UPDATE employee SET balance = balance - :cost WHERE card = :card");
    updateBalance.bindValue(":cost", totalCost);
    updateBalance.bindValue(":card", loginCard);
    if(!updateBalance.exec())
    {
        QMessageBox::critical(this,"错误","余额扣除失败");
        query.exec("ROLLBACK");
        return;
    }

    // 每个选中商品库存-1
    for(QString gid : idList)
    {
        QSqlQuery updateStock;
        updateStock.prepare("UPDATE product SET number = number - 1 WHERE id = :pid");
        updateStock.bindValue(":pid", gid);
        if(!updateStock.exec())
        {
            QMessageBox::critical(this,"错误","更新库存失败");
            query.exec("ROLLBACK");
            return;
        }
    }

    // 全部操作成功，提交事务
    query.exec("COMMIT");
    QMessageBox::information(this,"购买成功",QString("本次消费合计：%1 元").arg(totalCost));

    // 刷新商品表格
    on_refreshBtn_clicked();
}
