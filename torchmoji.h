#ifndef TORCHMOJI_H
#define TORCHMOJI_H
#include "VoxCommon.hpp"


// TorchMoji: Emotion contextualizer model (Cookie design: skipping last layer and using hidden states to feed TTS model)
// Allows for manipulation of emotion at inference time
class TorchMoji
{
private:
        // Word, ID
     std::map<std::string,int32_t> Dictionary;

     torch::jit::script::Module Model;

     void LoadDict(const std::string& Path);

     std::vector<int32_t> WordsToIDs(const std::vector<std::string> &Words);
public:
    TorchMoji();

    TorchMoji(const std::string& InitPath,const std::string& DPath);

    void Initialize(const std::string& Path,const std::string& DictPath);

    // Return hidden states of emotion state.
    // -> Seq: Vector of words
    // <- Returns float tensor of size VoxCommon::TorchMojiEmbSize containing hidden states, ready to feed into TTS model.
   TFTensor<float> Infer(const std::vector<std::string>& Seq);
};

#endif // TORCHMOJI_H
