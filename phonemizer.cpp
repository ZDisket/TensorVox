#include "phonemizer.h"
#include <fstream>
#include "ext/ZCharScanner.h"

#include <QString>
int32_t GetID(const std::vector<IdStr>& In, const std::string &InStr)
{
    for (const IdStr& It : In)
        if (It.STR == InStr)
            return It.ID;

    return -1;
}

std::string GetSTR(const std::vector<IdStr>& In, int32_t InID)
{
    for (const IdStr& It : In)
        if (It.ID == InID)
            return It.STR;

    return "";

}



std::vector<IdStr> Phonemizer::GetDelimitedFile(const std::string &InFname)
{


    std::ifstream InFile (InFname);

    int32_t CuID;
    std::string Tok;
    std::vector<IdStr> RetVec;


    std::string Line;
    while (std::getline(InFile, Line)) {

        if (Line.find("\t") == std::string::npos)
            continue;


        ZStringDelimiter Deline(Line);
        Deline.AddDelimiter("\t");

        CuID = stoi(Deline[1]);
        Tok = Deline[0];


        RetVec.push_back(IdStr{CuID,Tok});

    }

    return RetVec;


}

void Phonemizer::LoadDictionary(const std::string &InDictFn)
{

    std::ifstream InFile (InDictFn);

    std::string Word;
    std::string Phn;


    if (MapDict.size())
        MapDict.clear();


    std::string Line;
    while (std::getline(InFile, Line)) {

        if (Line.find("\t") == std::string::npos)
            continue;


        ZStringDelimiter Deline(Line);
        Deline.AddDelimiter("\t");

        Word = Deline[0];
        Phn = Deline[1];

        MapDict.insert({Word,Phn});


    }


}

std::string Phonemizer::DictLookup(const std::string &InWord)
{
    auto It = MapDict.find(InWord);

    if (It == MapDict.end())
        return "";

    return It->second;


}
// To remove from the string before dicting
const std::u32string StripPonct = U",.;!?";


std::string Phonemizer::CleanWord(const std::string &InW)
{
    // U32string = guaranteed 1 char = 1 value
    std::u32string Word = VoxUtil::StrToU32(InW);


    std::u32string RetWord;
    RetWord.reserve(Word.size());

    for (auto Ch : Word){
        if (StripPonct.find(Ch) == std::u32string::npos)
            RetWord.push_back(Ch);
    }

    return VoxUtil::U32ToStr(RetWord);
}


Phonemizer::Phonemizer()
{
    IsMinimal = true;

}

bool Phonemizer::Initialize(const std::string InPath, bool Minimal)
{
    IsMinimal = Minimal;



    // Load char indices
    CharId = GetDelimitedFile(InPath + "/char2id.txt");

    // If we're doing minimal loading then stop here
    if (IsMinimal)
        return true;


    PhnId = GetDelimitedFile(InPath + "/phn2id.txt");

    // Load model
    G2pModel.Initialize(InPath + "/model");

    LoadDictionary(InPath + "/dict.txt");




    IsMinimal = false;
    return true;


}




std::string Phonemizer::ProcessWord(const std::string &InWord,float Temperature)
{
    if (IsMinimal)
        return InWord;


    // First we try dictionary lookup
    // This is because the g2p model can be unreliable, we only want to use it for novel sentences

    std::string PhnDict = DictLookup(CleanWord(InWord));
    if (!PhnDict.empty())
        return PhnDict;

    std::vector<int32_t> InIndexes;
    std::u32string IterStr = VoxUtil::StrToU32(InWord);

    InIndexes.reserve(IterStr.size());


    // Turn word into indices
    for (const char32_t ch : IterStr)
    {
        std::u32string Single(1,ch);
        int32_t Idx = GetID(CharId,VoxUtil::U32ToStr(Single));

        if (Idx != -1)
            InIndexes.push_back(Idx);


    }

    TFTensor<int32_t> PhnPrediction = G2pModel.DoInference(InIndexes,Temperature);


    std::string RetStr = "";
    bool FirstIter = true;

    for (int32_t PhnIdx : PhnPrediction.Data)
    {
        std::string PhnTxt = GetSTR(PhnId,PhnIdx);
        if (!PhnTxt.empty())
        {
            if (!FirstIter)
                RetStr.append(" ");

            RetStr.append(PhnTxt);

        }

        FirstIter = false;
    }



    return  RetStr;

}

std::string Phonemizer::GetPhnLanguage() const
{
    return PhnLanguage;
}

void Phonemizer::SetPhnLanguage(const std::string &value)
{

    PhnLanguage = value;
}

std::string Phonemizer::GetGraphemeChars()
{

    std::string RetAllowed = "";
    for (const IdStr& Idx : CharId)
        RetAllowed.append(Idx.STR);

    return RetAllowed;

}

Phonemizer::~Phonemizer()
{

}




bool operator<(const StrStr &right, const StrStr &left)
{
  return right.Word.length() < left.Word.length();
}
