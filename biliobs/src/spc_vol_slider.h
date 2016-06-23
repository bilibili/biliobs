#ifndef SPCVOLSLIDER_H
#define SPCVOLSLIDER_H
#include "slider_interface.h"

class SpcVolSlider : public SliderInterface
{
    Q_OBJECT
public:
    SpcVolSlider(QWidget *parent = 0);

public:
    void setDynVol(int vol);
    int dynVol();
private:
    int dyn_vol_;


private:
  void paintEvent(QPaintEvent *) override;

  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;

private:
  int valToPos(int val) const;
  int posToVal(int pos) const;
//  int sliderPos() const;
private:
  bool is_pressed_;
  int p2p_off_;
  int pos_on_move_;

private:
  void internalChangedInform() override;

private:
  void initPix();
  QPixmap *btn_pix_;
  QSize btn_size_;

};

#endif // SPCVOLSLIDER_H
