#ifndef LOGINCARDWIDGET_H
#define LOGINCARDWIDGET_H

#include <QWidget>
#include "employeepunchwidget.h"

namespace Ui {
class LoginCardWidget;
}

class LoginCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginCardWidget(QWidget *parent = nullptr);
    ~LoginCardWidget();
signals:
    void getCard();
    void loginOK();
private slots:
    void onGetCard(const QString &cardId);

    void on_loginBtn_clicked();

    void on_punchBtn_clicked();   // 打开人脸考勤打卡窗口

private:
    Ui::LoginCardWidget *ui;
    EmployeePunchWidget *m_punchWidget = nullptr;
};

#endif // LOGINCARDWIDGET_H
