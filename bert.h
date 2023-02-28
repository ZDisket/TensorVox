#ifndef BERT_H
#define BERT_H

#include "VoxCommon.hpp"
// BERT: Class for inference of TorchScript-exported BERT.
class BERT
{
private:
    torch::jit::script::Module Model;

public:
    BERT();
    BERT(const std::string& Path,const std::string& DictPath);
    void Initialize(const std::string& Path,const std::string& DictPath);
};

#endif // BERT_H
