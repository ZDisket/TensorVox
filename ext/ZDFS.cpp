
#include "ZDFS.h"

bool ZDFS::WinBoolToStdBool(const BOOL& in_bInput)
{
	if (in_bInput == TRUE) { return true; } else {return false;}
}

BOOL ZDFS::StdBoolToWinBool(const bool & in_sbInput)
{
	if (in_sbInput) { return TRUE; }
	else { return FALSE; }

}

void ZDFS::EvalAddItem(const WIN32_FIND_DATAA & in_FinData, std::vector<SItem>& in_Vec)
{

	SItem Item;

	Item.FileSzHigh = in_FinData.nFileSizeHigh;
	Item.FileSzLow = in_FinData.nFileSizeLow;

	Item.Name = in_FinData.cFileName;
	if (Item.Name == ".." || Item.Name == ".") { return; }

    
	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { Item.IType = ZFS_TFOLDER; }
	else { Item.IType = ZFS_TFILE; }

	// Fill out attributes

	Item.Attributes.Archive = false;
	Item.Attributes.Hidden = false;
	Item.Attributes.Normal = false;
	Item.Attributes.ReadOnly = false;
	Item.Attributes.System = false;
	Item.Attributes.Temporary = false;

	// I have no idea how a single value can equal multiple things, 
	// but I saw many others and MSDN do it, so this must be fine

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
		Item.Attributes.Archive = true;
	}
	
	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
		Item.Attributes.Compressed = true;
	}
	
	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
		Item.Attributes.Hidden = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		Item.Attributes.Normal = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
		Item.Attributes.ReadOnly = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
		Item.Attributes.System = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
		Item.Attributes.Temporary = true;
	}

	FileTimeToSystemTime(&in_FinData.ftCreationTime, &Item.TimeOfCreation);
	FileTimeToSystemTime(&in_FinData.ftLastAccessTime, &Item.LastAccessTime);
	FileTimeToSystemTime(&in_FinData.ftLastWriteTime, &Item.LastWriteTime);

	in_Vec.push_back(Item);

}

void ZDFS::EvalAddItemW(const WIN32_FIND_DATAW & in_FinData, std::vector<SItemW>& in_wVec)
{
	SItemW Item;

	

	Item.FileSzHigh = in_FinData.nFileSizeHigh;
	Item.FileSzLow = in_FinData.nFileSizeLow;

	Item.Name = in_FinData.cFileName;

	if (Item.Name == L".." || Item.Name == L".") { return; }

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		Item.IType = ZFS_TFOLDER;
	else
		Item.IType = ZFS_TFILE; 

	// Fill out attributes

	Item.Attributes.Archive = false;
	Item.Attributes.Hidden = false;
	Item.Attributes.Normal = false;
	Item.Attributes.ReadOnly = false;
	Item.Attributes.System = false;
	Item.Attributes.Temporary = false;



	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
		Item.Attributes.Archive = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
		Item.Attributes.Compressed = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
		Item.Attributes.Hidden = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		Item.Attributes.Normal = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
		Item.Attributes.ReadOnly = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
		Item.Attributes.System = true;
	}

	if (in_FinData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
		Item.Attributes.Temporary = true;
	}

	FileTimeToSystemTime(&in_FinData.ftCreationTime, &Item.TimeOfCreation);
	FileTimeToSystemTime(&in_FinData.ftLastAccessTime, &Item.LastAccessTime);
	FileTimeToSystemTime(&in_FinData.ftLastWriteTime, &Item.LastWriteTime);

	in_wVec.push_back(Item);

}



ZDFS::ZDFS()
{
}

