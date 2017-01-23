#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <cstdlib>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <math.h>
//#include <vector>
#include <sstream>
#include <process.h>
//#include <atlimage.h>
#include <random>
#include <ctime>
#include <fstream>
#include "draw.h"
//#include "dxerr.h" this is outdated apparently
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "dwmapi.lib" )

/* MyApp.cpp
/*  Compile options needed: /Zi /D_DEBUG /MLd
/*            or use a
/*      Default Workspace for a Console Application to
/*      build a Debug version
*/




using namespace std;

const int choiceSideLength = 30;
const int playerCount = 32;
int desiredFPS = 144;
const int playerHeight = 72;
int dotRad = 2;
//const float maxAimbotDistance = 15;
float maxFovChange;
float origmaxFovChange = maxFovChange = 1.f;
float snapFovChange = 2.f;
//int shotwait = 1;
float smoothamount = 10.f;//percent of total movement in each step
float mouseEventSmoothAmount = 10.f;
float aimAnglesSmoothAmount = smoothamount;
const int aimbotSmoothDelay = 1;//in milliseconds

float PitchMinPunch = 2.f;
float PitchMaxPunch = 2.f;
float YawMinPunch = 2.f;
float YawMaxPunch = 2.f;



//these tend to change*********************************** Last realistic date that was ban flagged 07-23-15
const DWORD playerBase = 0x00AA6834;
const DWORD entityBase = 0x04AC90F4;
const DWORD viewMat1Offset = 0x04ABAC94;
const DWORD enginePositionOffset = 0x005C75A4;//also called dwClientState or EnginePointer; 0x006BD9BC
//const DWORD viewMat2Offset = 0x04A2AFE4;//not updated
//const DWORD viewMat3Offset = 0x04A2B2F4;//not updated 
//******************************************************* 
const DWORD dormantOffset = 0x000000e9;
const DWORD boneMatOffset = 0x00002698;//0x000042A8
const DWORD xyzOffset = 0x00000134;//m_vecOrigin
const DWORD teamOffset = 0x000000f0;
const DWORD healthOffset = 0x000000fc;
const DWORD isDeadOffest = 0x0000025b;
const DWORD loopDistance = 0x10;

enum Team { teamCT = 3, teamT = 2};


const int boneCount = 83;

enum BoneID
{
	AUTOBONE = -1,
	PELVIS = 0,
	LEAN_ROOT = 1,
	CAM_DRIVER = 2,
	SPINE_0 = 3,
	SPINE_1 = 4,
	SPINE_2 = 5,
	SPINE_3 = 6,
	NECK_0 = 7,
	HEAD_0 = 8,
	CLAVICLE_L = 9,
	ARM_UPPER_L = 10,
	ARM_LOWER_L = 11,
	HAND_L = 12,
	FINGER_MIDDLE_META_L = 13,
	FINGER_MIDDLE_0_L = 14,
	FINGER_MIDDLE_1_L = 15,
	FINGER_MIDDLE_2_L = 16,
	FINGER_PINKY_META_L = 17,
	FINGER_PINKY_0_L = 18,
	FINGER_PINKY_1_L = 19,
	FINGER_PINKY_2_L = 20,
	FINGER_INDEX_META_L = 21,
	FINGER_INDEX_0_L = 22,
	FINGER_INDEX_1_L = 23,
	FINGER_INDEX_2_L = 24,
	FINGER_THUMB_0_L = 25,
	FINGER_THUMB_1_L = 26,
	FINGER_THUMB_2_L = 27,
	FINGER_RING_META_L = 28,
	FINGER_RING_0_L = 29,
	FINGER_RING_1_L = 30,
	FINGER_RING_2_L = 31,
	WEAPON_HAND_L = 32,
	ARM_LOWER_L_TWIST = 33,
	ARM_LOWER_L_TWIST1 = 34,
	ARM_UPPER_L_TWIST = 35,
	ARM_UPPER_L_TWIST1 = 36,
	CLAVICLE_R = 37,
	ARM_UPPER_R = 38,
	ARM_LOWER_R = 39,
	HAND_R = 40,
	FINGER_MIDDLE_META_R = 41,
	FINGER_MIDDLE_0_R = 42,
	FINGER_MIDDLE_1_R = 43,
	FINGER_MIDDLE_2_R = 44,
	FINGER_PINKY_META_R = 45,
	FINGER_PINKY_0_R = 46,
	FINGER_PINKY_1_R = 47,
	FINGER_PINKY_2_R = 48,
	FINGER_INDEX_META_R = 49,
	FINGER_INDEX_0_R = 50,
	FINGER_INDEX_1_R = 51,
	FINGER_INDEX_2_R = 52,
	FINGER_THUMB_0_R = 53,
	FINGER_THUMB_1_R = 54,
	FINGER_THUMB_2_R = 55,
	FINGER_RING_META_R = 56,
	FINGER_RING_0_R = 57,
	FINGER_RING_1_R = 58,
	FINGER_RING_2_R = 59,
	WEAPON_HAND_R = 60,
	ARM_LOWER_R_TWIST = 61,
	ARM_LOWER_R_TWIST1 = 62,
	ARM_UPPER_R_TWIST = 63,
	ARM_UPPER_R_TWIST1 = 64,
	LEG_UPPER_L = 65,
	LEG_LOWER_L = 66,
	ANKLE_L = 67,
	BALL_L = 68,
	LFOOT_LOCK = 69,
	LEG_UPPER_L_TWIST = 70,
	LEG_UPPER_L_TWIST1 = 71,
	LEG_UPPER_R = 72,
	LEG_LOWER_R = 73,
	ANKLE_R = 74,
	BALL_R = 75,
	RFOOT_LOCK = 76,
	LEG_UPPER_R_TWIST = 77,
	LEG_UPPER_R_TWIST1 = 78,
	FINGER_PINKY_L_END = 79,
	FINGER_PINKY_R_END = 80,
	VALVEBIPED_WEAPON_BONE = 81,
	LH_IK_DRIVER = 82,
	PRIMARY_JIGGLE_JNT = 83,
};

const int Llegbones[4] = { 72, 71, 70, 0 };
const int Rlegbones[4] = { 79, 78, 77, 0 };

const int Larmbones[4] = { 13,  12, 11, 6 };
const int Rarmbones[4] = { 43, 42, 41, 6 };

