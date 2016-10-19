
#define CRTDBG_MAP_ALLOC





#include "stdafx.h"

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


const int playerCount = 32;
const int desiredFPS = 25;
PROCESSENTRY32 gameProcess;

const int QUIT_KEY = 0x75;
const DWORD xyzOffset = 0x134;
const DWORD playerBase = 0x00A7094C;
const DWORD entityBase = 0x04A13264;
const DWORD teamOffset = 0xF0;
const DWORD healthOffset = 0xFC;
const DWORD loopDistance = 0x10;
const DWORD viewMat1Offset = 0x04a087a4;
//const DWORD viewMat2Offset = 0x04A088E4;
//const DWORD viewMat3Offset = 0x04A08BF4;
const DWORD crosshairOffset = 0x23F8;


DWORD pid;
DWORD client;
DWORD engine;
DWORD vgui;
HANDLE process;

HWND csWindow;
HWND EspHWND;
HANDLE Handle;
RECT rect;

Gdiplus::Pen *enPen;
Gdiplus::Pen *fPen;
Gdiplus::SolidBrush *blBr;
//not mine
HPEN BoxPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
//not mine
HPEN OutlinePen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
LPCWSTR WName = L" ";
HINSTANCE Hinstance;



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
		ReadProcessMemory(process, (PBYTE*)(client + playerBase), &local, sizeof(DWORD), 0);
		ReadProcessMemory(process, (PBYTE*)(local + teamOffset), &team, sizeof(int), 0);
		ReadProcessMemory(process, (PBYTE*)(local + healthOffset), &health, sizeof(int), 0);
		ReadProcessMemory(process, (PBYTE*)(local + xyzOffset), &(position), sizeof(float[3]), 0);

		ReadProcessMemory(process, (PBYTE*)(client + viewMat1Offset), &viewMat, sizeof(viewMat), 0);
		ReadProcessMemory(process, (PBYTE*)(local + crosshairOffset), &crossID, sizeof(int), 0);




	};

}Me;



struct Player_t
{
	DWORD base;
	int team;
	int health;
	float position[3];
	float angles[3];


};

struct AllList_t
{

