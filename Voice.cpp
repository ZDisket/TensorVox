#include "Voice.h"
#include "ext/ZCharScanner.h"

std::vector<int32_t> Voice::CharsToID(const std::string & RawInTxt)
{

    std::cout << "CharsToID: " << RawInTxt << "\n";
    std::vector<int32_t> VecPhones;

    std::u32string InTxt = VoxUtil::StrToU32(RawInTxt);

    for (const auto& Char : InTxt)
    {
        size_t ArrID = 0;
        std::u32string CharAs;
        CharAs += Char;

        if (VoxUtil::FindInVec<std::u32string>(CharAs, Phonemes, ArrID))
            VecPhones.push_back(PhonemeIDs[ArrID]);
        else
            std::cout << "Voice::PhonemesToID() WARNING: Unknown phoneme " << Char << std::endl;



    }


    return VecPhones;

}

std::vector<int32_t> Voice::PhonemesToID(const std::string & RawInTxt)
{
    std::cout << "PhonemesToID: " << RawInTxt << "\n";
    ZStringDelimiter Delim(RawInTxt);
	Delim.AddDelimiter(" ");
    std::u32string InTxt = VoxUtil::StrToU32(RawInTxt);


	std::vector<int32_t> VecPhones;
	VecPhones.reserve(Delim.szTokens());


	for (const auto& Pho : Delim.GetTokens()) 
	{
		size_t ArrID = 0;
        std::u32string PhnU = VoxUtil::StrToU32(Pho);

        if (VoxUtil::FindInVec<std::u32string>(PhnU, Phonemes, ArrID))
            VecPhones.push_back(PhonemeIDs[ArrID]);
		else
            std::cout << "Voice::PhonemesToID() WARNING: Unknown phoneme " << Pho << std::endl;



	}


	return VecPhones;

}

void Voice::ReadPhonemes(const std::string &PhonemePath)
{
    std::ifstream Phone(PhonemePath);

    std::string Line;
    while (std::getline(Phone, Line))
    {
        if (Line.find("\t") == std::string::npos)
            continue;


        ZStringDelimiter Deline(Line);
        Deline.AddDelimiter("\t");

        Phonemes.push_back(VoxUtil::StrToU32(Deline[0]));
        PhonemeIDs.push_back(stoi(Deline[1]));




    }

}

void Voice::ReadSpeakers(const std::string &SpeakerPath)
{
    Speakers = VoxUtil::GetLinedFile(SpeakerPath);

}

void Voice::ReadEmotions(const std::string &EmotionPath)
{
    Emotions = VoxUtil::GetLinedFile(EmotionPath);

}


void Voice::ReadModelInfo(const std::string &ModelInfoPath)
{

    ModelInfo = "";
    std::vector<std::string> MiLines = VoxUtil::GetLinedFile(ModelInfoPath);

    for (const std::string& ss : MiLines)
        ModelInfo += ss + "\n";


}


Voice::Voice(const std::string & VoxPath, const std::string &inName, Phonemizer *InPhn)
{
    ReadModelInfo(VoxPath + "/info.txt");



    VoxInfo = VoxUtil::ReadModelJSON(VoxPath + "/info.json");

    const int32_t Tex2MelArch = VoxInfo.Architecture.Text2Mel;

    const bool IsVITS = Tex2MelArch == EText2MelModel::VITS || Tex2MelArch == EText2MelModel::DEVITS;

    if (Tex2MelArch == EText2MelModel::Tacotron2)
        MelPredictor = std::make_unique<Tacotron2>();
    else if (Tex2MelArch == EText2MelModel::FastSpeech2)
        MelPredictor = std::make_unique<FastSpeech2>();
    else if (Tex2MelArch == EText2MelModel::VITS)
        MelPredictor = std::make_unique<VITS>();
    else if (Tex2MelArch == EText2MelModel::DEVITS)
         MelPredictor = std::make_unique<DEVITS>();
    else
        MelPredictor = std::make_unique<Tacotron2Torch>();


    std::string MelPredInit = VoxPath + "/melgen";

    if (IsVITS)
        MelPredInit = VoxPath + "/vits.pt";

    if (Tex2MelArch == EText2MelModel::Tacotron2Torch)
        MelPredInit = VoxPath + "/tacotron2.pt";

    MelPredictor->Initialize(MelPredInit,(ETTSRepo::Enum)VoxInfo.Architecture.Repo);



    if (Tex2MelArch == EText2MelModel::DEVITS){
        Moji.Initialize(VoxPath + "/moji.pt", VoxPath + "/tm_dict.txt");
        BertFE.Initialize(VoxPath + "/bert.pt", VoxPath + "/bert_vocab.txt");

    }



    const int32_t VocoderArch = VoxInfo.Architecture.Vocoder;


    if (VocoderArch == EVocoderModel::iSTFTNet)
        Vocoder = std::make_unique<iSTFTNetTorch>();
    else if (VocoderArch == EVocoderModel::NullVocoder)
        Vocoder = nullptr;
    else
        Vocoder = std::make_unique<MultiBandMelGAN>();

    if (Vocoder.get())
    {


        Vocoder.get()->Initialize(VoxPath + "/vocoder");


    }









    if (InPhn)
        Processor.Initialize(InPhn);


    Name = inName;
    ReadPhonemes(VoxPath + "/phonemes.txt");
    ReadSpeakers(VoxPath + "/speakers.txt");
    ReadEmotions(VoxPath + "/emotions.txt");









}

void Voice::AddPhonemizer(Phonemizer *InPhn,ESpeakPhonemizer* InENGPhn)
{
    Processor.Initialize(InPhn,InENGPhn);
    Processor.GetTokenizer().SetNumberText(NumTxt,VoxCommon::CommonLangConst);


}

