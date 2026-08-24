#include <QTimer>
#include "adminofadmin.h"
#include "ui_adminofadmin.h"
#include "mysql.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QColor>

//把权限等级数字转为中文显示（1一级 2二级 3三级）
QString levelToText(int level)
{
    if(level == 1)
    {
        return QString("一级管理员");
    }
    else if(level == 2)
    {
        return QString("二级管理员");
    }
    return QString("三级管理员");
}

//权限等级着色：一级红加粗 / 二级蓝 / 三级灰
static QTableWidgetItem *makeLevelItem(int level)
{
    QTableWidgetItem *item = new QTableWidgetItem(levelToText(level));
    if(level == 1)
    {
        item->setForeground(QColor(0xd9, 0x53, 0x4f));
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
    }
    else if(level == 2)
    {
        item->setForeground(QColor(0x34, 0x76, 0xc2));
    }
    else
    {
        item->setForeground(QColor(0x99, 0x99, 0x99));
    }
    return item;
}

AdminOfAdmin::AdminOfAdmin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminOfAdmin)
{
    ui->setupUi(this);

    QStringList headers{"选择","用户名","权限等级"};
    ui->adminTableWidget->setColumnCount(headers.size());
    ui->adminTableWidget->setHorizontalHeaderLabels(headers);
    //所有列平均铺满
    ui->adminTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //斑马纹
    ui->adminTableWidget->setAlternatingRowColors(true);

    m_loginName.clear();
    m_loginLevel = 0;
    //默认隐藏"修改权限"按钮，登录后由 setLoginUser 按权限显示
    ui->setLevelBtn->setVisible(false);

    //打开页面自动刷新
    on_refreshBtn_clicked();
}

AdminOfAdmin::~AdminOfAdmin()
{
    delete ui;
}

void AdminOfAdmin::setLoginUser(const QString &name, int level)
{
    m_loginName = name;
    m_loginLevel = level;
    //仅一级管理员（root）拥有修改/删除其他管理员权限的权限
    ui->setLevelBtn->setVisible(level == 1);
    ui->deleteBtn->setVisible(level == 1);
}

void AdminOfAdmin::on_refreshBtn_clicked()
{
    QSqlQuery query;
    //只查用户名和权限，不查密码
    query.prepare("SELECT name, level FROM admin ORDER BY level ASC, name ASC");

    if(!query.exec())
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this,"错误","加载管理员数据失败：" + err.text());
        return;
    }

    ui->adminTableWidget->setRowCount(0);
    int row = 0;
    while(query.next())
    {
        ui->adminTableWidget->insertRow(row);

        //第一列：复选框，用户名存到 UserRole 供修改/删除时读取
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setData(Qt::UserRole, query.value("name"));
        ui->adminTableWidget->setItem(row, 0, checkItem);

        ui->adminTableWidget->setItem(row,1,new QTableWidgetItem(query.value("name").toString()));
        ui->adminTableWidget->setItem(row,2,makeLevelItem(query.value("level").toInt()));

        row++;
    }
    if(row == 0)
    {
        QMessageBox::information(this,"提示","暂无管理员记录");
    }
}

void AdminOfAdmin::on_searchBtn_clicked()
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
        SELECT name, level
        FROM admin
        WHERE name LIKE :key
        ORDER BY level ASC, name ASC
    )");
    query.bindValue(":key", "%" + keyword + "%");

    if (!query.exec())
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(this, "错误", "查询失败：" + err.text());
        return;
    }

    ui->adminTableWidget->setRowCount(0);
    int row = 0;
    bool hasData = false;

    while (query.next())
    {
        hasData = true;
        ui->adminTableWidget->insertRow(row);

        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setData(Qt::UserRole, query.value("name"));
        ui->adminTableWidget->setItem(row, 0, checkItem);

        ui->adminTableWidget->setItem(row,1,new QTableWidgetItem(query.value("name").toString()));
        ui->adminTableWidget->setItem(row,2,makeLevelItem(query.value("level").toInt()));
        row++;
    }

    if (!hasData)
    {
        QMessageBox::information(this, "提示", "未找到匹配的管理员");
    }

    //清空编辑框内容
    ui->searchLineEdit->clear();
}

