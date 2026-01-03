// EC_PWDownload_stub.cpp - Stub implementation for compilation
// P2SP download functionality is NOT available without the actual library

#include "stdafx.h"
#include "EC_PWDownload.h"

// Stub implementation - all functions return failure or do nothing
// P2SP download requires the actual p2sp_4th_lib.dll which is missing

CECPWDownload::CECPWDownload()
{
	m_hP2SPDLL = NULL;
	m_hDownloadThread = NULL;
	m_bP2SPInitOK = false;
	m_pPipe = NULL;
	m_bDownloadOK = false;
	m_bStopThread = false;
	::InitializeCriticalSection(&m_csInfoLock);
}

CECPWDownload::~CECPWDownload()
{
	::DeleteCriticalSection(&m_csInfoLock);
}

CECPWDownload& CECPWDownload::GetInstance()
{
	static CECPWDownload instance;
	return instance;
}

bool CECPWDownload::Init(HWND hWndClient)
{
	// P2SP library not available
	LogOutput("WARNING: P2SP download not available - missing library");
	return false;
}

void CECPWDownload::Release()
{
	// Nothing to release
}

bool CECPWDownload::StartDownload()
{
	// P2SP not available
	return false;
}

void CECPWDownload::StopDownload()
{
	// Nothing to stop
}

void CECPWDownload::OnEnterOrLeaveGame(bool bEnter)
{
	// P2SP not available
}

void CECPWDownload::Install()
{
	// P2SP not available - directly launch installer
	extern char g_szInstallDir[];
	std::string strWorkDir = std::string(g_szInstallDir) + "\\完美英雄搜\\";
	std::string strPath = strWorkDir + "\\install.exe";
	ShellExecuteA(NULL, "open", strPath.c_str(), NULL, strWorkDir.c_str(), SW_SHOWNORMAL);
}

void CECPWDownload::Quit()
{
	// Nothing to quit
}

void CECPWDownload::Tick()
{
	// Nothing to tick
}

bool CECPWDownload::GetTaskInfo(float& fProgress, float& fDownSpeed, float& fUpSpeed)
{
	fProgress = 0.0f;
	fDownSpeed = 0.0f;
	fUpSpeed = 0.0f;
	return false;
}

bool CECPWDownload::SetDownSpeed(int downspeed, int upspeed)
{
	return false;
}

bool CECPWDownload::GetMaxSpeed(int& downspeed, int& upspeed)
{
	downspeed = 0;
	upspeed = 0;
	return false;
}

int CECPWDownload::GetErrorCode()
{
	return -1; // Error: P2SP not available
}

bool CECPWDownload::p2sp_Start(int gameID) { return false; }
bool CECPWDownload::p2sp_Setting(const SettingItemT* setting) { return false; }
bool CECPWDownload::p2sp_GetSetting(SettingItemT* setting) { return false; }
bool CECPWDownload::p2sp_SetLsp(LSP_SETTING_FLAG flag) { return false; }
bool CECPWDownload::p2sp_AddDownload(const char* filemd5) { return false; }
bool CECPWDownload::p2sp_StartDownload(const char* filemd5) { return false; }
bool CECPWDownload::p2sp_StopDownload(const char* filemd5) { return false; }
bool CECPWDownload::p2sp_DeleteDownload(const char* filemd5) { return false; }
bool CECPWDownload::p2sp_GetTaskInformation(TaskInfomationT* info) { return false; }
bool CECPWDownload::p2sp_Stop() { return false; }
bool CECPWDownload::GetUpdateServer() { return false; }
bool CECPWDownload::GetFullClientMD5(std::string& strMD5, std::string& strFile) { return false; }
bool CECPWDownload::GetLastClientDL() { return false; }
void CECPWDownload::WriteLastClientDL() {}
bool CECPWDownload::StartP2SPEngine() { return false; }
