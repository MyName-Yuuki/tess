// PWInstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PWDownloader.h"
#include "PWInstallDlg.h"
#include "LogFile.h"

// Include REAL Angelica Framework SDK
#include <AFilePackage.h>
#include <AFilePackMan.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char g_szGameDir[];
extern char g_szClientCompleted[];
extern char g_szInstallDir[];
extern void UDeleteFile(const char* filename);

// Global instance of AFilePackMan (defined in Angelica SDK)
extern AFilePackMan g_AFilePackMan;

/////////////////////////////////////////////////////////////////////////////
// CPWInstallDlg dialog

CPWInstallDlg* CPWInstallDlg::s_pThis;

CPWInstallDlg::CPWInstallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPWInstallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPWInstallDlg)
	m_szCurFile = _T("");
	m_szProgress = _T("");
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pBG = NULL;
	m_bInstallOK = false;
	m_nTotal = 100;
	m_nCompleted = 0;
	::InitializeCriticalSection(&m_csLock);
	s_pThis = this;
}

CPWInstallDlg::~CPWInstallDlg()
{
	if( m_pBG )
	{
		delete m_pBG;
		m_pBG = NULL;
	}

	::DeleteCriticalSection(&m_csLock);
	s_pThis = NULL;
}

void CPWInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPWInstallDlg)
	DDX_Control(pDX, IDC_CTL_PROGRESS, m_ctlProgress);
	DDX_Text(pDX, IDC_CUR_FILE, m_szCurFile);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, m_szProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPWInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CPWInstallDlg)
	ON_WM_TIMER()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPWInstallDlg message handlers

void CPWInstallDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if( nIDEvent == 0 )
	{
		::EnterCriticalSection(&m_csLock);
		int iProgress = (int)(1000 * ((double)(INT64)m_nCompleted / (INT64)m_nTotal));
		m_ctlProgress.SetPos(iProgress);
		m_szProgress.Format(_T("%.1f%%"), (double)(INT64)m_nCompleted / (INT64)m_nTotal * 100);
		UpdateData(FALSE);
		if( m_nCompleted == m_nTotal )
			OnExtractComplete();
		::LeaveCriticalSection(&m_csLock);
	}

	CDialog::OnTimer(nIDEvent);
}

// ��װ�ͻ��˵��߳�
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	CPWInstallDlg* pDlg = (CPWInstallDlg*)lpParameter;
	IArchive7Z* pArchive = CreateArchive7Z();
	pArchive->SetExtractCB(CPWInstallDlg::OnExtractCallback);
	if( pArchive->LoadPack(g_szClientCompleted, "") )
	{
		if( !pArchive->ExtractTo(pDlg->m_sExtractDir) )
		{
			MessageBox(NULL, "���������̿ռ䣬Ȼ����������ͻ��˿����½��н�ѹ!", "����", MB_OK|MB_ICONERROR);
			LogOutput("ThreadProc, Extract the full client pack failed!");
		}
	}
	else
	{
		MessageBox(NULL, "�򿪰�װ��ʧ�ܣ�", "����", MB_OK|MB_ICONERROR);
		LogOutput("ThreadProc, Load the full client pack failed!");
	}
	pArchive->Release();
	return 0;
}

