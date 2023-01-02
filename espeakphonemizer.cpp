#include "espeakphonemizer.h"
#include <espeak/speak_lib.h>


static const std::u32string Punctuation_t = U",.;¡!¿?:-";

using namespace ESP;

std::string ESpeakPhonemizer::ToPhon(const std::string &InTxt)
{
    const char* TextPtr = InTxt.c_str();
    const void** OurPtr = (const void**)&TextPtr;
    const char* Phon = espeak_TextToPhonemes(OurPtr, espeakCHARS_AUTO, (int)PhonemePars.to_ulong());


    return std::string(Phon);
}

void ESpeakPhonemizer::Initialize(const std::string &DataPath, const std::string &VoiceName)
{
    // these are irrelevant because we don't play any audio, we just use the phonemizer
    espeak_AUDIO_OUTPUT output = AUDIO_OUTPUT_SYNCH_PLAYBACK;
    int buflength = 500, options = 0;


    espeak_Initialize(output, buflength, DataPath.c_str(), options);
    espeak_SetVoiceByName(VoiceName.c_str());

    PhonemePars[1] = 1; // set IPA


}

std::string ESpeakPhonemizer::Phonemize(const std::string &Input)
{
    std::u32string In = VoxUtil::StrToU32(Input);

    // ESpeak's phonemize function stops at punctuation, so we split it up into chunks, phonemize, then put them back together
    PunctSplitVec SplitVec = IterativePunctuationSplit(In, Punctuation_t);

    std::string Assembled = "";
    bool Space = false;
    for (const auto& Spli : SplitVec)
    {
        std::string Pibber = VoxUtil::U32ToStr(Spli.second);
        if (!Spli.first)
        {
            Pibber = ToPhon(Pibber);
            if (Space)
                Assembled += " ";


        }else
        {
            Space = true;


        }
        Assembled += Pibber;


    }

    return Assembled;

}

ESpeakPhonemizer::ESpeakPhonemizer()
{

}

ESP::PunctSplitVec ESP::IterativePunctuationSplit(const std::u32string &Input, const std::u32string &Punct)
{
    PunctSplitVec Ret;

    std::u32string CuStr = U"";
    for (const auto& Ch : Input) {

        if (Punct.find(Ch) != std::u32string::npos) {
            if (CuStr.size())
                Ret.push_back({ false,CuStr });

            std::u32string PunctOnly(1,Ch);
            Ret.push_back({ true, PunctOnly });
            CuStr = U"";

        }
        else {
            CuStr += Ch;
        }


    }
    Ret.push_back({ false,CuStr });
    return Ret;

}

