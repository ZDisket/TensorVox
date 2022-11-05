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


    // Go through the string and add spaces before and after punctuation.
    // This is because ExpandNumbers won't recognize numbers if they've got punctuation like 500, or .9000
    std::string SpaceChars(const std::string& InStr);



public:
	TextTokenizer();
	~TextTokenizer();

    void SetNumberText(Numbertext& INum,const std::string& Lang);

    std::vector<std::string> Tokenize(const std::string& InTxt, bool IsTacotron = false, bool IsTorchMoji = false);
    void SetAllowedChars(const std::string &value);
};

