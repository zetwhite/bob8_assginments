#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QString>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    void changeMoney(int number);
    void setEnable();
    int money{0};
    /* money = 0;
     * parameter constructor!! not assign operator
      so, it's better to express in form "money{0}"*/

private slots:
    void on_pb_10_clicked();

    void on_pb_50_clicked();

    void on_pb_100_clicked();

    void on_pb_500_clicked();

    void on_pb_coffee_clicked();

    void on_pb_tea_clicked();

    void on_pb_coke_clicked();

    void on_pb_getChange_clicked();

private:
    Ui::Widget *ui;
    const int coffeeValue = 100,  teaValue = 150, cokeValue = 200;
    const int c500 = 500, c100 = 100, c50 = 50, c10 = 10;
};

#endif // WIDGET_H
