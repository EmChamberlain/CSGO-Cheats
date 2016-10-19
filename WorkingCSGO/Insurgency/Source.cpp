#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <cstdlib>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <sstream>
#include <process.h>
#include <atlimage.h>



#define CRTDBG_MAP_ALLOC
/* MyDbgNew.h
/* Defines global operator new to allocate from
/* client blocks
*/
#ifdef _DEBUG
#define MYDEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
//allocations to be of _CLIENT_BLOCK type
#else
#define MYDEBUG_NEW
#endif // _DEBUG


/* MyApp.cpp
/*  Compile options needed: /Zi /D_DEBUG /MLd
/*            or use a
/*      Default Workspace for a Console Application to
/*      build a Debug version
*/




#ifdef _DEBUG
#define new MYDEBUG_NEW
#endif




using namespace std;

const int choiceSideLength = 30;
const int playerCount = 32;
const int desiredFPS = 60;
const int playerHeight = 72;

PROCESSENTRY32 gameProcess;

//these tend to change***********************************
const DWORD playerBase = 0x00a4a98c;
const DWORD entityBase = 0x049ec194;
const DWORD viewMat1Offset = 0x049e1724;
//const DWORD viewMat2Offset = 0x9CC00168;//NU
//const DWORD viewMat3Offset = 0x9CC00478;//NU
//*******************************************************
const DWORD xyzOffset = 0x00000134;
const DWORD teamOffset = 0x000000f0;
const DWORD healthOffset = 0x000000fc;
const DWORD isDeadOffest = 0x0000025b;
const DWORD loopDistance = 0x10;

const DWORD crosshairOffset = 0x000023f8;


DWORD pid;
DWORD client;
DWORD engine;
DWORD vgui;
HANDLE process;

HWND csWindow;
HWND EspHWND;
HANDLE Handle;
RECT rect;

HPEN fPen = CreatePen(PS_SOLID, 1, RGB(102, 0, 204));
HPEN enPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
HPEN redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
HBRUSH redBr = CreateSolidBrush(RGB(255, 0, 0));
HPEN greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
HBRUSH greenBr = CreateSolidBrush(RGB(0, 255, 0));
HPEN OutlinePen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
LPCWSTR WName = L"Window";
HINSTANCE Hinstance;

bool END = false;
bool fEspBool = false;
bool enEspBool = true;
bool triggerBool = true;

struct Me_t
{
	DWORD local;
	int team;
	int health;
	float position[3];
	float viewMat[4][4];
	int crossID;

	void getInfo()
	{


		ReadProcessMemory(process, (LPVOID)(client + playerBase), &local, sizeof(DWORD), 0);
		ReadProcessMemory(process, (LPVOID)(local + teamOffset), &team, sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(local + healthOffset), &health, sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(local + xyzOffset), &(position), sizeof(float[3]), 0);

		ReadProcessMemory(process, (LPVOID)(client + viewMat1Offset), &viewMat, sizeof(viewMat), 0);
		ReadProcessMemory(process, (LPVOID)(local + crosshairOffset), &crossID, sizeof(int), 0);




	};

}Me;



struct Player_t
{
	DWORD base;
	int team;
	int health;
	float position[3];
	bool isDead;


};

struct AllList_t
{
	Player_t playerArr[playerCount];
	void updateInfo(int playerNum)
	{
		ReadProcessMemory(process, (LPVOID)(client + entityBase + (playerNum*loopDistance)), &(playerArr[playerNum].base), sizeof(DWORD), 0);
		ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + teamOffset), &(playerArr[playerNum].team), sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + healthOffset), &(playerArr[playerNum].health), sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + xyzOffset), &(playerArr[playerNum].position), sizeof(float[3]), 0);
		ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + isDeadOffest), &(playerArr[playerNum].isDead), sizeof(bool), 0);

	};

}AllList;





