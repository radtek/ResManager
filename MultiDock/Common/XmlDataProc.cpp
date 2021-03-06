#include "stdafx.h"
#include "XmlDataProc.h"
#include "FileHelper.h"
#include "XmlConfig.h"

CXmlDataProc* CXmlDataProc::m_pInstance = NULL;


CXmlDataProc::CXmlDataProc()
{
	Initialize();
}

CXmlDataProc::~CXmlDataProc()
{
}

CXmlDataProc* CXmlDataProc::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new CXmlDataProc;
	}

	return m_pInstance;
}

void CXmlDataProc::DestroyInstance()
{
	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}


BOOL CXmlDataProc::Initialize()
{
	UINT nDllCount = AppXml()->GetAttributeInt(_T("DllCount"), 0);
	if (nDllCount > 0)
	{
		//read dll names.
		for (int i=0; i<nDllCount; i++)
		{
			CString strNode;
			strNode.Format(_T("Dll_%d\\DllName"), i);
			std::wstring strDllName = AppXml()->GetAttributeText(strNode.GetString(), _T(""));
			if (!strDllName.empty() && !IsDllAdded(CString(strDllName.c_str())))
			{
				UINT nCount = m_mapDll2Classnames.size();

				//1.read class names.
				CString strNode;
				strNode.Format(_T("Dll_%d\\OtherPaneClass\\Count"), i);
				int nClassCount = AppXml()->GetAttributeInt(strNode,0);
				if (nClassCount>0)
				{
					ListClasses listAllClasses;
					for (int j=0; j<nClassCount; j++)
					{
						strNode.Format(_T("Dll_%d\\OtherPaneClass\\Class_%d"), i, j);
						wstring wstrClassName = AppXml()->GetAttributeText(strNode, _T(""));
						if (!wstrClassName.empty())
						{
							listAllClasses.push_back(wstrClassName.c_str());
						}
					}

					m_mapDll2Classnames.insert(make_pair(strDllName.c_str(), listAllClasses));
				}


				//2.read child frame+view class name
				strNode.Format(_T("Dll_%d\\FrameWndClassName"), i);
				std::wstring strFramename = AppXml()->GetAttributeText(strNode.GetString(), _T(""));
				strNode.Format(_T("Dll_%d\\ViewClassName"), i);
				std::wstring strViewname  = AppXml()->GetAttributeText(strNode.GetString(), _T(""));
				if (!strFramename.empty() && !strViewname.empty())
				{
					AddFrameViewClassName(strDllName.c_str(),strFramename.c_str(), strViewname.c_str());
				}

				//3.Read workspace.
				ProcessFloatType(i, strDllName.c_str());
				ProcessDockType(i, strDllName.c_str());
				ProcessChildType(i, strDllName.c_str());

			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

UINT CXmlDataProc::GetDllIndex(CString& strDll)
{
	int nIndex=0;
	for (MapDllname2Classes::iterator it = m_mapDll2Classnames.begin();
		it != m_mapDll2Classnames.end(); ++it)
	{
		if (strDll.CompareNoCase(it->first) == 0)
		{
			break;
		}
		nIndex++;
	}

	return nIndex;
}

BOOL CXmlDataProc::GetDllNames(vector<CString>& vecDlls)
{
	vecDlls.clear();

	for (MapDllname2Classes::iterator it = m_mapDll2Classnames.begin();
		it != m_mapDll2Classnames.end(); ++it)
	{
		vecDlls.push_back(it->first);
	}

	return vecDlls.size()>0;
}

BOOL CXmlDataProc::GetClassNames(CString& strDll, vector<CString>& vecClasses)
{
	vecClasses.clear();
	
	MapDllname2Classes::iterator itFind = m_mapDll2Classnames.find(strDll);
	if (itFind != m_mapDll2Classnames.end())
	{
		ListClasses& listofClassName = itFind->second;
		for (ListClasses::iterator it = listofClassName.begin(); 
			it != listofClassName.end(); ++it)
		{
			CString& strClass = *it;
			//we read NULL from xml and cache , but we do not show it.
			if (strClass.CompareNoCase(_T("NULL")) != 0)
			{
				vecClasses.push_back(strClass);
			}
		}
	}
	
	return vecClasses.size() > 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL CXmlDataProc::IsClassNameAdded(CString & strDll, CString& strClassname)
{
	MapDllname2Classes::iterator itFind = m_mapDll2Classnames.find(strDll);
	if (itFind != m_mapDll2Classnames.end())
	{
		ListClasses& listofClasses = itFind->second;
		for(ListClasses::iterator it = listofClasses.begin(); it != listofClasses.end(); ++it)
		{
			CString& strClass = *it;
			if (strClass.CompareNoCase(strClassname) == 0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
BOOL CXmlDataProc::IsDllAdded(CString& strDllName)
{
	MapDllname2Classes::iterator itFind = m_mapDll2Classnames.find(strDllName);
	return itFind != m_mapDll2Classnames.end();
}

//一个float窗口一个stFloatWnd结构，如果classname，就在xml中写两个，他们的CRect可能不同。
void CXmlDataProc::ProcessFloatType(int nDllIndex, CString strDllname)
{
	CString strClassCountNode;
	strClassCountNode.Format(_T("Dll_%d\\FLOAT_GROUP\\WndCount"),nDllIndex);
	int nClassCount = AppXml()->GetAttributeInt(strClassCountNode.GetString(), 0);
	if (nClassCount>0)
	{
		for (int i=0; i<nClassCount; i++)
		{
			CString strnode;
			strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strnode, _T(""));
			if (!wsClassname.empty())
			{
				strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\WndName"), nDllIndex, i);
				wstring wsWndName = AppXml()->GetAttributeText(strnode, _T(""));

				CRect rcWnd;
				strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\SIZE\\left"), nDllIndex ,i);
				rcWnd.left = AppXml()->GetAttributeInt(strnode, 0);
				strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\SIZE\\top"), nDllIndex,i);
				rcWnd.top  = AppXml()->GetAttributeInt(strnode, 0);
				strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\SIZE\\right"), nDllIndex,i);
				rcWnd.right  = AppXml()->GetAttributeInt(strnode, 0);
				strnode.Format(_T("Dll_%d\\FLOAT_GROUP\\Wnd_%d\\SIZE\\bottom"), nDllIndex,i);
				rcWnd.bottom  = AppXml()->GetAttributeInt(strnode, 0);
				
				stFloatWnd oneItem;
				oneItem.strClass = wsClassname.c_str();
				oneItem.strWndName = wsWndName.c_str();
				oneItem.rcWnd = rcWnd;
				oneItem.strDllname = strDllname;

				m_listFloatWnds.push_back(oneItem);
			}
		}
	}
}

//每个dock实例在xml中写入一项。即使同类名的多个实例，也需要写多个xml项目。
//实例化的时候，各个方向相互dock，根据rect的大小，如果相同，就是一组，不同，即表示不是一组。
void CXmlDataProc::ProcessDockType(int nDllIndex, CString strDllname)
{
	//cache left dock panes
	CString strNode;
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\WndCount"), nDllIndex);
	int nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);
		

			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_LEFT;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}
		
	}

	//cache left group
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\LEFT_GROUP\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_LEFT_GROUP;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}

	//cache right dock panes.
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_RIGHT;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}

	//Cache right group 
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\RIGHT_GROUP\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_RIGHT_GROUP;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}

	//cache top dock panes.
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));


			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_TOP;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}

	//CACHE TOP GROUP
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));


			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\TOP_GROUP\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_TOP_GROUP;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}

	//Cache bottom dock panes.
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_BOTTON;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}
	}

	//CACHE BOTTOM GROUP 
	strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\WndCount"), nDllIndex);
	nWndCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nWndCount>0)
	{
		for (int i=0; i<nWndCount; i++)
		{
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\Name"), nDllIndex, i);
			wstring wsClassname = AppXml()->GetAttributeText(strNode, _T(""));

			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\WndName"), nDllIndex, i);
			wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcWnd;
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\SIZE\\left"), nDllIndex, i);
			rcWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\SIZE\\right"), nDllIndex, i);
			rcWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\SIZE\\top"), nDllIndex, i);
			rcWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\DOCK_GROUP\\BOTTOM_GROUP\\Wnd_%d\\SIZE\\bottom"), nDllIndex, i);
			rcWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);


			stDockWnd oneDock;
			oneDock.eDockType = ALIGN_BOTTOM_GROUP;
			oneDock.rcWnd = rcWnd;
			oneDock.strClass = wsClassname.c_str();
			oneDock.strWndName = wsWndName.c_str();
			oneDock.strDllname = strDllname;
			m_listDockWnds.push_back(oneDock);
		}

	}
}

