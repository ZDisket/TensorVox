#include "torchmoji.h"
#include "ext/ZCharScanner.h"

void TorchMoji::LoadDict(const std::string& Path)
{
   if (Dictionary.size())
       Dictionary.clear();

   std::vector<std::string> Lined = VoxUtil::GetLinedFile(Path);

   ZStringDelimiter Delim;
   Delim.AddDelimiter("\t");

   for (const auto& Li : Lined){
       Delim.SetText(Li);

       if (Delim.szTokens() < 2)
           continue;

       Dictionary.insert({Delim[0], std::stoi(Delim[1])});
   }
}

std::vector<int32_t> TorchMoji::WordsToIDs(const std::vector<std::string>& Words)
{
    std::vector<int32_t> IDs(VoxCommon::TorchMojiLen,0);

    for (size_t i = 0; i < Words.size();i++)
    {
        if (i + 1 > VoxCommon::TorchMojiLen)
            break;

        auto Iter = Dictionary.find(Words[i]);

        if (Iter == Dictionary.end())
            IDs[i] = 1; // unknown
        else
            IDs[i] = Iter->second;



    }

    return IDs;



}

TorchMoji::TorchMoji()
{

}

TorchMoji::TorchMoji(const std::string &InitPath, const std::string &DPath)
{
    Initialize(InitPath,DPath);

}

void TorchMoji::Initialize(const std::string &Path, const std::string &DictPath)
{

    Model = torch::jit::load(Path);
    LoadDict(DictPath);
}

TFTensor<float> TorchMoji::Infer(const std::vector<std::string> &Seq)
{
    torch::NoGradGuard no_grad;
    std::vector<int32_t> Input = WordsToIDs(Seq);

    auto InIDS = torch::tensor(Input).unsqueeze(0); // (1, TMLen)

    at::Tensor Output = Model({InIDS}).toTensor(); // (1, VoxCommon::TorchMojiEmbSize)

    Output = Output.squeeze(); // (TorchMojiEmbSize)

    TFTensor<float> Tens = VoxUtil::CopyTensor<float>(Output);


    return Tens;



}
