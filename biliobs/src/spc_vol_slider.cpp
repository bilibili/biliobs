#include "spc_vol_slider.h"

#include <assert.h>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include <QDebug>

SpcVolSlider::SpcVolSlider(QWidget *parent) : SliderInterface(parent)
{
    is_pressed_ = false;

    initPix();

    setRange(0, 300, 0);

    dyn_vol_ = 0;
}

void SpcVolSlider::setDynVol(int vol)
{
    dyn_vol_ = vol;

    update();
}

int SpcVolSlider::dynVol()
{
    return dyn_vol_;
}

void SpcVolSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor end_color(Qt::transparent);
    QColor normal_color(112, 204, 237);


    int slider_pos;
    if (is_pressed_) {
        slider_pos = pos_on_move_ - p2p_off_;
    }
    else {
        slider_pos = valToPos(value());
    }

    /*draw triangle*/
    int gradient_len;

    int gradient_pos = valToPos(dyn_vol_);
    gradient_len = height() - btn_size_.height() / 2 - gradient_pos;

    int hh = height();

    int triangle_pos_ = 13;
    int triangle_width_ = 14;


    QPainterPath bgr(QPointF(triangle_pos_, height()/* - btn_size_.height() / 2 + 1*/));
    bgr.lineTo(triangle_width_ + triangle_pos_, /*btn_size_.height() / 2 + 1*/0);
    bgr.lineTo(triangle_pos_, /*btn_size_.height() / 2 + 1*/0);
    bgr.lineTo(triangle_pos_, height()/* - btn_size_.height() / 2 + 1*/);
    painter.fillPath(bgr, QColor(230, 230, 230));


    QPainterPath bgr2(QPointF(triangle_pos_, slider_pos/* - btn_size_.height() / 2 + 1*/));
    bgr2.lineTo(triangle_width_ + triangle_pos_, slider_pos);
    bgr2.lineTo(triangle_width_ + triangle_pos_, height());
    bgr2.lineTo(triangle_pos_, height()/* - btn_size_.height() / 2 + 1*/);
    painter.fillPath(bgr2, QColor(199, 240, 255));

    if (gradient_len > 10) {
        painter.fillRect(triangle_pos_,
                         gradient_pos + 10,
                         triangle_width_,
                         gradient_len - 9,
                         normal_color);


        QLinearGradient gradient(0,
                                 gradient_pos,
                                 0,
                                 gradient_pos + 10);

        gradient.setColorAt(1, normal_color);

        gradient.setColorAt(0, end_color);

        QBrush brush(gradient);


        painter.fillRect(triangle_pos_,
                             gradient_pos,
                             triangle_width_,
                             10,
                             brush);

    } else {
            QLinearGradient gradient(0,
                                     height() - btn_size_.height() / 2 - gradient_len,
                                     0,
                                     height() - btn_size_.height() / 2);

            gradient.setColorAt(1, normal_color);
            gradient.setColorAt(0, end_color);
            QBrush brush(gradient);

            painter.fillRect(triangle_pos_,
                             height() - btn_size_.height() / 2 - gradient_len,
                             triangle_width_,
                             gradient_len,
                             brush);
    }


    QPainterPath path(QPointF(triangle_pos_ + 1, height()/* - btn_size_.height() / 2 + 1*/));

    path.lineTo(triangle_width_ + triangle_pos_, height()/* - btn_size_.height() / 2 + 1*/);
    path.lineTo(triangle_width_ + triangle_pos_, 0 /*btn_size_.height() / 2 + 1*/);

    path.lineTo(triangle_pos_ + 1, height()/* - btn_size_.height() / 2 - 2*/);

    painter.save();
        //painter.setCompositionMode(QPainter::CompositionMode_Clear);

    painter.fillPath(path, Qt::white);

    painter.restore();




    QPen pen(QColor(206, 216, 219));
    pen.setWidth(1);

    painter.setPen(pen);



    QFont ft = painter.font();
    //ft.setBold(true);
    ft.setFamily("Microsoft Yahei");
    painter.setFont(ft);

    /*graduation*/
    painter.drawLine(triangle_pos_, /*btn_size_.height() / 2*/0, triangle_pos_ + triangle_width_, /*btn_size_.height() / 2*/0);
    painter.drawText(triangle_pos_ + triangle_width_ + 5, btn_size_.height() / 2 + 3, "300%");

    painter.drawLine(triangle_pos_, height() / 2, triangle_pos_ + triangle_width_, height() / 2);
    painter.drawText(triangle_pos_ + triangle_width_ + 5, height() / 2, "100%");

    painter.drawLine(triangle_pos_, height() - 2/* - btn_size_.height() / 2*/, triangle_pos_ + triangle_width_, height() - 2 /*- btn_size_.height() / 2*/);
    painter.drawText(triangle_pos_ + triangle_width_ + 3 + 5, height() - btn_size_.height() / 2 + 3 , "0%");


    /*draw slider*/
    pen.setColor(QColor(79, 193, 233));
    pen.setWidth(2);

    painter.setPen(pen);




    painter.drawLine(btn_size_.width() / 2,
                             slider_pos,
                             btn_size_.width() / 2,
                             height()/* - btn_size_.height() / 2 - 1*/);



    pen.setColor(QColor(221, 221, 221));


    painter.setPen(pen);


    painter.drawLine(btn_size_.width() / 2,
                             slider_pos - 1,
                             btn_size_.width() / 2,
                             /*btn_size_.height() / 2 + 2*/ 0);



    painter.drawPixmap(0, slider_pos - btn_size_.height() / 2, *btn_pix_);



}

