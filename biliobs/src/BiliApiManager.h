#ifndef BILIAPIMANAGER_H
#define BILIAPIMANAGER_H

class IBiliAPI;

class BiliAPIMan
{
    static IBiliAPI* instance_;
    static int errorCode_;
    
public:
	static IBiliAPI* GetInstance();
    static int GetErrorCode();
};

#endif
