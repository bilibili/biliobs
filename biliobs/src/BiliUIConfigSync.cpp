#include "BiliUIConfigSync.hpp"
//#include "BiliUIConfigSync_filters.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFontComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>

#include "BiliApiManager.h"
#include "../biliapi/IBiliApi.h"

#include "obs-properties.h"
#include "BiliOBSUtility.hpp"

#include "circle_slider_slider.h"

#include "slider_interface.h"
//#include "spc_stride_slider.h"

extern "C"
{
#include "base64.h"
};

//=======================================================
//                 不同配置类型的定义
//=======================================================
template<typename ConfigTypeId>
struct ConfigType;

template<> struct ConfigType < BILI_CONFIG_BOOL >
{ 
	typedef bool T; 
};

template<> struct ConfigType < BILI_CONFIG_INT >
{
	typedef int64_t T;
};

template<> struct ConfigType < BILI_CONFIG_UINT >
{ 
	typedef uint64_t T; 
};

template<> struct ConfigType < BILI_CONFIG_DOUBLE > 
{ 
	typedef double T; 
};


template<> struct ConfigType < BILI_CONFIG_STRING > 
{ 
	typedef std::string T;
}; 

template<> struct ConfigType < BILI_CONFIG_ENCRYPTEDSTRING >
{
	typedef std::string T;
};


template<> struct ConfigType < BILI_DATA_STRING >
{
	typedef std::string T;
};

template<> struct ConfigType < BILI_DATA_DOUBLE >
{
	typedef double T;
};

template<> struct ConfigType < BILI_DATA_BOOL >
{
	typedef bool T;
};

template<> struct ConfigType < BILI_DATA_INT >
{
	typedef int64_t T;
};

template<> struct ConfigType < BILI_DATA_FONT >
{
	typedef QFont T;
};


template<>  struct ConfigType < BILI_PROP_DESC >
{
	typedef std::string T;
};

template<> struct ConfigType < BILI_PROP_LIST_STRING >
{
	typedef std::vector<std::pair<std::string, std::string> > T;
};

template<> struct ConfigType < BILI_PROP_LIST_INT >
{
	typedef std::vector<std::pair<std::string, int64_t> > T;
};

template<> struct ConfigType < BILI_PROP_LIST_DOUBLE >
{
	typedef std::vector<std::pair<std::string, double> > T;
};

//=======================================================
//                 读写配置文件的模板
//=======================================================
template<typename ConfigTypeId>
struct ConfigOp;


template < typename ConfigTypeId,
	typename ConfigType<ConfigTypeId>::T GetFunc(const config_t*, const char*, const char*),
	typename ConfigType<ConfigTypeId>::T GetDefaultFunc(const config_t*, const char*, const char*),
	void SetFunc(config_t*, const char*, const char*, typename ConfigType<ConfigTypeId>::T)
>
struct ConfigOpBase
{
	typedef typename ConfigType<ConfigTypeId>::T T;

	static bool Get(T* val, config_t* config, const char* index)
	{
		const char* section = index;

		const char* key = index;
		while (*key != 0) ++key;
		++key;

		if (config_has_user_value(config, section, key))
			*val = GetFunc(config, section, key);
		else
			if (config_has_default_value(config, section, key))
				*val = GetDefaultFunc(config, section, key);
			else
				return false;
		return true;
	}

	static void Put(const T& val, config_t* config, const char* index)
	{
		const char* section = index;

		const char* key = index;
		while (*key != 0) ++key;
		++key;
		
		SetFunc(config, section, key, val);
	}
};


template < typename ConfigTypeId,
	typename ConfigType<ConfigTypeId>::T GetFunc(obs_data_t*, const char*),
	typename ConfigType<ConfigTypeId>::T GetDefaultFunc(obs_data_t*, const char*),
	void SetFunc(obs_data_t*, const char*, typename ConfigType<ConfigTypeId>::T)