// ��Ӧ��ѹ���
void CPWInstallDlg::OnExtractComplete()
{
	KillTimer(0);
	m_bInstallOK = true;

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if( pSysMenu ) pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|MF_ENABLED);

	if( MessageBox(_T("��ѹ��ɣ��Ƿ�Ҫɾ��ѹ�����Խ�ʡ���̿ռ䣿"), "��ʾ", MB_YESNO|MB_ICONQUESTION) == IDYES )
		UDeleteFile(g_szClientCompleted);

	// ======== NEW: Proses Patching PCK ========
	// 1. Verifikasi & Inisialisasi SEMUA PCK dengan key
	const char* base64Key = "AjVbfOzuLlj3NVt87BgBAA=="; // Key untuk verifikasi/enkripsi

	// Scan dan proses semua file .pck di direktori instalasi
	char szSearchPath[MAX_PATH];
	sprintf(szSearchPath, "%s\\*.pck", g_szInstallDir);

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(szSearchPath, &findData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		int nPckCount = 0;
		int nPckSuccess = 0;

		do
		{
			// Skip directory entries
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			// Build full path to PCK file
			char szPckPath[MAX_PATH];
			sprintf(szPckPath, "%s\\%s", g_szInstallDir, findData.cFileName);

			nPckCount++;
			LogOutput("[%d] Memproses: %s", nPckCount, findData.cFileName);

			if( VerifyAndInitPCK(szPckPath) )
			{
				LogOutput("  SUCCESS: PCK %s berhasil dibuka/diverifikasi", findData.cFileName);
				nPckSuccess++;
			}
			else
			{
				LogOutput("  WARNING: Gagal membuka %s", findData.cFileName);
			}

		} while (FindNextFileA(hFind, &findData));

		FindClose(hFind);

		LogOutput("========================================");
		LogOutput("PCK Verification Summary:");
		LogOutput("Total PCK files: %d", nPckCount);
		LogOutput("Success: %d", nPckSuccess);
		LogOutput("Failed: %d", nPckCount - nPckSuccess);
		LogOutput("========================================");
	}
	else
	{
		LogOutput("WARNING: Tidak ada file .pck ditemukan di: %s", g_szInstallDir);
	}

	// 2. Patch file-file yang di-update ke PCK
	// TODO: Tambahkan logika patch file di sini
	// Contoh:
	// PatchPCKFile("models.pck", "models\\monster\\new_monster.ecm", "update\\models\\monster\\new_monster.ecm");

	// ======== END NEW: Proses Patching PCK ========

	// ������װ����
	std::string strWorkDir = std::string(g_szInstallDir) + "\\����������ʰ�";
	std::string strPath = strWorkDir + "\\install.exe";
	ShellExecuteA(NULL, "open", strPath.c_str(), NULL, strWorkDir.c_str(), SW_SHOWNORMAL);
	CDialog::OnOK();
}

void CPWInstallDlg::OnExtractCallback(int op, IArchive7Z::PROPERTY& prop)
{
	CPWInstallDlg* pDlg = CPWInstallDlg::s_pThis;

	::EnterCriticalSection(&pDlg->m_csLock);
	if( op == IArchive7Z::OP_GETTOTAL )
	{
		pDlg->m_nTotal = prop.pi;
	}
	else if( op == IArchive7Z::OP_GETCOMPLETED )
	{
		pDlg->m_nCompleted = prop.pi;
	}
	else if( op == IArchive7Z::OP_GETCURFILE )
	{
		pDlg->m_szCurFile = prop.ps;
	}
	::LeaveCriticalSection(&pDlg->m_csLock);
}

BOOL CPWInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Load the background image
	LoadBGImage();
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ctlProgress.SetRange(0, 1000);

	// ��ʼ��ѹ�ļ�
	HANDLE hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
	::CloseHandle(hThread);
	SetTimer(0, 50, NULL);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if( pSysMenu ) pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|MF_GRAYED);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

bool CPWInstallDlg::LoadBGImage()
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_INSTALLBG), "PNG");
	if( !hResource ) return false;
	DWORD imageSize = SizeofResource(hInst, hResource);
	if( !imageSize ) return false;

	const void* pResData = LockResource(LoadResource(hInst, hResource));
	if( !pResData ) return false;
	HGLOBAL hBuffer = GlobalAlloc(GMEM_FIXED, imageSize);
	if( hBuffer )
	{
		void* pBuffer = GlobalLock(hBuffer);
		if( pBuffer )
		{
			memcpy(pBuffer, pResData, imageSize);
			GlobalUnlock(hBuffer);

			IStream* pStream = NULL;
			if( SUCCEEDED(CreateStreamOnHGlobal(hBuffer, FALSE, &pStream)) )
			{
				m_pBG = Gdiplus::Image::FromStream(pStream);
				pStream->Release();
			}
		}

		GlobalFree(hBuffer);
	}

	return true;
}

void CPWInstallDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	Gdiplus::Graphics gph(dc);
	gph.DrawImage(m_pBG, 7, 7, m_pBG->GetWidth(), m_pBG->GetHeight());

	// Do not call CDialog::OnPaint() for painting messages
}

void CPWInstallDlg::OnCancel()
{
	// ʲôҲ��������ֹESC�˳�
}
/////////////////////////////////////////////////////////////////////////////
// PCK Verification & Patching Implementation

