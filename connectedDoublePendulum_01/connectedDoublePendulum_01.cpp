#include "framework.h"
#include "resource.h"

#define WIN_WIDTH	1024
#define WIN_HEIGHT	800
#define BOX_WIDTH	600
#define BOX_HEIGHT	500

#define	ID_EDIT1	101		// mass of a pendulum
#define	ID_EDIT2	102		// rubber band tension
#define	ID_EDIT3	103		// angle of the left pendulum
#define ID_EDIT4	104		// angle of the right pendulum

#define	BTN_START	201
#define BTN_STOP	202

#define	R	15

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
HINSTANCE g_hInst;
HWND hWndMain;
HWND hButton1;			// start button
HWND hButton2;			// stop button
HWND hEdit1;
HWND hEdit2;
HWND hEdit3;
HWND hEdit4;

HBITMAP hBufferBit;
LPCTSTR lpszClass = TEXT("ConnectedDoublePendulum_01");

RECT brt;

const double PI = 3.1415926535897;
double rodLength = 200.0;
double mass = 1.0;
double tension = 10.0;
double ang1 = 0.0;
double ang2 = 0.0;

const int delta = 60;			// time interval in ms
const double gravityAcc = 9.80;


class Point
{
public:
	int x;
	int y;
	Point() : x(0), y(0)
	{ }
};

class Spring
{
public:
	Point leftBar[2];
	Point rightBar[2];
	Point springDot[18];
	int radius;
	//int spLength;
	Spring() : radius(10)
	{ }
};

void SetSpring(Spring& sp, double distance);
void RotateSpring(Spring& sp, double angle);
void DrawSpring(HDC& hdc, Spring& sp, int transX, int transY);

class Pendulum
{
public:
	int xFixed;
	int yFixed;
	int xEnd;
	int yEnd;
	double theta;			// angle in radian
	double omega;

	Pendulum() : xFixed(0), yFixed(0), xEnd(0), yEnd(0), theta(0.0), omega(0.0)
	{ }
};

void DrawBackground(HDC& hdc);
void DrawPendulum(HDC& hdc, Pendulum& p1, Pendulum& p2);
void OnTimerDraw();
void InitPendulum(Pendulum& p1, Pendulum& p2, double th1, double th2, double mass_, double length, double k);
void UpdateParameters(Pendulum& p1, Pendulum& p2);

Pendulum pend1;
Pendulum pend2;

//Spring spring;
//Spring copiedSpring;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS wdc;
	g_hInst = hInstance;
	wdc.cbClsExtra = 0;
	wdc.cbWndExtra = 0;
	wdc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wdc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wdc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wdc.hInstance = hInstance;
	wdc.lpfnWndProc = WndProc;
	wdc.lpszClassName = lpszClass;
	wdc.lpszMenuName = nullptr;
	wdc.style = CS_HREDRAW | CS_VREDRAW;	// class style

	RegisterClass(&wdc);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIN_WIDTH, WIN_HEIGHT, nullptr,
		(HMENU)nullptr, hInstance, nullptr);

	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, nullptr, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

void DrawBackground(HDC& hdc)
{
	
	HPEN hPen, hOldPen;
	int i;
	hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 64));
	hOldPen = (HPEN)SelectObject(hdc, hPen);

	Rectangle(hdc, 0, 0, BOX_WIDTH, BOX_HEIGHT);

	
	// draw lattice lines
	for (i = 0; i < BOX_WIDTH; i+=10)
	{
		MoveToEx(hdc, i, 0, nullptr);
		LineTo(hdc, i, BOX_HEIGHT);
	}
	for (i = 0; i < BOX_HEIGHT; i+=10)
	{
		MoveToEx(hdc, 0, i, nullptr);
		LineTo(hdc, BOX_WIDTH, i);
	}
	DeleteObject(SelectObject(hdc, hOldPen));
	

	// draw Ceiling
	hPen = CreatePen(PS_SOLID, 5, RGB(0, 32, 32));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, 100, 100, nullptr);
	LineTo(hdc, 500, 100);
	for (i = 140; i <= 440; i += 20)
	{
		MoveToEx(hdc, i, 100, nullptr);
		LineTo(hdc, i + 20, 80);
	}
	DeleteObject(SelectObject(hdc, hOldPen));
	
}

