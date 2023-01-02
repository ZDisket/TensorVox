#ifndef ESPEAKPHONEMIZER_H
#define ESPEAKPHONEMIZER_H

/*

  ESpeakPhonemizer: Tool for IPA Text2Phon using ESpeak NG as backend.

*/
#include <iostream>
#include <string>
#include <bitset>
#include "VoxCommon.hpp"
#include <vector>

namespace ESP{
typedef std::pair<bool, std::u32string> PunctSplit;
typedef std::vector<PunctSplit> PunctSplitVec;


// Returns vector<pair<IS_PUNCTUATION,String>>
PunctSplitVec IterativePunctuationSplit(const std::u32string& Input, const std::u32string& Punct);

}

class ESpeakPhonemizer
{
private:
    static std::bitset<sizeof(int) * 8> PhonemePars;

public:

    // DataPath: Path to ESpeak NG data dir
    // VoiceName: Name of voice to use for phonemizing (like "Spanish (Latin America)")
    void Initialize(const std::string& DataPath,const std::string& VoiceName);


    // Phonemize text using ESpeak phonemizer
    // Unlike regular phonemizer, feed complete texts at once instead of just words.
    std::string Phonemize(const std::string& Input);

    ESpeakPhonemizer();
};

#endif // ESPEAKPHONEMIZER_H
