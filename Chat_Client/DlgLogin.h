#pragma once
#include "afxdialogex.h"
#include <map>
#include <string>

// DlgLogin 对话框
using namespace std;
class DlgLogin : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLogin)

public:
	DlgLogin(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgLogin();
	

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	map<string, string> userInfoSet; // 用于存储用户名和密码的集合
	static CString m_username;  // 用于存储用户名
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedSignup();
	afx_msg void OnBnClickedLogin();
	
	static CString GetUsername() {
		return m_username;
	}
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};
