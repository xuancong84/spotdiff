// spotdiff.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "spotdiff.h"
#include <time.h>
#include <vector>
#include <sstream>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	srand (time(NULL));

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPOTDIFF, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPOTDIFF));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPOTDIFF));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_SPOTDIFF);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//


char help[]="Game Manual:\n\
Left-click on the small block which you think is different on the 2 sides;\n\
if you are correct, it will turn black on both sides; if you are wrong, nothing will happen;\n\
Find all different small blocks to win the game!\n\
\n\
Hotkeys:\n\
+: increase block size (reduce difficulty)\n\
-: decrease block size (increase difficulty)\n\
PageUp: increase the number of different blocks by 1\n\
PageDown: decrease the number of different blocks by 1\n\
Tab: display differences\n\
Enter: start a new set\n\
ESC: make 2 sides the same\n\
F1~F12: start a new set with N differences";

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MAXIMIZE	| SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);
   MessageBox(hWnd, help, "Help notes", MB_OK);

   return TRUE;
}

HBITMAP g_hImage = NULL;
const int n_color=6;
const int g_min_middle_width=4;
int g_middle_width=8;
int block_size=10;
int n_diff=5, n_diff_remain=5, n_fail=0;
DWORD colors[n_color]={
	0x00ff0000, 0x0000ff00, 0x000000ff, 0x00ffff00, 0x0000ffff, 0x00ff00ff
};
vector <DWORD> colorL, colorR;
int h_offset;

DWORD rand_color()
{
	return colors[rand()%n_color];
}

HBITMAP create_image(HWND hwnd, bool bNew=true)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;

	// create color buffer
	int ncol = (width-g_min_middle_width)/2/block_size;
	int nrow = height/block_size;
	g_middle_width = width-ncol*2*block_size;

	if(bNew)
	{
		colorL = vector <DWORD> (ncol*nrow);
		colorR = vector <DWORD> (ncol*nrow);
		for(int y=0; y<nrow; ++y)
			for(int x=0; x<ncol; ++x)
				colorL[y*ncol+x] = rand_color();
		colorR = colorL;

		// add random color
		for(int x=0; x<n_diff; ++x){
			int X=rand()%ncol, Y=rand()%nrow;
			while(colorL[Y*ncol+X]!=colorR[Y*ncol+X])	// coordinate overlap with previous
				X=rand()%ncol, Y=rand()%nrow;
			while(colorL[Y*ncol+X]==colorR[Y*ncol+X])	// generated the same random color
				colorR[Y*ncol+X] = rand_color();
		}
		n_diff_remain = n_diff;
		n_fail = 0;

		ostringstream oss;
		oss << "pixel_size=" << block_size << "  n_diff=" << n_diff << "  n_total="
			<< ncol << "x" << nrow << "=" << ncol*nrow;
		SetWindowText(hwnd, oss.str().c_str());
	}

	// create pixel buffer
	vector <DWORD> buf (width*height,0x00ffffff);
	// left half
	for(int y=0; y<nrow; ++y)
		for(int x=0; x<ncol; ++x)
			for(int j=0; j<block_size; ++j)
				for(int i=0; i<block_size; ++i)
					buf[(y*block_size+j)*width+x*block_size+i]=colorL[y*ncol+x];
	// right half
	h_offset = ncol*block_size+g_middle_width;
	for(int y=0; y<nrow; ++y)
		for(int x=0; x<ncol; ++x)
			for(int j=0; j<block_size; ++j)
				for(int i=0; i<block_size; ++i)
					buf[(y*block_size+j)*width+x*block_size+i+h_offset]=colorR[y*ncol+x];

	return CreateBitmap(width, height, 1, 32, buf.data());
}

void update_image(HWND hwnd)
{
	if(g_hImage)
		DeleteObject(g_hImage);
	g_hImage = create_image(hwnd);
}

