#ifndef MELGEN_H
#define MELGEN_H

#include "ext/CppFlow/include/Model.h"
#include "VoxCommon.hpp"
#include <memory>

// MelGen: base virtual class for mel generators
class MelGen
{
public:
    MelGen();
    MelGen(const std::string& SavedModelFolder);


    // Generic inference function
    // Utilize ArgsFloat and ArgsInt for additional arguments for certain models
    virtual TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1) = 0;

    /*
    Initialize and load the model

    -> SavedModelFolder: Folder where the .pb, variables, and other characteristics of the exported SavedModel
    <- Returns: (bool)Success
    */
    virtual bool Initialize(const std::string& SavedModelFolder);


    std::unique_ptr<Model> CurrentMdl;

};

#endif // MELGEN_H
