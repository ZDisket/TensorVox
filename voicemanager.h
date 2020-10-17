#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H
#include "Voice.h"
#include <QString>
#include "phoneticdict.h"
#include "phonemizer.h"
class VoiceManager
{
private:
    std::vector<Voice*> Voices;
    std::vector<DictEntry> ManDict;

    std::vector<Phonemizer*> Phonemizers;

    Phonemizer* LoadPhonemizer(const QString &InPhnLang);




public:

    // Load a voice and return index in vector
    size_t LoadVoice(const QString& Voname);
    // Find a voice in Voices
    // Returns index in Voices vector, if not found returns -1
    int FindVoice(const QString& inName, bool autoload = true);

    Voice* operator[](size_t in);

    inline std::vector<Voice*>& GetVoices(){return Voices;}

    void SetDict(const std::vector<DictEntry>& InDict);


    VoiceManager();
    ~VoiceManager();
};

#endif // VOICEMANAGER_H
