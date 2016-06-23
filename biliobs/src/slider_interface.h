
#ifndef SLIDER_INTERFACE_H
#define SLIDER_INTERFACE_H

#include <QWidget>

#include "slider_input_accept_macro.h"

class SliderInterface : public QWidget {
  Q_OBJECT

public:
  enum Orientation {
    HORIZONTAL,
    VERTICAL
  };

  SliderInterface(QWidget *parent = 0) : QWidget(parent), step_(1), value_(0), min_val_(0), max_val_(100) 
  {
      setFocusPolicy(Qt::StrongFocus);
  }

  virtual ~SliderInterface() {}

signals:
  //void actionTriggered(int action);
  void rangeChanged(int min, int max, int val);
  //    void sliderMoved(int value);
  void sliderPressed();
  void sliderReleased();
  void valueChanged(int value);

  /*
  * range
  */
public:
  int	maximum() const { return max_val_; }
  int	minimum() const { return min_val_; }
  void setRange(int min, int max, int val);
private:
  int min_val_;
  int max_val_;
public:
  int	value() const { return value_; }
  void setValue(int);
private:
  int value_;

protected:
  virtual void internalChangedInform() = 0;

public:
  void setSingleStep(int step) { single_step_ = step; }
  int	singleStep() const { return single_step_; }
private:
  int single_step_;

INPUT_HANDLE_DEC

//public:
//  void setOrientation(Orientation direct);    { orientation_ = direct; } /*now no support for change*/
//  Orientation orientation() const { return orientation_; }
//private:
//  Orientation orientation_;

};


#endif