DWORD modName(wchar_t* moduleName, DWORD pid)
{
	MODULEENTRY32 modEntry = { 0 };
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap == INVALID_HANDLE_VALUE)
		return 0;
	modEntry.dwSize = sizeof(MODULEENTRY32);
	Module32First(snap, &modEntry);
	do
	{

		if (!_wcsicmp(modEntry.szModule, moduleName))
		{
			CloseHandle(snap);
			return (DWORD)(modEntry.modBaseAddr);
		}
	} while (Module32Next(snap, &modEntry));
	return 0;
}
float getWorldDistance(float* me, float* en)
{
	return sqrt(pow((en[0] - me[0]), 2) +
		pow((en[1] - me[1]), 2) +
		pow((en[2] - me[2]), 2)
		);
}

bool worldtoscreen(float *beg, float *end, RECT rectangle, bool eye)
{

	float test = 0.0f;
	if (eye)
	{
		end[1] = Me.viewMat[1][0] * beg[0] + Me.viewMat[1][1] * beg[1] + Me.viewMat[1][2] * (beg[2] + playerHeight) + Me.viewMat[1][3];
		test = Me.viewMat[3][0] * beg[0] + Me.viewMat[3][1] * beg[1] + Me.viewMat[3][2] * (beg[2] + playerHeight) + Me.viewMat[3][3];
		end[0] = Me.viewMat[0][0] * beg[0] + Me.viewMat[0][1] * beg[1] + Me.viewMat[0][2] * (beg[2] + playerHeight) + Me.viewMat[0][3];
	}
	else
	{
		end[1] = Me.viewMat[1][0] * beg[0] + Me.viewMat[1][1] * beg[1] + Me.viewMat[1][2] * beg[2] + Me.viewMat[1][3];
		test = Me.viewMat[3][0] * beg[0] + Me.viewMat[3][1] * beg[1] + Me.viewMat[3][2] * beg[2] + Me.viewMat[3][3];
		end[0] = Me.viewMat[0][0] * beg[0] + Me.viewMat[0][1] * beg[1] + Me.viewMat[0][2] * beg[2] + Me.viewMat[0][3];
	}

	if (test < 0.01f)
		return false;


	end[0] *= float(1 / test);
	end[1] *= float(1 / test);

	int width = (int)(rectangle.right - rectangle.left);
	int height = (int)(rectangle.bottom + rectangle.left);

	float x = (float)(width / 2);
	float y = (float)(height / 2);

	x += (float)(0.5 * end[0] * width + 0.5);
	y -= (float)(0.5 * end[1] * height + 0.5);
	end[0] = x + rectangle.left;
	end[1] = y + rectangle.top;

	return true;


}

//not mine
void Draw(HDC hdc, int x, int y, int eyex, int eyey, bool teamate, int health)
{
	int height = eyey - y;
	int width = height / 2;

	//int width = (int)(18100 / distance);
	//int height = (int)(36000 / distance);
	SelectObject(hdc, OutlinePen);
	SelectObject(hdc, WHITE_BRUSH);


	if (teamate && fEspBool)
	{
		Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		SelectObject(hdc, fPen);
		Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		return;
	}
	if (!teamate && enEspBool)
	{
		Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		DeleteObject(enPen);
		if (health >= 50)
		{
			enPen = CreatePen(PS_SOLID, 1, RGB(255 * (100 - health) / 50, 255, 0));
		}
		else
		{
			enPen = CreatePen(PS_SOLID, 1, RGB(255, 255 - (255 * (50 - health) / 50), 0));
		}
		SelectObject(hdc, enPen);
		Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		return;
	}
	return;

}





HANDLE GetProcess(const wchar_t* name, PROCESSENTRY32 *procEntry)
{
	pid = 0;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process32;
	process32.dwSize = sizeof(PROCESSENTRY32);
	if (snap == INVALID_HANDLE_VALUE)
	{
		CloseHandle(snap);
		return 0;
	}
	if (!(Process32First(snap, &process32)))
	{
		CloseHandle(snap);
		return 0;
	}
	do
	{
		if (!_wcsicmp(process32.szExeFile, name))
		{
			memcpy((void*)procEntry, (void*)&process32, sizeof(PROCESSENTRY32));
			CloseHandle(snap);
			pid = process32.th32ProcessID;
			break;
		}
	} while (Process32Next(snap, &process32));
	CloseHandle(snap);

	try{
		return OpenProcess(PROCESS_VM_READ, FALSE, pid);
	}
	catch (exception &e)
	{
		e.~exception();
	}

}

