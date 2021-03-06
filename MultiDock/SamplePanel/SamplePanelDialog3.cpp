// SamplePanelDialog2.cpp : implementation file
//

#include "stdafx.h"
#include "SamplePanel.h"
#include "SamplePanelDialog3.h"
#include "SamplePanelManager.h"

static CSamplePanelDialog3::stRegister s_Register;

// CSamplePanelDialog3 dialog

IMPLEMENT_DYNAMIC(CSamplePanelDialog3, CDialog)

	CSamplePanelDialog3::CSamplePanelDialog3(CWnd* pParent /*=NULL*/)
	: CDialog(CSamplePanelDialog3::IDD, pParent)
	, m_nRadio(FALSE)
{
	m_bPaneClosed = false;
	m_nRadioValue = -1;
}


CSamplePanelDialog3::~CSamplePanelDialog3()
{
}

void CSamplePanelDialog3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nRadio);
}


BEGIN_MESSAGE_MAP(CSamplePanelDialog3, CDialog)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_RECVALUE_VIEWER1_PANE2,OnRecvValueFromView)
	//ON_BN_CLICKED(IDC_BUTTON1, &CSamplePanelDialog3::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSamplePanelDialog3 message handlers

void CSamplePanelDialog3::OnClose()
{
	CWndManager::Instance()->RemoveCreatedWnd(this/*, _T("CSamplePanelDialog3")*/);
	// TODO: Add your message handler code here and/or call default
	m_bPaneClosed = true;
	CDialog::OnClose();
}

void CSamplePanelDialog3::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

BOOL CSamplePanelDialog3::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DestroyWindow();
}

void CSamplePanelDialog3::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::PostNcDestroy();
}


LRESULT CSamplePanelDialog3::OnRecvValueFromView(WPARAM W,LPARAM L)
{
	CString strValue;
	strValue.Format(_T("Receive value  = %d,which from SampleViewer of pane1."),m_nRadioValue);
	MessageBox(strValue,_T("工程二面板三收到消息："));

	m_nRadio = m_nRadioValue;
	UpdateData(FALSE);

	return 0;
}

void CSamplePanelDialog3::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}

CWnd* CSamplePanelDialog3::CreateDlgObj()
{
	return new CSamplePanelDialog3;
}
BOOL CSamplePanelDialog3::CreateWnd(CWnd* pParent, EPANE_ALIGNMENT eDockType, CString strWndName,BOOL bAutoDelete,CRect rect)
{
	USE_CUSTOM_RESOURCE(_T("SamplePanel.dll"));
	__super::Create(IDD, pParent);
	if (eDockType == ALIGN_CHILD_WITH_TITLE)
	{
		return TRUE;
	}
	else if (eDockType == ALIGN_CHILD_WITH_NO_TITLE)
	{
		ModifyStyle(WS_CAPTION|WS_BORDER|WS_SIZEBOX,0);
		return TRUE;
	}
	else
	{
		ModifyStyle(WS_BORDER|WS_CAPTION|WS_SIZEBOX,0);
		return CSamplePanelManager::Instance()->RegisterDockPane(this, strWndName.GetString(),eDockType, bAutoDelete,rect);
	}
}