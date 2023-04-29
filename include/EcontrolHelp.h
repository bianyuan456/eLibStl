//组件库需用通用函数引入此头文件
#include "ElibHelp.h"
#include <CommCtrl.h>

#include <unordered_map>

ESTL_NAMESPACE_BEGIN

std::vector<unsigned char> GetDataFromHBIT(HBITMAP hBitmap);

std::wstring MyInputBox(const std::wstring& title);

/// <summary>
/// 发送通用事件。
/// 向易语言通知控件通用事件的产生
/// </summary>
eStlInline bool SendToParentsHwnd(DWORD m_dwWinFormID, DWORD m_dwUnitID, INT uMsg, WPARAM wParam, LPARAM lParam) {

	if (uMsg == WM_SETFOCUS || uMsg == WM_KILLFOCUS || uMsg == WM_MOUSELAST || uMsg >= WM_MOUSEMOVE 
		&& uMsg <= WM_RBUTTONUP || uMsg >= WM_KEYDOWN && uMsg <= WM_CHAR)
	{
		//这几个事件全部转发给父组件
		EVENT_NOTIFY2 event(m_dwWinFormID, m_dwUnitID, 0);
		INT control_type = 0;
		if (uMsg != WM_CHAR && uMsg != WM_SETFOCUS && uMsg != WM_KILLFOCUS) 
		{
			if ((GetKeyState(VK_CONTROL) & 16384) != 0) 
				control_type = 1;
			if ((GetKeyState(VK_SHIFT) & 16384) != 0) 
				control_type = control_type | 2;
			if ((GetKeyState(VK_MENU) & 16384) != 0) 
				control_type = control_type | 4;
		}

		if (uMsg >= WM_MOUSEMOVE && uMsg <= WM_RBUTTONUP)
		{
			if (uMsg == WM_MOUSEMOVE)
				event.m_nEventIndex = -6;
			else
				event.m_nEventIndex = 512 - uMsg;
			event.m_nArgCount = 3;
			event.m_arg[0].m_inf.m_int = lParam & 65535;
			event.m_arg[1].m_inf.m_int = lParam >> 16;
			event.m_arg[2].m_inf.m_int = control_type;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			return true;
		}

		switch (uMsg)
		{
		case WM_SETFOCUS: {
			event.m_nEventIndex = -uMsg;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}
		case WM_KILLFOCUS: {
			event.m_nEventIndex = -uMsg;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}
		case WM_KEYDOWN: {
			event.m_nEventIndex = (247 - uMsg);
			event.m_nArgCount = 2;
			event.m_arg[0].m_inf.m_int = wParam;
			event.m_arg[1].m_inf.m_int = control_type;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}
		case WM_KEYUP: {
			event.m_nEventIndex = (247 - uMsg);
			event.m_nArgCount = 2;
			event.m_arg[0].m_inf.m_int = wParam;
			event.m_arg[1].m_inf.m_int = control_type;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}
		case WM_CHAR: {
			event.m_nEventIndex = -11;
			event.m_nArgCount = 1;
			event.m_arg[0].m_inf.m_int = wParam;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}case WM_MOUSELAST: {
			event.m_nEventIndex = 12;
			event.m_nArgCount = 2;
			event.m_arg[0].m_inf.m_int = (wParam >> 16) > 0 ? 1 : -1;
			event.m_arg[1].m_inf.m_int = control_type;
			elibstl::NotifySys(NRS_EVENT_NOTIFY2, (DWORD) & event, 0);
			break;
		}
		default:
			break;
		}
	}

	return true;
}

/// <summary>
/// 克隆Unicode字符串。
/// 内存分配释放由new/delete[]进行
/// </summary>
/// <param name="pszDst">目标字符串，如果不为NULL则函数会将其释放</param>
/// <param name="pszSrc">源字符串</param>
/// <param name="cchSrc">字符数</param>
/// <returns>复制的字符数</returns>
int DupStringForNewDeleteW(PWSTR& pszDst, PCWSTR pszSrc, int cchSrc = 0);

