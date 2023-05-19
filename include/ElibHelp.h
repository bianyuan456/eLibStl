#pragma once
#include"elib/lib2.h"
#include"elib/fnshare.h"
#include<vector>
#include<string>
#include <tuple>
#include"..\resource.h"
#include<assert.h>
#include"Tace.hpp"


#define ESTL_NAMESPACE_BEGIN namespace elibstl {
#define ESTL_NAMESPACE_END }

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#define eStlInline __forceinline
#else
#define eStlInline inline
#endif


#define MAKEINTATOMW(i)  (PWSTR)((ULONG_PTR)((WORD)(i)))

typedef LPCVOID PCVOID;

#define va_arg_idx(ap, idx, t) (*(t*)((ap)+_INTSIZEOF(t)*(idx)))

template<class... Args>
eStlInline int ArgsNum(Args&&... args)
{
	return sizeof...(args);
}

#define ESTLVAL(...) ArgsNum(__VA_ARGS__), __VA_ARGS__


eStlInline bool isUnicode(const unsigned char* data, size_t length) {
	// 检查字节序标记 (BOM)
	if (length >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
		// UTF-16LE 字节序，低位字节在前
		return true;
	}
	else if (length >= 2 && data[0] == 0xFE && data[1] == 0xFF) {
		// UTF-16BE 字节序，高位字节在前
		return true;
	}

	// 检查字符对应的字节序列
	for (size_t i = 0; i < length; i += 2) {
		unsigned char highByte = data[i];
		unsigned char lowByte = data[i + 1];

		if ((highByte >= 0xD8 && highByte <= 0xDB) && (lowByte >= 0xDC && lowByte <= 0xDF)) {
			// 高代理项和低代理项字节，合法的 UTF-16 编码
			continue;
		}

		if ((highByte >= 0xD8 && highByte <= 0xDB) || (lowByte >= 0xDC && lowByte <= 0xDF)) {
			// 高代理项或低代理项字节单独出现，非法的 UTF-16 编码
			return false;
		}
	}

	return true;
}

struct FucInfo
{
	CMD_INFO FucDef;
	PFN_EXECUTE_CMD pFuc;
	const char* FucName;
};
#define ESTLFNAME(s) s,#s

extern HMODULE g_elibstl_hModule;

ESTL_NAMESPACE_BEGIN
/// <summary>
/// Unicode到ANSI，使用默认代码页
/// </summary>
/// <param name="pszW">Unicode字符串指针</param>
/// <returns>ANSI字符串指针，应使用delete[]释放</returns>
PSTR W2A(PCWSTR pszW);

/// <summary>
/// ANSI到Unicode，使用默认代码页
/// </summary>
/// <param name="pszA">ANSI字符串指针</param>
/// <returns>Unicode字符串指针，应使用delete[]释放</returns>
PWSTR A2W(PCSTR pszA);

/// <summary>
/// 调用易语言支持库函数
/// </summary>
bool CallElibFunc(const char* elib_name, const char* def_name, PMDATA_INF pRetData, INT nArgCount, PMDATA_INF pArgInf);

/// <summary>
/// 多项选择。
/// 第一个参数指示从0开始的索引。
/// </summary>
template <class T>
eStlInline T MultiSelect(int n, ...)
{
	assert(n >= 0);
	va_list Args;
	va_start(Args, n);
	T Ret = va_arg_idx(Args, n, T);
	va_end(Args);
	return Ret;
}
ESTL_NAMESPACE_END