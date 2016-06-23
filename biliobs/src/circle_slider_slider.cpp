
#include "circle_slider_slider.h"
#include <QPainter>
#include <QPainterPath>

#include <QMouseEvent>
#include <QPaintEvent>

#include <assert.h>

CircleSliderSlider::CircleSliderSlider(QWidget *parent, Orientation orient) :
    SliderInterface(parent)
{
    orientation_ = HORIZONTAL;

    btn_pressed_ = false;

    initPix();
}

CircleSliderSlider::~CircleSliderSlider()
{
    delete btn_pix_;
}


void CircleSliderSlider::paintEvent(QPaintEvent *e)
{

    //    return;
    if (Orientation::HORIZONTAL != orientation_) {
        assert(false);
        return;
    }

    QPainter painter(this);
    int slider_pos;
    if (btn_pressed_) {
        slider_pos = pos_on_move_ + p2p_off_;
    }
    else {
        slider_pos = valToPixel(value());
    }

    QPen pen(QColor(79, 193, 233));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, btn_size_.height() / 2, slider_pos, btn_size_.height() / 2);

    pen.setColor(QColor(221, 221, 221));

    painter.setPen(pen);
    painter.drawLine(slider_pos, btn_size_.height() / 2, width(), btn_size_.height() / 2);


    QPoint pix_pos;
    if (btn_pressed_) {
        pix_pos.setX(pos_on_move_ + p2p_off_ - btn_size_.width() / 2);
        pix_pos.setY(0);
    }
    else {
        pix_pos = btnPos();
    }

    if (isEnabled())
        painter.drawPixmap(pix_pos, *btn_pix_);
    else
        painter.drawPixmap(pix_pos, *btn_disable_pix_);


}
void CircleSliderSlider::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    if (e->buttons() != Qt::LeftButton)
        return;

    if (orientation_ == Orientation::HORIZONTAL) {

        QPoint ps = e->pos();

        //if (ps.y() >= btn_size_.height())
        //  return;


        btn_pressed_ = true;
        QPoint pix_pos = btnPos();
        if (ps.x() < pix_pos.x() || ps.x() >= pix_pos.x() + btn_size_.width()) {
            /*位于 鼠标位置为 btn 之外*/

            if (ps.x() < btn_size_.width() / 2) {
                pos_on_move_ = ps.x();
                p2p_off_ = btn_size_.width() / 2 - pos_on_move_;

                if (value() != minimum()) {
                    setValue(minimum());

                    emit sliderPressed();
                    emit valueChanged(minimum());
                }
            }
            else if (ps.x() > width() - btn_size_.width() / 2) {
                pos_on_move_ = ps.x();
                p2p_off_ = (width() - btn_size_.width() / 2) - pos_on_move_;

                if (value() != maximum()) {
                    setValue(minimum());

                    emit sliderPressed();
                    emit valueChanged(maximum());
                }
            }
            else {
                pos_on_move_ = ps.x();
                p2p_off_ = 0;

                setValue(pixelToVal(pos_on_move_));
                emit sliderPressed();
                emit valueChanged(maximum());
            }
        } else {
            pos_on_move_ = ps.x();
            p2p_off_ = (pix_pos.x() + btn_size_.width() / 2) - pos_on_move_;
        }
          

    }

}
void CircleSliderSlider::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();

    if (btn_pressed_) {
        btn_pressed_ = false;
        update();
    }


}
void CircleSliderSlider::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();

    if (!btn_pressed_)
        return;

    if (orientation_ == Orientation::HORIZONTAL) {
        int ps = e->pos().x();
        int btn_pos = ps + p2p_off_;

        if (btn_pos < btn_size_.width() / 2) {
            if (pos_on_move_ == btn_size_.width() / 2)
                return;

            pos_on_move_ = btn_size_.width() / 2 - p2p_off_;
            if (value() != minimum()) {

                setValue(minimum());
                emit valueChanged(minimum());
            }
            else
                update();

        }
        else if (btn_pos > width() - btn_size_.width() / 2) {
            if (pos_on_move_ == width() - btn_size_.width() / 2)
                return;

            pos_on_move_ = width() - btn_size_.width() / 2 - p2p_off_;
            if (value() != maximum()) {

                setValue(maximum());
                emit valueChanged(maximum());
            }
            else
                update();
        }
        else {
            pos_on_move_ = ps;
            int val = pixelToVal(btn_pos);
            if (value() != val) {
                setValue(val);

                emit valueChanged(val);
            }
            else {
                update();
            }
        }
    }

}
void CircleSliderSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
    mousePressEvent(e);
}
void CircleSliderSlider::leaveEvent(QEvent *e)
{

}

//SpcStrideSlider::~SpcStrideSlider()
//{
//}


void CircleSliderSlider::internalChangedInform()
{
    update();
}

const QPoint CircleSliderSlider::btnPos() const
{
    if (orientation_ == Orientation::HORIZONTAL) {
        return QPoint(valToPixel(value()) - btn_size_.width() / 2, 0);
    }

    assert(false);
    return QPoint();
}
int CircleSliderSlider::valToPixel(int val) const
{
    if (orientation_ == Orientation::HORIZONTAL) {

        int ret = (double)(val - minimum()) / (double)(maximum() - minimum())
            * (width() - btn_size_.width()) + btn_size_.width() / 2;

        if (ret < btn_size_.width() / 2)
            return btn_size_.width() / 2;
        
        if (ret > width() - btn_size_.width() / 2)
            return width() - btn_size_.width() / 2;

        return ret;

    }
    else {
        assert(false);
        return 0;
    }

}
int CircleSliderSlider::pixelToVal(int px) const
{
    double val = double(px - btn_size_.width() / 2)
        / (double)(width() - btn_size_.width())
        * (maximum() - minimum()) + minimum();

    int ret = val;

    if (val - ret > 0.5) {
        ret++;
    }

    if (ret > maximum())
        return maximum();

    if (ret < minimum())
        return minimum();

    return ret;
}

void CircleSliderSlider::initPix()
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



    btn_disable_pix_ = new QPixmap(btn_size_);
    btn_disable_pix_->fill(Qt::transparent);

    QPainter painter2(btn_disable_pix_);

    lighterBluePen = QPen(QColor(221, 221, 221), 2);

    painter2.setRenderHint(QPainter::Antialiasing);
    painter2.setBrush(whiteBrush);
    painter2.setPen(lighterBluePen);
    painter2.drawPath(path);
    painter2.end();

}
