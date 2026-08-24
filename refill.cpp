#include "refill.h"
#include "ui_refill.h"

Refill::Refill(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Refill)
{
    ui->setupUi(this);
}

Refill::~Refill()
{
    delete ui;
}