bool ZDFS::FileExists(const std::string& in_sFilePath)
{
	if (GetFileAttributesA(in_sFilePath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) { return false; }

	return WinBoolToStdBool(PathFileExistsA(in_sFilePath.c_str()));
}

bool ZDFS::FileExists(const std::wstring& in_wsFilePath)
{
	if (GetFileAttributesW(in_wsFilePath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) { return false; }
	return WinBoolToStdBool(PathFileExistsW(in_wsFilePath.c_str()));

}

bool ZDFS::FolderExists(const std::string& in_sFolderPath)
{
	return GetFileAttributesA(in_sFolderPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY;


	//return WinBoolToStdBool(PathIsDirectoryA(in_sFolderPath.c_str()));
}

bool ZDFS::FolderExists(const std::wstring & in_wsFolderPath)
{
	return GetFileAttributesW(in_wsFolderPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY;

//	return WinBoolToStdBool(PathIsDirectoryW(in_wsFolderPath.c_str()));
}


bool ZDFS::MakeDir(const std::string & in_sFolderName)
{

	if (CreateDirectoryA(in_sFolderName.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ZDFS::MakeDir(const std::wstring & in_wsFolderName)
{
	if (CreateDirectoryW(in_wsFolderName.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<SItem> ZDFS::StuffInDirectory(const std::string & in_sPath,std::string Filter)
{
	std::vector<SItem> l_vStuff;

	WIN32_FIND_DATAA l_FindData;

	std::string NewPath = in_sPath + "\\" + Filter;
	HANDLE hFind = FindFirstFileA(NewPath.c_str(), &l_FindData);

	EvalAddItem(l_FindData, l_vStuff);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileA(hFind, &l_FindData) != 0) 
		{
			EvalAddItem(l_FindData, l_vStuff);

		}
		if (GetLastError() == ERROR_NO_MORE_FILES) {
			FindClose(hFind);
		}
	}
	return l_vStuff;

}

std::vector<SItemW> ZDFS::StuffInDirectory(const std::wstring & in_sPath,std::wstring wFilter)
{
	std::vector<SItemW> l_vStuff;

	WIN32_FIND_DATAW l_FindData;

	std::wstring NewPath = in_sPath + L"\\" + wFilter;
	HANDLE hFind = FindFirstFileW(NewPath.c_str(), &l_FindData);
	if (hFind == INVALID_HANDLE_VALUE) {return l_vStuff; }
	EvalAddItemW(l_FindData, l_vStuff);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileW(hFind, &l_FindData) != 0)
		{
			EvalAddItemW(l_FindData, l_vStuff);

		}
		if (GetLastError() == ERROR_NO_MORE_FILES) {
			FindClose(hFind);
		}
	}
	return l_vStuff;

}

std::vector<SItemW> ZDFS::RecursiveStuffInDirectory(const std::wstring & in_sPath, std::wstring wFilter)
{
	std::vector<SItemW> l_vStuff;

	WIN32_FIND_DATAW l_FindData;

	std::wstring NewPath = in_sPath + L"\\" + wFilter;
	HANDLE hFind = FindFirstFileW(NewPath.c_str(), &l_FindData);
	if (hFind == INVALID_HANDLE_VALUE) { return l_vStuff; }
	EvalAddItemW(l_FindData, l_vStuff);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileW(hFind, &l_FindData) != 0)
		{
			EvalAddItemW(l_FindData, l_vStuff);

		}
		if (GetLastError() == ERROR_NO_MORE_FILES) {
			FindClose(hFind);
		}
	}

	// Iter thru our entries and if it's a directory, repeat again.

	auto DIt = l_vStuff.begin();

	while (DIt != l_vStuff.end()) {
		SItemW& Item = *DIt;

		if (Item.IType == ZFS_TFOLDER)
			Item.SubEntries = RecursiveStuffInDirectory(in_sPath + L"\\" + Item.Name);

		++DIt;
	}

	return l_vStuff;
}

UINT64 ZDFS::GetSize(std::vector<SItemW>& Items)
{
	UINT64 Sz = 0;
	auto Dit = Items.begin();

	while (Dit != Items.end())
	{
		SItemW& Item = *Dit;
	
		Sz += Item.FileSzHigh + Item.FileSzLow;
		if (Item.IType == ZFS_TFOLDER)
			Sz += GetSize(Item.SubEntries);

		++Dit;
	}

	return Sz;

}

std::string ZDFS::RelativeToFullPath(const std::string & in_sConvPth)
{

	char* l_pTempBuffer = new char[RTF_ALLOC_SZ]; 

	std::string l_sFullPath = _fullpath(l_pTempBuffer, in_sConvPth.c_str(), 4096);
	delete[] l_pTempBuffer;

	return l_sFullPath;
	

}

std::wstring ZDFS::RelativeToFullPath(const std::wstring & in_wsConvPath)
{

	wchar_t* l_pTempBuffer = new wchar_t[RTF_ALLOC_SZ]; 

	std::wstring l_swFullPath = _wfullpath(l_pTempBuffer, in_wsConvPath.c_str(), 4096);
	delete[] l_pTempBuffer;

	return l_swFullPath;
}

bool ZDFS::FileCopy(const std::string & in_sOriginalFileName, const std::string & in_sNewFileName, bool Overwrite)
{
	bool fcret = WinBoolToStdBool(CopyFileA(in_sOriginalFileName.c_str(), in_sNewFileName.c_str(),StdBoolToWinBool(Overwrite)));
	return fcret;
}

bool ZDFS::FileCopy(const std::wstring & in_wsOriginalFileName, const std::wstring & in_wsNewFileName, bool Overwrite)
{
	bool fcret = WinBoolToStdBool(CopyFileW(in_wsOriginalFileName.c_str(), in_wsNewFileName.c_str(), StdBoolToWinBool(Overwrite)));
	return fcret;
}

std::wstring ZDFS::GetKnownFolderPath(KNOWNFOLDERID & kfid, const HANDLE handleIN, const DWORD & flags)
{

	wchar_t* wptr = NULL;
	if (SHGetKnownFolderPath(kfid,flags,handleIN,&wptr) == S_OK)
	{
		//MSDN: The returned path does not include a trailing backslash.
		// We add one ourselves.
		std::wstring wRet(wptr);
		wRet.append(L"\\");
		CoTaskMemFree(wptr);

		return wRet;


	}
	// If it gets here, it means something went wrong.
	return L"";

}

ZDFS::~ZDFS()
{
}