void SpcVolSlider::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    if (e->buttons() != Qt::LeftButton)
           return;



    QPoint ps = e->pos();

    if (ps.x() >= btn_size_.width())
        return;


    int slider_pos = valToPos(value());


    if (ps.y() >= slider_pos - btn_size_.height() / 2 && ps.y() < slider_pos + btn_size_.height() / 2) {
        /*click*/
        is_pressed_ = true;

        pos_on_move_ = ps.y();
        p2p_off_ = pos_on_move_ - valToPos(value());
        emit sliderPressed();

    } else {
        is_pressed_ = true;


        if (ps.y() > height() - btn_size_.height() / 2) {

            pos_on_move_ = ps.y();
            p2p_off_ = pos_on_move_ - (height() - btn_size_.height() / 2);


            if (value() != minimum()) {
                setValue(minimum());
                emit sliderPressed();
                emit valueChanged(minimum());
            }

        } else if (ps.y() < btn_size_.height() / 2) {
            pos_on_move_ = ps.y();

            p2p_off_ = pos_on_move_ - btn_size_.height() / 2;

            if (value() != maximum()) {
                setValue(maximum());
                emit sliderPressed();
                emit valueChanged(maximum());
            }
        } else {
            pos_on_move_ = ps.y();
            p2p_off_ = 0;

            setValue(posToVal(pos_on_move_));
            emit sliderPressed();
            emit valueChanged(value());
        }
    }




}

void SpcVolSlider::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
    if (is_pressed_) {
        is_pressed_ = false;
        emit sliderReleased();
        update();
    }
}

void SpcVolSlider::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();


    if (is_pressed_) {
        QPoint ps = e->pos();
        int slider_pos = ps.y() - p2p_off_;

        if (slider_pos < btn_size_.height() / 2) {

            if (value() != maximum()) {
                pos_on_move_ = p2p_off_ + btn_size_.height() / 2;
                setValue(maximum());
                emit valueChanged(maximum());
            } else {
                if (pos_on_move_ - p2p_off_ > btn_size_.height() / 2) {
                    pos_on_move_ = p2p_off_ + btn_size_.height() / 2;
                    update();
                }
            }
        } else if (slider_pos > height() - btn_size_.height() / 2) {
            if (value() != minimum()) {
                pos_on_move_ = p2p_off_ + (height() - btn_size_.height() / 2);
                setValue(minimum());
                emit valueChanged(minimum());
            } else {
                if (pos_on_move_ - p2p_off_ > height() - btn_size_.height() / 2) {
                    pos_on_move_ = p2p_off_ + (height() - btn_size_.height() / 2);
                    update();
                }
            }
        } else {

          int val = posToVal(slider_pos);

            if (val != value()) {
                pos_on_move_ = ps.y();

                setValue(val);
                emit valueChanged(value());
            } else {
                pos_on_move_ = ps.y();

                
                update();

            }

        }
    }


}

void SpcVolSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
    mousePressEvent(e);
}

int SpcVolSlider::valToPos(int val) const
{
    int ret;
    if (val > 100) {
        int ov = (double(val - 100) / 200) * ((height() - btn_size_.height()) / 2);
        ret = height() - (ov + height() / 2);

        if (ret < btn_size_.height() / 2)
            return btn_size_.height() / 2;

        if (ret > height() / 2)
            return height() / 2;
        return ret;
    }


    int ef = (double(val) / 100) * ((height() - btn_size_.height()) / 2);
    ret = height() - ef - btn_size_.height() / 2;

    if (ret < height() / 2)
        return height() / 2;

    if (ret > height() - btn_size_.height() / 2)
        return height() - btn_size_.height() / 2;

    return ret;
}
int SpcVolSlider::posToVal(int pos) const
{
    int ret;
    if (pos < height() / 2) {
        int ov = double(height() / 2 - pos) / ((height() - btn_size_.height()) / 2)  * 200;

        ret = ov + 100;

        if (ret < 100)
            return 100;

        if (ret > 300)
            return  300;

        return ret;
    }

    int lw = double((height() - btn_size_.height() / 2) - pos) / ((height() - btn_size_.height()) / 2) * 100;

    ret = lw;

    if (ret < 0)
        return 0;
    if (ret > 100)
        return 100;

    return ret;

}

void SpcVolSlider::internalChangedInform()
{
  update();
}


void SpcVolSlider::initPix()
{
    btn_size_ = QSize(16, 16);
    btn_pix_ = new QPixmap(btn_size_);
    btn_pix_->fill(Qt::transparent);

    QPainter painter(btn_pix_);

    QPainterPath path(QPointF(8, 8));
    path.addEllipse(1, 1, 14, 14);

    QPen lighterBluePen(QColor(167, 213, 232), 2);
    QBrush whiteBrush(QColor(255, 255, 255));

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(whiteBrush);
    painter.setPen(lighterBluePen);
    painter.drawPath(path);
    painter.end();
}