void leftclick()
{
	if ((GetKeyState(VK_LBUTTON) & 0x80) == 0)
	{
		INPUT    Input = { 0 };

		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		::SendInput(1, &Input, sizeof(INPUT));
		Sleep(10);

		Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		::SendInput(1, &Input, sizeof(INPUT));
		Sleep(500);
	}

}
void trigger(void* ptr)
{

	if (Me.crossID > playerCount)
		return;
	if (Me.crossID <= 0)
		return;

	if (AllList.playerArr[Me.crossID - 1].team != Me.team)
	{


		leftclick();
	}
	return;
}
//void espSingle(int playerNum)
//{
//
//	GetWindowRect(csWindow, &rect);
//
//
//	Me.getInfo();
//	AllList.playerArr[playerNum];
//	bool teamate = false;
//	float en[3];
//	if (AllList.playerArr[playerNum].team == Me.team)
//	{
//		teamate = true;
//	}
//	else
//	{
//		teamate = false;
//
//	}
//	//cout << playerNum << endl;
//	if (worldtoscreen(AllList.playerArr[playerNum].position, en, rect) && AllList.playerArr[playerNum].health >= 1)
//	{
//		//cout << "worldtoscreen true" << endl;
//		Draw(buffer,(int)(en[0] - rect.left), (int)(en[1] - rect.top), getWorldDistance(Me.position, AllList.playerArr[playerNum].position), teamate, AllList.playerArr[playerNum].health);
//
//	}
//
//
//
//	return;
//}



DWORD WINAPI TriggerLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (triggerBool)
		{
			Me.getInfo();
			trigger(nullptr);
		}
		else
			Sleep(5000);
	}
	ExitThread(0);
	return 0;
}

void fpsThrFunct(void *ptr)
{
	while (true)
		Sleep(DWORD(1000 / desiredFPS));
}

