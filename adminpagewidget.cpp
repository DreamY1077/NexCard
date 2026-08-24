#include "adminpagewidget.h"
#include "ui_adminpagewidget.h"
#include "myserial.h"
#include <QPainter>
#include <QMessageBox>
#include <QStyle>
#include <QTimer>
#include <QDateTime>
#include <QTreeWidgetItemIterator>

//为树形菜单添加标准图标，提升视觉层级
static void setupMenuIcons(QTreeWidget *tree)
{
    QStyle *st = tree->style();
    for(int i = 0; i < tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        const QString t = item->text(0);
        if(t == "员工信息管理")
            item->setIcon(0, st->standardIcon(QStyle::SP_DirHomeIcon));
        else if(t == "考勤管理")
            item->setIcon(0, st->standardIcon(QStyle::SP_FileDialogDetailedView));
        else if(t == "商品管理")
            item->setIcon(0, st->standardIcon(QStyle::SP_DriveHDIcon));
        else if(t == "异常反馈箱")
            item->setIcon(0, st->standardIcon(QStyle::SP_MessageBoxWarning));
        else if(t == "串口配置")
            item->setIcon(0, st->standardIcon(QStyle::SP_ComputerIcon));
        else if(t == "管理员信息管理")
        {
            //父项根据展开/收起状态显示文件夹打开/关闭图标
            item->setIcon(0, item->isExpanded()
                ? st->standardIcon(QStyle::SP_DirOpenIcon)
                : st->standardIcon(QStyle::SP_DirIcon));
        }

        for(int j = 0; j < item->childCount(); j++)
        {
            QTreeWidgetItem *child = item->child(j);
            const QString ct = child->text(0);
            if(ct == "管理员列表")
                child->setIcon(0, st->standardIcon(QStyle::SP_FileDialogListView));
            else if(ct == "管理员注册")
                child->setIcon(0, st->standardIcon(QStyle::SP_FileDialogNewFolder));
        }
    }
}

//手动递归遍历树，查找指定文本的所有 item（比 findItems+MatchRecursive 更安全，Qt 5.12 下后者行为不可靠）
static QList<QTreeWidgetItem*> findItemsRecursive(QTreeWidget *tree, const QString &text)
{
    QList<QTreeWidgetItem*> result;
    if(!tree)
    {
        return result;
    }
    QTreeWidgetItemIterator it(tree);
    while (*it)
    {
        QTreeWidgetItem *item = *it;
        if(item && item->text(0) == text)
        {
            result << item;
        }
        ++it;
    }
    return result;
}

//监听父项展开/收起，切换文件夹图标（展开=打开态，收起=关闭态）
static void setupParentToggleIcon(QTreeWidget *tree, const QString &parentText)
{
    QStyle *st = tree->style();
    auto updateIcon = [st, tree, parentText](){
        QList<QTreeWidgetItem*> items = findItemsRecursive(tree, parentText);
        for(QTreeWidgetItem *it : items)
        {
            it->setIcon(0, st->standardIcon(it->isExpanded()
                ? QStyle::SP_DirOpenIcon
                : QStyle::SP_DirIcon));
        }
    };
    QObject::connect(tree, &QTreeWidget::itemExpanded,   tree, updateIcon);
    QObject::connect(tree, &QTreeWidget::itemCollapsed,  tree, updateIcon);
}
AdminPageWidget::AdminPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminPageWidget)
{
    ui->setupUi(this);

    this->setWindowTitle("NexCard · 新卡智联");
    this->setWindowIcon(QIcon(":/res/app.ico"));

    m_adminRegPage = nullptr;//将页面指针初始为空
    m_serialpage = nullptr;
    m_adminOfEmployee = nullptr;
    m_adminOfAttendance = nullptr;
    m_adminOfShop = nullptr;
    m_adminOfAdmin = nullptr;
    m_adminFeedback = nullptr;

    m_loginAdmin.clear();
    m_loginLevel = 0;

    //菜单图标
    setupMenuIcons(ui->ngtTreeWidget);
    //父项（管理员信息管理）展开/收起时切换文件夹图标
    setupParentToggleIcon(ui->ngtTreeWidget, "管理员信息管理");

    //锁定左侧菜单 DockWidget：禁止拖动/脱离/关闭
    ui->ngtDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);

    //欢迎页信息卡片：串口状态
    QSerialPort *port = MySerial::getMyserial()->getMySerialPort();
    if(port->isOpen())
    {
        ui->serialValueLabel->setText(port->portName() + " 已连接");
    }
    else {
        ui->serialValueLabel->setText("未连接");
    }

    //欢迎页信息卡片：系统时间（每秒刷新）+ 串口状态实时同步
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        ui->timeValueLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd\nHH:mm:ss"));
        //串口状态实时刷新（打开/关闭后欢迎页立即反映）
        QSerialPort *port = MySerial::getMyserial()->getMySerialPort();
        if(port->isOpen())
        {
            ui->serialValueLabel->setText(port->portName() + " 已连接");
        }
        else {
            ui->serialValueLabel->setText("未连接");
        }
    });
    timer->start(1000);
    ui->timeValueLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd\nHH:mm:ss"));

    //欢迎页 tab 固定不可移除（隐藏首个 tab 的关闭按钮）
    ui->pageTabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
}

