#include "widget.h"
#include "ui_widget.h"
#include "mysql.h"
#include "myserial.h"
#include <QMessageBox>
#include <QPainter>
#include <QDebug>
#include <QTimer>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //设置登录界面窗口大小固定（品牌区+分段切换+卡片布局）
    this->setFixedSize(640,580);

    //让 Logo / 标题 / 副标题在卡片内水平居中（QSS alignment 只控 label 内部文本）
    ui->cardLayout->setAlignment(ui->logoLabel, Qt::AlignHCenter);
    ui->cardLayout->setAlignment(ui->titleLabel, Qt::AlignHCenter);
    ui->cardLayout->setAlignment(ui->subLabel, Qt::AlignHCenter);

    this->setWindowIcon(QIcon(":/res/app.ico"));
    this->setWindowTitle("NexCard · 新卡智联");
    ui->loginStackedWidget->setCurrentIndex(0);

    //实例化数据库
    MySql * db = MySql::getMySql();

    //初始化数据库列表
    db->creatMySql();

    //创建管理员主控页面
    aw = new AdminPageWidget;
    //监听管理员登录页面发回的loginOK信号（携带用户名和权限等级）
    LoginAdminWidget *p_admin = qobject_cast<LoginAdminWidget*>(ui->loginStackedWidget->widget(1));
    connect(p_admin,&LoginAdminWidget::loginOK,[=](const QString &name, int level){
        //把登录的管理员身份和权限传给主控界面
        aw->setLoginAdmin(name, level);
        this->hide();
        aw->show();
    });
    //监听管理员主控界面发回的adminExit信号
    connect(aw,&AdminPageWidget::adminExit,this,&Widget::adminPageExit);

    //创建员工主控页面
    ew = new EmployeePageWidget;
    //监听员工登录页面发回的loginOK信号
    LoginCardWidget *p_employee = qobject_cast<LoginCardWidget*>(ui->loginStackedWidget->widget(0));
    connect(p_employee,&LoginCardWidget::loginOK,[=](){
        //刷卡成功后刷新员工端欢迎页身份/串口状态
        ew->setLoginInfo();
        this->hide();
        ew->show();
    });
    //监听员工主控界面发回的employeeExit信号
    connect(ew,&EmployeePageWidget::employeeExit,this,&Widget::employeeExit);

    //实例化串口类
    MySerial * ms = MySerial::getMyserial();
    QSerialPort * m_port = ms->getMySerialPort();
    //初始化串口配置
    ms->initSerialProt();

    //默认打开串口
    if (!m_port->open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "错误", "打开串口失败：" + m_port->errorString());
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_cardBtn_clicked()
{
    ui->loginStackedWidget->setCurrentIndex(0);
}

void Widget::on_adminBtn_clicked()
{
    ui->loginStackedWidget->setCurrentIndex(1);
}

void Widget::on_registerBtn_clicked()
{
    ui->loginStackedWidget->setCurrentIndex(2);
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    //去除原 loginMag2.jpg 背景图，改为浅蓝渐变背景
    QLinearGradient gradient(0, 0, 0, this->height());
    gradient.setColorAt(0.0, QColor(0xf0, 0xf6, 0xfc));
    gradient.setColorAt(1.0, QColor(0xe3, 0xee, 0xf6));
    p.fillRect(this->rect(), gradient);
}

void Widget::adminPageExit()
{
    //管理员主控界面退出 登录主界面显出
    aw->hide();
    this->show();
}

void Widget::employeeExit()
{
    ew->hide();
    this->show();
}


