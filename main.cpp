#include "framework.h"
#include "main.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szMainWindowClass[MAX_LOADSTRING];
WCHAR szPaddleWindowClass[MAX_LOADSTRING];
WCHAR szBallWindowClass[MAX_LOADSTRING];
HBITMAP hBitmap = NULL;
OPENFILENAME ofn;

ATOM ParentRegisterClass(HINSTANCE hInstance);
ATOM ChildrenRegisterClass(HINSTANCE hInstance, WNDPROC proc, LPCWSTR className, HBRUSH bgColor);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PaddleWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BallWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void showBitmap(HWND hwnd);
COLORREF currentBg = RGB(124, 252, 0);

void ResetGame();
int onPaddCntr = 0;
int offPaddCntr = 0;
int paddleY;
bool playing = true;
bool bitMapLoaded = false;
bool colorChosen = false;

HFONT hFont = CreateFont(60, 25, 0, 0, 0, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));

RECT offPaddleRc, onPaddleRc;

HWND hWnd_main;
HWND hWnd_paddle;
HWND hWnd_ball;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	std::wstring title(L"Pong\0");
	title.copy(szTitle, title.size());

	std::wstring className(L"PONG\0");
	className.copy(szMainWindowClass, className.size());
	className = L"PADDLE\0";
	className.copy(szPaddleWindowClass, className.size());
	className = L"BALL\0";
	className.copy(szBallWindowClass, className.size());

	ParentRegisterClass(hInstance);
	ChildrenRegisterClass(hInstance, PaddleWndProc, szPaddleWindowClass, (HBRUSH)(COLOR_ACTIVECAPTION)+1);
	ChildrenRegisterClass(hInstance, BallWndProc, szBallWindowClass, (HBRUSH)CreateSolidBrush(RGB(255, 0, 0)));


	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return static_cast <int>(msg.wParam);
}

ATOM ParentRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hIconSm = nullptr;
	wcex.hCursor = nullptr;
	wcex.hbrBackground = reinterpret_cast <HBRUSH>(CreateSolidBrush(currentBg));
	wcex.lpszClassName = szMainWindowClass;
	wcex.lpszMenuName = MAKEINTRESOURCEW(MAIN_MENU);
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClassExW(&wcex);
}

