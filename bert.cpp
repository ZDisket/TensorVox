#include "bert.h"
#include <windows.h>

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
    Tokenizer = std::make_unique<FullTokenizer>(DictPath,true);


}

std::pair<TFTensor<float>, TFTensor<float> > BERT::Infer(const std::string &InText)
{
    torch::NoGradGuard no_grad;

    auto Tokens = Tokenizer->tokenize(InText + "\n");
    auto Ids = Tokenizer->convertTokensToIds(Tokens);

    std::vector<int32_t> InTokens(Ids.begin(),Ids.end());



    auto InIDS = torch::tensor(InTokens).unsqueeze(0); // (1, tokens)
    std::pair<TFTensor<float>,TFTensor<float>> BERTOutputs;

    try{
        auto Output = Model({InIDS}).toTuple(); // (hidden states, pooled)
        BERTOutputs.first = VoxUtil::CopyTensor<float>(Output.get()->elements()[0].toTensor());
        BERTOutputs.second = VoxUtil::CopyTensor<float>(Output.get()->elements()[1].toTensor());



    }

    catch (const std::exception& e) {
        int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)QString::fromStdString(e.what()).toStdWString().c_str(),
                    (LPCWSTR)L"Error1!!",
                    MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
                    );


        return BERTOutputs;

    }






    return BERTOutputs;



}
