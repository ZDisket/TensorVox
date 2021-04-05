#include "tfg2p.h"
TFG2P::TFG2P()
{
    G2P = nullptr;

}

TFG2P::TFG2P(const std::string &SavedModelFolder)
{
    G2P = nullptr;

    Initialize(SavedModelFolder);
}

bool TFG2P::Initialize(const std::string &SavedModelFolder)
{
    try {

        G2P = new cppflow::model(SavedModelFolder);

    }
    catch (...) {
        G2P = nullptr;
        return false;

    }
    return true;
}

TFTensor<int32_t> TFG2P::DoInference(const std::vector<int32_t> &InputIDs, float Temperature)
{
    if (!G2P)
        throw std::exception("Tried to do inference on unloaded or invalid model!");

    // Convenience reference so that we don't have to constantly derefer pointers.
    cppflow::model& Mdl = *G2P;


    // Convenience reference so that we don't have to constantly derefer pointers.

    cppflow::tensor input_ids{ InputIDs, std::vector<int64_t>{(int64_t)InputIDs.size()}};
    cppflow::tensor input_len{(int32_t)InputIDs.size()};
    cppflow::tensor input_temp{Temperature};





    auto Outs = Mdl({{"serving_default_input_ids:0",input_ids},
         {"serving_default_input_len:0",input_len},
         {"serving_default_input_temperature:0",input_temp}},{"StatefulPartitionedCall:0"});

    TFTensor<int32_t> RetTensor = VoxUtil::CopyTensor<int32_t>(Outs[0]);

    return RetTensor;


}

TFG2P::~TFG2P()
{
    if (G2P)
        delete G2P;

}
