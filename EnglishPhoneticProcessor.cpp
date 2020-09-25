#include "EnglishPhoneticProcessor.h"
#include "VoxCommon.hpp"

using namespace std;

bool EnglishPhoneticProcessor::Initialize(const std::string & PhoneticModelFn)
{
	if (!FileExists(PhoneticModelFn))
		return false;

	Phonemizer = new PhonetisaurusScript(PhoneticModelFn);



	return true;
}

std::string EnglishPhoneticProcessor::ProcessTextPhonetic(const std::string& InText, const std::vector<string> &InPhonemes,const std::vector<DictEntry>& InDict,ETTSLanguage::Enum InLanguage)
{
	if (!Phonemizer)
		return "ERROR";

    vector<string> Words = Tokenizer.Tokenize(InText,InLanguage);

	string Assemble = "";

	for (size_t w = 0; w < Words.size();w++) 
	{
		const string& Word = Words[w];

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


		vector<PathData> PhResults = Phonemizer->Phoneticize(Word, 1, 10000, 99.f, false, false, 0.99);
		for (const auto& padat : PhResults) {
			for (const auto& uni : padat.Uniques) {
				Assemble.append(Phonemizer->osyms_->Find(uni));
				Assemble.append(" ");
			}


		}




	}
	

	// Delete last space if there is


	if (Assemble[Assemble.size() - 1] == ' ')
		Assemble.pop_back();


	return Assemble;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor()
{
	Phonemizer = nullptr;
}

EnglishPhoneticProcessor::EnglishPhoneticProcessor(const std::string & PhModelFn)
{
	Phonemizer = nullptr;
	Initialize(PhModelFn);
}


EnglishPhoneticProcessor::~EnglishPhoneticProcessor()
{
	if (Phonemizer)
		delete Phonemizer;
}
