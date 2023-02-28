#include "bert.h"

BERT::BERT()
{

}

BERT::BERT(const std::string &Path, const std::string &DictPath)
{
    Initialize(Path, DictPath);
}

void BERT::Initialize(const std::string &Path, const std::string &DictPath)
{
    Model = torch::jit::load(Path);


}
