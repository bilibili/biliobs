
#ifndef PROPERTYDLGVOLSLIDERWID_H
#define PROPERTYDLGVOLSLIDERWID_H

#include <qwidget.h>
typedef struct obs_source  obs_source_t;
typedef struct calldata    calldata_t;
typedef struct obs_fader   obs_fader_t;

class CircleSliderSlider;
class QLabel;

class PropertyDlgVolSliderWid : public QWidget {
	Q_OBJECT

public:
	explicit PropertyDlgVolSliderWid(obs_source_t* source);
	~PropertyDlgVolSliderWid();

	int getOriVal() const;
	void revert();
	int getVolVal() const;
	void setVolVal(int vol);
	obs_source_t *getSource() const;

private:
	void resizeEvent(QResizeEvent *e);

private:
	void setupUi();
	void initVolControl();
	void destoryControl();

	//“Ù¡øøÿ÷∆
private slots:
	/*0 ~ 100 * 10*/
	void onVolSliderValChanged(int vol);
private:
	/*0 ~ 100*/
	void updateSliderVal(int val);
	void mSltVolumeMuted(bool mute);

	void updateVolFromObs();

	static void OBSVolumeChanged(void *data, calldata_t *calldata);
	static void OBSVolumeMuted(void *data, calldata_t *calldata);

	obs_fader_t *obs_fader_;
	obs_source_t *const obs_source_;
	CircleSliderSlider *vol_slider_;
	QLabel *val_label_;

	int ori_val_;
};


#endif