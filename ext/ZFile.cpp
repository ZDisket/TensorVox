#include "ZFile.h"

using namespace std;
int ZFile::EZFOpenModeToIos(const EZFOpenMode::Enum & input)
{
	/*
	hehe wall of ifs
	yanderedev amirite???
	*/
	if (input == EZFOpenMode::BinaryRead)
		return ios::in | ios::binary;
	else if (input == EZFOpenMode::BinaryWrite)
		return ios::out | ios::binary;
	else if (input == EZFOpenMode::TextRead)
		return ios::in;
	else if (input == EZFOpenMode::TextWrite)
		return ios::out;

	SysEndian = ZFUtil::GetSysEndianness();

	return ios::in | ios::binary;

}

ZFile::ZFile(const std::string & coFName, const EZFOpenMode::Enum & coMode)
{
	Open(coFName, coMode);
}

bool ZFile::Open(const std::string & in_sFileName, const EZFOpenMode::Enum & in_Mode)
{
	OpenMode = in_Mode;

    Stream.open(in_sFileName,(ios_base::openmode)EZFOpenModeToIos(in_Mode));
	return Stream.good();

}



void ZFile::Seek(const INT64 & in_Pos)
{
	if (OpenMode == EZFOpenMode::BinaryRead || OpenMode == EZFOpenMode::TextRead)
		Stream.seekg(in_Pos, ios::beg);
	else if (OpenMode == EZFOpenMode::BinaryWrite || OpenMode == EZFOpenMode::TextWrite)
		Stream.seekp(in_Pos, ios::beg);
}

INT64 ZFile::GetPos()
{
	if (OpenMode == EZFOpenMode::BinaryRead || OpenMode == EZFOpenMode::TextRead)
		return Stream.tellg();
	else if (OpenMode == EZFOpenMode::BinaryWrite || OpenMode == EZFOpenMode::TextWrite)
		return Stream.tellp();

	// NO TYPE?????????????
    return -1;
}

void ZFile::SeekToEnd()
{
    if (OpenMode == EZFOpenMode::BinaryRead || OpenMode == EZFOpenMode::TextRead)
        Stream.seekg(0, Stream.end);
    else if (OpenMode == EZFOpenMode::BinaryWrite || OpenMode == EZFOpenMode::TextWrite)
        Stream.seekp(0, Stream.end);
}

INT64 ZFile::GetFileLength()
{
	std::streampos lpos = GetPos();

	if (OpenMode == EZFOpenMode::BinaryRead || OpenMode == EZFOpenMode::TextRead)
		Stream.seekg(0, Stream.end);
	else if (OpenMode == EZFOpenMode::BinaryWrite || OpenMode == EZFOpenMode::TextWrite)
		Stream.seekp(0, Stream.end);

	const INT64 Len = GetPos();
	Seek(lpos);

	return Len;

}

void ZFile::Read(void * out, const INT64 & count)
{
	Stream.read((BYTE*)out, count);

}

void ZFile::Write(void * in, const INT64 & incount)
{
	Stream.write((BYTE*)in, incount);
	
}

ByteArr ZFile::ReadEntireFile()
{

	ByteArr ArrRet;

	Stream.seekg(0, Stream.end);
	INT64 length = Stream.tellg();
	Stream.seekg(0, Stream.beg);
	ArrRet.CAlloc(length);

	Stream.read(ArrRet.GetData(), length);
	
	return ArrRet;

}

void ZFile::WriteLine(const string &inLi)
{
    std::string Line = inLi + "\n";

    Write((void*)Line.data(),Line.size() * sizeof(char));

}

void ZFile::Write(const ByteArr & BrDat)
{
	Stream.write(BrDat.CoData(), BrDat.Size());
}

void ZFile::Close()
{
	Stream.close();
}

void ZFile::operator>>(ByteArr& BarDat) {
	size_t BaSz = 0;
	Read(BaSz);
	BarDat.CAlloc(BaSz);
	Stream.read(BarDat.GetData(), BaSz);

}


ZFile::ZFile()
{
}


ZFile::~ZFile()
{
}
