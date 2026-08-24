#include "logincardwidget.h"
#include "ui_logincardwidget.h"
#include "myserial.h"
#include "mysql.h"
#include <QMessageBox>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QIcon>

//把内联 SVG 字符串渲染到指定 QLabel
static void setSvgIcon(QLabel *label, const QString &svg)
{
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pix(22, 22);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    renderer.render(&painter);
    painter.end();
    label->setPixmap(pix);
}

LoginCardWidget::LoginCardWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginCardWidget)
{
    ui->setupUi(this);
    //清空卡号编辑框
    ui->cardLineEdit->clear();
    //设置卡号编辑框只读 只能刷卡
    ui->cardLineEdit->setReadOnly(true);
    //连接串口卡号接收信号
    connect(MySerial::getMyserial(),&MySerial::cardIdReceived,this,&LoginCardWidget::onGetCard);

    //卡片图标
    setSvgIcon(ui->cardIconLabel,
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='#4287d8'>"
        "<path d='M2 6v13a1 1 0 0 0 1 1h18a1 1 0 0 0 1-1V6a1 1 0 0 0-1-1H3a1 1 0 0 0-1 1zm20 5H2v-2h20v2z'/></svg>");
}

LoginCardWidget::~LoginCardWidget()
{
    delete ui;
}

void LoginCardWidget::onGetCard(const QString &cardId)
{
    ui->cardLineEdit->setText(cardId);
}

void LoginCardWidget::on_loginBtn_clicked()
{
    //获取编辑框卡号
    QString card = ui->cardLineEdit->text();

    //获取数据库地址
    MySql * db = MySql::getMySql();

    if(db->checkCard(card))
    {
        ui->cardLineEdit->clear();
        emit loginOK();
    }
    else {
        QMessageBox::information(this,"提示","该卡未注册！");
    }
}

void LoginCardWidget::on_punchBtn_clicked()
{
    //打开独立的人脸考勤打卡窗口（无需登录即可打卡）
    m_punchWidget = new EmployeePunchWidget();
    m_punchWidget->setAttribute(Qt::WA_DeleteOnClose);
    m_punchWidget->setWindowTitle("考勤打卡 · 人脸识别");
    m_punchWidget->setWindowIcon(QIcon(":/res/app.ico"));
    m_punchWidget->show();
}