/// <summary>
/// 克隆ANSI字符串。
/// 内存分配释放由new/delete[]进行
/// </summary>
/// <param name="pszDst">目标字符串，如果不为NULL则函数会将其释放</param>
/// <param name="pszSrc">源字符串</param>
/// <param name="cchSrc">字符数</param>
/// <returns>复制的字符数</returns>
int DupStringForNewDeleteA(PSTR& pszDst, PCSTR pszSrc, int cchSrc = 0);

/// <summary>
/// 取默认字体
/// </summary>
/// <param name="hWnd">控件窗口句柄</param>
/// <returns>默认字体LOGFONTA结构</returns>
eStlInline LOGFONTA GetEDefLOGFONT(HWND hWnd)
{
	LOGFONTA lf{};
	HDC hDC = GetDC(hWnd);
	lf.lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	strcpy(lf.lfFaceName, "宋体");
	lf.lfCharSet = GB2312_CHARSET;
	ReleaseDC(hWnd, hDC);
	return lf;
}

/// <summary>
/// 控件实用类：子类化管理器
/// </summary>
/// <typeparam name="TC">控件类，必须实现GetHWND()方法，返回控件窗口句柄</typeparam>
template<class TC>
class CSubclassMgr
{
private:
	using TCtrlInfo = std::unordered_map<HWND, TC*>;
	using TParentInfo = std::unordered_map<HWND, int>;

	TParentInfo& m_ParentInfo;
	TCtrlInfo& m_CtrlInfo;
	SUBCLASSPROC m_pfnParentSubclass;
	SUBCLASSPROC m_pfnCtrlSubclass;
	UINT_PTR m_uIDParent;
	UINT_PTR m_uIDCtrl;
public:
	/// <summary>
	/// 默认构造：弃置
	/// </summary>
	CSubclassMgr() = delete;

	/// <summary>
	/// 构造
	/// </summary>
	/// <param name="ParentInfo">父窗口引用计数哈希表</param>
	/// <param name="CtrlInfo">控件到控件类哈希表</param>
	/// <param name="pfnParentSubclass">父窗口子类化过程</param>
	/// <param name="pfnCtrlSubclass">控件子类化过程</param>
	/// <param name="uIDParent">父窗口子类化ID</param>
	/// <param name="uIDCtrl">控件子类化ID</param>
	CSubclassMgr(TParentInfo& ParentInfo, TCtrlInfo& CtrlInfo,
		SUBCLASSPROC pfnParentSubclass, SUBCLASSPROC pfnCtrlSubclass, UINT_PTR uIDParent, UINT_PTR uIDCtrl) :
		m_ParentInfo(ParentInfo), m_CtrlInfo(CtrlInfo),
		m_pfnParentSubclass(pfnParentSubclass), m_pfnCtrlSubclass(pfnCtrlSubclass), m_uIDParent(uIDParent), m_uIDCtrl(uIDCtrl) {}

	~CSubclassMgr() = default;

	/// <summary>
	/// 控件已创建
	/// </summary>
	/// <param name="pClass">控件类</param>
	void OnCtrlCreate(TC* pClass)
	{
		HWND hWnd = pClass->GetHWND();
		HWND hParent = GetParent(hWnd);
		m_CtrlInfo[hWnd] = pClass;

		SetWindowSubclass(hWnd, m_pfnCtrlSubclass, m_uIDCtrl, (DWORD_PTR)pClass);

		if (!m_ParentInfo.count(hParent))
		{
			SetWindowSubclass(hParent, m_pfnParentSubclass, m_uIDParent, 0);
			m_ParentInfo[hParent] = 1;
		}
		else
			m_ParentInfo[hParent]++;
	}

	/// <summary>
	/// 控件将被销毁
	/// </summary>
	/// <param name="pClass">控件类</param>
	void OnCtrlDestroy(TC* pClass)
	{
		HWND hWnd = pClass->GetHWND();
		HWND hParent = GetParent(hWnd);
		if (m_ParentInfo.count(hParent))
		{
			auto& v = m_ParentInfo[hParent];
			v--;
			if (v <= 0)
			{
				RemoveWindowSubclass(hParent, m_pfnParentSubclass, m_uIDParent);
				m_ParentInfo.erase(hParent);
			}
		}

		if (m_CtrlInfo.count(hWnd))
			m_CtrlInfo.erase(hWnd);
	}

