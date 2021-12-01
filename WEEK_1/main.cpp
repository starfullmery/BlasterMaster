#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

#define WINDOW_CLASS_NAME L"SampleWindow"
#define WINDOW_TITLE L"00-Intro"
#define WINDOW_ICON_PATH L"jason.ico"

HWND hWnd = 0;

#define D3DCOLOR_WHITE D3DCOLOR_XRGB(255, 255, 255) 

#define BACKGROUND_COLOR D3DCOLOR_XRGB(0, 0, 0)
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_FRAME_RATE 120

LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddv = NULL;

LPDIRECT3DSURFACE9 backBuffer = NULL;
int BackBufferWidth = 0;
int BackBufferHeight = 0;

LPD3DXSPRITE spriteHandler = NULL;

#define JASON_TEXTURE_PATH L"jason.png"
#define JASON_START_X 30.0f
#define JASON_START_Y 10.0f
#define JASON_START_VX 0.2f
#define JASON_WIDTH  26.0f
#define JASON_HEIGHT  35.0f

LPDIRECT3DTEXTURE9 texJason;

float jason_x = JASON_START_X;
float jason_vx = JASON_START_VX;
float jason_y = JASON_START_Y;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// DEBUG SUPPORT FUNCTIONS //////
#define _W(x) __W(x)
#define __W(x) L##x

#define VA_PRINTS(s){			\
	va_list argp;				\
	va_start(argp, fmt);		\
	vswprintf_s(s, fmt, argp);	\
	va_end(argp);				\
}

void DebugOut(wchar_t* fmt, ...)
{
	wchar_t s[1024];
	VA_PRINTS(s);
	SetWindowText(hWnd, s);
}

void DebugOutTitle(wchar_t* fmt, ...)
{
	wchar_t s[1024];
	VA_PRINTS(s);
	SetWindowText(hWnd, s);
}
/////////////////////////

void InitDirectX(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;

	// retrieve window width & height so that we can create backbuffer height & width accordingly 
	RECT r;
	GetClientRect(hWnd, &r);

	BackBufferWidth = r.right + 1;
	BackBufferHeight = r.bottom + 1;

	d3dpp.BackBufferHeight = BackBufferHeight;
	d3dpp.BackBufferWidth = BackBufferWidth;

	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddv);
	if (d3ddv == NULL)
	{
		DebugOut((wchar_t*)L"[ERROR] CreateDevice failed\n %s %d ", __FILE__, __LINE__);
		return;
	}

	d3ddv->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer); 

	// Initialize Direct3DX helper library

	D3DXCreateSprite(d3ddv, &spriteHandler);
	DebugOut((wchar_t*)L"[INFO] InitDirectX OK\n");
}

/*
* Load game resources. In this example, we only load a brick image 
*/

void LoadResources()
{
	HRESULT result = D3DXCreateTextureFromFileEx(
		d3ddv,
		JASON_TEXTURE_PATH,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_UNKNOWN,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DCOLOR_XRGB(255, 255, 255),
		NULL,
		NULL,
		&texJason);

	if (result != D3D_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] CreateTextureFromFileEx % s failed\n", JASON_TEXTURE_PATH);
		return;
	}
	DebugOut((wchar_t*)L"[INFO] Texture loaded Ok: %s \n", JASON_TEXTURE_PATH);
}

/*
	Update world status for this frame
	dt: time period between beginning of last frame and beginning of this frame

	IMPORTANT: no render-related code should be used inside this function 
*/

void Update(DWORD dt)
{
	//Uncomment the whole function to see the brick moves and bounces back when hitting left and right edges
	if (jason_x >= JASON_START_X && jason_y <= JASON_START_Y)
	{
		jason_y = JASON_START_Y;
		jason_x += jason_vx * dt;
	}
		

	if (jason_x >= BackBufferWidth - JASON_WIDTH && jason_y >= JASON_START_Y) 
	{
		jason_x = BackBufferWidth - JASON_WIDTH;
		jason_y += jason_vx * dt;
	} 

	if (jason_y >= BackBufferHeight - JASON_HEIGHT && jason_x <= BackBufferWidth - JASON_WIDTH)
	{
		jason_y = BackBufferHeight - JASON_HEIGHT;
		jason_x -= jason_vx * dt;
	}

	if (jason_y <= BackBufferHeight - JASON_HEIGHT && jason_x <= JASON_START_X)
	{
		jason_x = JASON_START_X;
		jason_y -= jason_vx * dt;
	}

}

/*
	Render a frame
	IMPORTANT: world status must NOT be changed during rendering
*/
void Render()
{
	if (d3ddv->BeginScene())
	{
		// Clear the whole window with a color
		d3ddv->ColorFill(backBuffer, NULL, BACKGROUND_COLOR);

		spriteHandler->Begin(D3DXSPRITE_ALPHABLEND);

		D3DXVECTOR3 p(jason_x, jason_y, 0);
		spriteHandler->Draw(texJason, NULL, NULL, &p, D3DCOLOR_WHITE);

		DebugOutTitle((wchar_t*)L"%s (%0.1f,%0.1f) v:%0.1f", WINDOW_TITLE, jason_x, jason_y, jason_vx);

		spriteHandler->End();
		d3ddv->EndScene();
	}

	// Display back buffer content to the screen
	d3ddv->Present(NULL, NULL, NULL, NULL);
}

HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	//Try this to see how the debug function prints out file and line 
	//wc.hInstance = (HINSTANCE)-100; 

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(hInstance, (LPCWSTR)WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd =
		CreateWindow(
			WINDOW_CLASS_NAME,
			WINDOW_TITLE,
			WS_OVERLAPPEDWINDOW, // WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenWidth,
			ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL);

	if (!hWnd)
	{
		DWORD ErrCode = GetLastError();
		DebugOut((wchar_t*)L"[ERROR] CreateWindow failed! ErrCode: %d\nAt: %s %d \n", ErrCode, _W(__FILE__), __LINE__);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int Run()
{
	MSG msg;
	int done = 0;
	DWORD frameStart = GetTickCount();
	DWORD tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD now = GetTickCount();

		// dt: the time between (beginning of last frame) and now
		// this frame: the frame we are about to render
		DWORD dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;
			Update(dt);
			Render();
		}
		else
			Sleep(tickPerFrame - dt);
	}

	return 1;
}

void CleanUp()
{
	texJason->Release();
	spriteHandler->Release();
	backBuffer->Release();
	d3ddv->Release();
	d3d->Release();

	DebugOut((wchar_t*)L"[INFO] Cleanup Ok\n");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hWnd = CreateGameWindow(hInstance, nCmdShow, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (hWnd == 0) return 0;

	InitDirectX(hWnd);

	LoadResources();
	Run();
	CleanUp();

	return 0;
}