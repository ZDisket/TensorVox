#pragma once
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"Shell32.lib")
#pragma comment(lib,"Ole32.lib")

/*===================================================================== 
* Introduction:  
* ZDFS: ZD Filesystem
* ================
* Description:
* Wrapper around Win32 API to provide easy filesystem functions in modern
* C++ stuff
* ====================
# Author: ZDisket
# Copyright (C) 2019 YOUR MOM GAY LOLOLOL
* ===================================================================== 
*/
#define ZDFS_MAN_INCLUDE
#ifdef ZDFS_MAN_INCLUDE
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <Shlwapi.h>
#include <Shlobj.h>

#endif

// Size of char to alloc to use RelativeToFullPath. 
// Only change if you're having trouble and it includes long paths.
#define RTF_ALLOC_SZ 4096
#define ALLF "*"
#define WALLF L"*"
typedef short Type;
#define ZFS_TFOLDER 1
#define ZFS_TFILE 0
struct FAttrib {
	// Note: If this is true, then it shouldn't have any of the other attributes
	bool Normal;

	bool Hidden;
	bool Temporary;
	bool Archive;
	bool ReadOnly;
	bool System;
	bool Compressed;
};
struct SItem // Struct that indicates anything found by StuffInDirectory()
{
	// ZFS_TFOLDER or ZFS_TFILE, true or false
	Type IType;

	std::string Name;
	FAttrib Attributes;

	UINT32 FileSzHigh;
	UINT32 FileSzLow;

	SYSTEMTIME TimeOfCreation;
	SYSTEMTIME LastAccessTime;
	SYSTEMTIME LastWriteTime;

};

struct SItemW // Unicode verstion of struct that indicates anything found by StuffInDirectory()
{
	// ZFS_TFOLDER or ZFS_TFILE, true or false
	Type IType;

	std::wstring Name;
	FAttrib Attributes;

	UINT32 FileSzHigh;
	UINT32 FileSzLow;

	SYSTEMTIME TimeOfCreation;
	SYSTEMTIME LastAccessTime;
	SYSTEMTIME LastWriteTime;

	// Only has stuff if it was created by RecursiveStuffInDirectory()
	std::vector<SItemW> SubEntries;

	SItemW() {

	}
	SItemW(const SItemW& Copy) {
		IType = Copy.IType;
		Name = Copy.Name;
		Attributes = Copy.Attributes;

		FileSzHigh = Copy.FileSzHigh;
		FileSzLow = Copy.FileSzLow;

		TimeOfCreation = Copy.TimeOfCreation;
		LastAccessTime = Copy.LastAccessTime;
		LastWriteTime = Copy.LastWriteTime;

		SubEntries = Copy.SubEntries;
	}

};



//! Alternate name: "I don't like boost::filesystem"
// 2019 NOTE: THIS SHOULD BE STATIC 
class ZDFS
{
private:
	bool WinBoolToStdBool(const BOOL& in_bInput);
	BOOL StdBoolToWinBool(const bool& in_sbInput);
	void EvalAddItem(const WIN32_FIND_DATAA& in_FinData, std::vector<SItem>& in_Vec);
	void EvalAddItemW(const WIN32_FIND_DATAW& in_FinData, std::vector<SItemW>& in_wVec);
	

public:
	ZDFS();

	//|> Checks if a file exists in that path (ANSI)
	//|<- Returns if exists
	//|!> Note: Returns false if it's a directory.
	bool FileExists(const std::string& in_sFilePath);

	//|> Checks if a file exists in that path (Unicode)
	//|<- Returns if exists
	//|!> Note: Returns false if it's a directory.
	bool FileExists(const std::wstring& in_wsFilePath);


	//|> Checks if a folder exists in that path (ANSI)
	//|<- Returns if exists
	bool FolderExists(const std::string& in_sFolderPath);

	//|> Checks if a folder exists in that path (Unicode)
	//|<- Returns if exists
	bool FolderExists(const std::wstring& in_wsFolderPath);


	//|>  Makes a folder (ANSI)
	//|<- Returns success
	//|<- Also returns true if it already exists
	bool MakeDir(const std::string& in_sFolderName);

	// Makes a folder (Unicode)
	//|<- Returns success
	//|<- Also returns true if it already exists
	bool MakeDir(const std::wstring& in_wsFolderName);

	//|> Returns a listing of stuff in the directory specified (ANSI)
	//|!> Note: Includes files and folders
	//|!> Note: Automatically adds "\\" at the end of string
	//|-> Filter: Filter string. Default is "*" (all files)
	//|<- If it fails, returns empty vector
	std::vector<SItem> StuffInDirectory(const std::string& in_sPath,std::string Filter = ALLF);

	//|> Returns a listing of stuff in the directory specified (Unicode)
	//|!> Note: Includes files and folders
	//|!> Note: Automatically adds "\\" at the end of string
	//|-> Filter: Filter string. Default is "*" (all files)
	//|<- If it fails, returns empty vector
	std::vector<SItemW> StuffInDirectory(const std::wstring& in_sPath,std::wstring wFilter = WALLF);

	//|> Returns a listing of stuff in the directory specified (Unicode), searching recursively in all subfolders
	//|!> Note: Includes files and folders
	//|!> Note: Automatically adds "\\" at the end of string
	//|-> Filter: Filter string. Default is "*" (all files)
	//|<- If it fails, returns empty vector
	std::vector<SItemW> RecursiveStuffInDirectory(const std::wstring& in_sPath, std::wstring wFilter = WALLF);

	//|> Returns the total size of a folder structure, searching recursively
	//-> Items: The items
	UINT64 GetSize(std::vector<SItemW>& Items);

    // Takes a path from the argument and if possible, returns a full path (ANSI)
	std::string RelativeToFullPath(const std::string& in_sConvPth);

	// Takes a path from the argument and if possible, returns a full path (Unicode)
	std::wstring RelativeToFullPath(const std::wstring& in_sConvPath);

	//|:> Copies file (ANSI)
	//|-> Overwrite (false by default): If true, overwrites existing file. 
	//|<- Returns success. If Overwrite is false, also returns false if it already exists.
	bool FileCopy(const std::string& in_sOriginalFileName, const std::string& in_sNewFileName,bool Overwrite = false);
	
	//|:> Copies file (Unicode)
	//|-> Overwrite (false by default): If true, overwrites existing file. 
	//|<- Returns success. If Overwrite is false, also returns false if it already exists.
	bool FileCopy(const std::wstring& in_wsOriginalFileName, const std::wstring& in_wsNewFileName, bool Overwrite = false);

	//|> Gets the path of a known folder id
	//|!> Note: Automatically adds "\\" at the end of the returned string
	//|-> kfid: Known folder ID.
	//|-> (optional) handleIN: An access token that represents a particular user. If not passed, defaults to NULL
	//|-> (optional) flags: Flags that specify special retrieval options. If not passed, defaults to NULL
	//|<- Returns the full path of a known folder ID, including a trailing backslash. For example, "C:\Users\"
	//|<- If it fails, returns an empty string.
	std::wstring GetKnownFolderPath(KNOWNFOLDERID& kfid, const HANDLE handleIN = NULL, const DWORD& flags = NULL);
	~ZDFS();
};

