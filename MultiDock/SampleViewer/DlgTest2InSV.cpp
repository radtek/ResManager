// DlgTest2InSV.cpp : implementation file
//

#include "stdafx.h"
#include "SampleViewer.h"
#include "DlgTest2InSV.h"
#include "afxdialogex.h"


static CDlgTest2InSV::stRegister s_Register;

IMPLEMENT_DYNAMIC(CDlgTest2InSV, CDialogEx)

CDlgTest2InSV::CDlgTest2InSV(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTest2InSV::IDD, pParent)
{

}

CDlgTest2InSV::~CDlgTest2InSV()
{
}

void CDlgTest2InSV::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTest2InSV, CDialogEx)
END_MESSAGE_MAP()


// CDlgTest2InSV message handlers

CWnd* CDlgTest2InSV::CreateDlgObj()
{
	return new CDlgTest2InSV;
}
BOOL CDlgTest2InSV::CreateWnd(CWnd* pParent)
{
	return __super::Create(IDD, pParent);
}