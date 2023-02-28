#include "tacotron2torch.h"

Tacotron2Torch::Tacotron2Torch()
{

}

bool Tacotron2Torch::Initialize(const std::string &SavedModelFolder, ETTSRepo::Enum InTTSRepo)
{
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().

        Model = torch::jit::load(SavedModelFolder);

    }
    catch (const c10::Error& e) {
        return false;

    }

    CurrentRepo = InTTSRepo;
    return true;

}

TFTensor<float> Tacotron2Torch::DoInference(const std::vector<int32_t> &InputIDs, const std::vector<float> &ArgsFloat, const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    // without this memory consumption is 4x
    torch::NoGradGuard no_grad;


    std::vector<int64_t> IInputIDs;
    IInputIDs.reserve(InputIDs.size());
    for (const int32_t& Id : InputIDs){
        int64_t casted = (int64_t)Id;
        IInputIDs.push_back(casted);

    }



    torch::TensorOptions Opts = torch::TensorOptions().requires_grad(false);

    // This Tacotron2 always takes in speaker IDs
    if (SpeakerID == -1)
        SpeakerID = 0;

    auto InSpkid = torch::tensor({SpeakerID},Opts);
    auto InIDS = torch::tensor(IInputIDs, Opts).unsqueeze(0);



    std::vector<torch::jit::IValue> inputs{ InSpkid,InIDS};



    // Infer
    c10::IValue Output = Model(inputs);


    // Output = list (mel_outputs, mel_outputs_postnet, gate_outputs, alignments)

    auto OutputL = Output.toList();

    auto MelTens = OutputL[1].get().toTensor();
    auto AttTens = OutputL[3].get().toTensor();//.transpose(1,2); // [1, dec_t, enc_t ] -> [1, enc_t, dec_t]


    Attention = VoxUtil::CopyTensor<float>(AttTens);


    return VoxUtil::CopyTensor<float>(MelTens);



}
