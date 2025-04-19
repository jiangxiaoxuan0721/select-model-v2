// DlgLogin.cpp: 实现文件
//

#include "pch.h"
#include "Chat_Client.h"
#include "afxdialogex.h"
#include "DlgLogin.h"
#include <iostream>
#include <fstream>
// DlgLogin 对话框
CString DlgLogin::m_username = _T("");

IMPLEMENT_DYNAMIC(DlgLogin, CDialogEx)
DlgLogin::DlgLogin(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN, pParent){}

DlgLogin::~DlgLogin(){}

void DlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DlgLogin, CDialogEx)
	ON_BN_CLICKED(BTN_SIGNUP, &DlgLogin::OnBnClickedSignup)
	ON_BN_CLICKED(IDC_LOGIN, &DlgLogin::OnBnClickedLogin)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


// DlgLogin 消息处理程序
BOOL DlgLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(RGB(60, 60, 100));
	//设置按钮绘制为自定义
	CButton* pBtn = (CButton*)GetDlgItem(BTN_SIGNUP);
	pBtn->SetButtonStyle(BS_OWNERDRAW);
	pBtn = (CButton*)GetDlgItem(IDC_LOGIN);
	pBtn->SetButtonStyle(BS_OWNERDRAW);

	//用可读写的方式打开"../userinfo.info",若不存在则创建，每行记录形式: "用户名:密码;"
	//读取文件内容并存储在一个 map<string,string> userInfoSet的集合中
	ifstream file("../userinfo.info", ios::in | ios::out | ios::app);
	if (!file) {
		ofstream createFile("../userinfo.info");
		createFile.close();
	}
	else {
		string line;
		while (getline(file, line)) {
			size_t pos = line.find(':');
			if (pos != string::npos) {
				string username = line.substr(0, pos);
				string password = line.substr(pos + 1);
				//处理密码后面的分号
				if (password.back() == ';') {
					password.pop_back();
				}
				userInfoSet.insert(make_pair(username, password));
			}
		}
		file.close();
	}
	return TRUE; 
}

BOOL DlgLogin::PreTranslateMessage(MSG* pMsg){
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		OnBnClickedLogin();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void DlgLogin::OnBnClickedSignup(){
	CString username, password;
	GetDlgItemText(EDT_USERNAME, username);
	GetDlgItemText(EDT_PASSWORD, password);
	if (username.IsEmpty() || password.IsEmpty()) {
		AfxMessageBox(L"用户名或密码不能为空！");
		return;
	}
	// 检查用户名是否已存在
	string user = CT2A(username, CP_UTF8);
	string pass = CT2A(password, CP_UTF8);
	if (userInfoSet.find(user) != userInfoSet.end()) {
		AfxMessageBox(L"用户名已存在，请选择其他用户名！");
		return;
	}
	// 将用户名和密码存储到集合中
	userInfoSet.insert(make_pair(user, pass));
	// 将用户名和密码写入文件
	ofstream file("../userinfo.info", ios::out | ios::app);
	if (file.is_open()) {
		file << user << ":" << pass << ";" << endl;
		file.close();
	}
	else {
		AfxMessageBox(L"无法打开配置文件！");
		return;
	}
	// 显示注册成功消息
	AfxMessageBox(L"注册成功！");
}

void DlgLogin::OnBnClickedLogin(){
	CString username, password;
	GetDlgItemText(EDT_USERNAME, username);
	GetDlgItemText(EDT_PASSWORD, password);
	if (username.IsEmpty() || password.IsEmpty()) {
		AfxMessageBox(L"用户名或密码不能为空！");
		return;
	}
	// 检查用户名和密码是否匹配
	string user = CT2A(username, CP_UTF8);
	string pass = CT2A(password, CP_UTF8);
	if (userInfoSet.find(user) == userInfoSet.end()) {
		AfxMessageBox(L"用户名不存在，请先注册！");
		return;
	}
	// 检查密码是否匹配
	if (userInfoSet[user] != pass) {
		AfxMessageBox(L"密码错误，请重新输入！");
		return;
	}
	// 登录成功，将用户名存储为静态成员
	DlgLogin::m_username = username;
	AfxMessageBox(L"登录成功！");
	OnOK();
}


HBRUSH DlgLogin::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor == CTLCOLOR_BTN || nCtlColor == CTLCOLOR_STATIC|| nCtlColor == CTLCOLOR_EDIT)
	{
		// 设置按钮文本（前景色）为白色
		pDC->SetTextColor(RGB(255, 255, 255));  // 白色文本

		// 设置按钮背景色（背景色）为红色
		pDC->SetBkColor(RGB(60, 60, 100));  // 红色背景

		// 创建一个背景的画刷
		static CBrush brushRed(RGB(60, 60, 100));
		return (HBRUSH)brushRed;  // 返回红色画刷，用于填充背景
	}

	// 对于其他控件，使用默认颜色
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

void DlgLogin::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct){
	if (nIDCtl == BTN_SIGNUP || nIDCtl == IDC_LOGIN) {
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rc = lpDrawItemStruct->rcItem;
		// 绘制按钮的背景
		pDC->FillSolidRect(rc, RGB(80, 80, 120));
		// 绘制按钮的文本设置为白色
		pDC->SetTextColor(RGB(255, 255, 255));
		if (nIDCtl == BTN_SIGNUP) {
			pDC->DrawText(_T("注册"), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else {
			pDC->DrawText(_T("登录"), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		// 绘制按钮的边框
		pDC->Draw3dRect(rc, RGB(255, 255, 255), RGB(255, 255, 255));
		
		// 绘制按钮点击效果
		if (lpDrawItemStruct->itemState & ODS_SELECTED) {
			pDC->Draw3dRect(rc, RGB(255, 255, 255), RGB(255, 255, 255));
			pDC->Draw3dRect(rc, RGB(120, 120, 120), RGB(120, 120, 120));
		}
	}

	CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
