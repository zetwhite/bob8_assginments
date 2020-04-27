#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setEnable();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::changeMoney(int number){
    money += number;
    ui->lcdNumber->display(money);
}

void Widget::setEnable(){
    bool cokeEnable, teaEnable, coffeeEnable;
    cokeEnable = (money >= cokeValue);
    teaEnable = (money >= teaValue);
    coffeeEnable = (money >= coffeeValue);
    ui->pb_coke->setEnabled(cokeEnable);
    ui->pb_tea->setEnabled(teaEnable);
    ui->pb_coffee->setEnabled(coffeeEnable);
}

void Widget::on_pb_10_clicked()
{
    changeMoney(c10);
    setEnable();
}

void Widget::on_pb_50_clicked()
{
    changeMoney(c50);
    setEnable();
}

void Widget::on_pb_100_clicked()
{
    changeMoney(c100);
    setEnable();
}

void Widget::on_pb_500_clicked()
{
    changeMoney(c500);
    setEnable();
}

void Widget::on_pb_coffee_clicked()
{
   changeMoney(-1*coffeeValue);
   setEnable();
}

void Widget::on_pb_tea_clicked()
{
    changeMoney(-1*teaValue);
    setEnable();
}

void Widget::on_pb_coke_clicked()
{
    changeMoney(-1*cokeValue);
    setEnable();
}

void Widget::on_pb_getChange_clicked()
{   QMessageBox msg;
    QString changeInfo500 = "$500 : " + QString::number(money/c500) + "\n";
    money = money%c500;
    QString changeInfo100 = "$100 : " + QString::number(money/c100) + "\n";
    money = money%c100;
    QString changeInfo50 = "$50 : " + QString::number(money/c50) + "\n";
    money = money%c50;
    QString changeInfo10 = "$10 : " + QString::number(money/c10) + "\n";
	money = money%c10; 
    msg.information(nullptr, "your change", changeInfo500 + changeInfo100 + changeInfo50 + changeInfo10);
    setEnable();
}
