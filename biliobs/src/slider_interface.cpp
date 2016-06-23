
#include "slider_interface.h"

#include <assert.h>

#include <QWheelEvent>
#include <QKeyEvent>

void SliderInterface::setRange(int min, int max, int val)
{
  if (min_val_ == min && max_val_ == max && value_ == val)
    return;
  min_val_ = min;
  max_val_ = max;

  assert(max_val_ >= min_val_);

  value_ = val;

  assert(value_ >= min_val_ && value_ <= max_val_);

  internalChangedInform();
}

void SliderInterface::setValue(int val)
{
  if (val > max_val_)
    val = max_val_;
  else if (val < min_val_)
    val = min_val_;

  if (value_ == val)
    return;

  value_ = val;
  internalChangedInform();
}

INPUT_HANDLE_DEF(SliderInterface)



void SliderInterface::stepUp()
{
    int v = value();
    setValue(v + step_);
    if (v != value())
        emit valueChanged(value());
}
void SliderInterface::stepDn()
{
    int v = value();
    setValue(v - step_);
    if (v != value())
        emit valueChanged(value());
}