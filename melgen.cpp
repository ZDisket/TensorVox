#include "melgen.h"

MelGen::MelGen()
{

}

MelGen::MelGen(const std::string &SavedModelFolder)
{
   Initialize(SavedModelFolder);

}

bool MelGen::Initialize(const std::string &SavedModelFolder)
{
    try {
        CurrentMdl = std::make_unique<cppflow::model>(SavedModelFolder);
    }
    catch (...) {
        return false;

    }
    return true;

}
