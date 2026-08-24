#ifndef USERPAGEWIDGET_H
#define USERPAGEWIDGET_H

#include <QWidget>

namespace Ui {
class UserPageWidget;
}

class UserPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserPageWidget(QWidget *parent = nullptr);
    ~UserPageWidget();

private:
    Ui::UserPageWidget *ui;
};

#endif // USERPAGEWIDGET_H
