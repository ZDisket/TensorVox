#include "ByteArr.h"
using namespace std;

void ByteArr::Realloc(const size_t & newSize)
{
	if (newSize < DataSz)
		return;

	BYTE* NewDat = new BYTE[newSize];





    smemcpy(NewDat, newSize, Data, DataSz);

	DataSz = newSize;

	delete[] Data;

	Data = NewDat;


}

void ByteArr::Init()
{
    Data = nullptr;
	DataSz = 0;
	CurrentPos = 0;
    DontDestroy = false;
}

ByteArr::ByteArr()
{
    Init();
}

void ByteArr::SetDestroy(const bool &set)
{
    DontDestroy = !set;
}

ByteArr::ByteArr(const size_t & InitSz)
{
    Init();
    CAlloc(InitSz);
}

ByteArr::ByteArr(BYTE * CopyArr, const size_t & ArrSz)
{
    Init();
    Assign(CopyArr, ArrSz);

}

ByteArr::ByteArr(const ByteArr & Cpy)
{
    Init();
    Assign(Cpy);
    CurrentPos = Cpy.Pos();
}

ByteArr::ByteArr(const std::vector<BYTE>& CpyBv)
{
    Init();
    Assign(CpyBv);
}

#ifdef _QT
ByteArr::ByteArr(const QByteArray &InitBar)
{
    Init();
    Assign(InitBar);
}
#endif
ByteArr::ByteArr(const std::vector<ByteArr> &BarC)
{
    Init();
    Assign(BarC);
}

std::vector<ByteArr> ByteArr::Split(const ulong &szportion)
{
    vector<ByteArr> Ret;

    size_t Pos = 0;

    size_t remaining = DataSz;
    while (remaining > 0)
    {
        size_t targetsz = szportion;

        if (Pos + szportion > DataSz)
            targetsz = DataSz - Pos;

        ByteArr Bar(Data + Pos,targetsz);
        Pos += targetsz;

        Ret.push_back(Bar);

        remaining -= targetsz;
    }




    return Ret;
}

void ByteArr::Request(const size_t &reqSz)
{
    const size_t oReq = CurrentPos + reqSz;

    if (oReq > DataSz)
        IncreaseSize(reqSz);

}

std::vector<BYTE> ByteArr::ToVector()
{
    return std::vector<BYTE>(Data, Data + DataSz);

}

const BYTE * ByteArr::CoData() const
{
    return Data;
}

BYTE &ByteArr::operator[](const size_t &Pos)
{
    return Data[Pos];

}

const BYTE &ByteArr::operator[](const size_t &cPos) const
{
    return Data[cPos];

}

void ByteArr::Advance(const size_t &adv)
{
    CurrentPos += adv;

}


void ByteArr::Assign(BYTE * cpyArr, const size_t & cpySz)
{
   CAlloc(cpySz);

    smemcpy(Data, cpySz, cpyArr, cpySz);
	DataSz = cpySz;

}

void ByteArr::Assign(const std::vector<BYTE>& CByteVec)
{
    CAlloc(CByteVec.size());

    smemcpy(Data, DataSz, CByteVec.data(), CByteVec.size());

}

void ByteArr::Assign(const ByteArr & CpyByte)
{

    CAlloc(CpyByte.Size());

    smemcpy(Data, DataSz, CpyByte.CoData(), CpyByte.Size());



}

void ByteArr::Assign(const std::vector<ByteArr> &BarComb)
{

    size_t total = 0;
    // Two iterations, one gets the size and the other appends.
    auto It = BarComb.begin();
    while (It != BarComb.end())
    {

        total += It->Size();

        ++It;
    }


    CAlloc(total);
    It = BarComb.begin();

    while (It != BarComb.end()){

        Add((void*)It->CoData(),It->Size());



        ++It;
    }


}

void ByteArr::Seek(const size_t &To)
{
    if (To > DataSz)
        throw std::invalid_argument("Tried to seek out of bounds!");

    CurrentPos = To;

}

void ByteArr::Add(void * inDat, const size_t & DatSz)
{
	const size_t Req = CurrentPos + DatSz;

	if (Req > DataSz)
        IncreaseSize(DatSz);

    smemcpy(Data + CurrentPos, DataSz, inDat, DatSz);

    CurrentPos += DatSz;
}

size_t ByteArr::Read(void *OutDat, const size_t &oDatSz)
{
    const size_t oReq = CurrentPos + oDatSz;

    if (oReq > DataSz)
        throw std::invalid_argument("Tried to read out of bounds!");

    smemcpy(OutDat,oDatSz,Data + CurrentPos,oDatSz);

    CurrentPos = oReq;

    return CurrentPos;

}

void ByteArr::CAlloc(const size_t & SetSize)
{
    if (Data && DataSz)
		delete[] Data;

	CurrentPos = 0;

	Data = new BYTE[SetSize];

	DataSz = SetSize;

}

