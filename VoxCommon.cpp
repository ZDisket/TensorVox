#include "VoxCommon.hpp"
#include "ext/json.hpp"
using namespace nlohmann;
#include <codecvt>
#include <locale>         // std::wstring_convert

const std::vector<std::string> Text2MelNames = {"FastSpeech2","Tacotron2"};
const std::vector<std::string> VocoderNames = {"Multi-Band MelGAN","MelGAN-STFT"};
const std::vector<std::string> RepoNames = {"TensorflowTTS","Mozilla/TTS"};

const std::vector<std::string> LanguageNames = {"English","Spanish", "German"};
const std::vector<std::string> LangaugeNamesNumToWords = {"en", "es","de"};


void VoxUtil::ExportWAV(const std::string & Filename, const std::vector<float>& Data, unsigned SampleRate) {
	AudioFile<float>::AudioBuffer Buffer;
	Buffer.resize(1);


	Buffer[0] = Data;
	size_t BufSz = Data.size();


	AudioFile<float> File;

    File.setAudioBuffer(Buffer);
	File.setAudioBufferSize(1, (int)BufSz);
	File.setNumSamplesPerChannel((int)BufSz);
	File.setNumChannels(1);
	File.setBitDepth(32);
	File.setSampleRate(SampleRate);

	File.save(Filename, AudioFileFormat::Wave);



}

// Process language value for vector indexes. Language value must adhere to standard.
uint32_t ProcessLanguageValue(int32_t LangVal)
{
    if (LangVal == -1)
        return 0;

    if (LangVal < -1)
        return LangVal * -1;

    return LangVal;

}

VoiceInfo VoxUtil::ReadModelJSON(const std::string &InfoFilename)
{
    const size_t MaxNoteSize = 80;


    std::ifstream JFile(InfoFilename);
    json JS;


    try {
        JFile >> JS;
    } catch(json::parse_error Err) {
        QMessageBox::critical(nullptr,"JSON parse error",QString::fromUtf8(Err.what()));
    }


    JFile.close();

    auto Arch = JS["architecture"];

    ArchitectureInfo CuArch;
    CuArch.Repo = Arch["repo"].get<int>();
    CuArch.Text2Mel = Arch["text2mel"].get<int>();
    CuArch.Vocoder = Arch["vocoder"].get<int>();

    // Now fill the strings
    CuArch.s_Repo = RepoNames[CuArch.Repo];
    CuArch.s_Text2Mel = Text2MelNames[CuArch.Text2Mel];
    CuArch.s_Vocoder = VocoderNames[CuArch.Vocoder];

    // Language value for the info
    int32_t RawLang = JS["language"].get<int32_t>();


    // Language value for the vectors
    int32_t LanguageValue = ProcessLanguageValue(RawLang);



    // If the voice is char then the pad value must be a string of the EOS token ID (like "148").
    std::string EndToken = JS["pad"].get<std::string>();

    // If it's phonetic then it's the token str, like "@EOS"
    if (RawLang > -1)
        EndToken =  " " + EndToken; // In this case we add a space for separation since we directly append the value to the prompt


    VoiceInfo Inf{JS["name"].get<std::string>(),
                 JS["author"].get<std::string>(),
                 JS["version"].get<int>(),
                 JS["description"].get<std::string>(),
                 CuArch,
                 JS["note"].get<std::string>(),
                 JS["sarate"].get<uint32_t>(),
                 RawLang,
                LanguageNames[LanguageValue],
                LangaugeNamesNumToWords[LanguageValue],
                 EndToken};

    if (Inf.Note.size() > MaxNoteSize)
        Inf.Note = Inf.Note.substr(0,MaxNoteSize);

    return Inf;







}

std::vector<std::string> VoxUtil::GetLinedFile(const std::string &Path)
{
    std::vector<std::string> RetLines;
    std::ifstream Fi(Path);

    if (!Fi.good()) // File not exists, ret empty vec
        return RetLines;

    std::string Line;
    while (std::getline(Fi, Line))
    {
        if (Line.size() > 1)
            RetLines.push_back(Line);


    }

    return RetLines;
}

std::string VoxUtil::U32ToStr(const std::u32string &InU32)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> Converter;
    return Converter.to_bytes(InU32);



}

std::u32string VoxUtil::StrToU32(const std::string &InStr)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> Converter;
    return Converter.from_bytes(InStr);

}