>
struct ConfigOpDataBase
{
	typedef typename ConfigType<ConfigTypeId>::T T;
	static bool Get(T* val, obs_data_t* config, const char* index)
	{
		if (obs_data_has_user_value(config, index))
			*val = GetFunc(config, index);
		else
			if (obs_data_has_default_value(config, index))
				*val = GetDefaultFunc(config, index);
			else
				return false;
		return true;
	}

	typedef typename ConfigType<ConfigTypeId>::T T;
	static void Put(const T& val, obs_data_t* config, const char* index)
	{
		SetFunc(config, index, val);
	}
};

template<typename ConfigTypeId,
	typename ConfigType<ConfigTypeId>::T GetFunc(obs_properties_t*, const char*)
>
struct ConfigOpPropBase
{
	typedef typename ConfigType<ConfigTypeId>::T T;

	static bool Get(T* val, obs_properties_t* props, const char* index)
	{
		obs_property_t* prop = obs_properties_get(props, index);
		if (prop == 0)
			return false;
		else
		{
			*val = GetFunc(props, index);
			return true;
		}
	}
};

//=======================================================
//为原有配置文件读写函数包装一层，使其支持std::string和加密字串
//=======================================================

std::string config_get_stdstring(const config_t* config, const char* section, const char* key)
{
	const char* val = config_get_string(config, section, key);
	if (val)
		return val;
	else
		return "";
}

std::string config_get_default_stdstring(const config_t* config, const char* section, const char* key)
{
	const char* val = config_get_default_string(config, section, key);
	if (val)
		return val;
	else
		return "";
}

void config_set_stdstring(config_t* config, const char* section, const char* key, std::string val)
{
	config_set_string(config, section, key, val.c_str());
}

std::string config_get_encryptedstdstring(const config_t* config, const char* section, const char* key)
{
	return config_get_stdstring(config, section, key);
}

std::string config_get_default_encryptedstdstring(const config_t* config, const char* section, const char* key)
{
	return config_get_default_stdstring(config, section, key);
}

void config_set_encryptedstdstring(config_t* config, const char* section, const char* key, std::string val)
{
	config_set_stdstring(config, section, key, val);
}



std::string obs_data_get_stdstring(obs_data_t* data, const char* name)
{
	const char* val = obs_data_get_string(data, name);
	if (val)
		return val;
	else
		return "";
}

std::string obs_data_get_default_stdstring(obs_data_t* data, const char* name)
{
	const char* val = obs_data_get_default_string(data, name);
	if (val)
		return val;
	else
		return "";
}

void obs_data_set_stdstring(obs_data_t* data, const char* name, std::string val)
{
	obs_data_set_string(data, name, val.c_str());
}


//对字体的支持
QFont obs_data_get_qfont(obs_data_t *data, const char *name){

	obs_data_t  *font_obj = obs_data_get_obj(data, name);

	const char *face = obs_data_get_string(font_obj, "face");
	const char *style = obs_data_get_string(font_obj, "style");
	int        size = (int)obs_data_get_int(font_obj, "size");
	uint32_t   flags = (uint32_t)obs_data_get_int(font_obj, "flags");

	QFont font;
	if (face) {
		font.setFamily(face);
		font.setStyleName(style);
	}

	if (size)
		font.setPointSize(size); 

	if (flags & OBS_FONT_BOLD) font.setBold(true);
	if (flags & OBS_FONT_ITALIC) font.setItalic(true);
	if (flags & OBS_FONT_UNDERLINE) font.setUnderline(true);
	if (flags & OBS_FONT_STRIKEOUT) font.setStrikeOut(true);

	return font;
}

QFont obs_data_get_default_qfont(obs_data_t* data, const char* name){

	return obs_data_get_qfont(data, name);
}

