#ifndef BERT_H
#define BERT_H

#include "VoxCommon.hpp"

#include "berttokenizer.h"

// BERT: Class for inference of TorchScript-exported BERT.
class BERT
{
private:
    torch::jit::script::Module Model;
    std::unique_ptr<FullTokenizer> Tokenizer;

public:
    BERT();
    BERT(const std::string& Path,const std::string& DictPath);
    void Initialize(const std::string& Path,const std::string& DictPath);


    // Do inference on BERT model.
    // Returns 2 tensors:
    // [1, tokens, channels] : Hidden states
    // [1, channels] : Pooled embeddings
    std::pair<TFTensor<float>,TFTensor<float>> Infer(const std::string& InText);
};

#endif // BERT_H
