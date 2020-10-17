#ifndef PHONEMIZER_H
#define PHONEMIZER_H
#include "tfg2p.h"
#include <tuple>
#include <set>
#include <algorithm>

struct IdStr{
    int32_t ID;
    std::string STR;
};


struct StrStr{
    std::string Word;
    std::string Phn;
};



class Phonemizer
{
private:
    TFG2P G2pModel;

    std::vector<IdStr> CharId;
    std::vector<IdStr> PhnId;

    std::vector<IdStr> GetDelimitedFile(const std::string& InFname);


    // Sorry, can't use set, unordered_map or any other types. (I tried)
    std::vector<StrStr> Dictionary;

    void LoadDictionary(const std::string& InDictFn);

    std::string DictLookup(const std::string& InWord);


    std::string PhnLanguage;
public:
    Phonemizer();
    bool Initialize(const std::string InPath);
    std::string ProcessWord(const std::string& InWord, float Temperature = 0.1f);
    std::string GetPhnLanguage() const;
    void SetPhnLanguage(const std::string &value);
};


#endif // PHONEMIZER_H
