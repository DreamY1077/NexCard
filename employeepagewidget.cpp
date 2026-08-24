#include "employeepagewidget.h"
#include "ui_employeepagewidget.h"
#include "mysql.h"
#include "myserial.h"
#include <QtDebug>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <QDateTime>

//为树形菜单添加标准图标，提升视觉层级
static void setupMenuIcons(QTreeWidget *tree)
{
    QStyle *st = tree->style();
    for(int i = 0; i < tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        const QString t = item->text(0);
        if(t == "个人信息")
            item->setIcon(0, st->standardIcon(QStyle::SP_DirHomeIcon));
        else if(t == "考勤管理")
            item->setIcon(0, st->standardIcon(QStyle::SP_FileDialogDetailedView));
        else if(t == "商品库")
            item->setIcon(0, st->standardIcon(QStyle::SP_DriveHDIcon));
        else if(t == "余额充值")
            item->setIcon(0, st->standardIcon(QStyle::SP_DriveFDIcon));
        else if(t == "问题反馈")
            item->setIcon(0, st->standardIcon(QStyle::SP_MessageBoxQuestion));
        else if(t == "考勤打卡")
            item->setIcon(0, st->standardIcon(QStyle::SP_MediaPlay));
    }
}

EmployeePageWidget::EmployeePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeePageWidget)
{
    ui->setupUi(this);

    this->setWindowTitle("NexCard · 新卡智联");
    this->setWindowIcon(QIcon(":/res/app.ico"));

    m_dataPage = nullptr;
    m_attendancePage = nullptr;
    m_shopPage = nullptr;
    m_refillPage = nullptr;
    m_feedbackPage = nullptr;
    m_punchPage = nullptr;

    //菜单图标
    setupMenuIcons(ui->ngtTreeWidget);

    //锁定左侧菜单 DockWidget：禁止拖动/脱离/关闭
    ui->ngtDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);

    //欢迎页默认显示（登录前），登录后由 setLoginInfo() 更新
    ui->welLabel->setText("您好，员工");
    ui->identityValueLabel->setText("未识别");

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
        //串口状态实时刷新
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

void EmployeePageWidget::setLoginInfo()
{
    //刷卡登录成功后调用：根据当前 MySerial 记录的卡号查询员工姓名，更新显示
    QString card = MySerial::getMyserial()->getCardId();
    QString name = MySql::getMySql()->findEmployeeName(card);
    if(name.isEmpty())
    {
        ui->welLabel->setText("您好，员工");
        ui->identityValueLabel->setText("未识别");
    }
    else {
        ui->welLabel->setText(QString("您好，%1（%2）").arg(name).arg(card));
        ui->identityValueLabel->setText(QString("%1\n%2").arg(name).arg(card));
    }

    //刷新串口状态（刷卡时可能串口已打开）
    QSerialPort *port = MySerial::getMyserial()->getMySerialPort();
    if(port->isOpen())
    {
        ui->serialValueLabel->setText(port->portName() + " 已连接");
    }
    else {
        ui->serialValueLabel->setText("未连接");
    }
}

EmployeePageWidget::~EmployeePageWidget()
{
    delete ui;
}

void EmployeePageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    //去除原 controlMag.jpg 背景图，改为纯浅色背景
    p.fillRect(this->rect(), QColor(0xf5, 0xf8, 0xfc));
}

void EmployeePageWidget::on_ngtTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString menuText = item->text(0);

    //个人信息页面添加
    if(menuText == "个人信息")
    {
        if(m_dataPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_dataPage = new EmployeeDataWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_dataPage,"个人信息");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_dataPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //考勤信息页面添加
    if(menuText == "考勤管理")
    {
        if(m_attendancePage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_attendancePage = new EmployeeAttendanceWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_attendancePage,"考勤管理");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_attendancePage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //商品库页面
    if(menuText == "商品库")
    {
        if(m_shopPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_shopPage = new EmployeeOfShop(this);
            int newIndex = ui->pageTabWidget->addTab(m_shopPage,"商品库");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_shopPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //余额充值页面
    if(menuText == "余额充值")
    {
        if(m_refillPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_refillPage = new EmployeeRefill(this);
            int newIndex = ui->pageTabWidget->addTab(m_refillPage,"余额充值");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_refillPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //问题反馈页面
    if(menuText == "问题反馈")
    {
        if(m_feedbackPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_feedbackPage = new EmployeeFeedbackWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_feedbackPage,"问题反馈");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_feedbackPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }

    //考勤打卡页面（占位，人脸识别打卡后续实现）
    if(menuText == "考勤打卡")
    {
        if(m_punchPage == nullptr)
        {
            //首次点击：创建页面并添加到标签栏
            m_punchPage = new EmployeePunchWidget(this);
            int newIndex = ui->pageTabWidget->addTab(m_punchPage,"考勤打卡");
            ui->pageTabWidget->setCurrentIndex(newIndex);//自动跳转到新页面
        }
        else {
            //已打开过的页面：直接切换到该标签页
            int existIndex = ui->pageTabWidget->indexOf(m_punchPage);
            ui->pageTabWidget->setCurrentIndex(existIndex);
        }
        return;
    }
}

void EmployeePageWidget::on_pageTabWidget_tabCloseRequested(int index)
{
    //欢迎页固定不可关闭（防止删除欢迎页导致 QTimer 访问已释放控件崩溃）
    if(index == 0)
    {
        return;
    }
    //获取当前要关闭的页面对象
    QWidget *closePage = ui->pageTabWidget->widget(index);

    //如果指针不为空，则把指针置空，下次点击可重新创建
    if(closePage == m_dataPage)
    {
        m_dataPage = nullptr;
    }
    else if(closePage == m_attendancePage)
    {
        m_attendancePage = nullptr;
    }
    else if(closePage == m_shopPage)
    {
        m_shopPage = nullptr;
    }
    else if(closePage == m_refillPage)
    {
        m_refillPage = nullptr;
    }
    else if(closePage == m_feedbackPage)
    {
        m_feedbackPage = nullptr;
    }
    else if(closePage == m_punchPage)
    {
        m_punchPage = nullptr;
    }

    //从标签栏移除该页面
    ui->pageTabWidget->removeTab(index);
    //释放页面内存
    closePage->deleteLater();
}

void EmployeePageWidget::on_exitBtn_clicked()
{
    // 从后往前遍历Tab（跳过固定的欢迎页 index 0——欢迎页含 timeValueLabel，
    // 若被删除，欢迎页 QTimer 每秒访问已释放控件会 use-after-free 崩溃）
    int totalTab = ui->pageTabWidget->count();
    for(int index = totalTab - 1; index >= 1; index--)
    {
        QWidget *closePage = ui->pageTabWidget->widget(index);

        if(closePage == m_dataPage)
        {
            m_dataPage = nullptr;
        }
        else if(closePage == m_attendancePage)
        {
            m_attendancePage = nullptr;
        }
        else if(closePage == m_shopPage)
        {
            m_shopPage = nullptr;
        }
        else if(closePage == m_refillPage)
        {
            m_refillPage = nullptr;
        }
        else if(closePage == m_feedbackPage)
        {
            m_feedbackPage = nullptr;
        }
        else if(closePage == m_punchPage)
        {
            m_punchPage = nullptr;
        }

        // 移除标签、释放页面
        ui->pageTabWidget->removeTab(index);
        closePage->deleteLater();
    }

    emit employeeExit();
}