ATOM ChildrenRegisterClass(HINSTANCE hInstance, WNDPROC proc, LPCWSTR className, HBRUSH bgColor)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.lpfnWndProc = proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hIconSm = nullptr;
	wcex.hCursor = nullptr;
	wcex.hbrBackground = bgColor;
	wcex.lpszClassName = className;
	wcex.lpszMenuName = nullptr;
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	//the main window
	hWnd_main = CreateWindowW(szMainWindowClass, szTitle,
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		GetSystemMetrics(SM_CXSCREEN) / 2 - 250, GetSystemMetrics(SM_CYSCREEN) / 2 - 175, 500, 350, nullptr,
		nullptr, hInstance, nullptr);

	//setting WS_EX_LAYERED style
	SetWindowLong(hWnd_main, GWL_EXSTYLE,
		GetWindowLong(hWnd_main, GWL_EXSTYLE) | WS_EX_LAYERED);
	//setting transparency
	SetLayeredWindowAttributes(hWnd_main, 0, (255 * 80) / 100, LWA_ALPHA);

	//paddle window
	RECT rc;
	GetClientRect(hWnd_main, &rc);
	hWnd_paddle = CreateWindowW(szPaddleWindowClass, nullptr, WS_CLIPSIBLINGS | WS_CHILD | WS_EX_LAYERED,
		rc.right - 15, rc.bottom / 2 - 35, 15, 70, hWnd_main, nullptr, hInstance, nullptr);
	HRGN region = CreateRectRgn(0, 0, rc.right, rc.bottom / 2 + 35); //https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createrectrgn
	SetWindowRgn(hWnd_paddle, region, true);
	paddleY = rc.bottom / 2 - 35;
	//ball window
	hWnd_ball = CreateWindowW(szBallWindowClass, nullptr, WS_CLIPSIBLINGS | WS_CHILD | WS_EX_LAYERED,
		120, 50, 15, 15, hWnd_main, nullptr, hInstance, nullptr);

	region = CreateEllipticRgn(0, 0, 15, 15);
	SetWindowRgn(hWnd_ball, region, true);



	if (!hWnd_main || !hWnd_paddle || !hWnd_ball)
	{
		return FALSE;
	}

	ShowWindow(hWnd_main, nCmdShow);
	UpdateWindow(hWnd_main);
	ShowWindow(hWnd_paddle, nCmdShow);
	UpdateWindow(hWnd_paddle);
	ShowWindow(hWnd_ball, nCmdShow);
	UpdateWindow(hWnd_ball);

	return TRUE;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case ID_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(ID_DIALOG), hWnd, About);
			break;
		case ID_NEWGAME:
			ResetGame();
			break;

		case ID_BACKGROUND_COLOR:
		{	//https://stackoverflow.com/questions/14628922/in-win32-how-can-a-change-color-dialog-be-used-to-change-static-text
			//http://winapi.freetechsecrets.com/win32/WIN32Choosing_a_Color.htm

			CHOOSECOLOR color = { 0 };
			color.lStructSize = sizeof(color);
			static COLORREF colorRef[16];
			color.hwndOwner = hWnd_main;
			color.lpCustColors = (LPDWORD)colorRef;
			color.Flags = CC_FULLOPEN | CC_RGBINIT;
			if (ChooseColor(&color))
			{
				colorChosen = true;
				currentBg = color.rgbResult;
				SetClassLongPtrA(hWnd_main, GCLP_HBRBACKGROUND, (LONG_PTR)reinterpret_cast <HBRUSH>(CreateSolidBrush(currentBg)));
				InvalidateRect(hWnd_main, NULL, TRUE);
			}
		}
		break;
		case ID_BACKGROUND_BITMAP:
		{//https://www.daniweb.com/programming/software-development/code/217307/a-simple-getopenfilename-example
		//https://stackoverflow.com/questions/14337983/load-hbitmap-from-bmp-file-in-win32-project
		//http://www.winprog.org/tutorial/bitmaps.html
			OPENFILENAME ofn;
			WCHAR szFile[100];
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"All\0*.*\0*.bmp\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			GetOpenFileName(&ofn);

			HMENU menu = GetMenu(hWnd);
			EnableMenuItem(menu, ID_BACKGROUND_TILE, MF_ENABLED);
			EnableMenuItem(menu, ID_BACKGROUND_STRETCH, MF_ENABLED);
			CheckMenuItem(menu, ID_BACKGROUND_STRETCH, MF_CHECKED);
			hBitmap = (HBITMAP)LoadImage(NULL, 
				ofn.lpstrFile, 
				IMAGE_BITMAP, 0, 0, 
				LR_LOADFROMFILE);
			if (hBitmap != NULL)
			{
				bitMapLoaded = true;
				HDC hdc = GetDC(hWnd_main);
				HDC hdcMem = CreateCompatibleDC(hdc);
				SelectObject(hdcMem, hBitmap);
				BITMAP bm;
				GetObject(hBitmap, sizeof(bm), &bm);
				StretchBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				DeleteDC(hdcMem);
				ReleaseDC(hWnd_main, hdc);
				DeleteObject(hBitmap);
			}
			else
			{
				MessageBox(hWnd_main, L"Could'nt load bitmap", L"Error", MB_OK | MB_ICONERROR);
			}
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	case WM_MOUSEMOVE:
	{//https://learn.microsoft.com/en-us/windows/win32/learnwin32/mouse-movement
	//https://learn.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations

		int mouseY = GET_Y_LPARAM(lParam);

		RECT paddleRc;
		GetWindowRect(hWnd_paddle, &paddleRc);
		int widthPaddle = paddleRc.right - paddleRc.left;
		int heightPaddle = paddleRc.bottom - paddleRc.top;

		POINT pt = { paddleRc.left, paddleRc.top };
		ScreenToClient(hWnd_main, &pt);
		pt.y = mouseY - heightPaddle / 2;

		RECT mainClientRc;
		GetClientRect(hWnd_main, &mainClientRc);
		if (pt.y < mainClientRc.top)
			pt.y = mainClientRc.top;
		else if (pt.y + heightPaddle >= mainClientRc.bottom)
			pt.y = mainClientRc.bottom - heightPaddle;
		paddleY = pt.y;
		MoveWindow(hWnd_paddle, pt.x, pt.y, widthPaddle, heightPaddle, true);
		InvalidateRect(hWnd_main, &mainClientRc, false);
		InvalidateRect(hWnd_paddle, &paddleRc, false);
	}
	break;

	case WM_PAINT:
	{//https://stackoverflow.com/questions/19885770/convert-int-to-lpcwstr-by-using-wsprintf
	// https://www.codeproject.com/Questions/1006882/how-to-draw-text-in-win-project-window
	// https://stackoverflow.com/questions/221411/how-can-i-specify-a-font-for-a-window-created-through-createwindow
	//https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta?redirectedfrom=MSDN
	//https://www.quora.com/How-do-you-find-an-RGB-complementary-color
		PAINTSTRUCT ps;
		HDC hdc = GetDC(hWnd_main);
		BeginPaint(hWnd_main, &ps);
		SetBkMode(hdc, TRANSPARENT);
		RECT rc;
		GetClientRect(hWnd_main, &rc);
		int width = 150;
		int height = 150;
		int paddingTop = 30;

		SetRect(&offPaddleRc, rc.right / 4 - width / 2 , paddingTop, rc.right / 4 + width / 2, paddingTop + height);
		SetRect(&onPaddleRc, rc.right * 3 / 4 - width / 2, paddingTop, rc.right * 3 / 4 + width / 2, paddingTop + height);

		wchar_t onPaddleText[10];
		wchar_t offPaddleText[10];
		wsprintfW(onPaddleText, L"%d", onPaddCntr);
		wsprintfW(offPaddleText, L"%d", offPaddCntr);

		SetTextColor(hdc, currentBg ^ 0x00FFFFFF);
		SelectObject(hdc, hFont);
		DrawText(hdc, offPaddleText, wcslen(offPaddleText), &offPaddleRc, DT_CENTER);
		DrawText(hdc, onPaddleText, wcslen(onPaddleText), &onPaddleRc, DT_CENTER);
		
		//bitmap
		if(bitMapLoaded)
			showBitmap(hWnd_main);
		ReleaseDC(hWnd_main, hdc);
		EndPaint(hWnd_main, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


LRESULT CALLBACK PaddleWndProc(HWND hWnd_paddle, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd_paddle, message, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK BallWndProc(HWND hWnd_ball, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	static int ballX = 0;
	static int ballY = 0;
	static int xChange = 5;
	static int yChange = 5;

	switch (message)
	{
	case WM_CREATE:
	{
		SetTimer(hWnd_ball, 1, 50, NULL);
	}

	case WM_TIMER:
	{
		RECT mainRc, paddleRc, ballRc;
		GetClientRect(hWnd_main, &mainRc);
		GetWindowRect(hWnd_paddle, &paddleRc);
		GetWindowRect(hWnd_ball, &ballRc);

		if (playing)
		{

			if (ballX >= mainRc.right - (ballRc.right - ballRc.left) - (paddleRc.right - paddleRc.left)
				&& (ballY >= paddleY && ballY <= paddleY + (paddleRc.bottom - paddleRc.top))) {
				xChange = abs(xChange) * -1;
				onPaddCntr++;
				InvalidateRect(NULL, &onPaddleRc, true);
				//WriteScore(hWnd_main, paddleCounter, paddleCounter);
			}
			else if (ballX >= mainRc.right - (ballRc.right - ballRc.left)) {
				xChange = abs(xChange) * -1;
				offPaddCntr++;
				InvalidateRect(NULL, &offPaddleRc, true);
			}
			if (ballX <= 0)
				xChange = abs(xChange);
			if (ballY >= mainRc.bottom - (ballRc.bottom - ballRc.top))
				yChange = abs(yChange) * -1;
			if (ballY <= 0)
				yChange = abs(yChange);

			ballX = ballX + xChange;
			ballY = ballY + yChange;

			MoveWindow(hWnd_ball, ballX, ballY, ballRc.right - ballRc.left, ballRc.bottom - ballRc.top, TRUE);
			InvalidateRect(hWnd_main, &mainRc, false);
		}
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd_ball, message, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM
	lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return static_cast <INT_PTR>(TRUE);

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)

		{
			EndDialog(hDlg, LOWORD(wParam));
			return static_cast <INT_PTR>(TRUE);
		}
		break;
	}
	return static_cast <INT_PTR>(FALSE);
}

void ResetGame() {
	playing = false;
	onPaddCntr = offPaddCntr = 0;
	MoveWindow(hWnd_ball, 0, 0, 15, 15, true);
	InvalidateRect(NULL, &onPaddleRc, true);
	InvalidateRect(NULL, &offPaddleRc, true);
	playing = true;
}

void showBitmap(HWND hwnd)
{
	hBitmap = (HBITMAP)LoadImage(NULL, ofn.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBitmap == NULL)
	{
		return;
	}


	HDC hdc = GetDC(hwnd);

	HDC memDC = CreateCompatibleDC(hdc);

	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, memDC, 0, 0, SRCCOPY);

	SelectObject(memDC, oldBitmap);
	DeleteDC(memDC);
	ReleaseDC(hwnd, hdc);
	DeleteObject(hBitmap);
}