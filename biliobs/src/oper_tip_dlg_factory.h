#ifndef OPERTIPDLGFACTORY_H
#define OPERTIPDLGFACTORY_H

class OperTipDlg;
class OperTipDlgFactory
{
public:
    enum DlgType {
        USED_PUSHED,
        CUT_RETRY,
        ERROR_CAN_TRY,
        HOTKEY_DUPLICATE,
        LINK_ERROR_SIMPLE,
        LINK_FIAL,
        NEED_REPUSH,
        NO_OPEN_LIVE,
        START_BROAD_FAIL,
        NAME_DUPLICATE,
        APP_EXIT,
        BROADCAST_IS_CUT_OFF
    };
public:
    static OperTipDlg *makeDlg(DlgType type);

};

#endif // OPERTIPDLGFACTORY_H
