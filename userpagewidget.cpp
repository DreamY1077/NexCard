#include "userpagewidget.h"
#include "ui_userpagewidget.h"

UserPageWidget::UserPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserPageWidget)
{
    ui->setupUi(this);
}

UserPageWidget::~UserPageWidget()
{
    delete ui;
}