void obs_data_set_qfont(obs_data_t* data, const char* name, QFont font){

	obs_data_t *fontObj = obs_data_create();
	obs_data_set_string(fontObj, "face", (font.family()).toUtf8().data());
	obs_data_set_string(fontObj, "style", (font.styleName()).toUtf8().data());
	obs_data_set_int(fontObj, "size", font.pointSize());

	int flags  = font.bold() ? OBS_FONT_BOLD : 0;
	flags |= font.italic() ? OBS_FONT_ITALIC : 0;
	flags |= font.underline() ? OBS_FONT_UNDERLINE : 0;
	flags |= font.strikeOut() ? OBS_FONT_STRIKEOUT : 0;
	obs_data_set_int(fontObj, "flags", flags);

	obs_data_set_obj(data, "font", fontObj);
	obs_data_release(fontObj);
	return;
}

template<class T, class GetFuncT, int formatId>
std::vector<T> obs_properties_get_list(obs_properties_t* props, const char* name, GetFuncT GetFunc)
{
	obs_property_t* prop = obs_properties_get(props, name);
	if (prop)
	{
		if (obs_property_get_type(prop) == OBS_PROPERTY_LIST)
		{
			if (obs_property_list_format(prop) == formatId)
			{
				int count = obs_property_list_item_count(prop);
				std::vector<T> r;
				r.reserve(count);

				for (int i = 0; i < count; ++i)
				{
					r.push_back(std::make_pair(obs_property_list_item_name(prop, i), GetFunc(prop, i)));
				}

				return std::move(r);
			}
		}
	}

	return std::vector<T>();
}

std::vector <std::pair<std::string, std::string>> obs_properties_get_stringlist(obs_properties_t* props, const char* name)
{
	return obs_properties_get_list<std::pair<std::string, std::string>, const char*(*)(obs_property_t*, size_t), OBS_COMBO_FORMAT_STRING>(props, name, obs_property_list_item_string);
}

std::vector <std::pair<std::string, int64_t>> obs_properties_get_intlist(obs_properties_t* props, const char* name)
{
	return obs_properties_get_list<std::pair<std::string, int64_t>, int64_t(*)(obs_property_t*, size_t), OBS_COMBO_FORMAT_INT>(props, name, obs_property_list_item_int);
}

//=======================================================
//                 每个类型读写的模板
//=======================================================
template<> struct ConfigOp<BILI_CONFIG_BOOL> : public ConfigOpBase < BILI_CONFIG_BOOL, &config_get_bool, &config_get_default_bool, &config_set_bool > {};
template<> struct ConfigOp<BILI_CONFIG_INT> : public ConfigOpBase < BILI_CONFIG_INT, &config_get_int, &config_get_default_int, &config_set_int >{};
template<> struct ConfigOp<BILI_CONFIG_UINT> : public ConfigOpBase < BILI_CONFIG_UINT, &config_get_uint, &config_get_default_uint, &config_set_uint >{};
template<> struct ConfigOp<BILI_CONFIG_DOUBLE> : public ConfigOpBase < BILI_CONFIG_DOUBLE, &config_get_double, &config_get_default_double, &config_set_double >{};
template<> struct ConfigOp<BILI_CONFIG_STRING> : public ConfigOpBase < BILI_CONFIG_STRING, &config_get_stdstring, &config_get_default_stdstring, &config_set_stdstring >{};
template<> struct ConfigOp<BILI_CONFIG_ENCRYPTEDSTRING> : public ConfigOpBase < BILI_CONFIG_ENCRYPTEDSTRING, &config_get_encryptedstdstring, &config_get_default_encryptedstdstring, &config_set_encryptedstdstring >{};

template<> struct ConfigOp<BILI_DATA_STRING> : public ConfigOpDataBase < BILI_DATA_STRING, &obs_data_get_stdstring, &obs_data_get_default_stdstring, &obs_data_set_stdstring >{};
template<> struct ConfigOp<BILI_DATA_INT> : public ConfigOpDataBase < BILI_DATA_INT, &obs_data_get_int, &obs_data_get_default_int, &obs_data_set_int >{};
template<> struct ConfigOp<BILI_DATA_BOOL> : public ConfigOpDataBase < BILI_DATA_BOOL, &obs_data_get_bool, &obs_data_get_default_bool, &obs_data_set_bool >{};
template<> struct ConfigOp<BILI_DATA_FONT> : public ConfigOpDataBase < BILI_DATA_FONT, &obs_data_get_qfont, &obs_data_get_default_qfont, &obs_data_set_qfont >{};
template<> struct ConfigOp<BILI_DATA_DOUBLE> : public ConfigOpDataBase < BILI_DATA_DOUBLE, &obs_data_get_double, &obs_data_get_default_double, &obs_data_set_double >{};

