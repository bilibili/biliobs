#include "spc_stride_slider.h"

#include <assert.h>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include <QDebug>

static const int sc_text_height = 13;

SpcStrideSlider::SpcStrideSlider(QWidget *parent /*= 0*/, Orientation orient /*= Orientation::HORIZONTAL*/) :
      SliderInterface(parent)
{
  if (orient != Orientation::HORIZONTAL) {
    /*not for VERTICAL*/
    assert(false);
  }

  orientation_ = HORIZONTAL;



  btn_pressed_ = false;

  mid_left_len_ = 0.8;
  mid_val_ = (maximum() - minimum()) * mid_left_len_ + minimum();


  initPix();
}

SpcStrideSlider::~SpcStrideSlider()
{
    delete btn_pix_;
}

void SpcStrideSlider::stepUp()
{
    if (value() == mid_val_) {
        setValue(maximum());

        emit valueChanged(value());
    }
    else if (value() < mid_val_) {
        setValue(value() + step_);
        emit valueChanged(value());
    }
}
void SpcStrideSlider::stepDn()
{
    if (value() == maximum()) {
        setValue(mid_val_);
        emit valueChanged(value());
    }
    else {
        setValue(value() - step_);
        emit valueChanged(value());
    }

}

void SpcStrideSlider::initDisplay(int begin, int mid, int end)
{
  //if (orientation_ == Orientation::HORIZONTAL) {
  //  setFixedHeight(btn_size_.height() + 20);
  //}

  setRange(begin, end, begin);
  mid_val_ = mid;
  mid_left_len_ = 0.80;
}

int SpcStrideSlider::midPos() const
{
  if (orientation_ == Orientation::HORIZONTAL) {
    return (width() - btn_size_.width()) * mid_left_len_ + btn_size_.width() / 2;
  }
  else
    return 0;
}

void SpcStrideSlider::paintEvent(QPaintEvent *e)
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
  painter.drawLine(0, sc_text_height + btn_size_.height() / 2, slider_pos, sc_text_height + btn_size_.height() / 2);

  pen.setColor(QColor(221, 221, 221));

  painter.setPen(pen);
  painter.drawLine(slider_pos, sc_text_height + btn_size_.height() / 2, width(), sc_text_height + btn_size_.height() / 2);


  QPoint pix_pos;
  if (btn_pressed_) {
    pix_pos.setX(pos_on_move_ + p2p_off_ - btn_size_.width() / 2);
    pix_pos.setY(sc_text_height);
  }
  else {
    pix_pos = btnPos();
  }

  painter.drawPixmap(pix_pos, *btn_pix_);


