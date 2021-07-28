#ifndef PHONEMIZER_H
#define PHONEMIZER_H
#include "tfg2p.h"
#include <tuple>
#include <set>
#include <algorithm>
#include <Numbertext.hxx>

struct IdStr{
    int32_t ID;
    std::string STR;
};


struct StrStr{
    std::string Word;
    std::string Phn;
};

// Length, start index in vec
typedef std::pair<size_t,size_t> VBucket;

class Phonemizer
{
private:
    TFG2P G2pModel;

    std::vector<IdStr> CharId;
    std::vector<IdStr> PhnId;

    std::vector<VBucket> DictBuckets;

    Numbertext NumTxt;
    std::string NumTxtLang;

    bool IsMinimal;





    std::vector<IdStr> GetDelimitedFile(const std::string& InFname);


    // Sorry, can't use set, unordered_map or any other types. (I tried)
    std::vector<StrStr> Dictionary;

    void LoadDictionary(const std::string& InDictFn);

    std::string DictLookup(const std::string& InWord);

    size_t GetBucketIndex(size_t InSize);


    std::string PhnLanguage;
public:
    int32_t PhnLangID;
public:
    Phonemizer();
    /*
     * Initialize a phonemizer
     * Expects: (if Minimal == false)
     * - Two files consisting in TOKEN \t ID:
     * -- char2id.txt: Translation from input character to ID the model can accept
     * -- phn2id.txt: Translation from output ID from the model to phoneme
     * - A model/ folder where a G2P-Tensorflow model was saved as SavedModel
     * - dict.txt: Phonetic dictionary. First it searches the word there and if it can't be found then it uses the model.
     * - (two-char name).sor: Name of num2text.
     *
     * If Minimal == true, it only requires the .sor and char2id (for determining allowed graphemes only,
     * the IDs can be arbitrary in this case)
     * A Minimal phonemizer only serves to hold values useful to the processor and tokenizer, for char-based models.

    */
    bool Initialize(const std::string InPath, const std::string& NLangName, bool Minimal);


    std::string ProcessWord(const std::string& InWord, float Temperature = 0.1f);
    std::string GetPhnLanguage() const;
    void SetPhnLanguage(const std::string &value);

    std::string GetGraphemeChars();

    inline Numbertext& GetNumTxt() {return NumTxt;}

    ~Phonemizer();

    inline const std::string& GetNumTxtLang() {return NumTxtLang;}
};


bool operator<(const StrStr& right,const StrStr& left);
#endif // PHONEMIZER_H
