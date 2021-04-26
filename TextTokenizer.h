#pragma once
#include <vector>
#include <string>
#include "VoxCommon.hpp"
#include "Numbertext.hxx"

class TextTokenizer
{
private:
    std::u32string AllowedChars;

	std::vector<std::string> ExpandNumbers(const std::vector<std::string>& SpaceTokens);

    Numbertext* CuNumber;

    std::string NumLang;


public:
	TextTokenizer();
	~TextTokenizer();

    void SetNumberText(Numbertext& INum,const std::string& Lang);

    std::vector<std::string> Tokenize(const std::string& InTxt, ETTSLanguage::Enum Language = ETTSLanguage::English, bool IsTacotron = false);
    void SetAllowedChars(const std::string &value);
};

