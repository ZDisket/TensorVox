#include "TextTokenizer.h"
#include "ext/ZCharScanner.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <algorithm>




// Punctuation, this gets auto-converted to SIL
const std::u32string punctuation_f = U",.-;";

// For Tacotron2, including question and other marks
const std::u32string punctuation_tac = U",.;¡!¿?:";



using namespace std;

void TextTokenizer::SetAllowedChars(const std::string &value)
{
    AllowedChars = VoxUtil::StrToU32(value);
}

vector<string> TextTokenizer::ExpandNumbers(const std::vector<std::string>& SpaceTokens)
{
	vector<string> RetVec;
	RetVec.reserve(SpaceTokens.size());

	for (auto& Token : SpaceTokens) {
		char* p;
        strtol(Token.c_str(), &p, 10);
		if (*p) {
			RetVec.push_back(Token);
		}
		else {
            std::string ModTk = Token;
            CuNumber->numbertext(ModTk,NumLang);

            std::replace(ModTk.begin(),ModTk.end(),'-',' ');

            RetVec.push_back(ModTk);


		}
	}

	return RetVec;

}

string TextTokenizer::SpaceChars(const string &InStr)
{
    std::u32string AsmStr = U"";
    std::u32string Stry = VoxUtil::StrToU32(InStr);


    for (size_t i = 0; i < Stry.size();i++)
    {
        auto uChar = Stry[i];
        if (punctuation_tac.find(uChar) != std::u32string::npos)
        {
            AsmStr += U" ";
            AsmStr += uChar;
            AsmStr += U" ";

        }
        else
        {
            AsmStr += uChar;

        }





    }

    return VoxUtil::U32ToStr(AsmStr);


}

TextTokenizer::TextTokenizer()
{
}

TextTokenizer::~TextTokenizer()
{
}

void TextTokenizer::SetNumberText(Numbertext &INum, const string &Lang)
{
    CuNumber = &INum;
    NumLang = Lang;

}



vector<string> TextTokenizer::Tokenize(const std::string & InTxt,ETTSLanguage::Enum Language,bool IsTacotron)
{
	vector<string> ProcessedTokens;



    std::string TxtPreProc = SpaceChars(InTxt);

    ZStringDelimiter Delim(TxtPreProc);
	Delim.AddDelimiter(" ");

    vector<string> DelimitedTokens = Delim.GetTokens();



	// Single word handler
    if (!Delim.szTokens())
        DelimitedTokens.push_back(TxtPreProc);

    DelimitedTokens = ExpandNumbers(DelimitedTokens);

    std::u32string punctuation = punctuation_f;

    if (IsTacotron)
        punctuation = punctuation_tac;




	// We know that the new vector is going to be at least this size so we reserve
	ProcessedTokens.reserve(DelimitedTokens.size());

	/*
	In this step we go through the string and only allow qualified character to pass through.
	*/
    for (size_t TokCtr = 0; TokCtr < DelimitedTokens.size();TokCtr++)
    {
        // We are now using U32string because it's guaranteed to be 1 character = 1 element
        const auto& tok = VoxUtil::StrToU32(DelimitedTokens[TokCtr]);
        std::u32string AppTok = U"";


        if (tok.find(U"@") != string::npos)
        {

            ProcessedTokens.push_back(VoxUtil::U32ToStr(tok));
            continue;

        }

		for (size_t s = 0;s < tok.size();s++)
		{


            if (AllowedChars.find(tok[s]) != std::u32string::npos)
                AppTok += tok[s];


			// Punctuation handler
            // This time we explicitly add a token to the vector
            if (punctuation.find(tok[s]) != std::u32string::npos) {
				// First, if the assembled string isn't empty, we add it in its current state
				// Otherwise, the SIL could end up appearing before the word.

                if (!AppTok.empty()) {
                    ProcessedTokens.push_back(VoxUtil::U32ToStr(AppTok));

                    AppTok = U"";
				}

                if (IsTacotron){

                    // Double at-symbol is handled later
                    AppTok += U"@@";
                    AppTok += tok[s];

                }
                else{
                    AppTok = U"@SIL";
                }

                ProcessedTokens.push_back(VoxUtil::U32ToStr(AppTok));
                AppTok = U"";
                continue;

			}






		}
        if (!AppTok.empty())
        {
            ProcessedTokens.push_back(VoxUtil::U32ToStr(AppTok));
            AppTok = U"";


        }

	}
	// Prevent out of range error if the user inputs one word
	if (ProcessedTokens.size() > 1) 
	{
		if (ProcessedTokens[ProcessedTokens.size() - 1] == "SIL")
			ProcessedTokens.pop_back();
	}


	return ProcessedTokens;
}
