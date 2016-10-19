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
//#include <atlimage.h>
#include <random>
#include <ctime>
#include "draw.h"
#include "dxerr.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "dwmapi.lib" )
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







#ifdef _DEBUG
#define new MYDEBUG_NEW
#endif




using namespace std;

const int choiceSideLength = 30;
const int playerCount = 32;
const int desiredFPS = 60;
const int playerHeight = 72;
const int dotRad = 3;
//const float maxAimbotDistance = 15;
float maxFovChange = 1.5f;
//int shotwait = 1;
const float smoothamount = 100.f;

float PitchMinPunch = 2.f;
float PitchMaxPunch = 2.f;
float YawMinPunch = 2.f;
float YawMaxPunch = 2.f;

PROCESSENTRY32 gameProcess;

//these tend to change*********************************** Last realistic date that was ban flagged 07-23-15
const DWORD playerBase = 0x6C96D0;
const DWORD entityBase = 0x6E29E4;
const DWORD viewMat1Offset = 0x6CDC54;
const DWORD enginePositionOffset = 0x451BF0;//also called dwClientState
//const DWORD viewMat2Offset = 0x04A129F4;//not updated
//const DWORD viewMat3Offset = 0x04A12D04;//not updated 
//******************************************************* 
//const DWORD dormantOffset = 0x000000e9;
//const DWORD boneMatOffset = 0x00000a78;
const DWORD xyzOffset = 0x00000134;
const DWORD teamOffset = 0x000000f0;
const DWORD healthOffset = 0x000000fc;
//const DWORD isDeadOffest = 0x0000025b;
const DWORD loopDistance = 0x10;
//const int tHeadBone = 11;
//const int ctHeadBone = 10;
//const int chestBone = 4;
const DWORD crosshairOffset = 0x242C;//2424

//const DWORD velocityOffset = 0x00000110;
//const DWORD shotsFiredOffset = 0x00001d60;
//const DWORD punchAngOffset = 0x000013e8;
//const DWORD vecViewOffset = 0x00000104;
const DWORD viewAngOffset = 0x4B40;


DWORD pid;
DWORD client;
DWORD engine;
DWORD vgui;
HANDLE process;

HWND csWindow;
HWND EspHWND;
HANDLE Handle;
RECT rect;


LPCWSTR WName = L"Window";
HINSTANCE Hinstance;

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;

CDraw cdraw;
static MARGINS margins = { -1, -1, -1, -1 };

int win_width = 800;
int win_height = 600;

bool END = false;
bool fEspBool = false;
bool enEspBool = true;
bool triggerBool = false;
bool headDotBool = true;
bool aimbotBool = true;
bool aimbotHeadBool = true;
bool stealthModeBool = false;

std::string debugMsg = "no changes";
struct vec3_t
{
	float x;
	float y;
	float z;
};
struct Me_t
{
	DWORD local;
	DWORD enginePointerAddr;
	int team;
	int health;
	float position[3];
	float viewMat[4][4];
	int crossID;
	int playerToAimAt;
	int shotsFired;
	vec3_t vecVelocity;
	vec3_t punchAngs;
	vec3_t vecView;
	vec3_t viewAng;
	void getInfo()
	{


		ReadProcessMemory(process, (LPVOID)(client + playerBase), &local, sizeof(DWORD), 0);
		ReadProcessMemory(process, (LPVOID)(engine + enginePositionOffset), &enginePointerAddr, sizeof(DWORD), 0);
		ReadProcessMemory(process, (LPVOID)(local + teamOffset), &team, sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(local + healthOffset), &health, sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(local + xyzOffset), &(position), sizeof(float[3]), 0);

		ReadProcessMemory(process, (LPVOID)(client + viewMat1Offset), &viewMat, sizeof(viewMat), 0);
		ReadProcessMemory(process, (LPVOID)(local + crosshairOffset), &crossID, sizeof(int), 0);
		//ReadProcessMemory(process, (LPVOID)(local + shotsFiredOffset), &shotsFired, sizeof(int), 0);
		//ReadProcessMemory(process, (LPVOID)(local + velocityOffset), &(vecVelocity), sizeof(vec3_t), 0);
		//ReadProcessMemory(process, (LPVOID)(local + punchAngOffset), &(punchAngs), sizeof(vec3_t), 0);
		//ReadProcessMemory(process, (LPVOID)(local + vecViewOffset), &(vecView), sizeof(vec3_t), 0);
		//ReadProcessMemory(process, (LPVOID)(enginePointerAddr + viewAngOffset), &(viewAng), sizeof(vec3_t), 0);


	};
	void setViewAngles(vec3_t aimAng)
	{
		//WriteProcessMemory(process, (LPVOID)(enginePointerAddr + viewAngOffset), &(aimAng), sizeof(vec3_t), 0);
	};

}Me;


