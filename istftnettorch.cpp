#include "istftnettorch.h"
#include <windows.h>
bool iSTFTNetTorch::Initialize(const std::string &VocoderPath)
{
    torch::Device device(torch::kCPU);

    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().

        std::string VCP = VocoderPath + ".pt";

        Model = torch::jit::load(VCP,device);

    }
    catch (const c10::Error& e) {
        QMessageBox::critical(nullptr,"r",e.what_without_backtrace());
        return false;

    }
    try{
        std::string PostPath = VocoderPath + "-post.pt";
        Post = torch::jit::load(PostPath,device);
        PostLoaded = true;
    }
    catch (const c10::Error& e){
        PostLoaded = false;

    }



    return true;

}

TFTensor<float> iSTFTNetTorch::DoInference(const TFTensor<float> &InMel)
{
    // without this memory consumption is 4x
    torch::NoGradGuard no_grad;
    torch::Device device(torch::kCPU);
    auto TorchMel = torch::tensor(InMel.Data,device).reshape(InMel.Shape).transpose(1,2); // [1, frames, n_mels] -> [1, n_mels, frames]



    try{
        at::Tensor Output = Model({TorchMel}).toTensor().squeeze(); // (audio frames)
        if (PostLoaded)
            Output = Post({Output.unsqueeze(0).toType(at::ScalarType::Float)}).toTensor();


        TFTensor<float> Tens = VoxUtil::CopyTensor<float>(Output);


        return Tens;
    }
    catch (const std::exception& e) {
        int msgboxID = MessageBox(
            NULL,
            (LPCWSTR)QString::fromStdString(e.what()).toStdWString().c_str(),
            (LPCWSTR)L"Account Details",
            MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
        );


        return TFTensor<float>();

    }





}

iSTFTNetTorch::iSTFTNetTorch()
{

}