template<> struct ConfigOp<BILI_PROP_LIST_STRING> : public ConfigOpPropBase < BILI_PROP_LIST_STRING, obs_properties_get_stringlist > {};
template<> struct ConfigOp<BILI_PROP_LIST_INT> : public ConfigOpPropBase < BILI_PROP_LIST_INT, obs_properties_get_intlist > {};

//=======================================================
//                 各控件的操作函数
//=======================================================
template<typename QTWIDGET>
struct WidgetOperator;

template<>
struct WidgetOperator<QLineEdit>
{
	static bool Put(QLineEdit* qle, std::string* val)
	{
		if (qle == 0)
			return false;
		qle->setText(val->c_str());
		return true;
	}

	static bool Get(QLineEdit* qle, std::string* val)
	{
		if (qle == 0)
			return false;
		*val = qle->text().toUtf8().data();
		return true;
	}

    static bool Put(QLineEdit* qle, double* val)
    {
        if (qle == 0)
            return false;
        qle->setText(QString::number(*val));
        return true;
    }

    static bool Get(QLineEdit* qle, double* val)
    {
        if (qle == 0)
            return false;

        bool ok;

        *val = qle->text().toDouble(&ok);
        return ok;
    }


    static bool Put(QLineEdit* qle, uint64_t* val)
    {
        if (qle == 0)
            return false;
        qle->setText(QString::number(*val));
        return true;
    }

    static bool Get(QLineEdit* qle, uint64_t* val)
    {
        if (qle == 0)
            return false;

        bool ok;

        *val = qle->text().toUInt(&ok);
        return ok;
    }
};

template<>
struct WidgetOperator <QCheckBox>
{
	static bool Put(QCheckBox* qcb, bool* val)
	{
		if (qcb == 0)
			return false;
		qcb->setCheckState(*val ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		return true;
	}

	static bool Get(QCheckBox* qcb, bool* val)
	{
		if (qcb == 0)
			return false;
		*val = qcb->checkState() == Qt::CheckState::Checked ? true : false;
		return true;
	}
};

template<>
struct WidgetOperator<QComboBox>
{

    //==============begin 读取属性用的============
    template<class SupportedType>
    struct TypeConverter
    {
        static SupportedType To(const SupportedType& pIn) { return pIn; }
        static SupportedType From(const SupportedType& pIn) { return pIn; }
    };

    template<>
    struct TypeConverter<std::string>
    {
        static const char* To(const std::string& pIn) { return pIn.c_str(); }
        static std::string From(const char* pIn) { return pIn; }
    };

    template<class ValueType>
    static bool Put(QComboBox* cb, std::vector<std::pair<std::string, ValueType> >* val)
    {
        if (cb == 0)
            return false;

        cb->clear();
        for (auto& x : *val){
            if (!QString::compare(x.first.c_str(), "Default")){
                cb->addItem(QObject::tr("Default"), TypeConverter<ValueType>::To(x.second));
            }
            else
                cb->addItem(x.first.c_str(), TypeConverter<ValueType>::To(x.second));
        }

        return true;
    }
    //==============end 读取属性用的============


    static bool Get(QComboBox* cb, uint64_t* val)
    {
        if (cb == 0)
            return false;

        *val = cb->itemData(cb->currentIndex()).toULongLong();

        return true;
    }

    static bool Get(QComboBox* cb, int64_t* val)
    {
        if (cb == 0)
            return false;

        *val = cb->currentData().toLongLong();
        return true;
    }

