#include <windows.h>
#include <ShObjIdl.h>
#include <shlobj.h>
#include <stdio.h>
#include "GetRecycleFilesInfo.h"


#define OUTPUT_FLENAME "recycle_filesinfo.txt"

typedef IShellDetails	FAR*	PSHELLDETAILS;
typedef IShellFolder2	FAR*	LPSHELLFOLDER2;

HWND			m_hWnd			= FALSE;            
LPSHELLFOLDER2	m_pFolder2		= FALSE;
LPSHELLFOLDER	m_pRecycleBin	= FALSE;


int WriteOutFile(char * info){
	HANDLE hf = CreateFileA(OUTPUT_FLENAME,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (hf != INVALID_HANDLE_VALUE)
	{
		int ret = SetFilePointer(hf,0,0,FILE_END);
		DWORD dwcnt = 0;
		ret = WriteFile(hf,info,lstrlenA(info),&dwcnt,0);
		CloseHandle(hf);
		if (ret && dwcnt == lstrlenA(info))
		{
			return TRUE;
		}
	}

	return FALSE;
}



void GetName (STRRET str, LPSTR lpszName)
{
	LPMALLOC pMalloc	= NULL;
	CHAR szPath[MAX_PATH] = {0};

	HRESULT hr= SHGetMalloc(&pMalloc); // windows memory management pointer needed later

	ZeroMemory (szPath, sizeof (szPath));
	switch (str.uType)
	{
	case STRRET_CSTR:
		lstrcpyA (szPath, str.cStr);
		break;
	case STRRET_OFFSET:
		break;
	case STRRET_WSTR:
		WideCharToMultiByte (CP_ACP, 0, str.pOleStr, -1, szPath, sizeof (szPath), NULL, NULL);
		pMalloc->Free (str.pOleStr);
		break;
	}

	if (NULL != lpszName)
	{
		lstrcpyA (lpszName, szPath);
	}
	pMalloc->Release();
}



void HeaderFolder ()
{
	LPMALLOC pMalloc = NULL;
	PSHELLDETAILS pDetails = NULL;
	HRESULT hr = S_OK;

	hr =SHGetMalloc(&pMalloc); // windows memory management pointer needed later
	hr = m_pRecycleBin->CreateViewObject (m_hWnd, IID_IShellDetails, (LPVOID*)&pDetails);
	if (SUCCEEDED (hr))
	{
		CHAR szTemp[MAX_PATH] = {0};
		SHELLDETAILS sd;
		int iSubItem = 0;

		while (SUCCEEDED (hr))
		{
			hr = pDetails->GetDetailsOf (NULL , iSubItem, &sd);
			if (SUCCEEDED (hr))
			{
				switch (sd.str.uType)
				{
				case STRRET_CSTR:
					lstrcpyA (szTemp, sd.str.cStr);
					break;
				case STRRET_OFFSET:
					break;
				case STRRET_WSTR:
					WideCharToMultiByte (CP_ACP, 0, sd.str.pOleStr, -1, szTemp, sizeof (szTemp), NULL, NULL);
					pMalloc->Free (sd.str.pOleStr);
					break;
				}
				//m_List.InsertColumn (iSubItem , szTemp, LVCFMT_LEFT, 100);
				iSubItem ++;
			}
		}
	}
	if (NULL != pDetails)
	{
		pMalloc->Free (pDetails);
	}
	pMalloc->Release();
}



void HeaderFolder2 ()
{
	CHAR szTemp[MAX_PATH] = {0};
	LPMALLOC pMalloc = NULL;
	HRESULT hr = S_OK;
	SHELLDETAILS sd;
	int iSubItem = 0;

	hr = SHGetMalloc(&pMalloc); // windows memory management pointer needed later

	// We'are asking the object the list of available columns.
	// For each, we are adding them to the control in the right order.
	while (SUCCEEDED (hr))
	{
		hr = m_pFolder2->GetDetailsOf (NULL , iSubItem, &sd);
		if (SUCCEEDED (hr))
		{
			switch (sd.str.uType)
			{
			case STRRET_CSTR:
				lstrcpyA (szTemp, sd.str.cStr);
				break;
			case STRRET_OFFSET:
				break;
			case STRRET_WSTR:
				WideCharToMultiByte (CP_ACP, 0, sd.str.pOleStr, -1, szTemp, sizeof (szTemp), NULL, NULL);
				pMalloc->Free (sd.str.pOleStr);
				break;
			}
			//m_List.InsertColumn (iSubItem , szTemp, LVCFMT_LEFT, 100);
			iSubItem ++;
		}
	}
	pMalloc->Release();
}


BOOL GetFolder2 ()
{
	BOOL			bReturn			= FALSE;
	STRRET			strRet			= {0};
	LPMALLOC		pMalloc			= NULL;
	LPSHELLFOLDER	pDesktop		= NULL;
	LPITEMIDLIST	pidlRecycleBin	= NULL;
	HRESULT			hr				= S_OK;
	
	if (NULL != m_pFolder2)
	{
		m_pFolder2->Release ();
		m_pFolder2 = NULL;
	}

	hr = SHGetMalloc(&pMalloc); // windows memory management pointer needed later

	if ( (SUCCEEDED (SHGetDesktopFolder(&pDesktop))) &&(SUCCEEDED (SHGetSpecialFolderLocation (m_hWnd, CSIDL_BITBUCKET, &pidlRecycleBin))) )
	{
		if (SUCCEEDED (pDesktop->BindToObject(pidlRecycleBin, NULL, IID_IShellFolder2, (LPVOID *)&m_pFolder2)))
		{
			if (S_OK == pDesktop->GetDisplayNameOf (pidlRecycleBin, SHGDN_NORMAL, &strRet))
			{
				char lpszName[MAX_PATH] = {0};
				GetName (strRet,lpszName);
			}

			bReturn = TRUE;
		}
	}

	if (NULL != pidlRecycleBin)
	{
		pMalloc->Free (pidlRecycleBin);
	}
	if (NULL != pDesktop)
	{
		pDesktop->Release();
	}
	pMalloc->Release();

	return bReturn;
}



int setQuestionMarkToSpace(char * str){
	for (int i =0;i < lstrlenA(str);i ++)
	{
		if (str[i] == '?')
		{
			str[i] = ' ';
		}
	}
	return TRUE;
}


void FillFolder2 ()
{
	LPMALLOC		pMalloc = NULL;
	CHAR			szTemp[MAX_PATH]={0};
	LPENUMIDLIST	penumFiles;
	LPITEMIDLIST	pidl = NULL;
	SHELLDETAILS	sd;
	int				iItem = 0;
	int				iSubItem = 0;
	int				iIndex = -1;
	SHFILEINFOA		fi;
	SFGAOF			sg = SFGAO_VALIDATE;
	HRESULT			hr = S_OK;

	hr=SHGetMalloc(&pMalloc); // windows memory management pointer needed later

	// Get the list of available objects
	hr = m_pFolder2->EnumObjects(m_hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS| SHCONTF_INCLUDEHIDDEN, &penumFiles);
	if (SUCCEEDED (hr))
	{
		// Iterate through list
		while (penumFiles->Next(1, &pidl, NULL) != S_FALSE)
		{
			//iItem = m_List.InsertItem (iItem, _T(""));
			//m_List.SetItemData (iItem, (DWORD)pidl);

			char infoseg[16][64] = {"name","source postion","delete datatime","size","type","create datetime","modify datatime","unknow",0};
			char fileinfo[4096] = {0};
			char *fileinfoptr = fileinfo;
			int infosize = 0;

			ZeroMemory (&fi, sizeof (fi));
			hr = SHGetFileInfoA ((LPCSTR)pidl, 0, &fi, sizeof (fi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);

			if (SUCCEEDED (hr))
			{
				iIndex = fi.iIcon;
				//m_List.SetItem (iItem, 0, LVIF_IMAGE, NULL, iIndex, 0, 0, 0);
			}

			// We iterate now in all the available columns.
			// Since it depends on the system, we "hope" that they are going to be as many 
			// and in the same order as when we have added the column's headers.

			hr = S_OK;
			iSubItem = 0;
				
			while (SUCCEEDED (hr))
			{
				hr = m_pFolder2->GetDetailsOf (pidl ,iSubItem, &sd);
				if (SUCCEEDED (hr))
				{
					switch (sd.str.uType)
					{
					case STRRET_CSTR:
						lstrcpyA(szTemp, sd.str.cStr);
						break;
					case STRRET_OFFSET:
						break;
					case STRRET_WSTR:
						WideCharToMultiByte (CP_ACP, 0, sd.str.pOleStr, -1, szTemp, sizeof (szTemp), NULL, NULL);
						pMalloc->Free (sd.str.pOleStr);
						break;
					}
					//m_List.SetItemText (iItem, iSubItem ,szTemp);

					if(iSubItem == 2 || iSubItem == 5 || iSubItem == 6 || iSubItem == 7)
					{
						int ret =setQuestionMarkToSpace(szTemp);
					}
					

					int tmpinfosize = wsprintfA(fileinfoptr,"%s=%s\r\n",infoseg[iSubItem],szTemp);
					fileinfoptr += tmpinfosize;
					infosize += tmpinfosize;

					iSubItem ++;
				}
			}

			wsprintfA(fileinfoptr,"\r\n");
			fileinfoptr += sizeof("\r\n");
			infosize += sizeof("\r\n");

			int ret=WriteOutFile(fileinfo);
		}
	}

	if (NULL != penumFiles)
	{
		penumFiles->Release ();
		penumFiles = NULL;
	}

	pMalloc->Release();
}




void FillFolder ()
{
	LPMALLOC		pMalloc			= NULL;
	CHAR			szTemp[MAX_PATH]= {0};
	LPENUMIDLIST	penumFiles		= NULL;
	LPITEMIDLIST	pidl			= NULL;
	PSHELLDETAILS	pDetails		= NULL;
	SHELLDETAILS	sd;
	int				iItem			= 0;
	int				iSubItem		= 0;
	HRESULT			hr				= S_OK;

	hr=SHGetMalloc(&pMalloc); // windows memory management pointer needed later
	hr = m_pRecycleBin->CreateViewObject (m_hWnd, IID_IShellDetails, (LPVOID*)&pDetails);

	// Iterate through list
	m_pRecycleBin->EnumObjects(m_hWnd, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS| SHCONTF_INCLUDEHIDDEN, &penumFiles);

	if (SUCCEEDED (hr))
	{
		while (penumFiles->Next(1, &pidl, NULL) != S_FALSE)
		{
			//iItem = m_List.InsertItem (iItem, _T(""));
			//m_List.SetItemData (iItem, (DWORD)pidl);

			hr = S_OK;
			iSubItem = 0;

			while (SUCCEEDED (hr))
			{
				hr = pDetails->GetDetailsOf (pidl , iSubItem, &sd);
				if (SUCCEEDED (hr))
				{
					switch (sd.str.uType)
					{
					case STRRET_CSTR:
						lstrcpyA (szTemp, sd.str.cStr);
						break;
					case STRRET_OFFSET:
						break;
					case STRRET_WSTR:
						WideCharToMultiByte (CP_ACP, 0, sd.str.pOleStr, -1, szTemp, sizeof (szTemp), NULL, NULL);
						pMalloc->Free (sd.str.pOleStr);
						break;
					}
					//m_List.SetItemText (iItem, iSubItem , szTemp);
					iSubItem ++;
				}
			}
		}
	}
	else
	{
	}
	if (NULL != pDetails)
	{
		pMalloc->Free (pDetails);
	}

	if (NULL != penumFiles)
	{
		penumFiles->Release ();
		penumFiles = NULL;
	}

	pMalloc->Release();
}





void UpdateList (void)
{
	DWORD			dwSize		= GetLogicalDriveStringsA(0, NULL);
	LPSTR			pszDrives	= (LPSTR)malloc((dwSize + 2) * sizeof (CHAR));
	LPMALLOC		pMalloc		= NULL;
	LPITEMIDLIST	pidl		= NULL;
	int				iDrive		= 0;
	int				iPos		= 0;
	//int				iMax		= m_List.GetItemCount ();
	HRESULT			hr			= S_OK;

	hr =SHGetMalloc(&pMalloc); // windows memory management pointer needed later
	//m_Empty.RemoveAllMenuItem ();

	// If we were able to get the names of available drives, then we'll check if
	// objects are available in their local RBin. If so, we'll add their name to
	// the button's menu as an option for the user to empty them individually.
	if (NULL != pszDrives)
	{
		LPSTR pstr = pszDrives;
		SHQUERYRBINFO qrbi;

		//m_Empty.AddMenuItem (IDC_FIRSTDRIVE, _T("Empty All"), 0);
		int ret =GetLogicalDriveStringsA((dwSize + 2) * sizeof (CHAR),(LPSTR)pszDrives);

		while (CHAR ('\0') != *pstr)
		{
			ZeroMemory (&qrbi, sizeof (qrbi));
			qrbi.cbSize = sizeof (qrbi);
			hr = SHQueryRecycleBinA (pstr, &qrbi);
			if (SUCCEEDED (hr))
			{
				if (0 != qrbi.i64NumItems)
				{
					iDrive ++;
					//m_Empty.AddMenuItem (IDC_FIRSTDRIVE + 128 * iDrive, pstr, 0);
				}
			}
			pstr += lstrlenA(pstr) + sizeof (CHAR);
		}
		free (pszDrives);
	}
	if (0 == iDrive)
	{
		//m_Empty.EnableWindow (FALSE);
	}

// 	for (iPos = 0 ; iPos < iMax ; iPos ++)
// 	{
// 		pidl = (LPITEMIDLIST)m_List.GetItemData (iPos);
// 		if (NULL != pidl)
// 		{
// 			pMalloc->Free (pidl);
// 		}
// 	}
// 	m_List.DeleteAllItems ();

	if (NULL != m_pFolder2)
	{
		FillFolder2 ();
	}
	else if (NULL != m_pRecycleBin)
	{
		FillFolder ();
	}

	//GetDlgItem (IDC_UNDELETEALL)->EnableWindow ((0 != m_List.GetItemCount ()) ? TRUE : FALSE);

	pMalloc->Release();
}


BOOL GetFolder ()
{
	BOOL			bReturn			= FALSE;
	STRRET			strRet			= {0};
	LPMALLOC		pMalloc			= NULL;
	LPSHELLFOLDER	pDesktop		= NULL;
	LPITEMIDLIST	pidlRecycleBin	= NULL;
	HRESULT			hr				= S_OK;

	hr = SHGetMalloc(&pMalloc); // windows memory management pointer needed later
	hr = SHGetDesktopFolder(&pDesktop);

	hr = SHGetSpecialFolderLocation (m_hWnd, CSIDL_BITBUCKET, &pidlRecycleBin);
	if (NULL != m_pRecycleBin)
	{
		m_pRecycleBin->Release ();
		m_pRecycleBin = NULL;
	}
	hr = pDesktop->BindToObject(pidlRecycleBin, NULL, IID_IShellFolder, (LPVOID *)&m_pRecycleBin);
	if (SUCCEEDED (hr))
	{
		bReturn = TRUE;
	}

	if (S_OK == pDesktop->GetDisplayNameOf (pidlRecycleBin, SHGDN_NORMAL, &strRet))
	{
		char lpszName[MAX_PATH]= {0};
		GetName (strRet,lpszName);
	}

	pMalloc->Free (pidlRecycleBin);
	pDesktop->Release();
	pMalloc->Release();

	return bReturn;
}














int GetRecycleFilesInfo(void)
{
	int iret = CoInitialize(NULL);
	if (TRUE == GetFolder2())
	{
		HeaderFolder2();
	}
	else if (TRUE == GetFolder())
	{
		HeaderFolder();
	}

	UpdateList();
	CoUninitialize();
	return TRUE;
}

char* findFileInfosInDir(char* dir) {
	char szLastDir[] = { '.','.',0 };
	WIN32_FIND_DATAA stWfd = { 0 };

	char strPath[MAX_PATH] = { 0 };
	lstrcpyA(strPath, dir);
	lstrcatA(strPath, "*.*");

	HANDLE hFind = FindFirstFileA(strPath, (LPWIN32_FIND_DATAA)&stWfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do
	{
		if (memcmp(stWfd.cFileName, szLastDir, 2) == 0 || memcmp(stWfd.cFileName, ".", 1) == 0)
		{
			continue;
		}
		else {

		}
	} while (FindNextFileA(hFind, (LPWIN32_FIND_DATAA)&stWfd));
	FindClose(hFind);
	return FALSE;
}


char* findFirstSubdirInDir(char* dir) {
	char szLastDir[] = { '.','.',0 };
	WIN32_FIND_DATAA stWfd = { 0 };

	char strPath[MAX_PATH] = { 0 };
	lstrcpyA(strPath, dir);
	lstrcatA(strPath, "*.*");

	HANDLE hFind = FindFirstFileA(strPath, (LPWIN32_FIND_DATAA)&stWfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	do
	{
		if (memcmp(stWfd.cFileName, szLastDir, 2) == 0 || memcmp(stWfd.cFileName, ".", 1) == 0)
		{
			continue;
		}

		if (stWfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			FindClose(hFind);
			char* dirname = stWfd.cFileName;
			return stWfd.cFileName;
		}
		else {
			char* filename = stWfd.cFileName;
		}
	} while (FindNextFileA(hFind, (LPWIN32_FIND_DATAA)&stWfd));
	FindClose(hFind);
	return FALSE;
}

int __stdcall WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd ){

	int ret = GetRecycleFilesInfo();

	/*
	char szsysdir[MAX_PATH]={0};
	char szcurdir[MAX_PATH]={0};
	int ret =GetCurrentDirectoryA(MAX_PATH,szcurdir);
	if (ret==FALSE)
	{
		return FALSE;
	}

	ret = GetSystemDirectoryA(szsysdir,MAX_PATH);
	if (ret == FALSE)
	{
		return FALSE;
	}

	char szrecycledir[MAX_PATH] = {0};
	szrecycledir[0] = szsysdir[0];
	szrecycledir[1] = ':';
	szrecycledir[2] = '\\';
	lstrcatA(szrecycledir,"$Recycle.Bin\\");

	char * lprecyclesubpath = findFirstSubdirInDir(szrecycledir);
	lstrcatA(szrecycledir,lprecyclesubpath);
	lstrcatA(szrecycledir,"\\");
	*/

	return TRUE;
}