struct Player_t
{
	DWORD base;
	DWORD boneBase;

	int team;
	int health;
	float position[3];
	bool isDead;
	bool isValid;
	vec3_t vecVelocity;

	float headPos[3];
	float chestPos[3];

	bool isDormant;


};

struct AllList_t
{
	Player_t playerArr[playerCount];
	void updateInfo(int playerNum)
	{

		bool testBool = true;
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(client + entityBase + (playerNum*loopDistance)), &(playerArr[playerNum].base), sizeof(DWORD), 0);
		//testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + boneMatOffset), &(playerArr[playerNum].boneBase), sizeof(DWORD), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + teamOffset), &(playerArr[playerNum].team), sizeof(int), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + healthOffset), &(playerArr[playerNum].health), sizeof(int), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + xyzOffset), &(playerArr[playerNum].position), sizeof(float[3]), 0);
		//testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + isDeadOffest), &(playerArr[playerNum].isDead), sizeof(bool), 0);
		//testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + velocityOffset), &(playerArr[playerNum].vecVelocity), sizeof(vec3_t), 0);
		//testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + dormantOffset), &(playerArr[playerNum].isDormant), sizeof(bool), 0);

		//testBool = testBool && 0 != readBones(playerNum);

		playerArr[playerNum].isValid = !(!testBool || playerArr[playerNum].isDormant);

