#ifndef __BUILD_BUILDCFG_BUILDCFG_H__
#define __BUILD_BUILDCFG_BUILDCFG_H__

/*

1   打开本工程的主资源文件，删除原有的版本信息，将version.rc添加到工程

2   打开 build/buildcfg 目录下对应的buildcfg.h文件
	找到该工程所在位置，请用英语定义以下三个宏，如：	

#ifdef BUILD_ETDBE // size : 20000
	#pragma comment(linker, "/base:0x4DA00000")   
	#pragma message("\t    ETDBE")
	#define VI_FILEDESCRIPTION	"Data Import"	//文件描述
	#define VI_INTERNALNAME		"dbe"			//内部名称
	#define VI_ORIGINALFILENAME	"etdbe.dll"		//原始DLL文件名
#endif

3.  在 ETDBE 工程设置的 Resources 页的"Preprocessor definitions"中添加本工程对应的宏，如 BUILD_ETDBE
	请注意所有配置(Debug/Release/...)都要加上
	在"Additionalresource include directories"中加上Coding/include目录的相对位置
	如 "../../builder/buildcfg"
	
 */

// -------------------------------------------------------------------------

#ifdef _WINDOWS_
#error "include buildcfg.h before include all other files"
#endif

#ifdef BUILD_BILIOBS_EXE
#pragma message("\t    BILIOBS_EXE")
#define VI_FILEDESCRIPTION	"Biliobs 主模块"
#define VI_INTERNALNAME		"biliobs.exe"
#define VI_ORIGINALFILENAME	"biliobs.exe"
#endif



#ifndef VI_FILEDESCRIPTION
#pragma error("not defined")
#define VI_FILEDESCRIPTION	  "Biliobs Module"
#define VI_INTERNALNAME		  ""
#define VI_ORIGINALFILENAME	  ""
#endif 
#ifndef VI_PRODUCT_NAME
  #define VI_PRODUCT_NAME	"Biliobs"
#endif

#endif /* __BUILD_BUILDCFG_BUILDCFG_H__ */
