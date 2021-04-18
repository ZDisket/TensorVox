#include "phoneticdict.h"
#include "ext/ZFile.h"

ZFILE_IOVR(DictEntry,inentr){
    right << inentr.Word;
    right << inentr.PhSpelling;
    right << inentr.Language;
    return right;
}

ZFILE_OOVR(DictEntry,entr){
    right >> entr.Word;
    right >> entr.PhSpelling;
    right >> entr.Language;
    return right;

}
PhoneticDict::PhoneticDict()
{

}

void PhoneticDict::Export(const QString &exfn)
{
    ZFile ofi;
    ofi.Open(exfn.toStdString(),EZFOpenMode::BinaryWrite);

    ofi << Entries;
    ofi.Close();


}

bool PhoneticDict::Import(const QString &infn)
{
    ZFile fi;
    if (!fi.Open(infn.toStdString(),EZFOpenMode::BinaryRead))
        return false;


    if (fi.GetFileLength() == 0){
        fi.Close();
        return true;

    }

    fi >> Entries;

    fi.Close();



    return true;



}


bool operator==(const DictEntry &left, const std::string &right)
{
    return left.Word == right;


}
