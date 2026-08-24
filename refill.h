#ifndef REFILL_H
#define REFILL_H

#include <QWidget>

namespace Ui {
class Refill;
}

class Refill : public QWidget
{
    Q_OBJECT

public:
    explicit Refill(QWidget *parent = nullptr);
    ~Refill();

private:
    Ui::Refill *ui;
};

#endif // REFILL_H
