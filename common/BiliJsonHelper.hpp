#ifndef BILIJSONHELPER_H
#define BILIJSONHELPER_H

#include <initializer_list>
#include <exception>
#include <memory>
#include <string>

#include "bili-lexicalcast.hpp"
#include "../third_party/jansson/include/jansson.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma region(JSON操作相关)


struct ThrowOnJsonError{};
struct NoThrowOnJsonError{};

//=======================================================
//                 JSON指针释放工具类
//=======================================================
struct JsonFreeT
{
	void operator()(json_t*& json) const
	{
		if (json != 0)
		{
			json_decref(json);
			json = 0;
		}
	}
};

typedef std::unique_ptr<json_t, JsonFreeT> JsonPtr;


//=======================================================
//                 JSON访问异常
//=======================================================
class JsonDataError {};
class JsonNoPath : public JsonDataError {};
class JsonWrongType : public JsonDataError {};

class JsonParseError : public JsonDataError, public json_error_t
{
	template<bool x>
	class StaticAssert;

	template<>
	class StaticAssert < true > {};
public:
	JsonParseError(const json_error_t& x)
	{
		StaticAssert<sizeof(*this) == sizeof(*&x)> staticAssert;
		staticAssert;

		const json_error_t* px = &x;
		*this = *reinterpret_cast<const JsonParseError*>(px);
	}
};

//=======================================================
//                 JSON路径查找
//=======================================================
inline json_t* GetPathNode(json_t* rootNode, const std::initializer_list<const char*>& args)
{
	json_t* curNode(rootNode);
	for (auto& path : args)
	{
		curNode = json_object_get(curNode, path);
		if (curNode == 0)
		{
			return 0;
		}
	}

	return curNode;
}

//=======================================================
//                 JSON不同类型数据获取
//=======================================================

template<int T>
struct JsonValueType;

template<int T>
struct JsonValueTypeBase
{
	static bool CheckType(json_t* obj)
	{
		return json_typeof(obj) == T;
	}
};

template<>
struct JsonValueType<JSON_OBJECT> : public JsonValueTypeBase < JSON_OBJECT >
{
	typedef json_t* T;
	static T Get(json_t* obj)
	{
		if (CheckType(obj) == false)
		{
			throw JsonWrongType();
		}
		return obj;
	}
};

template<>
struct JsonValueType<JSON_REAL> : public JsonValueTypeBase < JSON_REAL >
{
	typedef double T;
	static T Get(json_t* obj)
	{
		if (CheckType(obj) == false)
			throw JsonWrongType();
		return json_real_value(obj);
	}
};

template<>
struct JsonValueType<JSON_INTEGER> : public JsonValueTypeBase < JSON_INTEGER >
{
	typedef int64_t T;
	static T Get(json_t* obj)
	{
		if (CheckType(obj) == false)
		{
			double realVal = JsonValueType<JSON_REAL>::Get(obj);
			if (std::fabs(realVal - int64_t(realVal)) < 1e-6)
			{
				if (int64_t(realVal) != 0)
					return int64_t(realVal);
				else if (realVal == 0.0)
					return 0;
				else
					throw JsonWrongType();
			}
		}
		return json_integer_value(obj);
	}
};

template<>
struct JsonValueType<JSON_STRING> : public JsonValueTypeBase<JSON_STRING>
{
	typedef std::string T;
	static T Get(json_t* obj)
	{
		if (CheckType(obj) == false)
		{
			if (JsonValueType<JSON_INTEGER>::CheckType(obj))
				return lexical_cast<T>(JsonValueType<JSON_INTEGER>::Get(obj));
			else if (JsonValueType<JSON_REAL>::CheckType(obj))
				return lexical_cast<T>(JsonValueType<JSON_REAL>::Get(obj));
			else
				throw JsonWrongType();
		}
		return std::string(json_string_value(obj));
	}
};

class JsonArray
{
	JsonPtr arrayNode;
public:
	int GetSize()
	{
		return json_array_size(arrayNode.get());
	}

	template<int T>
	typename JsonValueType<T>::T GetVal(int index)
	{
		if (arrayNode.get() == 0)
			throw JsonNoPath();

		json_t* elem = json_array_get(arrayNode.get(), index);
		if (elem == 0)
			throw JsonNoPath();
		return JsonValueType<T>().Get(elem);
	}

	JsonArray(json_t* obj) : arrayNode(obj)
	{
		//因为要用智能指针存一份在自己这边
		//而这种智能指针在构造的时候并不会添加引用
		json_incref(obj);
	}
};

typedef std::unique_ptr<JsonArray> JsonArrayPtr;

template<>
struct JsonValueType<JSON_ARRAY> : public JsonValueTypeBase<JSON_ARRAY>
{
	typedef JsonArrayPtr T;

	static T Get(json_t* obj)
	{
		if (CheckType(obj) == false)
			throw JsonWrongType();
		return JsonArrayPtr(new JsonArray(obj));
	}
};

//=======================================================
//                 JSON主类
//=======================================================
struct BiliJson
{
private:
	std::string jsonData;
	JsonPtr json;
public:
	BiliJson(const char* jsonString)
		: BiliJson(jsonString, true)
	{
	}

	BiliJson(const char* jsonString, bool hasJsonString)
	{
		if (hasJsonString)
			jsonData = jsonString;

		json_error_t jsonErr;
		json.reset(json_loads(jsonData.c_str(), 0, &jsonErr));
		if (json.get() == 0)
			throw JsonParseError(jsonErr);
	}

	BiliJson(json_t* jsonObj)
	{
		json.reset(jsonObj);
	}

	//获取指定路径上的值
	template<int T>
	typename JsonValueType<T>::T GetVal(const std::initializer_list<const char*>& args)
	{
		JsonPtr node = GetNode(args);
		if (node.get() == 0)
			throw JsonNoPath();
		return JsonValueType<T>().Get(node.get());
	}

	//获取指定路径的节点
	JsonPtr GetNode(const std::initializer_list<const char*>& args)
	{
		json_t* node = GetPathNode(json.get(), args);
		if (node == 0)
			return 0;
		else
		{
			json_incref(node);
			return JsonPtr(node);
		}
	}

	//获取指定路径上的值，不会抛异常的版本
	template<int T>
	bool TryGetVal(typename JsonValueType<T>::T* val, const std::initializer_list<const char*>& args)
	{
		JsonPtr node = GetNode(args);
		if (node.get() == 0)
			return false;
		else
		{
			if (val != 0)
			{
				if (JsonValueType<T>::CheckType(node.get()) == false)
					return false;
				*val = JsonValueType<T>().Get(node.get());
			}
			return true;
		}
	}
};

typedef std::auto_ptr<BiliJson> BiliJsonPtr;

#pragma endregion
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
