#pragma once

/*
######################################
#
#
  ____________ _ _      
 |___  /  ____(_) |     
    / /| |__   _| | ___ 
   / / |  __| | | |/ _ \
  / /__| |    | | |  __/
 /_____|_|    |_|_|\___|
                        
                        
########################################
# Description: Defines ZFile class, one meant for easy serialization and writing of binary types,
# including commonly used std containers without much problem
#
# Author: ZDisket
# Copyright (C) 2018 YOUR MOM GAY LOLOLOL
#######################################
*/

#include <fstream>
#include <string>
#include <vector>

#include "ByteArr.h"

#define ZFILE_IOVR(cla,n) ZFile& operator<<(ZFile& right,const cla& n)
#define ZFILE_OOVR(cla,n) ZFile& operator>>(ZFile& right,cla& n)
// FStream that works with bytes
typedef std::basic_fstream<BYTE,std::char_traits<BYTE>> ufstream;



namespace EZFOpenMode {
	enum Enum {
		BinaryRead = 0,
		TextRead,
		BinaryWrite,
		TextWrite
	};
}

namespace EZFEndian {
	enum Enum {
		Big = 0,
		Little
	};
}



namespace ZFUtil {
	inline EZFEndian::Enum GetSysEndianness()
	{
		const int value{ 0x01 };
		const void * address = static_cast<const void *>(&value);
		const unsigned char * least_significant_address = static_cast<const unsigned char *>(address);
		return (*least_significant_address == 0x01) ? EZFEndian::Little : EZFEndian::Big;
	}

	template <typename T>
	void SwapEndian(T& var)
	{
		static_assert(std::is_pod<T>::value, "Type must be POD type for safety");
		std::array<char, sizeof(T)> varArray;
		std::memcpy(varArray.data(), &var, sizeof(T));
		for (int i = 0; i < static_cast<int>(sizeof(var) / 2); i++)
			std::swap(varArray[sizeof(var) - 1 - i], varArray[i]);
		std::memcpy(&var, varArray.data(), sizeof(T));
	}
}

// ZFile: Class for (mostly binary) file handling.
// Cannot be copied
class ZFile
{
private:

	ZFile(const ZFile&);

	BYTE * m_pData;
	bool FileOpened;
	ufstream Stream;

	EZFOpenMode::Enum OpenMode;
	EZFEndian::Enum SysEndian;

	int EZFOpenModeToIos(const EZFOpenMode::Enum& input);

public:
	ZFile();

	ZFile(const std::string& coFName, const EZFOpenMode::Enum& coMode);

	bool Open(const std::string& in_sFileName,const EZFOpenMode::Enum& in_Mode);

	void Seek(const INT64& in_Pos);
	INT64 GetPos();

    void SeekToEnd();
	
	 INT64 GetFileLength();
	// Reads from the file
	// Please pass a pointer to this
	void Read(void* out, const INT64& count);
	// Writes to the file
	// Please pass a pointer
	void Write(void* in, const INT64& incount);

	
	// Read the entire file into a byte array
	ByteArr ReadEntireFile();
	
	// Write with template argument to not pass size.
	// Only works with regular datatypes
    template <typename Dat>
	void Write(const Dat& dta)
	{
		Stream.write((BYTE*)&dta, sizeof(dta));
		
	
	}

    void WriteLine(const std::string& inLi);

	// Read with template argument to not pass size.
   // Only works with regular datatypes
	template <typename Dat>
	void Read(Dat& dta)
	{
		Stream.read((BYTE*)&dta, sizeof(dta));


	}

	

	// Write a string
	template<typename chardat>
	void Write(const std::basic_string<chardat>& Str) {
		// Get total len in bytes.
		const size_t LenInBytes = Str.length() * sizeof(chardat);

		// Write the string length (NOT in bytes)
		Write(Str.length());
		Stream.write((BYTE*)Str.data(),LenInBytes);
		
		
	
	}

	// Read a string
	template<typename chardat>
	void Read(std::basic_string<chardat>& Str) {

		size_t StrLen = 0;
		Read(StrLen);
		chardat* dpBuffer = new chardat[StrLen];


		Stream.read((BYTE*)dpBuffer, sizeof(chardat) * StrLen);

		// For some reason (witchcraft?) our buffer has more chars in it than we actually allocated, which should be impossible.
		// Thankfully, std::string's assign function allows for cutting.
		Str.assign(dpBuffer,0,StrLen);


		delete[] dpBuffer;

	}

	// Write a vector
	template<typename vdat>
	void Write(const std::vector<vdat>& Vec) {
		// Write size in bytes then vector size.
		Write(Vec.size());

		// Write vector size.

		auto It = Vec.begin();
		
		while (It != Vec.end()) {
			(*this) << *It;
			++It;
		}
		

	
	}

	// Read a vector
	template<typename vdat>
	void Read(std::vector<vdat>& Vec) {
		size_t vSz = 0;
		Read(vSz);
		
		Vec.resize(vSz);

		size_t i = 0;

		while (i != vSz) {
			(*this) >> Vec[i];
		
			++i;
		}


	}

	template <typename N>
	void Write(const N& Num, EZFEndian::Enum TargetEndian);

	// Write some stuff
	template<typename Ty>
	void operator<<(const Ty& In) {
		Write(In);
	
	}

	// Write a Byte Array RAW into the file, without the size. Useful for exporting
	void Write(const ByteArr& BrDat);

	void operator<<(const ByteArr& BarDat) {
		if (BarDat.CoData() == NULL) {
			throw new std::invalid_argument("ZFile tried to write invalid byte array!!");
		}

		Write(BarDat.Size());
		Stream.write(BarDat.CoData(), BarDat.Size());
	
	}
	// Read to a byte array. Note: DELETES AND REPLACES THE ALREADY EXISTING CONTENTS THERE!!
	void operator>>(ByteArr& BarDat);


	template<typename MTy>
	void operator>>(MTy& mIn) {
		Read(mIn);
	}

	void Close();


	~ZFile();
};

template<typename N>
// Function to write a value with target endianness.
inline void ZFile::Write(const N & Num, EZFEndian::Enum TargetEndian)
{
	if (SysEndian == TargetEndian)
		Write(Num);
	else
		Write(ZFUtil::SwapEndian(Num));


}