    template<class T>
    static bool Put(QComboBox* cb, T* val)
    {
        if (cb == 0)
            return false;

        int index = cb->findData(*val);
        if (index >= 0)
        {
            cb->setCurrentIndex(index);
            return true;
        }
        else
            return false;
    }

    static bool Get(QComboBox* cb, std::string* val)
    {
        if (cb == 0)
            return false;

        *val = cb->currentData().toString().toUtf8().data();
        return true;
    }

    template<>
    static bool Put<std::string>(QComboBox* cb, std::string* val)
    {
        if (cb == 0)
            return false;

        int index = cb->findData(val->c_str());
        if (index > 0)
            cb->setCurrentIndex(index);

        return true;
    }
};

template<>
struct WidgetOperator<QFontComboBox>
{
	static bool Get(QFontComboBox *fcb, QFont *f){
		if (!fcb)
			return false;
		*f = fcb->currentFont();

		QCheckBox *boldCB = qVPtr<QCheckBox>::toPtr(fcb->property("BoldStyleCheckBox"));
		QCheckBox *italicCB = qVPtr<QCheckBox>::toPtr(fcb->property("ItalicCheckBox"));

		f->setBold(boldCB->isChecked());
		f->setItalic(italicCB->isChecked());

		return true;
	}

	static bool Put(QFontComboBox* fcb, QFont *f) {
		if (fcb == 0)
			return false;

		QCheckBox *boldCB = qVPtr<QCheckBox>::toPtr(fcb->property("BoldStyleCheckBox"));
		QCheckBox *italicCB = qVPtr<QCheckBox>::toPtr(fcb->property("ItalicCheckBox"));

		if (boldCB && f->bold()) boldCB->setChecked(true);
		if (italicCB && f->italic()) italicCB->setChecked(true);

		fcb->blockSignals(true);
		fcb->setCurrentFont(*f);
		fcb->blockSignals(false);
		return true;
	}
};

template<> 
struct WidgetOperator<QPlainTextEdit> {

	static bool Get(QPlainTextEdit *plaintEdit, std::string *txt){
		if (plaintEdit == 0)
			return false;

		*txt = plaintEdit->toPlainText().toUtf8().data();
		return true;
	}

	static bool Put(QPlainTextEdit *plaintEdit, std::string *txt){
		if (plaintEdit == 0)
			return false;

		plaintEdit->clear();
		plaintEdit->appendPlainText(txt->c_str());
		return true;
	}
};

template<> struct WidgetOperator<QPushButton>
{
	static bool Get(QPushButton *btn, int64_t *color){
		if (btn == 0)
			return false;

		bool isOk = false;
		*color = btn->property("color1ColorInt64").toLongLong(&isOk);

		CircleSliderSlider *sld = qVPtr<CircleSliderSlider>::toPtr(btn->property("OpacitySlider"));
		//QSlider *sld = qVPtr<QSlider>::toPtr(wid->property("OpacitySlider"));

		if (sld){
			int alpha = sld->value();
			QLabel *lab = qVPtr<QLabel>::toPtr(btn->property("OpacityValLabel"));
			if (lab)
				lab->setText(QString("%1%").arg(alpha*100/255));
			*color = (*color & 0x00FFFFFF) | (alpha << 24);
		}

		return isOk;
	}

	static bool Put(QPushButton* btn, int64_t *color) {
		if (btn == 0)
			return false;

		CircleSliderSlider *sld = qVPtr<CircleSliderSlider>::toPtr(btn->property("OpacitySlider"));
		//QSlider *sld = qVPtr<QSlider>::toPtr(wid->property("OpacitySlider"));
		if (sld){
			int alpha = (*color & 0xFF000000) >> 24;
			sld->setValue(alpha);
		}

		SetPushButtonBackgroundColor(btn, *color);
		btn->setProperty("color1ColorInt64", *color);
		return true;
	}
};

template<>
struct WidgetOperator < QSlider > {

	static bool Put(QSlider *sld, int64_t *val){
		sld->blockSignals(true);
		sld->setValue(*val);
		sld->blockSignals(false);
		return true;
	}