AdminPageWidget::~AdminPageWidget()
{
    delete ui;
}

void AdminPageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    //去除原 controlMag.jpg 背景图，改为纯浅色背景
    p.fillRect(this->rect(), QColor(0xf5, 0xf8, 0xfc));
}


void AdminPageWidget::setLoginAdmin(const QString &name, int level)
{
    m_loginAdmin = name;
    m_loginLevel = level;

    //顶部欢迎语显示登录身份与权限等级
    QString levelText;
    if(level == 1)      levelText = "一级管理员";
    else if(level == 2) levelText = "二级管理员";
    else if(level == 3) levelText = "三级管理员";
    else                levelText = "未知权限";
    ui->welLabel->setText(QString("您好，管理员 %1 · %2").arg(name).arg(levelText));

    //欢迎页信息卡片：当前身份
    ui->identityValueLabel->setText(QString("%1\n%2").arg(name).arg(levelText));

    //三级管理员仅能查看管理员列表，隐藏"管理员注册"菜单
    //用手动递归遍历查找子项（避免 findItems+MatchRecursive 在 Qt 5.12 下的崩溃问题）
    QList<QTreeWidgetItem*> items = findItemsRecursive(ui->ngtTreeWidget, "管理员注册");
    for(QTreeWidgetItem *it : items)
    {
        it->setHidden(level == 3);
    }
}