//有几个嵌套，就写几个group,即，嵌套的嵌套，但有个问题，对于子窗口的子窗口，如何找他的parent呢？
//因为我们的xml里面记录的是类名信息，没有实例信息。
//场景：在一个view里面的child都创建后了，现在创建group_1的嵌套子窗口的子窗口。那么对于这个子窗口。如果对于view
//里面已经存在的多个同类名的实例，具体该找哪个是父窗口呢？
//===>通过判断该child的parent的rcWnd来判断。如果当前已经有多个改父类的实例了，那么就判断rWnd值是否一致。
//但有个前提！这些多实例的父窗口必须是view的子窗口层次的。
//建议：一般只有一个group：view里面放一些二级的child子窗口，不牵扯到三级的child子窗口。
void CXmlDataProc::ProcessChildType(int nDllIndex, CString strDllname)
{
	CString strNode;
	strNode.Format(_T("Dll_%d\\CHILD_GROUP\\GroupCount"), nDllIndex);
	int nGroupCount = AppXml()->GetAttributeInt(strNode, 0);
	if (nGroupCount>0)
	{
		for (int i=0; i<nGroupCount; i++)
		{

			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ParentWnd\\ClassName"), nDllIndex, i);
			wstring wsParentClassname = AppXml()->GetAttributeText(strNode, _T(""));

			CRect rcParentWnd;
			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ParentWnd\\left"), nDllIndex, i);
			rcParentWnd.left = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ParentWnd\\right"), nDllIndex, i);
			rcParentWnd.right = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ParentWnd\\top"), nDllIndex, i);
			rcParentWnd.top = AppXml()->GetAttributeInt(strNode, 0);
			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ParentWnd\\bottom"), nDllIndex, i);
			rcParentWnd.bottom = AppXml()->GetAttributeInt(strNode, 0);

			//read child item;
			//所有的child的父窗口和rect相同。表示是一组的。
			strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\ChildCount"), nDllIndex, i);
			int nChildCount = AppXml()->GetAttributeInt(strNode, 0);
			if (nChildCount>0)
			{
				for (int j=0; j<nChildCount;j++)
				{
					strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\ClassName"), nDllIndex, i, j);
					wstring wsChildClass = AppXml()->GetAttributeText(strNode, _T(""));
					if (!wsChildClass.empty())
					{
						//wnd name
						strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\WndName"), nDllIndex, i, j);
						wstring wsWndName = AppXml()->GetAttributeText(strNode, _T(""));


						CRect rcChild;
						strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\left"), nDllIndex, i, j);
						rcChild.left = AppXml()->GetAttributeInt(strNode, 0);
						strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\right"), nDllIndex, i, j);
						rcChild.right = AppXml()->GetAttributeInt(strNode, 0);
						strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\top"), nDllIndex, i, j);
						rcChild.top = AppXml()->GetAttributeInt(strNode, 0);
						strNode.Format(_T("Dll_%d\\CHILD_GROUP\\Group_%d\\ChildWnds\\Child_%d\\bottom"), nDllIndex, i, j);
						rcChild.bottom = AppXml()->GetAttributeInt(strNode, 0);
					
						map<UINT, ListOneDllChildWnds>::iterator itFind = m_mapDllIndex2Childlist.find(nDllIndex);
						if (itFind != m_mapDllIndex2Childlist.end())
						{
							ListOneDllChildWnds& listAllChild = itFind->second;
							
							stChildWndObj oneChild;
							oneChild.strParentClass = wsParentClassname.c_str();
							oneChild.strChildClass  = wsChildClass.c_str();
							oneChild.strWndName     = wsWndName.c_str();
							oneChild.rcParent = rcParentWnd;
							oneChild.rcChild  = rcChild;
							oneChild.strDllname = strDllname;
							listAllChild.push_back(oneChild);
						}
						else
						{
							ListOneDllChildWnds listAllChilds;
							stChildWndObj oneChild;
							oneChild.strParentClass = wsParentClassname.c_str();
							oneChild.strChildClass  = wsChildClass.c_str();
							oneChild.strWndName     = wsWndName.c_str();
							oneChild.rcParent = rcParentWnd;
							oneChild.rcChild  = rcChild;
							oneChild.strDllname = strDllname;
							listAllChilds.push_back(oneChild);

							m_mapDllIndex2Childlist.insert(make_pair(nDllIndex, listAllChilds));
						}	
					}
				}
			}
		}

	}
}
//void CXmlDataProc::AddClassName(CString& strDll, CString& strClassName, UINT nIndex)
//{
//	////we add 'null' class type but do not show in ui. 
//	//if (strClassName.CompareNoCase(_T("NULL")) == 0)
//	//{
//	//	return;
//	//}
//
//	MapDllname2Classes::iterator itFind = m_mapDll2Classnames.find(strDll);
//	if (itFind != m_mapDll2Classnames.end())
//	{
//		ListClasses& listofClasses = itFind->second;
//		listofClasses.push_back(strClassName);
//		return;
//	}
//
//	ListClasses listofclasses;
//	listofclasses.push_back(strClassName);
//	m_mapDll2Classnames.insert(make_pair(strDll, listofclasses));
//}
void CXmlDataProc::AddFrameViewClassName(LPCTSTR strDll, LPCTSTR strFrame, LPCTSTR strView)
{
	MapDllFrameView::iterator itFind = m_mapDll2FrameView.find(strDll);
	if (itFind != m_mapDll2FrameView.end())
	{
		return;
	}

	stDllFrameView oneItem;
	oneItem.strFrameClassName = strFrame;
	oneItem.strViewClassName  = strView;

	m_mapDll2FrameView.insert(make_pair(strDll, oneItem));
}

