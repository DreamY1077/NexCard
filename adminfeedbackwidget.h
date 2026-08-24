#ifndef ADMINFEEDBACKWIDGET_H
#define ADMINFEEDBACKWIDGET_H

#include <QWidget>

namespace Ui {
class AdminFeedbackWidget;
}

//异常反馈箱：汇总迟到/早退考勤记录 + 库存为 0 的商品
class AdminFeedbackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminFeedbackWidget(QWidget *parent = nullptr);
    ~AdminFeedbackWidget();

private slots:
    void on_refreshBtn_clicked();

private:
    Ui::AdminFeedbackWidget *ui;
};

#endif // ADMINFEEDBACKWIDGET_H
