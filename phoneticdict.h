#ifndef PHONETICDICT_H
#define PHONETICDICT_H
#include "ext/ZFile.h"
#include <string>
#include <QString>
struct DictEntry{
    std::string Word;
    std::string PhSpelling;
    std::string Language;
};


// Check if the base word is equal to this string
bool operator==(const DictEntry& left,const std::string& right);

ZFILE_OOVR(DictEntry,entr);

ZFILE_IOVR(DictEntry,inentr);
class PhoneticDict
{
public:
    PhoneticDict();

    void Export(const QString& exfn);
    bool Import(const QString &infn);

    std::vector<DictEntry> Entries;

private:

};

#endif // PHONETICDICT_H
