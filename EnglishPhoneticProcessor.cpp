#include "EnglishPhoneticProcessor.h"
#include "VoxCommon.hpp"

using namespace std;

bool EnglishPhoneticProcessor::Initialize(Phonemizer* InPhn, ESpeakPhonemizer *InENGPh)
{


    Phoner = InPhn;
    Tokenizer.SetAllowedChars(Phoner->GetGraphemeChars());
    ENG_Phonemizer = InENGPh;




	return true;
}


std::string EnglishPhoneticProcessor::ProcessTextPhonetic(const std::string& InText, const std::vector<u32string> &InPhonemes, const std::vector<DictEntry>& InDict, ETTSLanguageType::Enum InLanguageType, bool IsTac)
{
    if (!Phoner)
		return "ERROR";



    vector<string> Words = Tokenizer.Tokenize(InText,IsTac);

	string Assemble = "";


    if (InLanguageType == ETTSLanguageType::Char)
    {
        for (size_t w = 0; w < Words.size();w++)
        {
            Assemble.append(Words[w]);

            if (w > 0)
                Assemble.append(" ");

        }

        if (Assemble[Assemble.size() - 1] == ' ')
            Assemble.pop_back();

        return Assemble;



    }

    // Make a copy of the dict passed.
    std::vector<DictEntry> CurrentDict = InDict;


	for (size_t w = 0; w < Words.size();w++) 
	{
		const string& Word = Words[w];


        if (Word.size() > 22)
            continue;


        // Double email symbol indicates Tacotron punctuation handling
        if (Word.find("@@") != std::string::npos)
        {
            std::string AddPonct = Word.substr(2); // Remove the @@
            Assemble.append(" ");
            Assemble.append(AddPonct);
            Assemble.append(" ");

            continue;


        }

        if (Word.find("@") != std::string::npos){
            std::u32string AddPh = VoxUtil::StrToU32(Word.substr(1)); // Remove the @
            size_t OutId = 0;
            if (VoxUtil::FindInVec(AddPh,InPhonemes,OutId))
            {
                Assemble.append(VoxUtil::U32ToStr(InPhonemes[OutId]));
                Assemble.append(" ");


            }

            continue;

        }




        size_t OverrideIdx = 0;
        if (VoxUtil::FindInVec2<std::string,DictEntry>(Word,InDict,OverrideIdx))
        {
             Assemble.append(InDict[OverrideIdx].PhSpelling);
             Assemble.append(" ");
             continue;
        }



        std::string Res = Word;
        if (!ENG_Phonemizer)
            Res = Phoner->ProcessWord(Word,0.001f);

        // Cache the word in the override dict so next time we don't have to research it
        CurrentDict.push_back({Word,Res,""});

        Assemble.append(Res);
        Assemble.append(" ");





	}
	

    // eSpeak phonemizer takes in whole thing
    if (ENG_Phonemizer)
        Assemble = ENG_Phonemizer->Phonemize(Assemble);

    // Delete last space if there is
	if (Assemble[Assemble.size() - 1] == ' ')
		Assemble.pop_back();




	return Assemble;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor()
{
    Phoner = nullptr;
    ENG_Phonemizer = nullptr;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor(Phonemizer *InPhn, ESpeakPhonemizer *InENGPh)
{
    Initialize(InPhn,InENGPh);

}



EnglishPhoneticProcessor::~EnglishPhoneticProcessor()
{
    // Causes annoying crash on exit. It's also irrelevant because the OS frees what little memory this had.
    /*
    if (Phoner)
       delete Phoner;

   */
}
