#pragma once
#include "TextTokenizer.h"


#include "phoneticdict.h"
#include "phonemizer.h"
#include "espeakphonemizer.h"

class EnglishPhoneticProcessor
{
private:
	TextTokenizer Tokenizer;
    Phonemizer* Phoner;

    ESpeakPhonemizer* ENG_Phonemizer;

	inline bool FileExists(const std::string& name) {
        std::ifstream f(name.c_str());
		return f.good();
	}

public:
    bool Initialize(Phonemizer *InPhn,ESpeakPhonemizer* InENGPh = nullptr);
    std::string ProcessTextPhonetic(const std::string& InText, const std::vector<std::u32string> &InPhonemes, const std::vector<DictEntry>& InDict, ETTSLanguageType::Enum InLanguageType, bool IsTac);
	EnglishPhoneticProcessor();
    EnglishPhoneticProcessor(Phonemizer *InPhn,ESpeakPhonemizer* InENGPh = nullptr);
	~EnglishPhoneticProcessor();

    inline TextTokenizer& GetTokenizer() {return Tokenizer;}
};

