#pragma once

#include "slider_interface.h"
class SpcStrideSlider : public SliderInterface
{
public:
  SpcStrideSlider(QWidget *parent = 0, Orientation orient = Orientation::HORIZONTAL);
  ~SpcStrideSlider();

private:
  Orientation orientation_;
public:
  void setOrientation(Orientation direct)    { orientation_ = direct; } /*now no support for change*/
  Orientation orientation() const { return orientation_; }

public:
    void stepUp() override;
    void stepDn() override;


public:
  void initDisplay(int begin, int mid, int end);
private:
  int mid_val_;

  int midPos() const;
  double mid_left_len_;

private:
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void leaveEvent(QEvent *e) override;

private:
  bool btn_pressed_;

private:
  void internalChangedInform() override;

private:
  int p2p_off_;
  int pos_on_move_;

private:
  const QPoint btnPos() const;
  int valToPixel(int val) const;
  int pixelToVal(int px) const;

private:
  void initPix();
  QPixmap *btn_pix_;
  QSize btn_size_;

};

