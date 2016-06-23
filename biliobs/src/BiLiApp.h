#ifndef BILIAPP_H
#define BILIAPP_H

#include <QApplication>
#include "util/lexer.h"
#include "util/profiler.h"
#include "obs-app-api.hpp"

class BiLiOBSMainWid;

struct BaseLexer {
	lexer lex;
public:
	inline BaseLexer() {lexer_init(&lex);}
	inline ~BaseLexer() {lexer_free(&lex);}
	operator lexer*() {return &lex;}
};

class BiLiApp : public QApplication {

	Q_OBJECT

public:
	BiLiApp(int &argc, char **argv);
	~BiLiApp();

	bool mInitApp();

	TextLookup mTextLookup;
	inline const char *mGetString(const char *lookupVal) const { return mTextLookup.GetString(lookupVal); }
	inline config_t *mGetGlobalConfig() const {return mGlobalConfig;}
	profiler_name_store_t *mGetProfilerNameStore() const { return mProfilerNameStore; }
	const char *mGetRenderModule() const ;

	const char *mInputAudioSource() const;
	const char *mOutputAudioSource() const;

	BiLiOBSMainWid* mGetMainWindow() const;
	void mLoadTranslator();

private:
	ConfigFile mGlobalConfig;
	profiler_name_store_t          *mProfilerNameStore = nullptr;
	std::string mLocale;
	std::unique_ptr<QTranslator> mTrans;

	BiLiOBSMainWid *mBiLiOBSMainWid;
	bool mOBSInit();
	bool mOBSInitGlobalConfig();
	bool mOBSInitGlobalConfigDefaults();
};

inline BiLiApp *App() {return static_cast<BiLiApp *>(qApp);}
inline const char *Str(const char *lookup) {return App()->mGetString(lookup);}
#define QTStr(lookupVal) QString::fromUtf8(Str(lookupVal))

#endif // BILIAPP_H
