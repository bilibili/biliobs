#include "rounded_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

RoundedWidget::RoundedWidget(const QColor &background) : QWidget(), bgr_color_(background)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void RoundedWidget::paintEvent(QPaintEvent *e)
{
    (void)e;

    const int round_radius = 5;

    /*top-left*/
    QPainterPath path(QPoint(-1, -1));
    path.lineTo(round_radius - 1, 1);
    path.lineTo(round_radius - 1, 0);
    path.arcTo(0,
               0,
               round_radius * 2,
               round_radius * 2,
               90, 90);
    path.lineTo(-1, round_radius - 1);
    path.lineTo(-1, -1);

    /*top-right*/
    path.moveTo(width(), -1);
    path.lineTo(width(), round_radius - 1);
    path.lineTo(width() - 1, round_radius - 1);
    path.arcTo(width() - round_radius * 2,
               0,
               round_radius * 2,
               round_radius * 2,
               0,
               90);
    path.lineTo(width() - round_radius * 2, -1);
    path.lineTo(width(), -1);

    /*bottom-right*/
    path.moveTo(width(), height());
    path.lineTo(width() - round_radius * 2, height());
//    path.lineTo(width() - round_radius * 2, height() - 1);
    path.arcTo(width() - round_radius * 2,
               height() - round_radius * 2,
               round_radius * 2,
               round_radius * 2,
               270,
               90);
    path.lineTo(width(), height() - round_radius * 2);
    path.lineTo(width(), height());

    /*bottom-left*/
    path.moveTo(-1, height());
    path.lineTo(-1, height() - round_radius * 2);
    path.lineTo(0, height() - round_radius * 2);
    path.arcTo(0,
               height() - round_radius * 2,
               round_radius * 2,
               round_radius * 2,
               180,
               90);
    path.lineTo(round_radius - 1, height());
    path.lineTo(- 1, height());

    /*clip the corners*/
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), bgr_color_);

    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillPath(path, Qt::transparent);
}