	static bool Get(QSlider *sld, int64_t *val) {
		*val = sld->value();
		return true;
	}

	static bool Put(QSlider *sld, double *val){
		int64_t tmp = *val;
		return Put(sld, &tmp);
	}

	static bool Get(QSlider *sld, double *val) {
		int64_t tmp = 0;
		bool r = Get(sld, &tmp);
		if (r) *val = tmp;
		return r;
	}
};

template<>
struct WidgetOperator<CircleSliderSlider> {

	static bool Put(CircleSliderSlider *sld, int64_t *val){
		sld->blockSignals(true);
		sld->setValue(*val);
		sld->blockSignals(false);
		return true;
	}

	static bool Get(CircleSliderSlider *sld, int64_t *val) {
		*val = sld->value();
		return true;
	}

	static bool Put(CircleSliderSlider *slider, double *val){
		int64_t tmp = *val;
		return Put(slider, &tmp);
	}

	static bool Get(CircleSliderSlider *slider, double *val) {
		int64_t tmp = 0;
		bool r = Get(slider, &tmp);
		if (r) *val = tmp;
		return r;
	}
};

template<>
struct WidgetOperator<SliderInterface> {

  static bool Put(SliderInterface *sld, int64_t *val){
    sld->blockSignals(true);
    sld->setValue(*val);
    sld->blockSignals(false);
    return true;
  }

  static bool Get(SliderInterface *sld, int64_t *val) {
    *val = sld->value();
    return true;
  }

  static bool Put(SliderInterface *slider, double *val){
    int64_t tmp = *val;
    return Put(slider, &tmp);
  }

  static bool Get(SliderInterface *slider, double *val) {
    int64_t tmp = 0;
    bool r = Get(slider, &tmp);
    if (r) *val = tmp;
    return r;
  }
};

template<>
struct WidgetOperator<QButtonGroup>
{
	static bool Get(QButtonGroup* bg, std::string* selectedName)
	{
		for (QObject* x : bg->buttons())
		{
			auto rb = qobject_cast<QRadioButton*>(x);
			if (rb == nullptr)
				continue;

			if (rb->isChecked())
			{
				*selectedName = rb->objectName().toStdString();
				return true;
			}
		}
		return false;
	}

	static bool Put(QButtonGroup* bg, std::string* selectedName)
	{
		bool hasChecked = false;
		for (QObject* x : bg->buttons())
		{
			auto rb = qobject_cast<QRadioButton*>(x);
			if (rb == nullptr)
				continue;

			if (rb->objectName() == selectedName->c_str())
			{
				rb->setChecked(true);
				hasChecked = true;
			}
			else
				rb->setChecked(false);
		}

		//如果没找到就默认勾选第一个
		if (hasChecked)
			return true;
		else
		{
			bool toSet = true;
			for (QObject* x : bg->buttons())
			{
				auto rb = qobject_cast<QRadioButton*>(x);
				if (rb == nullptr)
					continue;
				rb->setChecked(toSet);
				toSet = false;
			}
			return false;
		}
	}
};

//=======================================================
//          操作函数模板
//=======================================================
template<class QTWidgetT, class DataSetTypeT, class DataTypeT>
bool WidgetToData(DataTypeT reserved, QTWidgetT* widget, DataSetTypeT* data, const char* index)
{
	ConfigType<DataTypeT>::T val;
	if (WidgetOperator<QTWidgetT>::Get(widget, &val) == false)
		return false;

	ConfigOp<DataTypeT>::Put(val, data, index);
	return true;
}

template<class QTWidgetT, class DataSetTypeT, class DataTypeT>
bool DataToWidget(DataTypeT reserved, QTWidgetT* widget, DataSetTypeT* data, const char* index)
{
	ConfigType<DataTypeT>::T val;
	if (ConfigOp<DataTypeT>::Get(&val, data, index) == false)
		return false;
	if (WidgetOperator<QTWidgetT>::Put(widget, &val) == false)
		return false;

	return true;
}

//=======================================================
//          强制实例化所有可能用到的操作函数
//=======================================================
template bool WidgetToData<QLineEdit, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QLineEdit*, config_t*, const char*);
template bool DataToWidget<QLineEdit, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QLineEdit*, config_t*, const char*);

