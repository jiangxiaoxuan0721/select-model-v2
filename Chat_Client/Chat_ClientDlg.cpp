#include "pch.h"
#include "framework.h"
#include "Chat_Client.h"
#include "Chat_ClientDlg.h"
#include "DlgLogin.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChatClientDlg 对话框
CChatClientDlg::CChatClientDlg(CWnd* pParent)
	: CDialogEx(IDD_CHAT_CLIENT_DIALOG, pParent)
	, m_ip(_T(""))
	, m_sendbuf(_T(""))
	, m_port(_T("")){
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	username = _T("");
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, BTN_CONNECT, c_connect);
	DDX_Control(pDX, BTN_SEND, c_send);
	DDX_Control(pDX, EDT_IP, c_ip);
	DDX_Text(pDX, EDT_IP, m_ip);
	DDX_Control(pDX, EDT_PORT, c_port);
	DDX_Text(pDX, EDT_PORT, m_port);
	DDX_Control(pDX, IDOK, c_quit);
	DDX_Control(pDX, LST_RECVBUF, c_recvbuf);
	DDX_Control(pDX, EDT_SENDBUF, c_sendbuf);
	DDX_Text(pDX, EDT_SENDBUF, m_sendbuf);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_CONNECT, &CChatClientDlg::OnBnClickedConnect)
	ON_BN_CLICKED(BTN_SEND, &CChatClientDlg::OnBnClickedSend)
	ON_MESSAGE(WM_SOCKET, &OnReceiveData)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// CChatClientDlg 消息处理程序
BOOL CChatClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetBackgroundColor(RGB(100, 100, 140),1);	
	// 设置按钮为自定义
	CButton* pBtn = (CButton*)GetDlgItem(BTN_CONNECT);
	pBtn->SetButtonStyle(BS_OWNERDRAW);
	pBtn = (CButton*)GetDlgItem(BTN_SEND);
	pBtn->SetButtonStyle(BS_OWNERDRAW);
	pBtn = (CButton*)GetDlgItem(IDOK);
	pBtn->SetButtonStyle(BS_OWNERDRAW);

	// 启用水平滚动条
	c_recvbuf.ModifyStyle(0, WS_HSCROLL);
	c_recvbuf.SetHorizontalExtent(1000);

	//弹出登陆界面
	DlgLogin dlgLogin;
	if (dlgLogin.DoModal() == IDOK) {
		username = DlgLogin::GetUsername();
		// 将窗口名字设置为用户名
		SetWindowText(L"Chat_Client - " + username);
	}
	else {
		AfxMessageBox(L"登陆失败！", MB_OK | MB_ICONSTOP);
		//退出程序
		exit(0);
	}
	// 设置此对话框的图标。  
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~WS_SIZEBOX); // 禁用可调整大小

	// 初始化 Winsock
	WSADATA wsaData{};
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		AfxMessageBox(L"初始化 Winsock 库失败！错误代码：" + WSAGetLastError(), MB_OK | MB_ICONSTOP);
	}
	sockCli = socket(AF_INET, SOCK_STREAM, 0);
	if (sockCli == INVALID_SOCKET) {
		int errorCode = WSAGetLastError();
		CString errorMsg;
		errorMsg.Format(L"创建套接字失败！错误代码: %d", errorCode);
		AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
		WSACleanup();
	}
	WSAAsyncSelect(sockCli, AfxGetMainWnd()->m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);

	m_ip = _T("127.0.0.1");
	m_port = _T("5566");
	UpdateData(FALSE);
	c_send.EnableWindow(FALSE);
	return TRUE;
}

BOOL CChatClientDlg::PreTranslateMessage(MSG* pMsg){
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		OnBnClickedSend();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CChatClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);

	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CChatClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatClientDlg::OnBnClickedConnect() {
	UpdateData(TRUE);
	addrSer.sin_family = AF_INET;
	addrSer.sin_port = htons(_ttoi(m_port));
	if (inet_pton(AF_INET, CT2A(m_ip), &addrSer.sin_addr) != 1) {
		AfxMessageBox(L"无效的 IP 地址！", MB_OK | MB_ICONSTOP);
		closesocket(sockCli);
		WSACleanup();
		return;
	}

	if (sockCli == INVALID_SOCKET) {
		AfxMessageBox(L"连接前套接字无效！", MB_OK | MB_ICONSTOP);
		return;
	}

	int res = connect(sockCli, (sockaddr*)&addrSer, sizeof(sockaddr));

	if (res == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK) {
			// 连接正在进行中
			AfxMessageBox(L"连接正在进行中，请稍候...", MB_OK);
			fd_set fdWritable;
			FD_ZERO(&fdWritable);
			FD_SET(sockCli, &fdWritable);

			// 使用 5 秒等待超时
			timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			// 检查是否可写
			int selectResult = select(0, NULL, &fdWritable, NULL, &timeout);
			if (selectResult > 0) {
				// 套接字是可写的，连接成功
				AfxMessageBox(L"客户端连接服务器成功");
				c_send.EnableWindow(TRUE);
				c_connect.EnableWindow(FALSE);
			}
			else if (selectResult == 0) {
				// 超时
				AfxMessageBox(L"连接超时，无法连接到服务器");
			}
			else {
				// 发生错误
				int selectError = WSAGetLastError();
				CString errorMsg;
				errorMsg.Format(L"选择错误，错误代码: %d", selectError);
				AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
			}
		}
		else {
			// 处理其他错误
			CString errorMsg;
			errorMsg.Format(L"客户端连接服务器失败，错误代码: %d", errorCode);
			AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
		}
	}
	else {
		AfxMessageBox(L"客户端连接服务器成功");
		c_send.EnableWindow(TRUE);
		c_connect.EnableWindow(FALSE);
	}
}