//  return;

  QFont ft = painter.font();
  ft.setBold(true);
  painter.setFont(ft);




  painter.drawText(0, 
                   sc_text_height - 5, 
                   QString::number(minimum()));

  //painter.drawLine(midPos(), 0, midPos(), height());
  painter.drawText(midPos() - 8, 
                   sc_text_height - 5,
                   QString::number(mid_val_));

  painter.drawText(width() - btn_size_.width() / 2 - 12, 
                   sc_text_height - 5,
                   QString::number(maximum()));

}
void SpcStrideSlider::mousePressEvent(QMouseEvent *e)
{
  e->accept();
  if (e->buttons() != Qt::LeftButton)
    return;

  if (orientation_ == Orientation::HORIZONTAL) {

    QPoint ps = e->pos();

    //if (ps.y() >= btn_size_.height())
    //  return;

    if (ps.y() < sc_text_height || ps.y() > sc_text_height + btn_size_.height())
        return;

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
      else {
        int mid_pos = midPos();

        if (ps.x() >= mid_pos) {
          if (ps.x() >= (mid_pos + (width() - btn_size_.width() / 2)) / 2) {
            pos_on_move_ =  width() - btn_size_.width() / 2;
            p2p_off_ = 0;

            setValue(maximum());
            emit sliderPressed();
            emit valueChanged(maximum());
          } else {
            pos_on_move_ = mid_pos;
            p2p_off_ = 0;

            if (value() != mid_val_) {
              setValue(mid_val_);
              emit sliderPressed();
              emit valueChanged(mid_val_);
            }
            
          }
        } else {

          pos_on_move_ = ps.x();
          p2p_off_ = 0;

          //int val = double(pos_on_move_ - btn_size_.width() / 2) 
          //              / (double)(mid_pos - -btn_size_.width() / 2) 
          //          * (mid_val_ - minimum()) + minimum();

          int val = pixelToVal(pos_on_move_);

          if (value() != val) {
            setValue(val);
            emit sliderPressed();
            emit valueChanged(value());
          }
        }
      }
    } else {
      pos_on_move_ = ps.x();
      p2p_off_ = valToPixel(value()) - pos_on_move_;
    }
   
  }

}
void SpcStrideSlider::mouseReleaseEvent(QMouseEvent *e)
{
  e->accept();

  if (btn_pressed_) {
    btn_pressed_ = false;
    update();
  }

  
}
void SpcStrideSlider::mouseMoveEvent(QMouseEvent *e)
{
  e->accept();

  if (!btn_pressed_)
    return;

  if (orientation_ == Orientation::HORIZONTAL) {
    int ps = e->pos().x();
    int btn_pos = ps + p2p_off_;
    int mid_pos = midPos();

    if (btn_pos < btn_size_.width() / 2) {
      if (pos_on_move_ == btn_size_.width() / 2)
        return;

      pos_on_move_ = btn_size_.width() / 2 - p2p_off_;
      if (value() != minimum()) {

        setValue(minimum());
        emit valueChanged(minimum());
      } else
        update();

    } else if (btn_pos < mid_pos) {

      pos_on_move_ = ps;
      int val = pixelToVal(btn_pos);
      if (value() != val) {
        setValue(val);

        emit valueChanged(val);
      } else {
        update();
      }
  
    } else {
      if (ps < (mid_pos + (width() - btn_size_.width() / 2)) / 2) {

        if (value() != mid_val_) {

          pos_on_move_ = mid_pos - p2p_off_;
          setValue(mid_val_);

          emit valueChanged(mid_val_);
        }
      }
      else {
        if (value() != maximum()) {

          pos_on_move_ = width() - btn_size_.width() / 2 - p2p_off_;
          setValue(maximum());

          emit valueChanged(maximum());
        }
      }
    }
  }
  
}
void SpcStrideSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
  mousePressEvent(e);
}
void SpcStrideSlider::leaveEvent(QEvent *e)
{

}

//SpcStrideSlider::~SpcStrideSlider()
//{
//}


void SpcStrideSlider::internalChangedInform()
{
  if (value() > mid_val_) {
    setValue(maximum());
  }

  update();
}

const QPoint SpcStrideSlider::btnPos() const
{
  if (orientation_ == Orientation::HORIZONTAL) {
    return QPoint(valToPixel(value()) - btn_size_.width() / 2, sc_text_height);
  }

  assert(false);
  return QPoint();
}
int SpcStrideSlider::valToPixel(int val) const
{
  if (orientation_ == Orientation::HORIZONTAL) {
    if (val == mid_val_)
      return midPos();

    if (val == maximum())
      return width() - btn_size_.width() / 2;

    int ret = (double)(val - minimum()) / (double)(mid_val_ - minimum()) 
              * (midPos() - btn_size_.width() / 2) + btn_size_.width() / 2;

    if (ret > midPos())
      return midPos();

    return ret;
  }
  else {
    assert(false);
    return 0;
  }

}
int SpcStrideSlider::pixelToVal(int px) const
{
  //if (px < width() - btn_size_.width() / 2 + 2 && px > width() - btn_size_.width() / 2 - 2)
  //  return mid_val_;

  if (px >= ((width() - btn_size_.width() / 2) + midPos()) / 2)
      return maximum();



  double val = double(px - btn_size_.width() / 2)
                / (double)(midPos() - btn_size_.width() / 2) 
                * (mid_val_ - minimum()) + minimum();

  int ret = val;
  if (val - ret > 0.5) {
      ret++;
  }

  if (ret > mid_val_)
    return mid_val_;

  return ret;
}

void SpcStrideSlider::initPix()
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
