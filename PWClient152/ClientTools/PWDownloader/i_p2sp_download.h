#ifndef _I_P2SP_DOWNLOAD_H_
#define _I_P2SP_DOWNLOAD_H_

// Stub implementation for P2SP download library
// This allows compilation but P2SP download functionality will not work

#pragma once

#include <windows.h>

// P2SP Types
typedef struct _TaskInfo
{
	char szFileMD5[64];
	char szFileName[MAX_PATH];
	unsigned __int64 nTotalSize;
	unsigned __int64 nCompletedSize;
	int nDownSpeed;
	int nUpSpeed;
	int nDownloadStatus;
} TaskInfomationT, *PTaskInfomationT;

typedef enum _LSP_SETTING_FLAG
{
	LSP_ENABLE,
	LSP_DISABLE
} LSP_SETTING_FLAG;

typedef struct _SettingItem
{
	int nDownSpeed;
	int nUpSpeed;
} SettingItemT, *PSettingItemT;

#endif // _I_P2SP_DOWNLOAD_H_