void CChatClientDlg::OnBnClickedSend() {
	UpdateData(TRUE);
	// 发送数据前先检查是否已连接
	if (sockCli == INVALID_SOCKET) {
		AfxMessageBox(L"与服务器断开连接！", MB_OK | MB_ICONSTOP);
		return;
	}
	// 发送数据前先检查是否输入有效数据
	if (m_sendbuf.IsEmpty()) {
		return;
	}
	// 获取当前时间并格式化
	CTime time = CTime::GetCurrentTime();
	CString t = time.Format(L"[%H:%M:%S]"); // 获取当前时间格式
	// 从文本框获取输入并清空文本框
	c_sendbuf.GetWindowTextW(m_sendbuf); // 从文本框获取输入到m_sendbuf
	// 当客户端输入“Bye”、“Quit”、“886”、“Exit”、“bye”、“quit”、“886”、“exit”等退出指令时，关闭客户端
	CString exit_words[] = { L"Bye", L"Quit", L"886", L"Exit", L"bye", L"quit", L"886", L"exit" };
	for (int i = 0; i < _countof(exit_words); i++) {
		if (m_sendbuf.CompareNoCase(exit_words[i]) == 0) {
			closesocket(sockCli);
			WSACleanup();
			Sleep(20);
			exit(0);
		}
	}
	c_sendbuf.SetWindowTextW(L""); // 清空文本框
	c_recvbuf.AddString(t + L"<我>:" + m_sendbuf); // 更新显示
	m_sendbuf += t; // 追加时间
	

	m_sendbuf += L"<username>" + username+L"</username>";	// 追加用户名发送

	//客户端发送数据格式为：message[time]<username>username</username>
	CT2A cA(m_sendbuf, CP_UTF8); // 将 CStringW 转换为 UTF-8（多字节）
	const char* c_buff = cA; // 获取转换后的 char*

	// 发送数据时需使用字节长度
	int sendResult = send(sockCli, c_buff, strlen(c_buff), 0); // 使用 strlen 计算字节长度
	if (sendResult == SOCKET_ERROR) {
		AfxMessageBox(L"与服务器断开连接！");
		return;
	}

	// 更新 UI 控件以显示发送的数据
	m_sendbuf.Empty(); // 清空发送缓冲
}

LRESULT CChatClientDlg::OnReceiveData(WPARAM wParam, LPARAM lParam) {
	SOCKET s = wParam;
	char recvbuf[256]{};
	int bytesReceived = 0;
	CString serbuf = L"";
	if (WSAGETSELECTERROR(lParam)) {
		closesocket(s);
		return FALSE; // 关闭套接字并返回
	}

	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_READ: {
		bytesReceived = recv(sockCli, recvbuf, sizeof(recvbuf) - 1, 0); // 留出一个字节存放'\0'
		if (bytesReceived > 0) {
			recvbuf[bytesReceived] = '\0'; // 确保字符串以空字符终止

			// 使用 UTF-8 编码进行转换
			serbuf += CA2W(recvbuf, CP_UTF8);

			c_recvbuf.AddString(serbuf); // 更新显示
		}
		else {
			// 处理错误情况
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				AfxMessageBox(L"接收消息时出错");
				break;
			}
		}
		break; // 处理完 FD_READ 后跳出 switch
	}

	case FD_CLOSE:
		closesocket(s);
		WSACleanup();
		AfxMessageBox(L"与服务器断开连接！");
		break;

	default:
		break;
	}
	return LRESULT(TRUE);
}

HBRUSH CChatClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// 调用父类的 OnCtlColor 方法，获取默认的画刷
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// 处理编辑框、列表框的颜色
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX || nCtlColor == CTLCOLOR_BTN) {
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(80, 80, 120));  
		hbr = CreateSolidBrush(RGB(80, 80, 120));
	}
	return hbr;
}

void CChatClientDlg::OnCancel(){
	CDialogEx::OnCancel();
}

void CChatClientDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// 处理按钮的自定义绘制
	if (nIDCtl == BTN_CONNECT || nIDCtl == BTN_SEND || nIDCtl == IDOK) {
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rc = lpDrawItemStruct->rcItem;
		// 绘制按钮的背景
		if (lpDrawItemStruct->itemState & ODS_DISABLED) {
			pDC->FillSolidRect(rc, RGB(80, 80, 120));
		}
		else {
			pDC->FillSolidRect(rc, RGB(40, 40, 80));
		}
		// 绘制按钮的文本设置为白色
		pDC->SetTextColor(RGB(200, 200, 255));
		if (nIDCtl == BTN_CONNECT) {
			pDC->DrawText(_T("连接"), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else if (nIDCtl == BTN_SEND) {
			pDC->DrawText(_T("发送"), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else if (nIDCtl == IDOK) {
			pDC->DrawText(_T("退出"), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		// 绘制按钮点击效果
		if (lpDrawItemStruct->itemState & ODS_SELECTED) {
			pDC->Draw3dRect(rc, RGB(255, 255, 255), RGB(255, 255, 255));
			pDC->Draw3dRect(rc, RGB(120, 120, 120), RGB(120, 120, 120));
		}
	}
}