void DrawPendulum(HDC& hdc, Pendulum& p1, Pendulum& p2)
{
	HPEN hPen, hOldPen;
	HBRUSH hBrush, hOldBrush;
	Spring spring;
	double dist;
	double angle;

	hPen = CreatePen(PS_SOLID, 4, RGB(192, 64, 128));
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	hBrush = CreateSolidBrush(RGB(128, 128, 192));
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

	// draw pendulum 1 and pendulum 2
	MoveToEx(hdc, p1.xFixed, p1.yFixed, nullptr);
	LineTo(hdc, p1.xEnd, p1.yEnd);
	MoveToEx(hdc, p2.xFixed, p2.yFixed, nullptr);
	LineTo(hdc, p2.xEnd, p2.yEnd);
	DeleteObject(SelectObject(hdc, hOldPen));

	// draw spring with tension k
	dist = sqrt((double)((p2.xEnd - p1.xEnd) * (p2.xEnd - p1.xEnd) + (p2.yEnd - p1.yEnd) * (p2.yEnd - p1.yEnd)));
	angle = -atan((double)(p2.yEnd - p1.yEnd) / (double)(p2.xEnd - p1.xEnd));
	SetSpring(spring, dist);
	RotateSpring(spring, angle);
	DrawSpring(hdc, spring, p1.xEnd, p1.yEnd);

	//DeleteObject(SelectObject(hdc, hOldPen));
	hPen = (HPEN)GetStockObject(NULL_PEN);

	Ellipse(hdc, p1.xEnd - R, p1.yEnd - R, p1.xEnd + R, p1.yEnd + R);
	Ellipse(hdc, p2.xEnd - R, p2.yEnd - R, p2.xEnd + R, p2.yEnd + R);
	//DeleteObject(SelectObject(hdc, hOldPen));
	DeleteObject(SelectObject(hdc, hOldBrush));
}

void OnTimerDraw()
{
	HDC hdc, hMemDC;
	HBITMAP hOldBit;

	brt.left = 0; brt.top = 0;
	brt.right = BOX_WIDTH; brt.bottom = BOX_HEIGHT;
	
	hdc = GetDC(hWndMain);

	if (hBufferBit == nullptr)
	{
		hBufferBit = CreateCompatibleBitmap(hdc, BOX_WIDTH, BOX_HEIGHT);
	}
	hMemDC = CreateCompatibleDC(hdc);
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBufferBit);

	FillRect(hMemDC, &brt, GetSysColorBrush(COLOR_WINDOW));

	DrawBackground(hMemDC);
	DrawPendulum(hMemDC, pend1, pend2);
	SelectObject(hMemDC, hOldBit);
	DeleteDC(hMemDC);

	ReleaseDC(hWndMain, hdc);
}

void SetSpring(Spring& sp, double distance)
{
	//sp.spLength = (int)distance;
	int coreLength = (int)distance - 2 * 30;
	int pitch = coreLength / 17;
	int sgn = 1;
	sp.leftBar[0].x = 0; sp.leftBar[0].y = 0;
	sp.leftBar[0].x = 30; sp.leftBar[0].y = 0;
	sp.springDot[0].x = 30; sp.springDot[0].y = 0;
	for (int i = 1; i < 17; i++)
	{
		sp.springDot[i].x = 30 + i * pitch;
		sp.springDot[i].y = 0 + sgn * sp.radius;
		sgn *= -1;
	}
	sp.springDot[17].x = sp.springDot[16].x + pitch;
	sp.springDot[17].y = 0;
	sp.rightBar[0].x = sp.springDot[17].x;
	sp.rightBar[0].y = 0;
	sp.rightBar[1].x = sp.rightBar[0].x + 40;
	sp.rightBar[1].y = 0;
}

