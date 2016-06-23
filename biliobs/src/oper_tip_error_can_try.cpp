#include "oper_tip_error_can_try.h"
#include "ui_oper_tip_error_can_try.h"
#include <QGraphicsDropShadowEffect>

OperTipErrorCanTry::OperTipErrorCanTry(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperTipErrorCanTry)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    connect(ui->OperTipCloseBtn, &QPushButton::clicked, this, &OperTipErrorCanTry::onCancelBtnClicked);
    connect(ui->OperTipCancelBtn, &QPushButton::clicked, this, &OperTipErrorCanTry::onCancelBtnClicked);
    connect(ui->OperTipRePushBtn, &QPushButton::clicked, this, &OperTipErrorCanTry::rePush);

    QGraphicsDropShadowEffect *shadowEfct = new QGraphicsDropShadowEffect(this);
    shadowEfct->setBlurRadius(20.0);
    shadowEfct->setColor(QColor(0, 0, 0, 100));
    shadowEfct->setOffset(0, 0);
    setGraphicsEffect(shadowEfct);
}

OperTipErrorCanTry::~OperTipErrorCanTry()
{
    delete ui;
}

void OperTipErrorCanTry::rePush()
{
    accept();
}
void OperTipErrorCanTry::onCancelBtnClicked()
{
    reject();
}

void OperTipErrorCanTry::mousePressEvent(QMouseEvent* e)
{
    moveHelper.mousePressEvent(this, frameGeometry(), e);
}

void OperTipErrorCanTry::mouseReleaseEvent(QMouseEvent* e)
{
    moveHelper.mouseReleaseEvent(this, e);
}

void OperTipErrorCanTry::mouseMoveEvent(QMouseEvent* e)
{
    moveHelper.mouseMoveEvent(this, e);
}