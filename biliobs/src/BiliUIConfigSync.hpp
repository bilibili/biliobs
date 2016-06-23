#ifndef BILIUICONFIGSYNC_H
#define BILIUICONFIGSYNC_H

#include <QWidget>
#include <string>
#include <functional>
#include <QVariant>

#include "../libobs/util/config-file.h"
#include "../libobs/obs-data.h"
#include "../libobs/obs-properties.h"

#include <stdint.h>

//=======================================================
//          支持的配置数据的类型和控件类型
//=======================================================
struct BILI_CONFIG_INT {};
struct BILI_CONFIG_UINT {};
struct BILI_CONFIG_BOOL {};
struct BILI_CONFIG_DOUBLE {};
struct BILI_CONFIG_STRING {};
struct BILI_CONFIG_ENCRYPTEDSTRING {};

struct BILI_DATA_BOOL {};
struct BILI_DATA_INT {};
struct BILI_DATA_DOUBLE {};
struct BILI_DATA_STRING {};
struct BILI_DATA_LASTTYPE {};
struct BILI_DATA_FONT {};

struct BILI_PROP_DESC {};
struct BILI_PROP_LIST_INT {};
struct BILI_PROP_LIST_DOUBLE {};
struct BILI_PROP_LIST_STRING {};

std::string config_get_encryptedstdstring(const config_t* config, const char* section, const char* key);
std::string config_get_default_encryptedstdstring(const config_t* config, const char* section, const char* key);
void config_set_encryptedstdstring(config_t* config, const char* section, const char* key, std::string val);

std::string config_get_stdstring(const config_t* config, const char* section, const char* key);
std::string config_get_default_stdstring(const config_t* config, const char* section, const char* key);
void config_set_stdstring(config_t* config, const char* section, const char* key, std::string val);

//=======================================================
//              字符串和二进制数据互相转换
//=======================================================
std::vector<char> BiliStrToBin(const std::string& str);
std::string BiliBinToStr(const std::vector<char>& data);


//=======================================================
//                  总操作函数
//=======================================================

template<typename T> class qVPtr{

public:
	static T* toPtr(QVariant val){
		return (T *)val.value<void *>();
	}
	static QVariant toVariant(T *ptr){
		return qVariantFromValue((void *)ptr);
	}
};


template<class QTWidgetT, class DataSetTypeT, class DataTypeT>
bool WidgetToData(DataTypeT reserved, QTWidgetT* widget, DataSetTypeT* data, const char* index);

template<class QTWidgetT, class DataSetTypeT, class DataTypeT>
bool DataToWidget(DataTypeT reserved, QTWidgetT* widget, DataSetTypeT* data, const char* index);

#endif