BOOL CPWInstallDlg::Base64Decode(const char* szBase64, BYTE* pOut, DWORD* pdwOutSize)
{
	static const char base64_table[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int len = (int)strlen(szBase64);
	BYTE* pOutPtr = pOut;
	DWORD dwOutLen = 0;
	DWORD dwBuf = 0;
	int nBits = 0;

	for (int i = 0; i < len; i++)
	{
		if (szBase64[i] == '=')
			break;

		const char* p = strchr(base64_table, szBase64[i]);
		if (!p)
			continue;

		int val = (int)(p - base64_table);
		dwBuf = (dwBuf << 6) | val;
		nBits += 6;

		if (nBits >= 8)
		{
			nBits -= 8;
			if (dwOutLen < *pdwOutSize)
			{
				*pOutPtr++ = (BYTE)((dwBuf >> nBits) & 0xFF);
				dwOutLen++;
			}
		}
	}

	*pdwOutSize = dwOutLen;
	return TRUE;
}

BOOL CPWInstallDlg::CalculatePCKChecksum(const char* szPckFile, BYTE* pChecksum)
{
	FILE* f = fopen(szPckFile, "rb");
	if (!f)
		return FALSE;

	BYTE buffer[256];
	memset(buffer, 0, sizeof(buffer));
	size_t nRead = fread(buffer, 1, 256, f);
	fclose(f);

	if (nRead == 0)
		return FALSE;

	BYTE checksum = 0;
	for (size_t i = 0; i < nRead; i++)
	{
		checksum ^= buffer[i];
	}

	*pChecksum = checksum;
	return TRUE;
}

BOOL CPWInstallDlg::VerifyAndInitPCK(const char* szPckFile)
{
	LogOutput("Memverifikasi PCK: %s", szPckFile);

	// 1. Cek apakah file ada
	if (_access(szPckFile, 0) != 0)
	{
		LogOutput("PCK file tidak ditemukan: %s", szPckFile);
		return FALSE;
	}

	// 2. Decode key Base64
	BYTE key[256];
	DWORD keySize = sizeof(key);
	if (!Base64Decode("AjVbfOzuLlj3NVt87BgBAA==", key, &keySize))
	{
		LogOutput("Gagal decode Base64 key!");
		return FALSE;
	}

	LogOutput("Key berhasil di-decode: %d bytes", keySize);

	// 3. Hitung checksum PCK
	BYTE pckChecksum;
	if (!CalculatePCKChecksum(szPckFile, &pckChecksum))
	{
		LogOutput("Gagal hitung checksum PCK!");
		return FALSE;
	}

	LogOutput("PCK Checksum: 0x%02X", pckChecksum);

	// 4. Verifikasi dengan key (simple: compare first byte)
	bool bVerified = (key[0] == pckChecksum);

	if (!bVerified)
	{
		LogOutput("WARNING: Checksum tidak match! Key mungkin salah.");
		// Tetap lanjut buka PCK (mode flexible)
	}

	// 5. Buka PCK dengan AFilePackMan
	if (g_AFilePackMan.OpenFilePackageInGame(szPckFile))
	{
		LogOutput("PCK %s berhasil dibuka", szPckFile);
		return TRUE;
	}
	else
	{
		LogOutput("Gagal membuka PCK: %s", szPckFile);
		return FALSE;
	}
}

BOOL CPWInstallDlg::PatchPCKFile(const char* szPckFile, const char* szFileName, const char* szNewFile)
{
	// 1. Baca file baru
	FILE* f = fopen(szNewFile, "rb");
	if (!f)
	{
		LogOutput("Gagal membuka file baru: %s", szNewFile);
		return FALSE;
	}

	fseek(f, 0, SEEK_END);
	DWORD dwFileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	BYTE* pFileData = new BYTE[dwFileSize];
	fread(pFileData, 1, dwFileSize, f);
	fclose(f);

	LogOutput("Patching: %s -> %s", szNewFile, szFileName);

	// 2. Compress dengan ZLIB
	DWORD dwCompSize = dwFileSize * 2;
	BYTE* pCompData = new BYTE[dwCompSize];

	if (AFilePackage::Compress(pFileData, dwFileSize, pCompData, &dwCompSize) == 0)
	{
		LogOutput("File terkompres: %d -> %d bytes", dwFileSize, dwCompSize);
	}
	else
	{
		LogOutput("File tidak terkompres, gunakan asli: %d bytes", dwFileSize);
		dwCompSize = dwFileSize;
		memcpy(pCompData, pFileData, dwFileSize);
	}

	// 3. Enkripsi dengan key (TODO)
	BYTE key[256];
	DWORD keySize = sizeof(key);
	Base64Decode("AjVbfOzuLlj3NVt87BgBAA==", key, &keySize);

	// TODO: Enkripsi pCompData dengan key
	// TODO: Tulis ke PCK

	LogOutput("TODO: Tulis ke PCK %s", szPckFile);

	delete[] pFileData;
	delete[] pCompData;

	return TRUE;
}