	Player_t getInfo(int playerNum)
	{
		Player_t p;

		ReadProcessMemory(process, (PBYTE*)(client + entityBase + (playerNum*loopDistance)), &(p.base), sizeof(DWORD), 0);
		ReadProcessMemory(process, (PBYTE*)(p.base + teamOffset), &(p.team), sizeof(int), 0);
		ReadProcessMemory(process, (PBYTE*)(p.base + healthOffset), &(p.health), sizeof(int), 0);
		ReadProcessMemory(process, (PBYTE*)(p.base + xyzOffset), &(p.position), sizeof(float[3]), 0);


		return p;


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

bool worldtoscreen(float *beg, float *end, RECT rect)
{

	float test = 0;

	end[1] = Me.viewMat[1][0] * beg[0] + Me.viewMat[1][1] * beg[1] + Me.viewMat[1][2] * beg[2] + Me.viewMat[1][3];
	test = Me.viewMat[3][0] * beg[0] + Me.viewMat[3][1] * beg[1] + Me.viewMat[3][2] * beg[2] + Me.viewMat[3][3];
	end[0] = Me.viewMat[0][0] * beg[0] + Me.viewMat[0][1] * beg[1] + Me.viewMat[0][2] * beg[2] + Me.viewMat[0][3];


	if (test<0.01)
		return false;


	end[0] *= (1 / test);
	end[1] *= (1 / test);

	int width = (int)(rect.right - rect.left);
	int height = (int)(rect.bottom - rect.top);

	float x = (float)(width / 2);
	float y = (float)(height / 2);
	x += (float)(0.5 * end[0] * width + 0.5);
	y -= (float)(0.5 * end[1] * height + 0.5);
	end[0] = x + rect.left;
	end[1] = y + rect.top;
	return true;


}

//not mine
void Draw(HDC hdc, int x, int y, float distance, bool teamate, int health)
{

	int width = int(20000 / distance);
	int height = int(45000 / distance);
	SelectObject(hdc, OutlinePen);
	SelectObject(hdc, WHITE_BRUSH);
	Rectangle(hdc, x - (width / 2), y - height, x + (width / 2), y);
	SelectObject(hdc, BoxPen);
	Rectangle(hdc, x - (width / 2), y - height, x + (width / 2), y);

}

void enDrawBorderBox(int x, int y, int w, int h, int health, Gdiplus::Graphics *g)
{




	if (health >= 50)
	{
		enPen->SetColor(Gdiplus::Color(255 * (100 - health) / 50, 255, 0));
	}
	else
	{
		enPen->SetColor(Gdiplus::Color(255, 255 - (255 * (50 - health) / 50), 0));
	}
	g->DrawRectangle(enPen, x, y, w, h);




}

void DrawBorderBox(int x, int y, int w, int h, int health, Gdiplus::Graphics *g)
{


	g->DrawRectangle(fPen, x, y, w, h);

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

	if (AllList.getInfo(Me.crossID - 1).team != Me.team)
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
//	AllList.getInfo(playerNum);
//	bool teamate = false;
//	float en[3];
//	if (AllList.getInfo(playerNum).team == Me.team)
//	{
//		teamate = true;
//	}
//	else
//	{
//		teamate = false;
//
//	}
//	//cout << playerNum << endl;
//	if (worldtoscreen(AllList.getInfo(playerNum).position, en, rect) && AllList.getInfo(playerNum).health >= 1)
//	{
//		//cout << "worldtoscreen true" << endl;
//		Draw(buffer,(int)(en[0] - rect.left), (int)(en[1] - rect.top), getWorldDistance(Me.position, AllList.getInfo(playerNum).position), teamate, AllList.getInfo(playerNum).health);
//
//	}
//
//
//
//	return;
//}



DWORD WINAPI TriggerLoop(LPVOID PARAMS)
{
	while (!GetAsyncKeyState(QUIT_KEY))
	{
		Me.getInfo();
		trigger(nullptr);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		cout << "Paint got called" << endl;
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



		//thread fpsThread(fpsThrFunct, nullptr);
		for (int playerNum = 0; playerNum < playerCount; playerNum++)
		{

			GetWindowRect(csWindow, &rect);


			Me.getInfo();
			AllList.getInfo(playerNum);
			bool teamate = false;
			float en[3];
			if (AllList.getInfo(playerNum).team == Me.team)
			{
				teamate = true;
			}
			else
			{
				teamate = false;

			}
			//cout << playerNum << endl;
			if (worldtoscreen(AllList.getInfo(playerNum).position, en, rect) && AllList.getInfo(playerNum).health >= 1)
			{
				//cout << "worldtoscreen true" << endl;
				Draw(Memhdc, (int)(en[0] - rect.left), (int)(en[1] - rect.top), floorf(10.0f * (getWorldDistance(Me.position, AllList.getInfo(playerNum).position)* 0.01905f) + 0.5f) / 10.0f, teamate, AllList.getInfo(playerNum).health);

			}

		}

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

	while (!GetAsyncKeyState(QUIT_KEY))
	{

		InvalidateRect(EspHWND, &rect, true);
		Sleep(33);
	}
	ExitThread(0);
}
DWORD WINAPI Main(LPVOID Params) {
	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	bool espBool = false;
	bool triggerBool = false;
	cout << "ESP and TriggerBot by Mazras" << endl;
	cout << "Please choose modules to activate.(y/n)" << endl;
	cout << "1.ESP" << endl;
	string input;
	cin >> input;
	espBool = input.compare("y") == 0;
	input = "";
	cout << "2.Trigger Bot" << endl;
	cin >> input;
	triggerBool = input.compare("y") == 0;




	cout << "Hold F6 to quit" << endl;



	try{ process = GetProcess(L"csgo.exe", &gameProcess); }
	catch (exception &e){ e.~exception(); }
	Sleep(20);


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
		MessageBox(NULL, L"Window could be created", L"Problem", MB_OK);
	}
	SetLayeredWindowAttributes(EspHWND, RGB(255, 255, 255), 255, LWA_COLORKEY);
	ShowWindow(EspHWND, 1);

	//CreateThread(0, 0x1000, &ReadPlayerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &TriggerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &WorkLoop, 0, 0, 0);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		cout << "Got a message" << endl;
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