#include "ONNXModel.h"
#include <dml_provider_factory.h>
#include <onnxruntime_c_api.h>
// We can't include this (or embed in the ONNXModel header) because the linker thinks they're already defined in
// subclasses like Crepe (???)
#include "ONNXUtil.hpp"

#include <iostream>
#include <filesystem>


#include <windows.h>
#include <string>

bool DoesFileExist(const std::wstring& path) {
    DWORD fileAttributes = GetFileAttributesW(path.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        return false; // The file does not exist or there is an error.
    }
    return true; // The file exists.
}

bool ONNXModel::SetUpDirectML(Ort::SessionOptions& SessionOptions)
{
    // This functions tells us it's deprecated but the alternative causes a crash, plus it's not mentioned in documentation (????)
    OrtStatusPtr Stat = OrtSessionOptionsAppendExecutionProvider_DML(SessionOptions, 0);

    OrtStatus* Status = (Stat);
    if (Status)
        return false; // Status NOT nullptr indicates failure

    // https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html#configuration-options
    /*
    The DirectML execution provider does not support the use of memory pattern optimizations or parallel execution in onnxruntime
    When supplying session options during InferenceSession creation, these options must be disabled or an error will be returned.
    */
    
    SessionOptions.DisableMemPattern();
    SessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
    


    return true;
}

void ONNXModel::DisableMemReuse(OrtSessionOptions* SessPtr)
{
}

std::pair<std::vector<const char*>, std::vector<const char*>> ONNXModel::GetInputOutputNamesChar(const std::vector<std::string>& InputNames, const std::vector<std::string>& OutputNames)
{
    return std::pair< std::vector<const char*>, std::vector<const char*>>{
        ONNXUtil::StringVectoCharVec(InputNames),
        ONNXUtil::StringVectoCharVec(OutputNames),
    };

}

ONNXModel::ONNXModel()
{
    ModelLoaded = false;
}

bool ONNXModel::Load(const std::wstring& ModelPath, const std::string& EnvName)
{

    if (!DoesFileExist(ModelPath))
         return false;

    Environment = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, EnvName.c_str());

    // Set up DirectML
    
    Ort::SessionOptions session_options;
    bool DML_Succ = SetUpDirectML(session_options);
    

    if (!DML_Succ)
        return false;

    Sess = std::make_unique<Ort::Session>(*Environment, ModelPath.c_str(), session_options);
    Alloc = std::make_unique<Ort::AllocatorWithDefaultOptions>();

    if (!Sess || !Alloc)
        return false;

    ModelLoaded = true;



    return true;
}

std::vector<std::string> ONNXModel::GetOutputNames()
{
    std::vector<std::string> OutputNames;
    for (std::size_t i = 0; i < Sess->GetOutputCount(); i++) {
        OutputNames.emplace_back(Sess->GetOutputNameAllocated(i, *Alloc).get());
    }
    return OutputNames;
}

std::vector<std::string> ONNXModel::GetInputNames()
{
    return std::vector<std::string>();
}

std::vector<ONNXTensorElementDataType> ONNXModel::GetInputTypes()
{
    std::vector<ONNXTensorElementDataType> InputTypes;
    for (std::size_t i = 0; i < Sess->GetInputCount(); i++) {
        InputTypes.emplace_back(Sess->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType());
    }


    return InputTypes;
}

std::pair<std::vector<std::vector<int64_t>>, std::vector<std::string>> ONNXModel::GetInputs()
{
 
    std::vector<std::vector<int64_t>> InputShapes;
    std::vector<std::string> InputNames;
    for (std::size_t i = 0; i < Sess->GetInputCount(); i++) {
        InputNames.emplace_back(Sess->GetInputNameAllocated(i, *Alloc).get());

        InputShapes.emplace_back(Sess->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
    }

    std::pair<std::vector<std::vector<int64_t>>, std::vector<std::string>> Ret(InputShapes, InputNames);
    return Ret;
}