BOOL CXmlDataProc::IsFrameViewBelongToProj(CString& strDll, CString& strClassname)
{
	MapDllFrameView::iterator itFind = m_mapDll2FrameView.find(strDll);
	if (itFind != m_mapDll2FrameView.end())
	{
		stDllFrameView& oneItem = itFind->second;
		if (strClassname.CompareNoCase(oneItem.strViewClassName) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CXmlDataProc::IsFrameViewClass(CString& strClassName)
{
	////hard code for mainframe class check.
	//if (strClassName.CompareNoCase(_T("CMainFrame")) == 0)
	//{
	//	return TRUE;
	//}

	for (MapDllFrameView::iterator it = m_mapDll2FrameView.begin();
		it != m_mapDll2FrameView.end(); ++it)
	{
		stDllFrameView& oneItem = it->second;
		if (strClassName.CompareNoCase(oneItem.strFrameClassName) == 0||
			strClassName.CompareNoCase(oneItem.strViewClassName) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//We assumpt that frame+view class names are different in each dll project.
BOOL CXmlDataProc::IsFrameViewLoaded(CString& strClassName)
{
	for (MapDllFrameView::iterator it = m_mapDll2FrameView.begin();
			it != m_mapDll2FrameView.end(); ++it)
	{
		stDllFrameView& oneItem = it->second;
		if (strClassName.CompareNoCase(oneItem.strFrameClassName) == 0||
			strClassName.CompareNoCase(oneItem.strViewClassName) == 0)
		{
			return oneItem.bLoaded;
		}
	}

	return FALSE;
}

BOOL CXmlDataProc::SetFrameViewLoadFlag(CString& strClassName)
{
	for (MapDllFrameView::iterator it = m_mapDll2FrameView.begin();
			it != m_mapDll2FrameView.end(); ++it)
	{
		stDllFrameView& oneItem = it->second;
		if (strClassName.CompareNoCase(oneItem.strFrameClassName) == 0||
			strClassName.CompareNoCase(oneItem.strViewClassName) == 0)
		{
			oneItem.bLoaded = TRUE;
		}
	}

	return FALSE;
}
