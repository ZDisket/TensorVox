#include "melgen.h"

MelGen::MelGen()
{

}

MelGen::MelGen(const std::string &SavedModelFolder, ETTSRepo::Enum InTTSRepo)
{
   Initialize(SavedModelFolder,InTTSRepo);

}

bool MelGen::Initialize(const std::string &SavedModelFolder,  ETTSRepo::Enum InTTSRepo)
{
    try {
        CurrentMdl = std::make_unique<cppflow::model>(SavedModelFolder);
    }
    catch (...) {
        return false;

    }
    CurrentRepo = InTTSRepo;
    return true;

}