template bool WidgetToData<QLineEdit, config_t, BILI_CONFIG_UINT>(BILI_CONFIG_UINT, QLineEdit*, config_t*, const char*);
template bool DataToWidget<QLineEdit, config_t, BILI_CONFIG_UINT>(BILI_CONFIG_UINT, QLineEdit*, config_t*, const char*);

template bool WidgetToData<QLineEdit, config_t, BILI_CONFIG_DOUBLE>(BILI_CONFIG_DOUBLE, QLineEdit*, config_t*, const char*);
template bool DataToWidget<QLineEdit, config_t, BILI_CONFIG_DOUBLE>(BILI_CONFIG_DOUBLE, QLineEdit*, config_t*, const char*);


template bool WidgetToData<QLineEdit, config_t, BILI_CONFIG_ENCRYPTEDSTRING>(BILI_CONFIG_ENCRYPTEDSTRING, QLineEdit*, config_t*, const char*);
template bool DataToWidget<QLineEdit, config_t, BILI_CONFIG_ENCRYPTEDSTRING>(BILI_CONFIG_ENCRYPTEDSTRING, QLineEdit*, config_t*, const char*);

template bool WidgetToData<QCheckBox, config_t, BILI_CONFIG_BOOL>(BILI_CONFIG_BOOL, QCheckBox*, config_t*, const char*);
template bool DataToWidget<QCheckBox, config_t, BILI_CONFIG_BOOL>(BILI_CONFIG_BOOL, QCheckBox*, config_t*, const char*);

template bool WidgetToData<QComboBox, config_t, BILI_CONFIG_UINT>(BILI_CONFIG_UINT, QComboBox*, config_t*, const char*);
template bool DataToWidget<QComboBox, config_t, BILI_CONFIG_UINT>(BILI_CONFIG_UINT, QComboBox*, config_t*, const char*);

template bool WidgetToData<QComboBox, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QComboBox*, config_t*, const char*);
template bool DataToWidget<QComboBox, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QComboBox*, config_t*, const char*);

template bool WidgetToData<QButtonGroup, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QButtonGroup*, config_t*, const char*);
template bool DataToWidget<QButtonGroup, config_t, BILI_CONFIG_STRING>(BILI_CONFIG_STRING, QButtonGroup*, config_t*, const char*);

template bool WidgetToData<CircleSliderSlider, config_t, BILI_CONFIG_INT>(BILI_CONFIG_INT, CircleSliderSlider*, config_t*, const char*);
template bool DataToWidget<CircleSliderSlider, config_t, BILI_CONFIG_INT>(BILI_CONFIG_INT, CircleSliderSlider*, config_t*, const char*);

template bool WidgetToData<SliderInterface, config_t, BILI_CONFIG_INT>(BILI_CONFIG_INT, SliderInterface*, config_t*, const char*);
template bool DataToWidget<SliderInterface, config_t, BILI_CONFIG_INT>(BILI_CONFIG_INT, SliderInterface*, config_t*, const char*);

//template bool WidgetToData<SpcStrideSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, SpcStrideSlider*, obs_data_t*, const char*);
//template bool DataToWidget<SpcStrideSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, SpcStrideSlider*, obs_data_t*, const char*);

template bool WidgetToData<QLineEdit, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QLineEdit*, obs_data_t*, const char*);
template bool DataToWidget<QLineEdit, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QLineEdit*, obs_data_t*, const char*);

template bool WidgetToData<QComboBox, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QComboBox*, obs_data_t*, const char*);
template bool DataToWidget<QComboBox, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QComboBox*, obs_data_t*, const char*);

template bool WidgetToData<QComboBox, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QComboBox*, obs_data_t*, const char*);
template bool DataToWidget<QComboBox, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QComboBox*, obs_data_t*, const char*);