void left_click(HWND hwnd, int posX, int posY)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;

	// create color buffer
	int ncol = (width-g_min_middle_width)/2/block_size;
	int nrow = height/block_size;
	g_middle_width = width-ncol*2*block_size;

	// clicked on right half
	if(posX>=h_offset)
		posX-=h_offset;

	int X=posX/block_size, Y=posY/block_size;
	if(X<0 || X>=ncol || Y<0 || Y>=nrow)	// outside region
		return;

	bool bNew = false;
	if(colorL[Y*ncol+X]==colorR[Y*ncol+X]){
		ostringstream oss;
		oss << "Fail=" << ++n_fail;
		SetWindowText(hwnd, oss.str().c_str());
	}else{
		colorL[Y*ncol+X]=colorR[Y*ncol+X]=0;
		ostringstream oss;
		oss << "Success, n_remaining=" << --n_diff_remain;
		SetWindowText(hwnd, oss.str().c_str());
		if(!n_diff_remain){
			ostringstream oss;
			oss << "You have found all successfully!\n" 
				<< "pixel_size=" << block_size << "  n_diff=" << n_diff << "  n_total="
				<< ncol << "x" << nrow << "=" << ncol*nrow << endl
				<< "Start new?";
			int ret = MessageBox(hwnd, oss.str().c_str(), "Congratulations", MB_YESNO);
			bNew = (ret==IDYES);
		}
	}
	DeleteObject(g_hImage);
	g_hImage = create_image(hwnd, bNew);
	InvalidateRect(hwnd,NULL,FALSE);
	UpdateWindow(hwnd);
}

void swap_color(HWND hwnd)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;

	// create color buffer
	int ncol = (width-g_min_middle_width)/2/block_size;
	int nrow = height/block_size;
	g_middle_width = width-ncol*2*block_size;

	for(int y=0; y<nrow; ++y)
		for(int x=0; x<ncol; ++x)
			if(colorL[y*ncol+x]!=colorR[y*ncol+x])
				swap(colorL[y*ncol+x], colorR[y*ncol+x]);
}

void show_diff(HWND hwnd)
{
	swap_color(hwnd);
	DeleteObject(g_hImage);
	g_hImage = create_image(hwnd, false);
	InvalidateRect(hwnd,NULL,FALSE);
	UpdateWindow(hwnd);

	Sleep(100);

	swap_color(hwnd);
	DeleteObject(g_hImage);
	g_hImage = create_image(hwnd, false);
	InvalidateRect(hwnd,NULL,FALSE);
	UpdateWindow(hwnd);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			// TODO: Add any drawing code here...
			if(!g_hImage)
				g_hImage = create_image(hWnd);

			HDC		hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ	oldBitmap = SelectObject(hdcMem, g_hImage);
			BITMAP	bitmap;

			GetObject(g_hImage, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		left_click(hWnd, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		{
			int n=0;
			switch(wParam){
			case VK_F12: ++n;
			case VK_F11: ++n;
			case VK_F10: ++n;
			case VK_F9:	 ++n;
			case VK_F8:	 ++n;
			case VK_F7:	 ++n;
			case VK_F6:	 ++n;
			case VK_F5:	 ++n;
			case VK_F4:	 ++n;
			case VK_F3:	 ++n;
			case VK_F2:	 ++n;
			case VK_F1:	 ++n;
			case VK_ESCAPE:
				n_diff=n;
			case VK_RETURN:
				update_image(hWnd);
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
				break;
			case VK_OEM_MINUS:
				if(block_size>1)
					block_size--;
				update_image(hWnd);
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
				break;
			case VK_OEM_PLUS:
				if(block_size<100)
					block_size++;
				update_image(hWnd);
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
				break;
			case VK_TAB:
				show_diff(hWnd);
				break;
			case VK_PRIOR:
				if(n_diff<100)
					n_diff++;
				update_image(hWnd);
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
				break;
			case VK_NEXT:
				if(n_diff>0)
					n_diff--;
				update_image(hWnd);
				InvalidateRect(hWnd,NULL,FALSE);
				UpdateWindow(hWnd);
				break;
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
