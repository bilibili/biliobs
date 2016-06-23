#include "BiLiOBSMainWid.h"
#include "../biliapi/IBiliApi.h"
#include "BiLiAudioDevSettingWid.h"
#include "BiliGlobalDatas.hpp"

#define OBS_AUDIO_CHANNEL_RANGE_FIRST 1
#define OBS_AUDIO_CHANNEL_RANGE_LAST 5

void BiLiOBSMainWid::mSltValumeSettingBtn() {

	QPoint pos;

    QSize s = mAudioDevSettingWid->size();

    pos.setX(ui.ValumeSettingBtn->mapToGlobal(
                                    (ui.ValumeSettingBtn->rect().topLeft() + ui.ValumeSettingBtn->rect().topRight()) / 2
                                  ).x() 
            - mAudioDevSettingWid->sizeHint().width() / 2
        );
    pos.setY(ui.LeftCenterWid->mapToGlobal(
                                   ui.LeftCenterWid->rect().bottomLeft()
                               ).y() 
            - mAudioDevSettingWid->height()
        );

    mAudioDevSettingWid->show();
    mAudioDevSettingWid->move(pos);
}

void BiLiOBSMainWid::mSltActivateAudioSource(OBSSource source) {

	AudioDevControl *audioDevCtrl = new AudioDevControl(source);
	mAudioDevCtrlItemsV.push_back(audioDevCtrl);

	/*do sorting, now only deal with less than 3 audio widgets*/
	assert(mAudioDevCtrlItemsV.size() <= 2);
	if (mAudioDevCtrlItemsV.size() == 2) {
		if (mAudioDevCtrlItemsV[0]->mGetName() == tr("AuxDev1").append(":")) {
			AudioDevControl *tmp = mAudioDevCtrlItemsV[0];
			mAudioDevCtrlItemsV[0] = mAudioDevCtrlItemsV[1];
			mAudioDevCtrlItemsV[1] = tmp;
		}
	}

	mAudioDevSettingWid->mReplaceItems(mAudioDevCtrlItemsV);
}

void BiLiOBSMainWid::mSltDeactivateAudioSource(OBSSource source) {

	for (size_t i = 0; i < mAudioDevCtrlItemsV.size(); i++){
		if (mAudioDevCtrlItemsV[i]->mGetSource() == source) {
			delete mAudioDevCtrlItemsV[i];
			mAudioDevCtrlItemsV.erase(mAudioDevCtrlItemsV.begin() + i);
			break;
		}
	}
}
