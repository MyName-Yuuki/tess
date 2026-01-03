// EC_Pipe_stub.cpp - Stub implementation for compilation
// Pipe communication for P2SP download

#include "stdafx.h"
#include "EC_Pipe.h"

CECPipe::CECPipe()
{
	m_hWndClient = NULL;
	m_hPipe = INVALID_HANDLE_VALUE;
	m_iPipeDataSize = 0;
	m_iSendedSize = 0;
}

CECPipe::~CECPipe()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		::DisconnectNamedPipe(m_hPipe);
		::CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	// Clear send queue
	for (SendDataQueue::iterator it = m_SendQueue.begin(); it != m_SendQueue.end(); ++it)
	{
		delete[] it->pBuf;
	}
	m_SendQueue.clear();
}

bool CECPipe::Init(HWND hClientWnd)
{
	m_hWndClient = hClientWnd;

	// Create named pipe
	m_hPipe = ::CreateNamedPipeA(
		APP_PIPE_NAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		PIPEDATA_MAXSIZE,
		PIPEDATA_MAXSIZE,
		NMPWAIT_USE_DEFAULT_OK,
		NULL
	);

	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		LogOutput("CECPipe::Init() - CreateNamedPipe failed: %d", GetLastError());
		return false;
	}

	return true;
}

void CECPipe::Tick()
{
	// Check if client is connecting
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		if (::ConnectNamedPipe(m_hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED)
		{
			// Process pipe data
			ProcessPipeData();

			// Process send queue
			ProcessSendData();
		}
	}
}

void CECPipe::OnPipeData(int iSize)
{
	m_iPipeDataSize += iSize;

	if (m_iPipeDataSize >= PIPEDATA_MAXSIZE)
	{
		ProcessPipeData();
		m_iPipeDataSize = 0;
	}
}

bool CECPipe::SendDownloadOK()
{
	CMD_HEAD cmd;
	cmd.id = OUT_DOWNLOAD_OK;
	return SendData(&cmd, sizeof(cmd));
}

void CECPipe::SendTaskInfo(bool bDown, float progress, float dspeed, float uspeed)
{
	CMD_TASKINFO cmd;
	cmd.isdownloading = bDown;
	cmd.progress = progress;
	cmd.dspeed = dspeed;
	cmd.uspeed = uspeed;
	SendData(&cmd, sizeof(cmd));
}

bool CECPipe::AddSendDataToQueue(void* pBuf, int iSize)
{
	SENDDATA data;
	data.pBuf = new char[iSize];
	memcpy(data.pBuf, pBuf, iSize);
	data.iSize = iSize;
	m_SendQueue.push_back(data);
	return true;
}

void CECPipe::ProcessSendData()
{
	if (m_SendQueue.empty())
		return;

	SENDDATA& data = m_SendQueue.front();

	DWORD dwWritten = 0;
	if (::WriteFile(m_hPipe, data.pBuf, data.iSize, &dwWritten, NULL))
	{
		OnDataSended(data);
		m_SendQueue.pop_front();
	}
}

bool CECPipe::SendData(void* pBuf, int iSize)
{
	if (m_iSendedSize > 0)
	{
		// Previous send not processed yet, add to queue
		return AddSendDataToQueue(pBuf, iSize);
	}

	DWORD dwWritten = 0;
	if (::WriteFile(m_hPipe, pBuf, iSize, &dwWritten, NULL))
	{
		m_iSendedSize = iSize;
		return true;
	}

	return false;
}

void CECPipe::OnDataSended(const SENDDATA& data)
{
	delete[] data.pBuf;
	m_iSendedSize = 0;
}

bool CECPipe::CheckValid(BYTE* pBuf, int iSize)
{
	// Validate pipe data size
	if (iSize <= 0 || iSize > PIPEDATA_MAXSIZE)
		return false;

	return true;
}

void CECPipe::ProcessPipeData()
{
	BYTE buffer[PIPEDATA_MAXSIZE];
	DWORD dwRead = 0;

	if (::ReadFile(m_hPipe, buffer, sizeof(buffer), &dwRead, NULL))
	{
		if (!CheckValid(buffer, dwRead))
		{
			LogOutput("CECPipe::ProcessPipeData() - Invalid data");
			return;
		}

		CMD_HEAD* pCmd = (CMD_HEAD*)buffer;

		switch (pCmd->id)
		{
		case IN_DOWNLOAD:
			// Start download - not implemented in stub
			break;

		case IN_INSTALL:
			// Start install - not implemented in stub
			break;

		case IN_QUIT:
			// Quit - not implemented in stub
			break;

		case IN_GETTASKINFO:
			// Get task info - not implemented in stub
			break;

		case IN_GETDOWNLOADOK:
			// Check download OK - not implemented in stub
			break;

		case IN_SWITCHGAME:
			// Switch game - not implemented in stub
			break;
		}
	}
}
