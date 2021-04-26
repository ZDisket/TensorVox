#include "EnglishPhoneticProcessor.h"
#include "VoxCommon.hpp"

using namespace std;

bool EnglishPhoneticProcessor::Initialize(Phonemizer* InPhn)
{


    Phoner = InPhn;
    Tokenizer.SetAllowedChars(Phoner->GetGraphemeChars());
    Tokenizer.SetNumberText(Phoner->GetNumTxt(),Phoner->GetNumTxtLang());



	return true;
}


std::string EnglishPhoneticProcessor::ProcessTextPhonetic(const std::string& InText, const std::vector<string> &InPhonemes, const std::vector<DictEntry>& InDict, ETTSLanguage::Enum InLanguage, bool IsTac)
{
    if (!Phoner)
		return "ERROR";



    vector<string> Words = Tokenizer.Tokenize(InText,InLanguage,IsTac);

	string Assemble = "";
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
            std::string AddPh = Word.substr(1); // Remove the @
            size_t OutId = 0;
            if (VoxUtil::FindInVec(AddPh,InPhonemes,OutId))
            {
                Assemble.append(InPhonemes[OutId]);
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


        std::string Res = Phoner->ProcessWord(Word,0.001f);

        // Cache the word in the override dict so next time we don't have to research it
        CurrentDict.push_back({Word,Res});

        Assemble.append(Res);
        Assemble.append(" ");





	}
	

	// Delete last space if there is


	if (Assemble[Assemble.size() - 1] == ' ')
		Assemble.pop_back();



	return Assemble;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor()
{
    Phoner = nullptr;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor(Phonemizer *InPhn)
{
    Initialize(InPhn);

}



EnglishPhoneticProcessor::~EnglishPhoneticProcessor()
{
    // Causes annoying crash on exit. It's also irrelevant because the OS frees what little memory this had.
    /*
    if (Phoner)
       delete Phoner;

   */
}
