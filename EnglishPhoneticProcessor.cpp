#include "EnglishPhoneticProcessor.h"
#include "VoxCommon.hpp"
const std::vector<std::string> OverrideInputs = {"the", "us","have","my","me"};
const std::vector<std::string> OverrideOutputs = {"DH AH", "AH S","HH AE V","M AY","M IY"};

using namespace std;

bool EnglishPhoneticProcessor::Initialize(const std::string & PhoneticModelFn)
{
	if (!FileExists(PhoneticModelFn))
		return false;

	Phonemizer = new PhonetisaurusScript(PhoneticModelFn);



	return true;
}

std::string EnglishPhoneticProcessor::ProcessTextPhonetic(const std::string & InText)
{
	if (!Phonemizer)
		return "ERROR";

	vector<string> Words = Tokenizer.Tokenize(InText);

	string Assemble = "";

	for (size_t w = 0; w < Words.size();w++) 
	{
		const string& Word = Words[w];
        cout << Word << std::endl;

        if (Word.find("@") != std::string::npos){
            std::string AddPh = Word.substr(1); // Remove the @
            size_t OutId = 0;
            if (VoxUtil::FindInVec(AddPh,enPhonemes,OutId))
            {
                Assemble.append(enPhonemes[OutId]);
                Assemble.append(" ");


            }

            continue;

        }



        size_t OverrideIdx = 0;
        if (VoxUtil::FindInVec<std::string>(Word,OverrideInputs,OverrideIdx))
        {
            Assemble.append(OverrideOutputs[OverrideIdx]);
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