const int Spinebones[7] = { 0, 3, 4, 5, 6, 7, 8 };

const DWORD crosshairOffset = 0x0000AA70;
const DWORD velocityOffset = 0x00000110;
const DWORD shotsFiredOffset = 0x0000A2C0;
const DWORD punchAngOffset = 0x0000301C;//0x00004C28
const DWORD vecViewOffset = 0x00000104;
const DWORD viewAngOffset = 0x00004D0C;

PROCESSENTRY32 gameProcess;

DWORD pid;
DWORD client;
DWORD engine;
HANDLE process;
HMODULE dll;

HWND csWindow;
HWND EspHWND;
WNDCLASSEX WClass;
HANDLE Handle;
RECT rect;


LPCWSTR WName = L"Window";
HINSTANCE Hinstance;

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;

CDraw *cdraw = new CDraw();
static MARGINS margins = { -1, -1, -1, -1 };

int win_width;
int win_height;

string iniPath = "C:\\H\\settings.ini";

bool END = false;
bool PAUSETHREADS = false;
bool fEspBool = false;
bool enEspBool = true;
bool triggerBool = false;
bool headDotBool = true;
bool aimbotBool = false;
BoneID aimbotBone = HEAD_0;
bool stealthModeBool = false;
bool boneEspBool = false;
bool smoothingActive = true;
bool mouseEventAimbotBool = false;
bool showMenu = true;

std::string debugMsg = "no changes";
bool showDebugMsg = false;

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
	vec3_t position;
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
		ReadProcessMemory(process, (LPVOID)(local + shotsFiredOffset), &shotsFired, sizeof(int), 0);
		ReadProcessMemory(process, (LPVOID)(local + velocityOffset), &(vecVelocity), sizeof(vec3_t), 0);
		ReadProcessMemory(process, (LPVOID)(local + punchAngOffset), &(punchAngs), sizeof(vec3_t), 0);
		ReadProcessMemory(process, (LPVOID)(local + vecViewOffset), &(vecView), sizeof(vec3_t), 0);
		ReadProcessMemory(process, (LPVOID)(enginePointerAddr + viewAngOffset), &(viewAng), sizeof(vec3_t), 0);

	};
	void setViewAngles(vec3_t aimAng)
	{
		WriteProcessMemory(process, (LPVOID)(enginePointerAddr + viewAngOffset), &(aimAng), sizeof(vec3_t), 0);
	};

}Me;


struct Player_t
{
	DWORD base;
	DWORD boneBase;

	int team;
	int health;
	vec3_t position;
	bool isDead;
	bool isValid;
	vec3_t vecVelocity;
	vec3_t boneArr[boneCount];

	bool isDormant;


};

struct AllList_t
{
	Player_t playerArr[playerCount];
	void updateInfo(int playerNum)
	{

		bool testBool = true;
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(client + entityBase + (playerNum*loopDistance)), &(playerArr[playerNum].base), sizeof(DWORD), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + boneMatOffset), &(playerArr[playerNum].boneBase), sizeof(DWORD), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + teamOffset), &(playerArr[playerNum].team), sizeof(int), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + healthOffset), &(playerArr[playerNum].health), sizeof(int), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + xyzOffset), &(playerArr[playerNum].position), sizeof(float[3]), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + isDeadOffest), &(playerArr[playerNum].isDead), sizeof(bool), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + velocityOffset), &(playerArr[playerNum].vecVelocity), sizeof(vec3_t), 0);
		testBool = testBool && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].base + dormantOffset), &(playerArr[playerNum].isDormant), sizeof(bool), 0);

		testBool = testBool && 0 != readBones(playerNum);

		playerArr[playerNum].isValid = !(!testBool || playerArr[playerNum].isDormant);
		
		return;
	};
	bool readBones(int playerNum)
	{
		for (int i = 0; i < 4; i++)
		{
			int x = Larmbones[i];
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}
		for (int i = 0; i < 4; i++)
		{
			int x = Rarmbones[i];
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}
		for (int i = 0; i < 4; i++)
		{
			int x = Llegbones[i];
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}
		for (int i = 0; i < 4; i++)
		{
			int x = Rlegbones[i];
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}
		for (int i = 0; i < 7; i++)
		{
			int x = Spinebones[i];
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}
		
		return true;
		/*for (int x = 0; x < boneCount; x++)
		{
			bool valid = true;
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x0C), &(playerArr[playerNum].boneArr[x].x), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x1C), &(playerArr[playerNum].boneArr[x].y), sizeof(float), 0);
			valid = valid && 0 != ReadProcessMemory(process, (LPVOID)(playerArr[playerNum].boneBase + 0x30 * x + 0x2C), &(playerArr[playerNum].boneArr[x].z), sizeof(float), 0);
			if (valid == false)
			{
				return false;
			}
		}*/
	};


}AllList;

struct boneStruct_t
{
	vec3_t Lleg[4];
	vec3_t Rleg[4];
	vec3_t Larm[4];
	vec3_t Rarm[4];
	vec3_t spine[7];
};


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
float getWorldDistance(vec3_t me, vec3_t en)
{
	return sqrt(pow((en.x - me.x), 2) +
		pow((en.y - me.y), 2) +
		pow((en.z - me.z), 2)
		);
}