void AdminPageWidget::on_ngtTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString menuText = item->text(0);

    //父菜单不切换页面，只展开折叠
    if(menuText == "管理员信息管理")
        return;

    //管理员列表页逻辑
    if(menuText == "管理员列表")
    {
        if(m_adminOfAdmin == nullptr)
        {
            //首次点击：创建页面并添加到标签栏，并把当前登录身份传入（控制修改/删除按钮显隐）
            m_adminOfAdmin = new AdminOfAdmin(this);
            m_adminOfAdmin->setLoginUser(m_loginAdmin, m_loginLevel);
            int newIndex = ui->pageTabWidget->addTab(m_adminOfAdmin,"管理员列表");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminOfAdmin);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //管理员注册页逻辑（三级管理员无注册权限，双保险拦截）
    if(menuText == "管理员注册")
    {
        if(m_loginLevel == 3)
        {
            QMessageBox::information(this,"提示","三级管理员无注册管理员权限！");
            return;
        }
        if(m_adminRegPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_adminRegPage = new AdminRegisterWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_adminRegPage,"管理员注册");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminRegPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //串口配置页
    if(menuText == "串口配置")
    {
        if(m_serialpage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_serialpage = new SerialPortWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_serialpage,"串口配置");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_serialpage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //员工信息管理页
    if(menuText == "员工信息管理")
    {
        if(m_adminOfEmployee == nullptr)
        {
            //首次点击：创建页面并添加到标签栏，并把当前登录权限传入（三级无"人脸录入"权限）
            m_adminOfEmployee = new AdminOfEmployee(this);
            m_adminOfEmployee->setLoginLevel(m_loginLevel);
            int newIndex = ui->pageTabWidget->addTab(m_adminOfEmployee,"员工信息管理");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminOfEmployee);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //考勤管理页面
    if(menuText == "考勤管理")
    {
        if(m_adminOfAttendance == nullptr)
        {
            //首次点击：创建页面并添加到标签栏，并把当前登录权限传入（控制增删改/规则按钮显隐）
            m_adminOfAttendance = new AdminOfAttendance(this);
            m_adminOfAttendance->setLoginLevel(m_loginLevel);
            int newIndex = ui->pageTabWidget->addTab(m_adminOfAttendance,"考勤管理");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminOfAttendance);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //商品管理页面
    if(menuText == "商品管理")
    {
        if(m_adminOfShop == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_adminOfShop = new AdminOfShop(this);
            int newIndex = ui->pageTabWidget->addTab(m_adminOfShop,"商品管理");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminOfShop);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //异常反馈箱页面
    if(menuText == "异常反馈箱")
    {
        if(m_adminFeedback == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_adminFeedback = new AdminFeedbackWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_adminFeedback,"异常反馈箱");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_adminFeedback);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }
}

void AdminPageWidget::on_pageTabWidget_tabCloseRequested(int index)
{
    //欢迎页固定不可关闭（防止删除欢迎页导致 QTimer 访问已释放控件崩溃）
    if(index == 0)
    {
        return;
    }
    //获取当前要关闭的页面对象
    QWidget *closePage = ui->pageTabWidget->widget(index);

    //如果指针不为空，则把指针置空，下次点击可重新创建
    if(closePage == m_adminRegPage)
    {
        m_adminRegPage = nullptr;
    }
    else if(closePage == m_serialpage)
    {
        m_serialpage = nullptr;
    }
    else if(closePage == m_adminOfEmployee)
    {
        m_adminOfEmployee = nullptr;
    }
    else if(closePage == m_adminOfAttendance)
    {
        m_adminOfAttendance = nullptr;
    }
    else if(closePage == m_adminOfShop)
    {
        m_adminOfShop = nullptr;
    }
    else if(closePage == m_adminOfAdmin)
    {
        m_adminOfAdmin = nullptr;
    }
    else if(closePage == m_adminFeedback)
    {
        m_adminFeedback = nullptr;
    }

    //从标签栏移除该页面
    ui->pageTabWidget->removeTab(index);
    //释放页面内存
    closePage->deleteLater();
}

void AdminPageWidget::on_exitBtn_clicked()
{
    // 从后往前遍历Tab（跳过固定的欢迎页 index 0——欢迎页含 timeValueLabel，
    // 若被删除，欢迎页 QTimer 每秒访问已释放控件会 use-after-free 崩溃）
    int totalTab = ui->pageTabWidget->count();
    for(int index = totalTab - 1; index >= 1; index--)
    {
        QWidget *closePage = ui->pageTabWidget->widget(index);

        if(closePage == m_adminRegPage)
        {
            m_adminRegPage = nullptr;
        }
        else if(closePage == m_serialpage)
        {
            m_serialpage = nullptr;
        }
        else if(closePage == m_adminOfEmployee)
        {
            m_adminOfEmployee = nullptr;
        }
        else if(closePage == m_adminOfAttendance)
        {
            m_adminOfAttendance = nullptr;
        }
        else if(closePage == m_adminOfShop)
        {
            m_adminOfShop = nullptr;
        }
        else if(closePage == m_adminOfAdmin)
        {
            m_adminOfAdmin = nullptr;
        }
        else if(closePage == m_adminFeedback)
        {
            m_adminFeedback = nullptr;
        }

        // 移除标签、释放页面
        ui->pageTabWidget->removeTab(index);
        closePage->deleteLater();
    }

    emit adminExit();
}