		return;
	};
	/*bool readBones(int playerNum)
	{
		bool toReturn = true;
		int headOff = 0;
		if (playerArr[playerNum].team == 3)
		{
			//headOff = ctHeadBone;
		}
		else
		{
			//headOff = tHeadBone;
		}
		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * headOff + 0x0C), &(playerArr[playerNum].headPos[0]), sizeof(float), 0);
		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * headOff + 0x1C), &(playerArr[playerNum].headPos[1]), sizeof(float), 0);
		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * headOff + 0x2C), &(playerArr[playerNum].headPos[2]), sizeof(float), 0);

		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * chestBone + 0x0C), &(playerArr[playerNum].chestPos[0]), sizeof(float), 0);
		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * chestBone + 0x1C), &(playerArr[playerNum].chestPos[1]), sizeof(float), 0);
		toReturn = toReturn && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * chestBone + 0x2C), &(playerArr[playerNum].chestPos[2]), sizeof(float), 0);
		return toReturn;
	};
	*/

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
float distanceFromCenter(float x, float y)
{
	float centerX = win_width * 0.5f;
	float centerY = win_height * 0.5f;
	return sqrt((x - centerX)*(x - centerX) + (y - centerY)*(y - centerY));
}
float getVecDistance(vec3_t start, vec3_t end)
{
	return sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2) + pow(end.z - start.z, 2));
}
vec3_t getZeroVec()
{
	vec3_t toReturn;
	toReturn.x = 0;
	toReturn.y = 0;
	toReturn.z = 0;
	return toReturn;
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
/*void drawtext(HDC hdc, int x, int y, const char * text)
{
	SetTextColor(hdc, RGB(0, 255, 0));
	SetBkColor(hdc, RGB(255, 255, 255));
	TextOutA(hdc, x, y, text, strlen(text));
}*/
void Draw(int x, int y, int eyex, int eyey, bool teamate, int health)
{
	int height = y - eyey;
	int width = height / 2;
	
	/*if (width < 5)
	{
		MessageBox(NULL, L"Width less than 5", L"Problem", MB_OK);
		std::string test1 = "Height=" + std::to_string(height);
		MessageBoxA(NULL, test1.c_str(), "Problem", MB_OK);
	}*/
	if (teamate && fEspBool)
	{
		//Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		//MessageBox(NULL, L"Width2:" + width, L"Problem", MB_OK);
		cdraw.Box(x - (width / 2) - 1, y - height - 1, width + 2, height + 2, 3, BLACK(255));
		cdraw.Box(x - (width / 2), y - height, width, height, 1, D3DCOLOR_ARGB(255, 102, 0, 104));
		return;
	}
	if (!teamate && enEspBool)
	{
		//Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		DWORD color = D3DCOLOR_ARGB(255, 255, 255, 255);
		if (health >= 50)
		{
			//enPen = CreatePen(PS_SOLID, 1, RGB(255 * (100 - health) / 50, 255, 0));
			color = D3DCOLOR_ARGB(255, 255 * (100 - health) / 50, 255, 0);
		}
		else
		{
			//enPen = CreatePen(PS_SOLID, 1, RGB(255, 255 - (255 * (50 - health) / 50), 0));
			color = D3DCOLOR_ARGB(255, 255, 255 - (255 * (50 - health) / 50), 0);
		}
		//Rectangle(hdc, x - (width / 2), y + height, x + (width / 2), y);
		//MessageBox(NULL, L"Width3:" + width, L"Problem", MB_OK);
		cdraw.Box(x - (width / 2) - 1, y - height - 1, width + 2, height + 2, 3, BLACK(255));
		cdraw.Box(x - (width / 2), y - height, width, height, 1, color);
		return;
	}
	return;

}
/*int GetClosestPlayerToCrossHair(float maxAimDist)
{
float lowest = maxAimDist;
bool highestChanged = false;
if (Me.crossID > 0 && Me.crossID <= playerCount && AllList.playerArr[Me.crossID - 1].team != Me.team)
{
Me.playerToAimAt = Me.crossID - 1;
return;
}
for (int x = 0; x < playerCount; x++)
{
float *enHead = (float*)(malloc(sizeof(float) * 2));
float Dist;
if (worldtoscreen(AllList.playerArr[x].headPos, enHead, rect, false))
{
Dist = distanceFromCenter(enHead[0], enHead[1]);
}
else
{
Dist = maxAimDist;
}
if (Dist < lowest && AllList.playerArr[x].team != Me.team)
{
lowest = Dist;
Me.playerToAimAt = x;
highestChanged = true;
}
free(enHead);
}
if (!highestChanged)
{
Me.playerToAimAt = -1;
}
return;
}*/
int GetClosestPlayerToCrossHair()
{
	float lowest = FLT_MAX;
	int toReturn = -1;
	if (Me.crossID > 0 && Me.crossID <= playerCount && AllList.playerArr[Me.crossID - 1].team != Me.team)
	{
		return Me.crossID - 1;
	}
	for (int x = 0; x < playerCount; x++)
	{
		float *enHead = (float*)(malloc(sizeof(float) * 2));
		float Dist = FLT_MAX;
		if (worldtoscreen(AllList.playerArr[x].headPos, enHead, rect, false))
		{
			Dist = distanceFromCenter(enHead[0], enHead[1]);
		}
		if (Dist < lowest && AllList.playerArr[x].team != Me.team)
		{
			lowest = Dist;
			toReturn = x;

		}
		free(enHead);
	}
	return toReturn;
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
		return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
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
		Sleep(10);
	}

}
void sendDownPress()
{
	INPUT Input = { 0 };
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
}
void sendUpPress()
{
	INPUT Input = { 0 };
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
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
		Sleep(500);
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
bool targetWithinRange(int playerNum, float maxFovChange)
{

}
vec3_t velocityComp(vec3_t enPos, vec3_t enVel, vec3_t meVel)
{
	enPos.x += (enVel.x) * (0.2f / smoothamount);
	enPos.y += (enVel.y) * (0.2f / smoothamount);
	enPos.z += (enVel.z) * (0.2f / smoothamount);
	enPos.x -= (meVel.x) * (0.2f / smoothamount);
	enPos.y -= (meVel.y) * (0.2f / smoothamount);
	enPos.z -= (meVel.z) * (0.2f / smoothamount);
	return enPos;
}
vec3_t convertToVec(float in[3])
{
	vec3_t toReturn;
	toReturn.x = in[0];
	toReturn.y = in[1];
	toReturn.z = in[2];
	return toReturn;
}
float randomFloat(float min, float max)
{
	random_device Random;
	mt19937 RandomGen(Random());
	uniform_real<float> Randomdistribute(min, max);
	return Randomdistribute(RandomGen);
}
bool checkAngle(vec3_t in)
{
	return in.x <= 89.0f && in.x >= -89.0f && in.y <= 180.0f && in.y >= -180.0f;
}
vec3_t calcAngle(vec3_t PlayerPos, vec3_t EnemyPos, vec3_t PunchAngs, vec3_t VecViewOrigin)
{
	vec3_t AimAngles;
	vec3_t delta = { (PlayerPos.x - EnemyPos.x), (PlayerPos.y - EnemyPos.y), ((PlayerPos.z + VecViewOrigin.z) - EnemyPos.z) };
	float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
	AimAngles.x = atanf(delta.z / hyp) * 57.295779513082f - PunchAngs.x * randomFloat(PitchMinPunch, PitchMaxPunch);
	AimAngles.y = atanf(delta.y / delta.x) * 57.295779513082f - PunchAngs.y * randomFloat(YawMinPunch, YawMaxPunch);
	AimAngles.z = 0.0f;
	if (delta.x >= 0.0)
	{
		AimAngles.y += 180.0f;
	}
	return AimAngles;
}
vec3_t clampAngle(vec3_t SrcAngles)
{
	if (SrcAngles.x > 89.0f && SrcAngles.x <= 180.0f)
	{
		SrcAngles.x = 89.0f;
	}
	if (SrcAngles.x > 180.f)
	{
		SrcAngles.x -= 360.f;
	}
	if (SrcAngles.x < -89.0f)
	{
		SrcAngles.x = -89.0f;
	}
	if (SrcAngles.y > 180.f)
	{
		SrcAngles.y -= 360.f;
	}
	if (SrcAngles.y < -180.f)
	{
		SrcAngles.y += 360.f;
	}
	if (SrcAngles.z != 0.0f)
	{
		SrcAngles.z = 0.0f;
	}
	return SrcAngles;
}
vec3_t smoothAngle(vec3_t SrcAngles, vec3_t DestAngles, float SmoothAmount)
{
	vec3_t SmoothedAngles;
	SmoothedAngles.x = DestAngles.x - SrcAngles.x;
	SmoothedAngles.y = DestAngles.y - SrcAngles.y;
	SmoothedAngles.z = 0.0f;
	SmoothedAngles = clampAngle(SmoothedAngles);
	SmoothedAngles.x = SrcAngles.x + SmoothedAngles.x / 100.0f * smoothamount;
	SmoothedAngles.y = SrcAngles.y + SmoothedAngles.y / 100.0f * smoothamount;
	SmoothedAngles.z = 0.0f;
	SmoothedAngles = clampAngle(SmoothedAngles);
	return SmoothedAngles;
}
DWORD WINAPI ShootLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (aimbotBool)
		{
			if ((GetKeyState(VK_MBUTTON) & 0x80) != 0)
			{
				sendDownPress();
				while ((GetKeyState(VK_MBUTTON) & 0x80) != 0)
				{
					Sleep(1);
				}
				sendUpPress();
			}
			else
			{
				Sleep(1);
			}
		}
		else
		{
			Sleep(1000);
		}
	}
	ExitThread(0);
}
//while I dont know the correct offsets
/*DWORD WINAPI AimbotLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (aimbotBool)
		{

			if ((GetKeyState(VK_MBUTTON) & 0x80) != 0)
			{
				int playerNum = GetClosestPlayerToCrossHair();
				while (playerNum == -1 && (GetKeyState(VK_MBUTTON) & 0x80) != 0)
				{

					playerNum = GetClosestPlayerToCrossHair();
					Sleep(1);
				}

				if (playerNum != -1)
				{

					if (!AllList.playerArr[playerNum].isDead)
					{

						while (!AllList.playerArr[playerNum].isDead && (GetKeyState(VK_MBUTTON) & 0x80) != 0 && AllList.playerArr[playerNum].isValid)
						{
							vec3_t aimPos;
							if (aimbotHeadBool)
								aimPos = convertToVec(AllList.playerArr[playerNum].headPos);
							else
								aimPos = convertToVec(AllList.playerArr[playerNum].chestPos);

							vec3_t mePos = convertToVec(Me.position);
							vec3_t compEnemyPos = velocityComp(aimPos, AllList.playerArr[playerNum].vecVelocity, Me.vecVelocity);
							vec3_t aimAngles;
							aimAngles = clampAngle(calcAngle(mePos, compEnemyPos, Me.punchAngs, Me.vecView));

							if ((getVecDistance(Me.viewAng, aimAngles)) < maxFovChange)
							{
								aimAngles = clampAngle(smoothAngle(Me.viewAng, aimAngles, smoothamount));
								if (checkAngle(aimAngles))
									Me.setViewAngles(aimAngles);
							}
							else
							{
								playerNum = GetClosestPlayerToCrossHair();
							}
						}

						while ((GetKeyState(VK_MBUTTON) & 0x80) != 0)
						{
							Sleep(1);
						}
					}
				}

			}

		}
		else
		{
			Sleep(1000);
		}
	}
	ExitThread(0);
}
*/

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
		{
			Sleep(1000);
		}
	}
	ExitThread(0);
	return 0;
}