void AdminOfAdmin::on_setLevelBtn_clicked()
{
    //仅一级管理员可用（按钮已按权限显隐，此处双保险）
    if(m_loginLevel != 1)
    {
        QMessageBox::information(this,"提示","无修改管理员权限的权限！");
        return;
    }

    //收集勾选的管理员用户名（参考考勤管理：修改只能选一条）
    QStringList checkedList;
    int total = ui->adminTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->adminTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            checkedList << item->data(Qt::UserRole).toString();
        }
    }

    if(checkedList.isEmpty())
    {
        QMessageBox::information(this,"提示","请勾选一位要修改权限的管理员");
        return;
    }
    if(checkedList.size() > 1)
    {
        QMessageBox::warning(this,"提示","只能勾选一条记录修改！");
        return;
    }

    QString selectName = checkedList.first();

    //root 唯一一级管理员，不允许修改其权限
    if(selectName == "root")
    {
        QMessageBox::warning(this,"提示","root 为唯一一级管理员，权限不可修改！");
        return;
    }

    //选择目标权限等级（只能设为二级或三级，保证一级管理员唯一）
    QStringList levelList{"二级管理员","三级管理员"};
    //默认选中该管理员当前等级，方便查看原权限
    int curLevel = MySql::getMySql()->findAdminLevel(selectName);
    int defaultIndex = (curLevel == 3) ? 1 : 0;
    bool ok = false;
    QString choice = QInputDialog::getItem(this,"修改权限",
        QString("管理员 [%1] 当前为%2\n将权限修改为：").arg(selectName)
            .arg(curLevel == 3 ? "三级管理员" : "二级管理员"),
        levelList, defaultIndex, false, &ok);
    if(!ok || choice.isEmpty())
        return;

    int newLevel = (choice == "二级管理员") ? 2 : 3;

    MySql *db = MySql::getMySql();
    if(!db->updateAdminLevel(selectName, newLevel))
    {
        QMessageBox::critical(this,"错误","修改权限失败！");
        return;
    }

    QMessageBox::information(this,"成功",
        QString("已将 [%1] 修改为%2").arg(selectName).arg(choice));
    //刷新列表
    on_refreshBtn_clicked();
}

void AdminOfAdmin::on_deleteBtn_clicked()
{
    //仅一级管理员可用（按钮已按权限显隐，此处双保险）
    if(m_loginLevel != 1)
    {
        QMessageBox::information(this,"提示","无删除管理员的权限！");
        return;
    }

    //收集勾选的管理员用户名
    QStringList checkedList;
    int total = ui->adminTableWidget->rowCount();
    for(int r = 0; r < total; r++)
    {
        QTableWidgetItem *item = ui->adminTableWidget->item(r, 0);
        if(item && item->checkState() == Qt::Checked)
        {
            checkedList << item->data(Qt::UserRole).toString();
        }
    }

    if(checkedList.isEmpty())
    {
        QMessageBox::information(this,"提示","请先勾选要删除的管理员！");
        return;
    }

    //root 不允许删除
    if(checkedList.contains("root"))
    {
        QMessageBox::warning(this,"提示","root 为唯一一级管理员，不允许删除！");
        return;
    }
    //不允许删除当前登录账号
    if(checkedList.contains(m_loginName))
    {
        QMessageBox::warning(this,"提示","不能删除当前登录的账号！");
        return;
    }

    // 弹窗确认，防止误删
    int ret = QMessageBox::question(this,"确认删除",
        QString("你选中了%1个管理员，删除后无法恢复，确定继续？").arg(checkedList.size()),
        QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes)
        return;

    // 逐条参数化删除
    QSqlQuery query;
    bool allOk = true;
    for(const QString &name : checkedList)
    {
        query.prepare("DELETE FROM admin WHERE name=:name");
        query.bindValue(":name", name);
        if(!query.exec())
        {
            allOk = false;
            QSqlError err = query.lastError();
            QMessageBox::critical(this,"错误",QString("删除 [%1] 失败：").arg(name) + err.text());
            break;
        }
    }

    if(allOk)
    {
        QMessageBox::information(this,"成功",QString("已删除%1个管理员！").arg(checkedList.size()));
    }
    // 删除完成刷新表格
    on_refreshBtn_clicked();
}