void RotateSpring(Spring& sp, double angle)
{
	int tempLeftX0 = sp.leftBar[0].x;
	int tempLeftY0 = sp.leftBar[0].y;
	int tempLeftX1 = sp.leftBar[1].x;
	int tempLeftY1 = sp.leftBar[1].y;
	int tempRightX0 = sp.rightBar[0].x;
	int tempRightY0 = sp.rightBar[0].y;
	int tempRightX1 = sp.rightBar[1].x;
	int tempRightY1 = sp.rightBar[1].y;
	int tempX, tempY;

	// doing rotation
	sp.leftBar[0].x = (int)((double)tempLeftX0 * cos(angle) + (double)tempLeftY0 * sin(angle));
	sp.leftBar[0].y = (int)((double)tempLeftX0 * -sin(angle) + (double)tempLeftY0 * cos(angle));
	sp.leftBar[1].x = (int)((double)tempLeftX1 * cos(angle) + (double)tempLeftY1 * sin(angle));
	sp.leftBar[1].y = (int)((double)tempLeftX1 * -sin(angle) + (double)tempLeftY1 * cos(angle));

	sp.rightBar[0].x = (int)((double)tempRightX0 * cos(angle) + (double)tempRightY0 * sin(angle));
	sp.rightBar[0].y = (int)((double)tempRightX0 * -sin(angle) + (double)tempRightY0 * cos(angle));
	sp.rightBar[1].x = (int)((double)tempRightX1 * cos(angle) + (double)tempRightY1 * sin(angle));
	sp.rightBar[1].y = (int)((double)tempRightX1 * -sin(angle) + (double)tempRightY1 * cos(angle));

	for (int i = 0; i < 18; i++)
	{
		tempX = sp.springDot[i].x;
		tempY = sp.springDot[i].y;
		sp.springDot[i].x = (int)((double)tempX * cos(angle) + (double)tempY * sin(angle));
		sp.springDot[i].y = (int)((double)tempX * -sin(angle) + (double)tempY * cos(angle));
	}
}

void DrawSpring(HDC& hdc, Spring& sp, int transX, int transY)
{
	HPEN hPen, hOldPen;
	hPen = CreatePen(PS_SOLID, 3, RGB(64, 64, 128));
	hOldPen = (HPEN)SelectObject(hdc, hPen);

	MoveToEx(hdc, transX + sp.leftBar[0].x, transY + sp.leftBar[0].y, nullptr);
	LineTo(hdc, transX + sp.leftBar[1].x, transY + sp.leftBar[1].y);

	for (int i = 1; i < 18; i++)
	{
		MoveToEx(hdc, transX + sp.springDot[i - 1].x, transY + sp.springDot[i - 1].y, nullptr);
		LineTo(hdc, transX + sp.springDot[i].x, transY + sp.springDot[i].y);
	}

	MoveToEx(hdc, transX + sp.rightBar[0].x, transY + sp.rightBar[0].y, nullptr);
	LineTo(hdc, transX + sp.rightBar[1].x, transY + sp.rightBar[1].y);
	DeleteObject(SelectObject(hdc, hOldPen));
}

void InitPendulum(Pendulum& p1, Pendulum& p2, double th1, double th2, double mass_, double length, double k)
{
	mass = mass_;
	rodLength = length;
	tension = k;
	
	p1.theta = th1;
	p1.omega = 0.0;
	p1.xFixed = 200; 
	p1.yFixed = 100;
	p1.xEnd = p1.xFixed + (int)(rodLength * sin(p1.theta));
	p1.yEnd = p1.yFixed + (int)(rodLength * cos(p1.theta));

	p2.theta = th2;
	p2.omega = 0.0;
	p2.xFixed = 400;
	p2.yFixed = 100;
	p2.xEnd = p2.xFixed + (int)(rodLength * sin(p2.theta));
	p2.yEnd = p2.yFixed + (int)(rodLength * cos(p2.theta));
}