template bool WidgetToData<QFontComboBox, obs_data_t, BILI_DATA_FONT>(BILI_DATA_FONT, QFontComboBox*, obs_data_t*, const char*);
template bool DataToWidget<QFontComboBox, obs_data_t, BILI_DATA_FONT>(BILI_DATA_FONT, QFontComboBox*, obs_data_t*, const char*);

template bool WidgetToData<QCheckBox, obs_data_t, BILI_DATA_BOOL>(BILI_DATA_BOOL, QCheckBox*, obs_data_t*, const char*);
template bool DataToWidget<QCheckBox, obs_data_t, BILI_DATA_BOOL>(BILI_DATA_BOOL, QCheckBox*, obs_data_t*, const char*);

template bool WidgetToData<QPushButton, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QPushButton*, obs_data_t*, const char*);
template bool DataToWidget<QPushButton, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QPushButton*, obs_data_t*, const char*);

template bool WidgetToData<QPlainTextEdit, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QPlainTextEdit*, obs_data_t*, const char*);
template bool DataToWidget<QPlainTextEdit, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QPlainTextEdit*, obs_data_t*, const char*);

template bool WidgetToData<QSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QSlider*, obs_data_t*, const char*);
template bool DataToWidget<QSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, QSlider*, obs_data_t*, const char*);

template bool WidgetToData<QSlider, obs_data_t, BILI_DATA_DOUBLE>(BILI_DATA_DOUBLE, QSlider*, obs_data_t*, const char*);
template bool DataToWidget<QSlider, obs_data_t, BILI_DATA_DOUBLE>(BILI_DATA_DOUBLE, QSlider*, obs_data_t*, const char*);

template bool WidgetToData<CircleSliderSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, CircleSliderSlider*, obs_data_t*, const char*);
template bool DataToWidget<CircleSliderSlider, obs_data_t, BILI_DATA_INT>(BILI_DATA_INT, CircleSliderSlider*, obs_data_t*, const char*);

template bool WidgetToData<QButtonGroup, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QButtonGroup*, obs_data_t*, const char*);
template bool DataToWidget<QButtonGroup, obs_data_t, BILI_DATA_STRING>(BILI_DATA_STRING, QButtonGroup*, obs_data_t*, const char*);

template bool WidgetToData<CircleSliderSlider, obs_data_t, BILI_DATA_DOUBLE>(BILI_DATA_DOUBLE, CircleSliderSlider*, obs_data_t*, const char*);
template bool DataToWidget<CircleSliderSlider, obs_data_t, BILI_DATA_DOUBLE>(BILI_DATA_DOUBLE, CircleSliderSlider*, obs_data_t*, const char*);


template bool DataToWidget<QComboBox, obs_properties_t, BILI_PROP_LIST_STRING>(BILI_PROP_LIST_STRING, QComboBox*, obs_properties_t*, const char*);
template bool DataToWidget<QComboBox, obs_properties_t, BILI_PROP_LIST_INT>(BILI_PROP_LIST_INT, QComboBox*, obs_properties_t*, const char*);


//=======================================================
//            字符串和二进制数据互相转换
//=======================================================
std::vector<char> BiliStrToBin(const std::string& str)
{
	if (str.empty() == true)
		return std::vector<char>();

	int dataLen = base64_declen(str.length());
	std::vector<char> result;
	result.resize(dataLen);
	int decodedDataLen;
	char* pEnd = (char*)base64_decode((uint8_t*)&str[0], &result[0], &decodedDataLen);
	if (decodedDataLen != 0)
	{
		result.resize(decodedDataLen);
		return std::move(result);
	}
	else
		return std::vector<char>();
}

std::string BiliBinToStr(const std::vector<char>& data)
{
	if (data.empty() == true)
		return std::string();

	int b64strlen = base64_enclen(data.size());
	std::string result;
	result.resize(b64strlen);
	base64_encode(&data[0], data.size(), (uint8_t*)&result[0]);
	return std::move(result);
}