void ByteArr::operator>>(ByteArr &BaEx)
{
    // We explicitly export and import sizes in unsigned 64 bits to make sure
    // there are no compatibility problems between 32 and 64 bit architectures

    // Get the size
    UINT64 tmpSize = 0;
    (*this) >> tmpSize;

    // Request the size from the other byte array

    BaEx.Request((size_t)tmpSize);

    // Perform a copy directly onto the other array
    smemcpy(BaEx.Data + BaEx.Pos(),BaEx.Size(),Data + CurrentPos,(size_t)tmpSize);


    // Advance those positions
    BaEx.Advance(tmpSize);
    CurrentPos += tmpSize;



}

void ByteArr::operator<<(const ByteArr &BaAdd)
{

    // We explicitly export and import sizes in unsigned 64 bits to make sure
    // there are no compatibility problems between 32 and 64 bit architectures


    (*this) << (UINT64)BaAdd.Size();
    Add(BaAdd.Data,BaAdd.Size());





}

// QT Functions ##########################################################
#ifdef _QT


void ByteArr::Assign(const QByteArray &QBar)
{
    CAlloc((size_t)QBar.size());

    smemcpy(Data,DataSz,QBar.data(),(size_t)QBar.size());

}

QByteArray ByteArr::ToQByteArr()
{
    QByteArray QB((const char*)Data,(int)DataSz);
    return QB;

}
#endif
#ifdef USE_ZDFS
void ByteArr::operator<<(const SItemW &ItemEx)
{
    // Write our attributes
    (*this) << ItemEx.Attributes;

    // Write basic data


    (*this) << ItemEx.FileSzHigh;
    (*this) << ItemEx.FileSzLow;
    (*this) << ItemEx.IType;

    (*this) << ItemEx.LastAccessTime;
    (*this) << ItemEx.LastWriteTime;

    (*this) << ItemEx.Name;
    (*this) << ItemEx.TimeOfCreation;

    // Write subentries
    (*this) << ItemEx.SubEntries;
}

void ByteArr::operator<<(const SYSTEMTIME &SysTime)
{
    // Convert it to file time to export easier;
    FILETIME TimeC;
    SystemTimeToFileTime(&SysTime, &TimeC);

    (*this) << TimeC.dwHighDateTime;
    (*this) << TimeC.dwLowDateTime;

}

void ByteArr::operator<<(const FAttrib &Atr)
{
    (*this) << Atr.Archive;
        (*this) << Atr.Compressed;
        (*this) << Atr.Hidden;
        (*this) << Atr.Normal;
        (*this) << Atr.ReadOnly;
        (*this) << Atr.System;
        (*this) << Atr.Temporary;


}

void ByteArr::operator>>(SItemW &ItemEx)
{
    // Write our attributes
    (*this) >> ItemEx.Attributes;

    // Write basic data


    (*this) >> ItemEx.FileSzHigh;
    (*this) >> ItemEx.FileSzLow;
    (*this) >> ItemEx.IType;

    (*this) >> ItemEx.LastAccessTime;
    (*this) >> ItemEx.LastWriteTime;

    (*this) >> ItemEx.Name;
    (*this) >> ItemEx.TimeOfCreation;

    // Write subentries
    (*this) >> ItemEx.SubEntries;

}

void ByteArr::operator>>(SYSTEMTIME &SysTime)
{

        FILETIME TimeC;

        (*this) >> TimeC.dwHighDateTime;
        (*this) >> TimeC.dwLowDateTime;

        FileTimeToSystemTime(&TimeC, &SysTime);
}

void ByteArr::operator>>(FAttrib &ExAtr)
{
    (*this) >> ExAtr.Archive;
        (*this) >> ExAtr.Compressed;
        (*this) >> ExAtr.Hidden;
        (*this) >> ExAtr.Normal;
        (*this) >> ExAtr.ReadOnly;
        (*this) >> ExAtr.System;
        (*this) >> ExAtr.Temporary;

}
#endif
#ifdef _QT
void ByteArr::operator<<(const QByteArray &QBarEx)
{
    ByteArr Temp;
    Temp.Assign(QBarEx);

    (*this) << Temp;

}

void ByteArr::operator>>(QByteArray &QBarry)
{

    ByteArr Temp1;
    (*this) >> Temp1;

   QBarry.append(Temp1.ToQByteArr());


}

#endif
// QT Functions ##########################################################


ByteArr::~ByteArr()
{
	try {
        if (Data && DataSz && !DontDestroy)
			delete[] Data;
	
	}
	catch (...) {
	// Who the hell gives a shit about exceptions here???
	
	}

}

void smemcpy(void* dest,const size_t& destsz, const void* src,const size_t& count ){

    if (count > destsz)
        throw std::invalid_argument("memcpy_s, destionation size is lower than the source!!");

    memcpy(dest,src,count);


}
