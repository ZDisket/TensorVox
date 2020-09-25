#include "voicemanager.h"
#define SAFE_DELETE(pdel)if (pdel){delete pdel;}
#include <QCoreApplication>

size_t VoiceManager::LoadVoice(const QString &Voname)
{
    Voices.push_back(new Voice(QString(QCoreApplication::applicationDirPath() + "/models/" + Voname).toStdString(),Voname.toStdString()));
    Voices[Voices.size() - 1]->SetDictEntries(ManDict);
    return Voices.size() - 1;
}

int VoiceManager::FindVoice(const QString &inName, bool autoload)
{
    for (size_t i = 0; i < Voices.size();i++)
    {
        if (Voices[i]->Name == inName.toStdString())
            return (int)i;




    }

    if (autoload)
        return (int)LoadVoice(inName);
    else
        return -1;


}

Voice *VoiceManager::operator[](size_t in)
{

    return Voices[in];

}

void VoiceManager::SetDict(const std::vector<DictEntry> &InDict)
{
    ManDict = InDict;

}

VoiceManager::VoiceManager()
{

}

VoiceManager::~VoiceManager()
{
    for (Voice* Vo : Voices)
    {

        SAFE_DELETE(Vo)

    }

    Voices.clear();

}