//void espLooper(void *ptr)
//{
//
//	
//
//	DeleteObject(buffer);
//	return;
//}
void drawChoiceRectangles(HDC hdc)
{
	int spaceDist = int(choiceSideLength / 2);
	int startX = rect.left + spaceDist;
	int startY = rect.top + spaceDist;
	if (enEspBool)
	{
		SelectObject(hdc, greenPen);
		SelectObject(hdc, greenBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}
	else
	{
		SelectObject(hdc, redPen);
		SelectObject(hdc, redBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}
	startX += spaceDist + (spaceDist / 2);
	if (fEspBool)
	{
		SelectObject(hdc, greenPen);
		SelectObject(hdc, greenBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}
	else
	{
		SelectObject(hdc, redPen);
		SelectObject(hdc, redBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}
	startX += spaceDist + (spaceDist / 2);
	if (triggerBool)
	{
		SelectObject(hdc, greenPen);
		SelectObject(hdc, greenBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}
	else
	{
		SelectObject(hdc, redPen);
		SelectObject(hdc, redBr);
		Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
	}

	SelectObject(hdc, WHITE_BRUSH);
	return;

}
void resetPlayerList()
{
	for (int x = 0; x < playerCount; x++)
	{

		AllList.playerArr[x].isDead = true;
		AllList.playerArr[x].health = 0;
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		GetWindowRect(csWindow, &rect);
		int win_width = rect.right - rect.left;
		int win_height = rect.bottom + rect.left;
		PAINTSTRUCT ps;
		HDC Memhdc;
		HDC hdc;
		HBITMAP Membitmap;
		hdc = BeginPaint(hwnd, &ps);
		Memhdc = CreateCompatibleDC(hdc);
		Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
		SelectObject(Memhdc, Membitmap);
		FillRect(Memhdc, &rect, WHITE_BRUSH);

		if (fEspBool || enEspBool)
		{
			for (int playerNum = 0; playerNum < playerCount; playerNum++)
			{
				if (!AllList.playerArr[playerNum].isDead)
				{
					float *en = (float*)(malloc(sizeof(float) * 2));
					bool teamate = AllList.playerArr[playerNum].team == Me.team;
					if (worldtoscreen(AllList.playerArr[playerNum].position, en, rect, false) && AllList.playerArr[playerNum].health > 0 && (getWorldDistance(Me.position, AllList.playerArr[playerNum].position))>float(playerHeight / 15))
					{
						float *enEye = (float*)(malloc(sizeof(float) * 2));
						worldtoscreen(AllList.playerArr[playerNum].position, enEye, rect, true);
						Draw(Memhdc, en[0], en[1], enEye[0], enEye[1], teamate, AllList.playerArr[playerNum].health);
					}
				}
			}
		}

		drawChoiceRectangles(Memhdc);

		BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
		DeleteObject(Membitmap);
		DeleteDC(Memhdc);
		DeleteDC(hdc);
		EndPaint(hwnd, &ps);
		ValidateRect(hwnd, &rect);
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
DWORD WINAPI WorkLoop(LPVOID PARAMS)
{

	while (!END)
	{
		InvalidateRect(EspHWND, &rect, true);
		Sleep(DWORD(1000 / desiredFPS));
	}
	ExitThread(0);
}
DWORD WINAPI HotkeyLoop(LPVOID PARAMS)
{
	while (!(GetAsyncKeyState(VK_F6)<0 && GetAsyncKeyState(VK_CONTROL)<0))
	{
		if (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			enEspBool = !enEspBool;
			while (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(200);
		}
		if (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			fEspBool = !fEspBool;
			while (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(200);
		}
		if (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			triggerBool = !triggerBool;
			while (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(200);
		}
		if (GetAsyncKeyState(VK_F10) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			resetPlayerList();
			while (GetAsyncKeyState(VK_F10) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(200);
		}
		Sleep(200);
	}
	END = true;
	SendMessage(EspHWND, WM_CLOSE, 0, 0);
	UnregisterClass(WName, Hinstance);
	ExitThread(0);
}
DWORD WINAPI PlayerLoop(LPVOID PARAMS)
{
	while (!END)
	{
		Me.getInfo();
		for (int x = 0; x < playerCount; x++)
		{
			AllList.updateInfo(x);
		}

	}
	ExitThread(0);
}
DWORD WINAPI Main(LPVOID Params) {

	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

	csWindow = NULL;
	while (!csWindow)
	{
		csWindow = FindWindow(0, L"Counter-Strike: Global Offensive");
		Sleep(200);
	}
	GetClientRect(csWindow, &rect);
	process = NULL;
	while (!process)
	{
		process = GetProcess(L"csgo.exe", &gameProcess);
		Sleep(200);
	}

	while (client == 0x0)
	{

		client = modName(L"client.dll", gameProcess.th32ProcessID);

	}
	while (engine == 0x0)
	{
		engine = modName(L"engine.dll", gameProcess.th32ProcessID);

	}





	WNDCLASSEX WClass;
	MSG Msg;
	WClass.cbSize = sizeof(WNDCLASSEX);
	WClass.style = 0;
	WClass.lpfnWndProc = WndProc;
	WClass.cbClsExtra = 0;
	WClass.cbWndExtra = 0;
	WClass.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLong(csWindow, GWL_HINSTANCE));
	WClass.hIcon = NULL;
	WClass.hCursor = NULL;
	WClass.hbrBackground = WHITE_BRUSH;
	WClass.lpszMenuName = IDC_CROSS;
	WClass.lpszClassName = WName;
	WClass.hIconSm = NULL;
	if (!RegisterClassEx(&WClass))
	{
		MessageBox(NULL, L"Couldnt register", L"Problem", MB_OK);
		return 0;
	}

	EspHWND = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED,
		WName,
		WName,
		WS_POPUP,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom + rect.left,
		NULL, NULL, Hinstance, NULL);

	if (EspHWND == NULL)
	{
		MessageBox(NULL, L"Couldnt create window", L"Problem", MB_OK);
	}
	SetLayeredWindowAttributes(EspHWND, RGB(255, 255, 255), 255, LWA_COLORKEY);
	ShowWindow(EspHWND, 1);


	CreateThread(0, 0x1000, &TriggerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &WorkLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &PlayerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &HotkeyLoop, 0, 0, 0);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		Sleep(1);
	}

	ExitThread(0);
	return 0;

}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {CreateThread(0, 0x1000, &Main, 0, 0, 0); }
	}
	return TRUE;
}