void UpdateParameters(Pendulum& p1, Pendulum& p2)
{
	double c1 = gravityAcc / rodLength + tension / mass;
	double c2 = tension / mass;
	//double c1 = 3.0;
	//double c2 = 2.0;


	p1.omega = p1.omega - ((double)delta / 1000.0) * (c1 * sin(p1.theta) - c2 * sin(p2.theta));
	p2.omega = p2.omega - ((double)delta / 1000.0) * (c1 * sin(p2.theta) - c2 * sin(p1.theta));

	p1.theta = p1.theta + ((double)delta / 1000.0) * p1.omega;
	p2.theta = p2.theta + ((double)delta / 1000.0) * p2.omega;

	p1.xEnd = p1.xFixed + (int)(rodLength * sin(p1.theta));
	p1.yEnd = p1.yFixed + (int)(rodLength * cos(p1.theta));

	p2.xEnd = p2.xFixed + (int)(rodLength * sin(p2.theta));
	p2.yEnd = p2.yFixed + (int)(rodLength * cos(p2.theta));
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static RECT refreshZone;
	static const TCHAR* Mes = TEXT("Double Pendulum demo using Euler method");
	static const TCHAR* Mes2 = TEXT("Motion eq. is updated everytime WM_TIMER is sent to WndProc.");
	//static TCHAR err[128];
	TCHAR desc[64];
	TCHAR temp[64];

	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		SetWindowText(hWnd, TEXT("Connected Double Pendulum Demo using Win32 GDI"));

		hButton1 = CreateWindow(TEXT("button"), TEXT("Start"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			700, 550, 100, 25, hWnd, (HMENU)BTN_START, g_hInst, nullptr);

		hButton2 = CreateWindow(TEXT("button"), TEXT("Stop"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			850, 550, 100, 25, hWnd, (HMENU)BTN_STOP, g_hInst, nullptr);

		// edit windows
		hEdit1 = CreateWindow(TEXT("edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
			780, 200, 100, 25, hWnd, (HMENU)ID_EDIT1, g_hInst, nullptr);
		hEdit2 = CreateWindow(TEXT("edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
			780, 280, 100, 25, hWnd, (HMENU)ID_EDIT2, g_hInst, nullptr);
		hEdit3 = CreateWindow(TEXT("edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
			780, 360, 100, 25, hWnd, (HMENU)ID_EDIT3, g_hInst, nullptr);
		hEdit4 = CreateWindow(TEXT("edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
			780, 440, 100, 25, hWnd, (HMENU)ID_EDIT4, g_hInst, nullptr);

		_stprintf_s(temp, TEXT("%f"), 1.0);
		SetWindowText(hEdit1, temp);
		_stprintf_s(temp, TEXT("%f"), 10.0);
		SetWindowText(hEdit2, temp);
		_stprintf_s(temp, TEXT("%f"), 0.0);
		SetWindowText(hEdit3, temp);
		_stprintf_s(temp, TEXT("%f"), 0.0);
		SetWindowText(hEdit4, temp);

		refreshZone.left = 30;
		refreshZone.top = 80;
		refreshZone.right = 670;
		refreshZone.bottom = 720;
		InitPendulum(pend1, pend2, PI/180.0*0.0, PI/180.0*0.0, mass, 200.0, tension);
		OnTimerDraw();
		//SetTimer(hWnd, 1, delta, nullptr);
		return 0;

	case WM_TIMER:
		UpdateParameters(pend1, pend2);
		OnTimerDraw();
		//_stprintf_s(err, TEXT("position : %f, %f"), pend1.omega, pend1.theta);
		InvalidateRect(hWnd, &refreshZone, true);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 200, 40, Mes, lstrlen(Mes));
		TextOut(hdc, 135, 70, Mes2, lstrlen(Mes2));
		LoadString(g_hInst, IDS_STRING1, desc, 64);
		TextOut(hdc, 740, 175, desc, lstrlen(desc));
		LoadString(g_hInst, IDS_STRING2, desc, 64);
		TextOut(hdc, 785, 255, desc, lstrlen(desc));
		LoadString(g_hInst, IDS_STRING3, desc, 64);
		TextOut(hdc, 730, 335, desc, lstrlen(desc));
		LoadString(g_hInst, IDS_STRING4, desc, 64);
		TextOut(hdc, 725, 415, desc, lstrlen(desc));
		if (hBufferBit)
		{
			DrawBitmap(hdc, 50, 100, hBufferBit);
		}
		EndPaint(hWnd, &ps);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_START:
			MessageBeep(0);
			SetTimer(hWnd, 1, delta, nullptr);
			break;

		case BTN_STOP:
			MessageBeep(0);
			KillTimer(hWnd, 1);
			break;

		case ID_EDIT1:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				GetWindowText(hEdit1, temp, 64);
				mass = (double)wcstod(temp, nullptr);
				InitPendulum(pend1, pend2, PI / 180.0 * ang1, PI / 180.0 * ang2, mass, 200.0, tension);
				break;
			}
		case ID_EDIT2:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				GetWindowText(hEdit2, temp, 64);
				tension = (double)wcstod(temp, nullptr);
				InitPendulum(pend1, pend2, PI / 180.0 * ang1, PI / 180.0 * ang2, mass, 200.0, tension);
				break;
			}
		case ID_EDIT3:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				GetWindowText(hEdit3, temp, 64);
				ang1 = (double)wcstod(temp, nullptr);
				InitPendulum(pend1, pend2, PI / 180.0 * ang1, PI / 180.0 * ang2, mass, 200.0, tension);
				break;
			}
		case ID_EDIT4:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				GetWindowText(hEdit4, temp, 64);
				ang2 = (double)wcstod(temp, nullptr);
				InitPendulum(pend1, pend2, PI / 180.0 * ang1, PI / 180.0 * ang2, mass, 200.0, tension);
				break;
			}
		}
		return 0;

	case WM_DESTROY:
		KillTimer(hWnd, 1);
		if (hBufferBit)
		{
			DeleteObject(hBufferBit);
		}
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}


void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}