bool worldtoscreen(vec3_t begVec, vec3_t *endVec, RECT rectangle)
{
	//Quaternion maths
	float beg[3] = { begVec.x, begVec.y, begVec.z };
	float test = 0.0f;

	endVec->y = Me.viewMat[1][0] * beg[0] + Me.viewMat[1][1] * beg[1] + Me.viewMat[1][2] * beg[2] + Me.viewMat[1][3];
	test = Me.viewMat[3][0] * beg[0] + Me.viewMat[3][1] * beg[1] + Me.viewMat[3][2] * beg[2] + Me.viewMat[3][3];
	endVec->x = Me.viewMat[0][0] * beg[0] + Me.viewMat[0][1] * beg[1] + Me.viewMat[0][2] * beg[2] + Me.viewMat[0][3];

	if (test < 0.01f)
		return false;


	endVec->x *= float(1 / test);
	endVec->y *= float(1 / test);

	int width = (int)(rectangle.right - rectangle.left);
	int height = (int)(rectangle.bottom + rectangle.left);

	float x = (float)(width / 2);
	float y = (float)(height / 2);

	x += (float)(0.5 * endVec->x * width + 0.5);
	y -= (float)(0.5 * endVec->y * height + 0.5);
	endVec->x = x + rectangle.left;
	endVec->y = y + rectangle.top;

	return true;


}
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
		cdraw->Box(x - (width / 2) - 1, y - height - 1, width + 2, height + 2, 3, BLACK(255));
		cdraw->Box(x - (width / 2), y - height, width, height, 1, D3DCOLOR_ARGB(255, 102, 0, 104));
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
		cdraw->Box(x - (width / 2) - 1, y - height - 1, width + 2, height + 2, 3, BLACK(255));
		cdraw->Box(x - (width / 2), y - height, width, height, 1, color);
		return;
	}
	return;
}

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
		vec3_t *enHead = (vec3_t*)(malloc(sizeof(vec3_t)));
		float Dist = FLT_MAX;
		if (worldtoscreen(AllList.playerArr[x].boneArr[HEAD_0], enHead, rect))
		{
			Dist = distanceFromCenter(enHead->x, enHead->y);
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
void trigger()
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
	if (!smoothingActive)
		return DestAngles;
	vec3_t SmoothedAngles;
	SmoothedAngles.x = DestAngles.x - SrcAngles.x;
	SmoothedAngles.y = DestAngles.y - SrcAngles.y;
	SmoothedAngles.z = 0.0f;
	SmoothedAngles = clampAngle(SmoothedAngles);
	SmoothedAngles.x = SrcAngles.x + (SmoothedAngles.x / 100.0f) * smoothamount;
	SmoothedAngles.y = SrcAngles.y + (SmoothedAngles.y / 100.0f) * smoothamount;
	SmoothedAngles.z = 0.0f;
	SmoothedAngles = clampAngle(SmoothedAngles);
	return SmoothedAngles;
}
pair<vec3_t,bool> getAimPos(int playerNum)
{
	if (aimbotBone == -1)
	{
		pair<vec3_t, bool> toReturn;
		float lowest = FLT_MAX;
		BoneID aimBoneArr[] = { HEAD_0, SPINE_3, SPINE_0};//order matters!
		for (BoneID bone : aimBoneArr)
		{
			float dist = FLT_MAX;
			vec3_t aimPos = AllList.playerArr[playerNum].boneArr[bone];
			vec3_t compEnemyPos = velocityComp(aimPos, AllList.playerArr[playerNum].vecVelocity, Me.vecVelocity);
			vec3_t aimAngles = clampAngle(calcAngle(Me.position, compEnemyPos, Me.punchAngs, Me.vecView));
			dist = getVecDistance(Me.viewAng, aimAngles);
			if (dist < lowest)
			{
				lowest = dist;
				toReturn.first = aimAngles;
			}
		}
		toReturn.second = lowest < maxFovChange;
		return toReturn;
	}
	else
	{
		float dist = FLT_MAX;
		vec3_t aimPos = AllList.playerArr[playerNum].boneArr[aimbotBone];
		vec3_t compEnemyPos = velocityComp(aimPos, AllList.playerArr[playerNum].vecVelocity, Me.vecVelocity);
		vec3_t aimAngles = clampAngle(calcAngle(Me.position, compEnemyPos, Me.punchAngs, Me.vecView));
		dist = getVecDistance(Me.viewAng, aimAngles);
		pair<vec3_t, bool> toReturn;
		toReturn.first = aimAngles;
		toReturn.second = dist < maxFovChange;
		return toReturn;
	}
}
pair<vec3_t, bool> getAimPosBoneOnly(int playerNum)
{
	if (aimbotBone == -1)
	{
		pair<vec3_t, bool> toReturn;
		float lowest = FLT_MAX;
		BoneID aimBoneArr[] = { HEAD_0, SPINE_3, SPINE_0 };//order matters!
		for (BoneID bone : aimBoneArr)
		{
			float dist = FLT_MAX;
			vec3_t aimPos = AllList.playerArr[playerNum].boneArr[bone];
			vec3_t compEnemyPos = velocityComp(aimPos, AllList.playerArr[playerNum].vecVelocity, Me.vecVelocity);
			vec3_t aimAngles = clampAngle(calcAngle(Me.position, compEnemyPos, Me.punchAngs, Me.vecView));
			dist = getVecDistance(Me.viewAng, aimAngles);
			if (dist < lowest)
			{
				lowest = dist;
				toReturn.first = aimPos;
			}
		}
		toReturn.second = lowest < maxFovChange;
		return toReturn;
	}
	else
	{
		float dist = FLT_MAX;
		vec3_t aimPos = AllList.playerArr[playerNum].boneArr[aimbotBone];
		vec3_t compEnemyPos = velocityComp(aimPos, AllList.playerArr[playerNum].vecVelocity, Me.vecVelocity);
		vec3_t aimAngles = clampAngle(calcAngle(Me.position, compEnemyPos, Me.punchAngs, Me.vecView));
		dist = getVecDistance(Me.viewAng, aimAngles);
		pair<vec3_t, bool> toReturn;
		toReturn.first = aimPos;
		toReturn.second = dist < maxFovChange;
		return toReturn;
	}
}
void mouseEventAimAtPos(vec3_t in)
{
	int ScreenCenterY = win_height * 0.5f;
	int ScreenCenterX = win_width * 0.5f;

	float TargetX = 0;
	float TargetY = 0;

	//X Axis
	if (in.x != 0)
	{
		if (in.x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - in.x);
			if (smoothingActive)
				TargetX /= smoothamount;
			else
				TargetX /= 2;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (in.x < ScreenCenterX)
		{
			TargetX = in.x - ScreenCenterX;
			if (smoothingActive)
				TargetX /= smoothamount;
			else
				TargetX /= 2;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	//Y Axis

	if (in.y != 0)
	{
		if (in.y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - in.y);
			if (smoothingActive)
				TargetY /= smoothamount;
			else
				TargetY /= 2;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (in.y < ScreenCenterY)
		{
			TargetY = in.y - ScreenCenterY;
			if (smoothingActive)
				TargetY /= smoothamount;
			else
				TargetY /= 2;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}
	//Mouse even't will round to 0 a lot of the time, so we can add this, to make it more accurrate on slow speeds.
	if (fabs(TargetX) < 1)
	{
		if (TargetX > 0.1)
		{
			TargetX = 1;
		}
		if (TargetX < -0.1)
		{
			TargetX = -1;
		}
	}
	if (fabs(TargetY) < 1)
	{
		if (TargetY > 0.1)
		{
			TargetY = 1;
		}
		if (TargetY < -0.1)
		{
			TargetY = -1;
		}
	}
	mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);
}
DWORD WINAPI MouseEventAimbotLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (!PAUSETHREADS)
		{
			if (aimbotBool && csWindow == GetForegroundWindow())
			{

				if ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
				{
					int playerNum = GetClosestPlayerToCrossHair();
					while (playerNum == -1 && (GetKeyState(VK_LBUTTON) & 0x80) != 0)
					{
						playerNum = GetClosestPlayerToCrossHair();
						Sleep(1);
					}

					if (playerNum != -1)
					{
						if (!AllList.playerArr[playerNum].isDead)
						{
							while (!AllList.playerArr[playerNum].isDead && (GetKeyState(VK_LBUTTON) & 0x80) != 0 && AllList.playerArr[playerNum].isValid)
							{
								pair<vec3_t, bool> aimPair = getAimPosBoneOnly(playerNum);
								vec3_t *pos = (vec3_t*)(malloc(sizeof(vec3_t)));
								if (worldtoscreen(aimPair.first, pos, rect) && (getWorldDistance(Me.position, AllList.playerArr[playerNum].position)) > float(playerHeight / 15))
								{
									if (aimPair.second)
									{
										mouseEventAimAtPos(*pos);
										Sleep(aimbotSmoothDelay);
									}
									else
										playerNum = GetClosestPlayerToCrossHair();
								}
								else
									playerNum = GetClosestPlayerToCrossHair();
								free(pos);
							}
							while ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
								Sleep(1);
						}
					}

				}

			}
			else
				Sleep(1000);
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}
DWORD WINAPI AimbotLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (!PAUSETHREADS)
		{
			if (aimbotBool && csWindow == GetForegroundWindow())
			{

				if ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
				{
					int playerNum = GetClosestPlayerToCrossHair();
					while (playerNum == -1 && (GetKeyState(VK_LBUTTON) & 0x80) != 0)
					{
						playerNum = GetClosestPlayerToCrossHair();
						Sleep(1);
					}

					if (playerNum != -1)
					{
						if (!AllList.playerArr[playerNum].isDead)
						{
							while (!AllList.playerArr[playerNum].isDead && (GetKeyState(VK_LBUTTON) & 0x80) != 0 && AllList.playerArr[playerNum].isValid)
							{
								if (!mouseEventAimbotBool)
								{
									pair<vec3_t, bool> aimPair = getAimPos(playerNum);
									vec3_t aimAngles = aimPair.first;
									if (aimPair.second)
									{
										aimAngles = clampAngle(smoothAngle(Me.viewAng, aimAngles, smoothamount));
										if (checkAngle(aimAngles))
											Me.setViewAngles(aimAngles);
										if (smoothingActive)
											Sleep(aimbotSmoothDelay);
									}
									else
										playerNum = GetClosestPlayerToCrossHair();
								}
								else
								{
									pair<vec3_t, bool> aimPair = getAimPosBoneOnly(playerNum);
									vec3_t *pos = (vec3_t*)(malloc(sizeof(vec3_t)));
									if (worldtoscreen(aimPair.first, pos, rect) && (getWorldDistance(Me.position, AllList.playerArr[playerNum].position)) > float(playerHeight / 15))
									{
										if (aimPair.second)
										{
											pos->x += (Me.punchAngs.y * win_width / 90);
											pos->y -= (Me.punchAngs.x * win_height / 90);
											mouseEventAimAtPos(*pos);
											Sleep(aimbotSmoothDelay);
										}
										else
											playerNum = GetClosestPlayerToCrossHair();
									}
									else
										playerNum = GetClosestPlayerToCrossHair();
									free(pos);
								}
							}
							while ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
								Sleep(1);
						}
					}

				}

			}
			else
				Sleep(1000);
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}

DWORD WINAPI TriggerLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (!PAUSETHREADS)
		{
			if (triggerBool && csWindow == GetForegroundWindow())
			{
				Me.getInfo();
				trigger();
			}
			else
			{
				Sleep(1000);
			}
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}



//void espLooper(void *ptr)
//{
//
//	
//
//	DeleteObject(buffer);
//	return;
//}
string boneName(BoneID in)
{
	switch (in)
	{
		case HEAD_0:
			return "H";
		case SPINE_3:
			return "C";
		case SPINE_0:
			return "B";
		case AUTOBONE:
			return "Auto";
		default:
			return "UNIDENTIFIED";
	}

}
BoneID getNextAimBone(BoneID in)
{
	switch (in)
	{
	case HEAD_0:
		return SPINE_3;
	case SPINE_3:
		return HEAD_0;
	case SPINE_0:
		return HEAD_0;
	default:
		return HEAD_0;
	}
}
void drawChoiceRectangles()
{
	int spaceDist = int(choiceSideLength / 2);
	int startX = rect.left + spaceDist;
	int startY = rect.top + spaceDist;
	//debugMsg
	if(showDebugMsg) cdraw->Text(debugMsg.c_str(), win_width / 2, win_height / 2, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));

	if (aimbotBool)
	{
		stringstream ss;

		//maxFovChange
		ss << maxFovChange;
		cdraw->Text(ss.str().c_str(), startX, startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));
		
		//smoothamount
		ss.str(string());
		ss << smoothamount;
		cdraw->Text(ss.str().c_str(), startX + (2 * spaceDist), startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));


		//drawtext(hdc, startX, startY + (2 * spaceDist), ss.str().c_str());
		cdraw->Text(boneName(aimbotBone).c_str(), startX + (4 * spaceDist), startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));

		ss.str(string());
		if (smoothingActive)
			ss << "Smth";
		else
			ss << "Sp";
		cdraw->Text(ss.str().c_str(), startX + (7 * spaceDist), startY + spaceDist, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));
	}


	if (enEspBool)
	{
		//SelectObject(hdc, greenPen);
		//SelectObject(hdc, greenBr);
		//Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		//SelectObject(hdc, redPen);
		//SelectObject(hdc, redBr);
		//Rectangle(hdc, startX, startY, startX + spaceDist, startY + spaceDist);
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (fEspBool)
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (boneEspBool)
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (headDotBool)
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (triggerBool)
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	startX += spaceDist + (spaceDist / 2);
	if (aimbotBool)
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, GREEN(255));
	}
	else
	{
		cdraw->BoxFilled(startX, startY, spaceDist, spaceDist, RED(255));
	}
	return;

}
void drawMenu()
{
	DWORD green = D3DCOLOR_ARGB(255, 0, 255, 0);
	DWORD red = D3DCOLOR_ARGB(255, 255, 0, 0);
	DWORD yellow = D3DCOLOR_ARGB(255, 255, 255, 0);
	stringstream ss;
	//debugMsg
	if (showDebugMsg) cdraw->Text(debugMsg.c_str(), win_width / 2, win_height / 2, lefted, 0, true, yellow, WHITE(0));

	int startX = rect.left + choiceSideLength;
	int startY = rect.top + choiceSideLength;
	if (aimbotBool && !showMenu)
	{
		startY += choiceSideLength;
		ss.str("");
		//maxFovChange
		ss << maxFovChange;
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
		//smoothamount
		ss.str("");
		ss << smoothamount;
		cdraw->Text(ss.str().c_str(), startX + choiceSideLength, startY, lefted, 0, true, green, WHITE(0));


		//drawtext(hdc, startX, startY + (2 * spaceDist), ss.str().c_str());
		cdraw->Text(boneName(aimbotBone).c_str(), startX + (2 * choiceSideLength), startY, lefted, 0, true, green, WHITE(0));

		ss.str("");
		if (smoothingActive)
			ss << "Smth";
		else
			ss << "Sp";
		cdraw->Text(ss.str().c_str(), startX + (4 * choiceSideLength), startY, lefted, 0, true, green, WHITE(0));
		return;
	}
	if (!showMenu)
		return;
	cdraw->BoxBordered(rect.left + 2, rect.top + choiceSideLength, choiceSideLength * 8, choiceSideLength * 15, 2, D3DCOLOR_ARGB(255, 0, 0, 0), D3DCOLOR_ARGB(255, 128, 0, 255));
	startY += choiceSideLength;
	ss.str("");
	ss << "Menu: Insert";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));

	startY += choiceSideLength;
	ss.str("");
	ss << "Enemy Esp: ";
	if (enEspBool)
	{
		ss << "ON    F1";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F1";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Friendly Esp: ";
	if (fEspBool)
	{
		ss << "ON    F2";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F2";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Bone Esp: ";
	if (boneEspBool)
	{
		ss << "ON    F3";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F3";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Head Dot: ";
	if (headDotBool)
	{
		ss << "ON    F4";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F4";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Trigger Bot: ";
	if (triggerBool)
	{
		ss << "ON    F5";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F5";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Aim Bot: ";
	if (aimbotBool)
	{
		ss << "ON    F6";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF   F6";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Aim Bone: " << boneName(aimbotBone) << "         F7, q";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));

	startY += choiceSideLength;
	ss.str("");
	ss << "Auto Bone: ";
	if (aimbotBone == AUTOBONE)
	{
		ss << "ON  F8";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, green, WHITE(0));
	}
	else
	{
		ss << "OFF F8";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, red, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Aimbot Style: ";
	if (mouseEventAimbotBool)
	{
		ss << "mouse_event  F9";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));
	}
	else
	{
		ss << "write_angles F9";
		cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));
	}
	startY += choiceSideLength;
	ss.str("");
	ss << "Aim Style: ";
	if (smoothingActive)
		ss << "Smth  Ctrl + q";
	else
		ss << "Sp    Ctrl + q";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));

	ss.precision(3);

	startY += choiceSideLength;
	ss.str("");
	ss << "Fov: " << maxFovChange << "    Page Up, Page Down";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));

	startY += choiceSideLength;
	ss.str("");
	ss << "Smooth Amount: " << smoothamount << "    Plus, Minus";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));

	startY += choiceSideLength;
	ss.str("");
	ss << "QUIT with F12";
	cdraw->Text(ss.str().c_str(), startX, startY, lefted, 0, true, yellow, WHITE(0));
	return;

}
void drawBoneNumbers(int pNum)
{
		Player_t p = AllList.playerArr[pNum];
		for (int y = 0; y < boneCount; y++)
		{
			vec3_t *screenPos = (vec3_t*)(malloc(sizeof(vec3_t)));
			worldtoscreen(p.boneArr[y], screenPos, rect);
			if (p.boneBase != 0)
			{
				char *number = (char*)(malloc(sizeof(char)));
				_itoa_s(y, number, sizeof(char),10);
				const char* inputStr = number;
				cdraw->Text(inputStr, screenPos->x, screenPos->y, centered, 0, true, D3DCOLOR_ARGB(255, 0, 255, 0), WHITE(0));
				free(number);
			}
			free(screenPos);
		}
}
void drawBoneEsp(int pNum)
{
	int width = 3;
	Player_t p = AllList.playerArr[pNum];
	if (p.boneBase == 0)
		return;
	boneStruct_t boneStruct;
	if (p.team == teamCT)
	{
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Lleg[i] = p.boneArr[Llegbones[i]];
			if (!boneStruct.Lleg[i].x || !boneStruct.Lleg[i].y || !boneStruct.Lleg[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Rleg[i] = p.boneArr[Rlegbones[i]];
			if (!boneStruct.Rleg[i].x || !boneStruct.Rleg[i].y || !boneStruct.Rleg[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Larm[i] = p.boneArr[Larmbones[i]];
			if (!boneStruct.Larm[i].x || !boneStruct.Larm[i].y || !boneStruct.Larm[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Rarm[i] = p.boneArr[Rarmbones[i]];
			if (!boneStruct.Rarm[i].x || !boneStruct.Rarm[i].y || !boneStruct.Rarm[i].z)
				return;
		}
		for (int i = 0; i < 7; i++)
		{
			boneStruct.spine[i] = p.boneArr[Spinebones[i]];
			if (!boneStruct.spine[i].x || !boneStruct.spine[i].y || !boneStruct.spine[i].z)
				return;
		}
	}
	else if (p.team == teamT)
	{
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Lleg[i] = p.boneArr[Llegbones[i]];
			if (!boneStruct.Lleg[i].x || !boneStruct.Lleg[i].y || !boneStruct.Lleg[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Rleg[i] = p.boneArr[Rlegbones[i]];
			if (!boneStruct.Rleg[i].x || !boneStruct.Rleg[i].y || !boneStruct.Rleg[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Larm[i] = p.boneArr[Larmbones[i]];
			if (!boneStruct.Larm[i].x || !boneStruct.Larm[i].y || !boneStruct.Larm[i].z)
				return;
		}
		for (int i = 0; i < 4; i++)
		{
			boneStruct.Rarm[i] = p.boneArr[Rarmbones[i]];
			if (!boneStruct.Rarm[i].x || !boneStruct.Rarm[i].y || !boneStruct.Rarm[i].z)
				return;
		}
		for (int i = 0; i < 7; i++)
		{
			boneStruct.spine[i] = p.boneArr[Spinebones[i]];
			if (!boneStruct.spine[i].x || !boneStruct.spine[i].y || !boneStruct.spine[i].z)
				return;
		}
	}
	else
		return;

	vec3_t prevBone;
	vec3_t currentBone;

	if (!worldtoscreen(boneStruct.Lleg[0], &prevBone, rect))
		return;
	for (int i = 1; i < 4; i++)
	{
		if (worldtoscreen(boneStruct.Lleg[i], &currentBone, rect))
		{
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, width, BLACK(255));
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, 1, WHITE(255));
			prevBone.x = currentBone.x;
			prevBone.y = currentBone.y;
		}
		else
			return;
	}

	if (!worldtoscreen(boneStruct.Rleg[0], &prevBone, rect))
		return;
	for (int i = 1; i < 4; i++)
	{
		if (worldtoscreen(boneStruct.Rleg[i], &currentBone, rect))
		{
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, width, BLACK(255));
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, 1, WHITE(255));
			prevBone.x = currentBone.x;
			prevBone.y = currentBone.y;
		}
		else
			return;
	}

	if (!worldtoscreen(boneStruct.Larm[0], &prevBone, rect))
		return;
	for (int i = 1; i < 4; i++)
	{
		if (worldtoscreen(boneStruct.Larm[i], &currentBone, rect))
		{
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, width, BLACK(255));
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, 1, WHITE(255));
			prevBone.x = currentBone.x;
			prevBone.y = currentBone.y;
		}
		else
			return;
	}

	if (!worldtoscreen(boneStruct.Rarm[0], &prevBone, rect))
		return;
	for (int i = 1; i < 4; i++)
	{
		if (worldtoscreen(boneStruct.Rarm[i], &currentBone, rect))
		{
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, width, BLACK(255));
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, 1, WHITE(255));
			prevBone.x = currentBone.x;
			prevBone.y = currentBone.y;
		}
		else
			return;
	}

	if (!worldtoscreen(boneStruct.spine[0], &prevBone, rect))
		return;
	for (int i = 1; i < 7; i++)
	{
		if (worldtoscreen(boneStruct.spine[i], &currentBone, rect))
		{
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, width, BLACK(255));
			cdraw->Line(prevBone.x, prevBone.y, currentBone.x, currentBone.y, 1, WHITE(255));
			prevBone.x = currentBone.x;
			prevBone.y = currentBone.y;
		}
		else
			return;
	}
	
}
void initD3Dx(HWND hwnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	HRESULT hResult;
	if (d3d != NULL)
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
		const wchar_t *resultStr = L"issue creating device"; //DXGetErrorString(hResult);
		MessageBox(NULL, resultStr, L"Problem", MB_OK);
	}

	static bool initFonts = false;
	cdraw->GetDevice(d3ddev);
	cdraw->Reset();

	if (!initFonts)
	{
		cdraw->AddFont(L"Tahoma", 15, false, false);
		initFonts = true;
	}
}
void clearD3Dx()
{
	if(cdraw) delete cdraw;
	if (d3d) d3d->Release();
	if (d3ddev) d3ddev->Release();
}
int initEspHwnd()
{
	GetClientRect(csWindow, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom + rect.left;

	Hinstance = GetModuleHandle(NULL);
	
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
}
void clearEspHwnd()
{
	//just fucking die holy fuck
	SendMessage(EspHWND, WM_CLOSE, 0, 0);
	UnregisterClass(WName, Hinstance);
}
void clearGlobals()
{
	//shitty code practices got me here, fuck you 5 years ago Matthew
	ZeroMemory(&gameProcess, sizeof(PROCESSENTRY32));

	pid = NULL;
	client = NULL;
	engine = NULL;
	process = NULL;
	dll = NULL;

	csWindow = NULL;
	EspHWND = NULL;
	Handle = NULL;
	ZeroMemory(&rect, sizeof(RECT));
	
	Hinstance = NULL;

	win_width = 0;
	win_height = 0;
}
void clearEsp()
{
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene
	d3ddev->EndScene();    // ends the 3D scene
	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}
void renderEsp()
{
	GetClientRect(csWindow, &rect);
	win_width = rect.right - rect.left;
	win_height = rect.bottom + rect.left;
	
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene

	if (cdraw->Font()) cdraw->OnLostDevice();

	if (!stealthModeBool)
	{
		if (fEspBool || enEspBool || boneEspBool || headDotBool)
		{
			for (int playerNum = 0; playerNum < playerCount; playerNum++)
			{
				if (!AllList.playerArr[playerNum].isDead && AllList.playerArr[playerNum].isValid)
				{
					vec3_t *en = (vec3_t*)(malloc(sizeof(vec3_t)));
					bool teamate = AllList.playerArr[playerNum].team == Me.team;
					if (worldtoscreen(AllList.playerArr[playerNum].position, en, rect) && AllList.playerArr[playerNum].health > 0 && (getWorldDistance(Me.position, AllList.playerArr[playerNum].position))>float(playerHeight / 15))
					{
						vec3_t *enTopBox = (vec3_t*)(malloc(sizeof(vec3_t)));
						vec3_t enTopBoxVec = AllList.playerArr[playerNum].position;
						enTopBoxVec.z = enTopBoxVec.z + playerHeight;
						worldtoscreen(enTopBoxVec, enTopBox, rect);
						Draw(en->x, en->y, enTopBox->x, enTopBox->y, teamate, AllList.playerArr[playerNum].health);
						vec3_t *enHeadBone = (vec3_t*)(malloc(sizeof(vec3_t)));
						worldtoscreen(AllList.playerArr[playerNum].boneArr[HEAD_0], enHeadBone, rect);
						if (boneEspBool && AllList.playerArr[playerNum].boneBase != 0)
						{
							if ((teamate && fEspBool) || (!teamate && enEspBool))
								drawBoneEsp(playerNum);
						}
						if (headDotBool && AllList.playerArr[playerNum].boneBase != 0)
						{
							
							if ((teamate && fEspBool) || (!teamate && enEspBool))
								cdraw->CircleFilled(enHeadBone->x, enHeadBone->y, dotRad, 360, full, 32, RED(255));
								//cdraw->Text(ss.str().c_str(), enHeadBone->x, enHeadBone->y, centered, 0, true, YELLOW(255), BLACK(255));
						}	
						//drawBoneNumbers(playerNum);
						free(enTopBox);
						free(enHeadBone);
					}
					free(en);
				}
			}
		}
		//drawChoiceRectangles();
		drawMenu();
	}

	d3ddev->EndScene();    // ends the 3D scene
	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}
void updatePlayers()
{
	Me.getInfo();
	for (int x = 0; x < playerCount; x++)
	{
		AllList.updateInfo(x);
	}
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
		DestroyWindow(hwnd);
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
		if (!PAUSETHREADS)
		{
			if (csWindow == GetForegroundWindow())
				renderEsp();
			else
			{
				clearEsp();
				Sleep(1000);
			}
			Sleep(int(1000 / desiredFPS));
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}
DWORD WINAPI HotkeyLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (!PAUSETHREADS)
		{
			//show menu or not
			if (GetAsyncKeyState(VK_INSERT) < 0)
			{
				showMenu = !showMenu;
				while (GetAsyncKeyState(VK_INSERT) < 0)
					Sleep(50);
			}
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
			//increase decrease smooth percentage
			if (GetAsyncKeyState(VK_ADD) < 0)
			{
				if (GetAsyncKeyState(VK_CONTROL) < 0)
				{
					smoothamount += 0.1f;
				}
				else
				{
					smoothamount += 1.f;
				}
				while (GetAsyncKeyState(VK_ADD) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_SUBTRACT) < 0)
			{
				if (GetAsyncKeyState(VK_CONTROL) < 0)
				{
					smoothamount -= 0.1f;
				}
				else
				{
					smoothamount -= 1.f;
				}
				while (GetAsyncKeyState(VK_SUBTRACT) < 0)
					Sleep(50);
			}
			//increase decrease smooth percentage
			if (aimbotBool && triggerBool)
			{
				triggerBool = false;
			}
			if (GetAsyncKeyState(VK_F1) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				enEspBool = !enEspBool;
				while (GetAsyncKeyState(VK_F1) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_F2) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				fEspBool = !fEspBool;
				while (GetAsyncKeyState(VK_F2) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_F3) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				boneEspBool = !boneEspBool;
				while (GetAsyncKeyState(VK_F3) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_F4) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				headDotBool = !headDotBool;
				while (GetAsyncKeyState(VK_F4) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_F5) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				triggerBool = !triggerBool;

				if (aimbotBool && triggerBool)
					aimbotBool = false;
				while (GetAsyncKeyState(VK_F5) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			if (GetAsyncKeyState(VK_F6) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				aimbotBool = !aimbotBool;

				if (triggerBool && aimbotBool)
					triggerBool = false;
				while (GetAsyncKeyState(VK_F6) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			//change aimbot target
			if (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				aimbotBone = getNextAimBone(aimbotBone);
				while (GetAsyncKeyState(VK_F7) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			//Q key
			if (GetAsyncKeyState(0x51) < 0 && !(GetAsyncKeyState(VK_CONTROL) < 0))
			{
				aimbotBone = getNextAimBone(aimbotBone);
				while (GetAsyncKeyState(0x51) < 0 && !(GetAsyncKeyState(VK_CONTROL) < 0))
					Sleep(50);
			}
			//change aimbot target
			//change to snap
			//Q key
			if (GetAsyncKeyState(0x51) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				smoothingActive = !smoothingActive;
				if (!smoothingActive)
					maxFovChange = snapFovChange;
				else
					maxFovChange = origmaxFovChange;

				while (GetAsyncKeyState(0x51) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			//change to snap
			//change aimbot target to auto
			if (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				aimbotBone = AUTOBONE;
				while (GetAsyncKeyState(VK_F8) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			//change aimbot target to auto
			if (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				mouseEventAimbotBool = !mouseEventAimbotBool;
				if (mouseEventAimbotBool)
					smoothamount = mouseEventSmoothAmount;
				else
					smoothamount = aimAnglesSmoothAmount;
				while (GetAsyncKeyState(VK_F9) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)
					Sleep(50);
			}
			//stealth mode
			if (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_F10) < 0)
			{
				stealthModeBool = !stealthModeBool;
				while (GetAsyncKeyState(VK_F12) < 0 && GetAsyncKeyState(VK_F11) < 0 && GetAsyncKeyState(VK_F10) < 0)
					Sleep(50);
			}
			//stealth mode
			Sleep(50);
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}
DWORD WINAPI PlayerLoop(LPVOID PARAMS)
{
	while (!END)
	{
		if (!PAUSETHREADS)
		{
			updatePlayers();
			Sleep(1);
		}
		else
			Sleep(1000);
	}
	ExitThread(0);
}
bool checkIfPastExpir(int ex)
{
	time_t t;
	time(&t);
	return t >= ex;


}
bool setIniSettings()
{
	string data[20];
	ifstream read;
	if (ifstream(iniPath))
	{
		read.open(iniPath);
		string linedata;
		int currentline = 1;
		while (!read.eof())
		{
			getline(read, linedata);
			data[currentline] = linedata;
			currentline++;
		}
		read.close();
	}
	else
	{
		return false;
	}
	/*
	bool fEspBool = true;
	bool enEspBool = true;
	bool triggerBool = false;
	bool headDotBool = true;
	bool aimbotBool = true;
	bool boneEspBool = false;
	*/
	data[1] = data[1].substr(data[1].find("=") + 1); //desiredFPS
	data[2] = data[2].substr(data[2].find("=") + 1); //dotRad
	data[3] = data[3].substr(data[3].find("=") + 1); //maxFovChange
	data[4] = data[4].substr(data[4].find("=") + 1); //smoothamount
	data[5] = data[5].substr(data[5].find("=") + 1); //fEspBool
	data[6] = data[6].substr(data[6].find("=") + 1); //enEspBool
	data[7] = data[7].substr(data[7].find("=") + 1); //headDotBool
	data[8] = data[8].substr(data[8].find("=") + 1); //triggerBool
	data[9] = data[9].substr(data[9].find("=") + 1); //aimbotBool
	data[10] = data[10].substr(data[10].find("=") + 1); //boneEspBool

	desiredFPS = stof(data[1].c_str());
	dotRad = stof(data[2].c_str());
	maxFovChange = stof(data[3].c_str());
	origmaxFovChange = stof(data[3].c_str());
	smoothamount = stof(data[4].c_str());
	fEspBool = stoi(data[5].c_str());
	enEspBool = stoi(data[6].c_str());
	headDotBool = stoi(data[7].c_str());
	triggerBool = stoi(data[8].c_str());
	aimbotBool = stoi(data[9].c_str());
	boneEspBool = stoi(data[10].c_str());
	return true;
}
DWORD WINAPI Main(LPVOID Params)  {

	setIniSettings();
	csWindow = NULL;
	while (!csWindow)
	{
		csWindow = FindWindow(0, L"Counter-Strike: Global Offensive");
		Sleep(1000);
	}
	process = NULL;
	while (!process)
	{
		process = GetProcess(L"csgo.exe", &gameProcess);
		Sleep(200);
	}
	//***************************************************************************************
	//in Unix Time
	/*
	if (checkIfPastExpir(1450051200))
	{
	MessageBox(NULL, L"Couldnt retrieve offsets, Probably outdated.", L"Problem", MB_OK);
	return 0;
	}
	*/
	//***************************************************************************************
	while (client == 0x0)
	{

		client = modName(L"client.dll", gameProcess.th32ProcessID);

	}
	while (engine == 0x0)
	{
		engine = modName(L"engine.dll", gameProcess.th32ProcessID);

	}

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
	//GetClientRect(csWindow, &rect);
	//win_width = rect.right - rect.left;
	//win_height = rect.bottom + rect.left;

	//Hinstance = GetModuleHandle(NULL);
	//WNDCLASSEX WClass;

	//ZeroMemory(&WClass, sizeof(WNDCLASSEX));


	///*WClass.cbSize = sizeof(WNDCLASSEX);
	//WClass.style = 0;
	//WClass.lpfnWndProc = WndProc;
	//WClass.cbClsExtra = 0;
	//WClass.cbWndExtra = 0;
	//WClass.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLong(csWindow, GWL_HINSTANCE));
	//WClass.hIcon = NULL;
	//WClass.hCursor = NULL;
	//WClass.hbrBackground = WHITE_BRUSH;
	//WClass.lpszMenuName = IDC_CROSS;
	//WClass.lpszClassName = WName;
	//WClass.hIconSm = NULL;*/
	//WClass.cbSize = sizeof(WNDCLASSEX);
	//WClass.style = CS_HREDRAW | CS_VREDRAW;
	//WClass.lpfnWndProc = WndProc;
	//WClass.hInstance = Hinstance;
	//WClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//WClass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	//WClass.lpszClassName = WName;
	//if (!RegisterClassEx(&WClass))
	//{
	//	MessageBox(NULL, L"Couldnt register", L"Problem", MB_OK);
	//	return 0;
	//}
	//EspHWND = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
	//	WName,
	//	L"",
	//	WS_POPUP,
	//	rect.left, rect.top,
	//	win_width, win_height,
	//	0,
	//	NULL,
	//	Hinstance,
	//	NULL);

	//if (EspHWND == NULL)
	//{
	//	MessageBox(NULL, L"Couldnt create window", L"Problem", MB_OK);
	//	return 0;
	//}
	////SetWindowLong(EspHWND, GWL_EXSTYLE, (int)GetWindowLong(EspHWND, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	//SetLayeredWindowAttributes(EspHWND, RGB(0, 0, 0), 255, ULW_COLORKEY | LWA_ALPHA);
	//if (FAILED(DwmExtendFrameIntoClientArea(EspHWND, &margins)))
	//{
	//	MessageBox(NULL, L"Couldnt extend margins", L"Problem", MB_OK);
	//	return 0;
	//}
	//ShowWindow(EspHWND, SW_SHOWDEFAULT);
	initEspHwnd();
	initD3Dx(EspHWND);

	const int NUM_THREADS = 5;
	HANDLE threads[NUM_THREADS];
	threads[0] = CreateThread(0, 0x1000, &TriggerLoop, 0, 0, 0);
	threads[1] = CreateThread(0, 0x1000, &WorkLoop, 0, 0, 0);
	threads[2] = CreateThread(0, 0x1000, &PlayerLoop, 0, 0, 0);
	threads[3] = CreateThread(0, 0x1000, &HotkeyLoop, 0, 0, 0);
	threads[4] = CreateThread(0, 0x1000, &AimbotLoop, 0, 0, 0);
	//threads[4] = CreateThread(0, 0x1000, &MouseEventAimbotLoop, 0, 0, 0);
	//threads[5] = CreateThread(0, 0x1000, &ShootLoop, 0, 0, 0);

	/*MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		Sleep(100);
	}*/
	HWND testHwnd = FindWindow(0, L"Counter-Strike: Global Offensive");
	while ((!(GetAsyncKeyState(VK_F12)<0 && GetAsyncKeyState(VK_CONTROL)<0)) && testHwnd)
	{
		testHwnd = FindWindow(0, L"Counter-Strike: Global Offensive");
		Sleep(100);
	}
	END = true;
	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

	clearD3Dx();
	clearEspHwnd();

	TerminateProcess(GetCurrentProcess(), 0);
	FreeLibraryAndExitThread(dll,0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	dll = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {CreateThread(0, 0x1000, &Main, 0, 0, 0); }
	}

	return TRUE;
}