//void espLooper(void *ptr)
//{
//
//	
//
//	DeleteObject(buffer);
//	return;
//}
void drawChoiceRectangles()
{
	int spaceDist = int(choiceSideLength / 2);
	int startX = rect.left + spaceDist;
	int startY = rect.top + spaceDist;

	if (aimbotBool)
	{
		stringstream ss;
		ss << maxFovChange;

		//drawtext(hdc, startX, startY + spaceDist, ss.str().c_str());
		cdraw.Text(ss.str().c_str(), startX, startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));

		if (aimbotHeadBool)
		{
			stringstream ss;
			ss << "Head";
			//drawtext(hdc, startX, startY + (2 * spaceDist), ss.str().c_str());
			cdraw.Text(ss.str().c_str(), startX + (2 * spaceDist), startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));
		}
		else
		{
			stringstream ss;
			ss << "Chest";
			cdraw.Text(ss.str().c_str(), startX + (2 * spaceDist), startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));
		}
	}


	if (enEspBool)
	{
		//SelectObject(hdc, greenPen);
		//SelectObject(hdc, greenBr);
		//Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		//SelectObject(hdc, redPen);
		//SelectObject(hdc, redBr);
		//Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (fEspBool)
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (triggerBool)
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (headDotBool)
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (aimbotBool)
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw.BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	return;

}
void initD3Dx(HWND hwnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	HRESULT hResult;
	if (d3d!=NULL)
	{
		D3DPRESENT_PARAMETERS params;
		ZeroMemory(&params, sizeof(D3DPRESENT_PARAMETERS));

		params.Windowed = TRUE;
		params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		params.BackBufferFormat = D3DFMT_A8R8G8B8;
		params.EnableAutoDepthStencil = TRUE;
		params.AutoDepthStencilFormat = D3DFMT_D16;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;
		params.PresentationInterval = 0x80000000L;

		DWORD dwMSQAAQuality = 0;
		if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, true, D3DMULTISAMPLE_NONMASKABLE, &dwMSQAAQuality)))
		{
			params.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
			params.MultiSampleQuality = dwMSQAAQuality - 1;
		}

		hResult = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3ddev);

	}

	if (hResult != D3D_OK)
	{
		MessageBox(NULL, L"Could not create d3d device", L"Problem", MB_OK);
		const wchar_t *resultStr = DXGetErrorString(hResult);
		MessageBox(NULL, resultStr, L"Problem", MB_OK);
	}
}
void renderEsp()
{
	GetClientRect(csWindow, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom + rect.left;
	if (csWindow != GetForegroundWindow())
		return;


	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene

	static bool initFonts = false;
	cdraw.GetDevice(d3ddev);
	cdraw.Reset();

	if (!initFonts)
	{
		cdraw.AddFont(L"Tahoma", 15, false, false);
		initFonts = true;
	}

	if (cdraw.Font()) cdraw.OnLostDevice();

	if (!stealthModeBool)
	{
		if (fEspBool || enEspBool)
		{
			for (int playerNum = 0; playerNum < playerCount; playerNum++)
			{
				if (!AllList.playerArr[playerNum].isDead && AllList.playerArr[playerNum].isValid)
				{
					float *en = (float*)(malloc(sizeof(float) * 2));
					bool teamate = AllList.playerArr[playerNum].team == Me.team;
					if (worldtoscreen(AllList.playerArr[playerNum].position, en, rect, false) && AllList.playerArr[playerNum].health > 0 && (getWorldDistance(Me.position, AllList.playerArr[playerNum].position))>float(playerHeight / 15))
					{
						float *enEye = (float*)(malloc(sizeof(float) * 2));
						worldtoscreen(AllList.playerArr[playerNum].position, enEye, rect, true);
						Draw(en[0], en[1], enEye[0], enEye[1], teamate, AllList.playerArr[playerNum].health);
						float *enHeadBone = (float*)(malloc(sizeof(float) * 2));
						worldtoscreen(AllList.playerArr[playerNum].headPos, enHeadBone, rect, false);
						if (headDotBool && AllList.playerArr[playerNum].boneBase != 0)
						{
							//Ellipse(Memhdc, enHeadBone[0] - dotRad, enHeadBone[1] + dotRad, enHeadBone[0] + dotRad, enHeadBone[1] - dotRad);
							cdraw.CircleFilled(enHeadBone[0], enHeadBone[1], dotRad, 360, full, 32, RED(255));
						}
						free(enEye);
						free(enHeadBone);
					}
					free(en);

				}
			}
		}
		drawChoiceRectangles();
	}

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_PAINT:
	{
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
DWORD WINAPI WorkLoop(LPVOID PARAMS)
{

	while (!END)
	{
		Me.getInfo();
		for (int x = 0; x < playerCount; x++)
		{
			AllList.updateInfo(x);
		}
		renderEsp();
		Sleep(int(1000 / desiredFPS));
	}
	ExitThread(0);
}
DWORD WINAPI HotkeyLoop(LPVOID PARAMS)
{
	while (!(GetAsyncKeyState(VK_F6)<0 && GetAsyncKeyState(VK_CONTROL)<0))
	{
		//increase decrease aimbot distance
		if (GetAsyncKeyState(VK_PRIOR) < 0)
		{
			if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				maxFovChange += 0.1f;
			}
			else
			{
				maxFovChange += 1.f;
			}
			while (GetAsyncKeyState(VK_PRIOR) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_NEXT) < 0)
		{
			if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				maxFovChange -= 0.1f;
			}
			else
			{
				maxFovChange -= 1.f;
			}
			while (GetAsyncKeyState(VK_NEXT) < 0)
				Sleep(50);
		}
		//increase decrease aimbot distance

		if (aimbotBool && triggerBool)
		{
			triggerBool = false;
		}
		if (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			enEspBool = !enEspBool;
			while (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			fEspBool = !fEspBool;
			while (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			triggerBool = !triggerBool;

			aimbotBool = !aimbotBool;
			while (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_F10) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			headDotBool = !headDotBool;
			while (GetAsyncKeyState(VK_F10) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			aimbotBool = !aimbotBool;

			triggerBool = !triggerBool;
			while (GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		//change aimbot target
		if (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			aimbotHeadBool = !aimbotHeadBool;
			while (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
				Sleep(50);
		}
		if (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_F10) < 0)
		{
			stealthModeBool = !stealthModeBool;
			while (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_F10) < 0)
				Sleep(50);
		}
		//change aimbot target
		Sleep(50);
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
bool checkIfPastExpir(int ex)
{
	time_t t;
	time(&t);
	return t >= ex;


}
DWORD WINAPI Main(LPVOID params) {

	//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	wchar_t* gameWinName = L"INSURGENCY";
	csWindow = NULL;
	while (!csWindow)
	{
		csWindow = FindWindow(0, gameWinName);
		Sleep(200);
	}
	GetClientRect(csWindow, &rect);
	process = NULL;
	while (!process)
	{
		process = GetProcess(L"insurgency.exe", &gameProcess);
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



	GetClientRect(csWindow, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom + rect.left;

	Hinstance = GetModuleHandle(NULL);
	WNDCLASSEX WClass;

	ZeroMemory(&WClass, sizeof(WNDCLASSEX));

	
	/*WClass.cbSize = sizeof(WNDCLASSEX);
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
	WClass.hIconSm = NULL;*/
	WClass.cbSize = sizeof(WNDCLASSEX);
	WClass.style = CS_HREDRAW | CS_VREDRAW;
	WClass.lpfnWndProc = WndProc;
	WClass.hInstance = Hinstance;
	WClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WClass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	WClass.lpszClassName = WName;
	if (!RegisterClassEx(&WClass))
	{
		MessageBox(NULL, L"Couldnt register", L"Problem", MB_OK);
		return 0;
	}
	EspHWND = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		WName,
		L"",
		WS_POPUP,
		rect.left, rect.top,
		win_width, win_height,
		0,
		NULL,
		Hinstance,
		NULL);

	if (EspHWND == NULL)
	{
		MessageBox(NULL, L"Couldnt create window", L"Problem", MB_OK);
		return 0;
	}
	//SetWindowLong(EspHWND, GWL_EXSTYLE, (int)GetWindowLong(EspHWND, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetLayeredWindowAttributes(EspHWND, RGB(0, 0, 0), 255, ULW_COLORKEY | LWA_ALPHA);
	if (FAILED(DwmExtendFrameIntoClientArea(EspHWND, &margins)))
	{
		MessageBox(NULL, L"Couldnt extend margins", L"Problem", MB_OK);
		return 0;
	}
	ShowWindow(EspHWND, SW_SHOWDEFAULT);

	
	initD3Dx(EspHWND);
	

	CreateThread(0, 0x1000, &TriggerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &WorkLoop, 0, 0, 0);
	//CreateThread(0, 0x1000, &PlayerLoop, 0, 0, 0);
	CreateThread(0, 0x1000, &HotkeyLoop, 0, 0, 0);

	MSG Msg;
	//::SetWindowPos(FindWindow(NULL, WName), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		Sleep(100);
	}

	ExitThread(0);
	return 0;

}
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: 
	{
		Hinstance = hInstance;
		CreateThread(0, 0x1000, &Main, 0, 0, 0); 
	}
	}
	return TRUE;
}