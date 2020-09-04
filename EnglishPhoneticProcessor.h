#pragma once
#include "TextTokenizer.h"

// Suppress the about 300 warnings from FST and Phonetisaurus headers
#pragma warning(push, 0)
#include <include/PhonetisaurusScript.h>
#pragma warning(pop)

const std::vector<std::string> enPhonemes = { "AA","AA0","AA1","AA2","AE","AE0","AE1","AE2","AH","AH0","AH1","AH2","AO","AO0","AO1",
"AO2","AW","AW0","AW1","AW2","AY","AY0","AY1","AY2","B","CH","D","DH","EH","EH0","EH1","EH2","ER","ER0","ER1","ER2","EY","EY0","EY1",
"EY2","F","G","HH","IH","IH0","IH1","IH2","IY","IY0","IY1","IY2","JH","K","L","M","N","NG","OW","OW0","OW1","OW2","OY","OY0","OY1","OY2",
"P","R","S","SH","T","TH","UH","UH0","UH1","UH2","UW","UW0","UW1","UW2","V","W","Y","Z","ZH","SIL","END" };


class EnglishPhoneticProcessor
{
private:
	TextTokenizer Tokenizer;
	PhonetisaurusScript* Phonemizer;

	inline bool FileExists(const std::string& name) {
		ifstream f(name.c_str());
		return f.good();
	}

public:
	bool Initialize(const std::string& PhoneticModelFn);
	std::string ProcessTextPhonetic(const std::string& InText);
	EnglishPhoneticProcessor();
	EnglishPhoneticProcessor(const std::string& PhModelFn);
	~EnglishPhoneticProcessor();
};

