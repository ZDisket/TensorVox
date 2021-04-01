#pragma once
#include "TextTokenizer.h"


#include "phoneticdict.h"
#include "phonemizer.h"

class EnglishPhoneticProcessor
{
private:
	TextTokenizer Tokenizer;
    Phonemizer* Phoner;

	inline bool FileExists(const std::string& name) {
        std::ifstream f(name.c_str());
		return f.good();
	}

public:
    bool Initialize(Phonemizer *InPhn);
    std::string ProcessTextPhonetic(const std::string& InText, const std::vector<std::string> &InPhonemes, const std::vector<DictEntry>& InDict, ETTSLanguage::Enum InLanguage, bool IsTac);
	EnglishPhoneticProcessor();
    EnglishPhoneticProcessor(Phonemizer *InPhn);
	~EnglishPhoneticProcessor();
};