	/// <summary>
	/// 父窗口将被销毁
	/// </summary>
	/// <param name="hParent"></param>
	void OnParentDestroy(HWND hParent)
	{
		RemoveWindowSubclass(hParent, m_pfnParentSubclass, m_uIDParent);
		if (m_ParentInfo.count(hParent))
			m_ParentInfo.erase(hParent);
	}
};

eStlInline DWORD ModifyWindowStyle(HWND hWnd, DWORD dwNew, DWORD dwMask = 0u, BOOL bAutoMaskNew = FALSE, int idx = GWL_STYLE)
{
	if (bAutoMaskNew)
		dwMask |= dwNew;
	DWORD dwStyle = GetWindowLongPtrW(hWnd, idx);
	dwStyle &= ~dwMask;
	dwStyle |= dwNew;
	SetWindowLongPtrW(hWnd, idx, dwStyle);
	return dwStyle;
}

template<class...T>
eStlInline int MultiSelectWndStyle(HWND hWnd, int idx, T...dwStyle)
{
	int c = sizeof...(dwStyle);
	for (int i = 0; i < c; ++i)
	{
		if (GetWindowLongPtrW(hWnd, idx) & ESTLPRIV_MultiSelectHelp___(i, dwStyle...))
			return i;
	}
	return -1;
}

template<class U, class...T>
eStlInline int MultiSelectEqual(U i, T...j)
{
	int c = sizeof...(j);
	for (int i = 0; i < c; ++i)
	{
		if (i == ESTLPRIV_MultiSelectHelp___(i, j...))
			return i;
	}
	return -1;
}
ESTL_NAMESPACE_END

struct EFONTDATA
{
	HFONT m_hFont;
	LOGFONTA m_data;
	EFONTDATA()
	{
		memset(this, 0, sizeof(EFONTDATA));
	}
};

// 创建控件接口参数定义
#define STD_ECTRL_CREATE_ARGS \
	LPVOID pAllData, int cbData, DWORD dwStyle, int x, int y, int cx, int cy, \
	HWND hParent, UINT nID, BOOL blInDesignMode, DWORD dwWinFormID, DWORD dwUnitID
// 创建控件接口参数
#define STD_ECTRL_CREATE_REAL_ARGS \
	pAllData, cbData, dwStyle, x, y, cx, cy, \
	hParent, nID, blInDesignMode, dwWinFormID, dwUnitID
// 易语言创建控件接口参数定义，跟上面的略有不同
#define STD_EINTF_CREATE_ARGS \
	LPBYTE pAllData, INT cbData, DWORD dwStyle, HWND hParent, UINT nID, \
	HMENU hMenu, INT x, INT y, INT cx, INT cy, DWORD dwWinFormID, DWORD dwUnitID, HWND hDesignWnd, BOOL blInDesignMode
// 声明子类化管理器所需成员
#define SUBCLASS_MGR_DECL(Class) \
	public: \
		static std::unordered_map<HWND, Class*> m_CtrlSCInfo; \
		static std::unordered_map<HWND, int> m_ParentSCInfo; \
	private: \
		static elibstl::CSubclassMgr<Class> m_SM;
// 初始化子类化管理器所需成员
#define SUBCLASS_MGR_INIT(Class, uIDParent, uIDCtrl) \
	std::unordered_map<HWND, Class*> Class::m_CtrlSCInfo{}; \
	std::unordered_map<HWND, int> Class::m_ParentSCInfo{}; \
	elibstl::CSubclassMgr<Class> Class::m_SM{ m_ParentSCInfo,m_CtrlSCInfo,ParentSubclassProc,CtrlSubclassProc,uIDParent,uIDCtrl };
// 父窗口子类化过程函数头
#define SUBCLASS_PARENT_FNHEAD \
	static LRESULT CALLBACK ParentSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
// 控件子类化过程函数头
#define SUBCLASS_CTRL_FNHEAD \
	static LRESULT CALLBACK CtrlSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
#define SUBCLASS_RET_DEFPROC \
	return DefSubclassProc(hWnd, uMsg, wParam, lParam)