#include "oper_tip_dlg_factory.h"
#include "oper_tip_dlg.h"

#include "oper_tip_config_used_and_pushed_content_wgt.h"
#include "oper_tip_cut_and_retry_content_wgt.h"
#include "oper_tip_error_can_try_content_wgt.h"
#include "oper_tip_hotkey_duplicate_content_wgt.h"
#include "oper_tip_link_error_simple_contentwgt.h"
#include "oper_tip_link_fail_content_wgt.h"
#include "oper_tip_need_repush_content_wgt.h"
#include "oper_tip_no_open_live_content_wgt.h"
#include "oper_tip_start_broad_fail_content_wgt.h"
#include "oper_tip_duplicate_name_content_wgt.h"
#include "oper_tip_exit_content_wgt.h"
#include "oper_tip_broadcast_is_cut_off.h"

OperTipDlg *OperTipDlgFactory::makeDlg(DlgType type)
{
    OperTipDlg *ret;
    QWidget *content;
    switch (type) {
    case USED_PUSHED:
        ret = new OperTipDlg();
        content = new OperTipConfigUsedAndPushedContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 185 + 9 * 2);
        break;
    case CUT_RETRY:
        ret = new OperTipDlg();
        content = new OperTipCutAndRetryContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 185 + 9 * 2);
        break;
    case ERROR_CAN_TRY:
        ret = new OperTipDlg();
        content = new OperTipErrorCanTryContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 180 + 9 * 2);
        break;
    case HOTKEY_DUPLICATE:
        ret = new OperTipDlg();
        content = new OperTipHotkeyDuplicateContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 155 + 9 * 2);
        break;
    case LINK_ERROR_SIMPLE:
        ret = new OperTipDlg();
        content = new OperTipLinkErrorSimpleContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 185 + 9 * 2);
        break;
    case LINK_FIAL:
        ret = new OperTipDlg();
        content = new OperTipLinkFailContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 185 + 9 * 2);
        break;
    case NEED_REPUSH:
        ret = new OperTipDlg();
        content = new OperTipNeedRepushContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(330 + 9 * 2, 185 + 9 * 2);
        break;
    case NO_OPEN_LIVE:
        ret = new OperTipDlg();
        content = new OperTipNoOpenLiveContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(353 + 9 * 2, 220 + 9 * 2);
        ret->setNotTipCheckBox(((OperTipNoOpenLiveContentWgt*)content)->notTipCheckBox());
        break;
    case START_BROAD_FAIL:
        ret = new OperTipDlg();
        content = new OperTipStartBroadFailContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(353 + 9 * 2, 220 + 9 * 2);
        break;
    case NAME_DUPLICATE:
        ret = new OperTipDlg();
        content = new OperTipDuplicateNameContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(353 + 9 * 2, 144 + 9 * 2);
        break;
    case APP_EXIT:
        ret = new OperTipDlg();
        content = new OperTipExitContentWgt(ret->getContentContainer(), ret);
        ret->setFixedSize(353 + 9 * 2, 140 + 9 * 2);
        break;
    case BROADCAST_IS_CUT_OFF:
        ret = new OperTipDlg();
        content = new OperTipBroadcastIsCutOff(ret->getContentContainer(), ret);
        ret->setFixedSize(353 + 9 * 2, 178 + 9 * 2);
        break;
    default:
        return 0;
        break;
    }

    return ret;
}