void Voice::LoadNumberText(const std::string &NumTxtPath)
{
    NumTxt.load(VoxCommon::CommonLangConst,NumTxtPath);
}

std::string Voice::PhonemizeStr(const std::string &Prompt)
{


    return Processor.ProcessTextPhonetic(Prompt,Phonemes,CurrentDict,
                                                           (ETTSLanguageType::Enum)VoxInfo.LangType,
                                                           true); // default voxistac to true to preserve punctuation.

}


VoxResults Voice::Vocalize(const std::string & Prompt, float Speed, int32_t SpeakerID, float Energy, float F0, int32_t EmotionID,const std::string& EmotionOvr)
{



    const int32_t Text2MelN = VoxInfo.Architecture.Text2Mel;

    bool VoxIsTac = Text2MelN != EText2MelModel::FastSpeech2;

    std::string PromptToFeed = Prompt;
    if (VoxInfo.LangType != ETTSLanguageType::Char && Text2MelN != EText2MelModel::Tacotron2Torch)
        PromptToFeed += VoxInfo.EndPadding;

    std::string PhoneticTxt = Processor.ProcessTextPhonetic(PromptToFeed,Phonemes,CurrentDict,
                                                            (ETTSLanguageType::Enum)VoxInfo.LangType,
                                                           VoxIsTac);
    TFTensor<float> Mel;
    TFTensor<float> Attention;

    std::vector<int32_t> InputIDs;

    if (Text2MelN == EText2MelModel::Tacotron2Torch)
        PhoneticTxt += VoxInfo.EndPadding;



    if (VoxInfo.LangType == ETTSLanguageType::Char){
        InputIDs = CharsToID(PhoneticTxt);
        InputIDs.push_back(std::stoi(VoxInfo.EndPadding));


    }
    else
    {
        if (VoxInfo.LangType == ETTSLanguageType::IPA)
            InputIDs = CharsToID(PhoneticTxt);
        else
            InputIDs = PhonemesToID(PhoneticTxt);




    }

    std::vector<float> FloatArgs;
    std::vector<int32_t> IntArgs;



    if (Text2MelN == EText2MelModel::Tacotron2)
    {

        Mel = ((Tacotron2*)MelPredictor.get())->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID, EmotionID);
        Attention = ((Tacotron2*)MelPredictor.get())->Attention;

    }
    else if (Text2MelN == EText2MelModel::Tacotron2Torch)
    {
        Mel = MelPredictor.get()->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID, EmotionID);
        Attention = ((Tacotron2Torch*)MelPredictor.get())->Attention;
    }
    else if (Text2MelN == EText2MelModel::FastSpeech2)
    {

        FloatArgs = {Speed,Energy,F0};

        Mel = ((FastSpeech2*)MelPredictor.get())->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID, EmotionID);

    }else if (Text2MelN == EText2MelModel::VITS)
    {
        FloatArgs = {Speed};


        TFTensor<float> Audio = MelPredictor.get()->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID,EmotionID);
        Attention = ((VITS*)MelPredictor.get())->Attention;

        std::vector<float> AudioData = Audio.Data;

        Mel.Shape.push_back(-1); // Tell the plotter that we have no mel to plot

        // As VITS is fully E2E, we return here

        return {AudioData,Attention,Mel};

    }else // DE-VITS
    {
        FloatArgs = {Speed};
        std::vector<std::string> MojiInput = Processor.GetTokenizer().Tokenize(EmotionOvr,true,true);
        TFTensor<float> MojiStates = Moji.Infer(MojiInput);

        auto BERTOutputs = BertFE.Infer(Prompt);




        TFTensor<float> Audio = ((DEVITS*)MelPredictor.get())->DoInferenceDE(InputIDs, MojiStates,
                                                                             BERTOutputs.first,FloatArgs,
                                                                             IntArgs,SpeakerID,EmotionID);
        Attention = ((VITS*)MelPredictor.get())->Attention;

        std::vector<float> AudioData = Audio.Data;

        Mel.Shape.push_back(-1); // Tell the plotter that we have no mel to plot

        // As VITS is fully E2E, we return here

        return {AudioData,Attention,Mel};
    }

    // Vocoder inference


    TFTensor<float> AuData = Vocoder.get()->DoInference(Mel);
    std::vector<float> AudioData;

    if (AuData.Shape.size() > 1)
    {

        int64_t Width = AuData.Shape[0];
        int64_t Height = AuData.Shape[1];
        int64_t Depth = AuData.Shape[2];
        //int z = 0;


        AudioData.resize(Height);

        // Code to access 1D array as if it were 3D
        for (int64_t x = 0; x < Width;x++)
        {
            for (int64_t z = 0;z < Depth;z++)
            {
                for (int64_t y = 0; y < Height;y++) {
                    int64_t Index = x * Height * Depth + y * Depth + z;
                    AudioData[(size_t)y] = AuData.Data[(size_t)Index];

                }

            }
        }

    }
    else
    {
        // Pre-flattened vocoder output
        AudioData = AuData.Data;

    }



    if (!AudioData.size())
        QMessageBox::critical(nullptr,"f","ss");

    return {AudioData,Attention,Mel};
}

void Voice::SetDictEntries(const std::vector<DictEntry> &InEntries)
{
    for (const DictEntry& Entr : InEntries)
    {
        if (Entr.Language != VoxInfo.s_Language_Fullname)
            continue;

        CurrentDict.push_back(Entr);

    }

}

Voice::~Voice()
{
}
