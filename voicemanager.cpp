#include "voicemanager.h"
#define SAFE_DELETE(pdel)if (pdel){delete pdel;}
#include <QCoreApplication>

Phonemizer* VoiceManager::LoadPhonemizer(const QString& InPhnLang,int32_t InLangNum)
{

    for (Phonemizer*& Phn : Phonemizers)
    {
       if (Phn->GetPhnLanguage() == InPhnLang.toStdString())
           return Phn;


    }


    Phonemizer* CreatePhn = new Phonemizer;

    // Initialize regularly or minimally
    CreatePhn->Initialize(QString(QCoreApplication::applicationDirPath() + "/g2p/" + InPhnLang).toStdString(),
                          InLangNum == ETTSLanguageType::Char);

    CreatePhn->SetPhnLanguage(InPhnLang.toStdString());


    Phonemizers.push_back(CreatePhn);

    return Phonemizers[Phonemizers.size() - 1];


}

size_t VoiceManager::LoadVoice(const QString &Voname)
{
    Voice* NuVoice = new Voice(QString(QCoreApplication::applicationDirPath() + "/models/" + Voname).toStdString(),Voname.toStdString(),nullptr);

    QString PLang = QString::fromStdString(NuVoice->GetInfo().s_Language_Fullname);
    NuVoice->AddPhonemizer(LoadPhonemizer(PLang,NuVoice->GetInfo().LangType));

    std::string NumTxtPath = QString(QCoreApplication::applicationDirPath() + "/num2txt/" +
                                     QString::fromStdString(NuVoice->GetInfo().s_Language) + ".sor").toStdString();

    NuVoice->LoadNumberText(NumTxtPath);

    Voices.push_back(NuVoice);
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

    for (Phonemizer* Phni : Phonemizers)
    {
        SAFE_DELETE(Phni)


    }
    for (Voice* Vo : Voices)
    {

        SAFE_DELETE(Vo)

    }

    Voices.clear();
    Phonemizers.clear();



}
