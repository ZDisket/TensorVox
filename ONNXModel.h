#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include <memory>
#include <sstream>
#include <algorithm>



template<typename T>
struct ONXTensor {
	std::vector<T> Data;
	std::vector<int64_t> Shape;

	ONXTensor(Ort::Value& Tens)
	{
		Shape = Tens.GetTensorTypeAndShapeInfo().GetShape();

		
		int64_t product = 1;
		for (int64_t dim : Shape)
			product *= dim;

		Data = std::vector<T>(Tens.GetTensorData<float>(), Tens.GetTensorData<float>() + product);

	}
	ONXTensor(const ONXTensor& CpyT)
	{
		Data = CpyT.Data;
		Shape = CpyT.Shape;
	}
	ONXTensor() {
	

	}
};


/*
ONNXModel: Base class for ONNX models, running on DirectML.
*/
class ONNXModel
{
private:
	bool ModelLoaded;
	bool IsGPU;
	std::wstring DeviceName;

	std::unique_ptr<Ort::Env> Environment;


	std::unique_ptr<Ort::Session> Sess;
	std::unique_ptr<Ort::AllocatorWithDefaultOptions> Alloc;
	
	/*
	 Sets up DirectML backend
	 Takes in NON-CONST (direct operation on) reference to session options, and returns success
	 If succeeds, use the session options to create the session
	*/
	bool SetUpDirectML(Ort::SessionOptions& SessionOptions);

	void DisableMemReuse(OrtSessionOptions* SessPtr);

	std::pair<std::vector<const char*>, std::vector<const char*>> GetInputOutputNamesChar(const std::vector<std::string>& InputNames, const std::vector<std::string>& OutputNames);

public:
	ONNXModel();
	inline ONNXModel(const std::wstring& InitModelPath) { Load(InitModelPath); };

	inline bool IsLoaded() const { return ModelLoaded; }
	inline bool IsGPUDevice() const { return IsGPU; }
	inline const std::wstring& GetDeviceName() const { return DeviceName; }


	/*
	Load model in DirectML mode.
	Inputs:

	ModelPath: Path of model 

	Returns:
	Success (true), or failure (false)
	*/
	virtual bool Load(const std::wstring& ModelPath, const std::string& EnvName = "defaultenv");

	/*
	Forward pass through the model, for simple calling when all tensors are of one type.

	Inputs:

	- Inputs: Tensor datas
	- InputShapes: Shapes of each tensor
	- InputNames: Names of the input tensors. Optional; pass an empty one and they will be auto-fetched
	- OutputNames: Names of the output tensors. Optional; pass an empty one and they will be auto-fetched

	*/
	template<typename D>          // NOTE: Inputs should be a const reference, but vec_to_tensor requires a non-const one.
	std::vector<Ort::Value> Forward(std::vector<std::vector<D>>& Inputs, const std::vector<std::vector<int64_t>>& InputShapes,
									std::vector<std::string> InputNames, std::vector<std::string> OutputNames) 
	{
		// Make input tensors
		std::vector<Ort::Value> InputTensors;
		Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
		for (size_t i = 0; i < Inputs.size(); i++) 
		{
			// Ort::Value::Value(const Ort::Value &) is deleted, so we have to directly push back the output of the fun instead of using a variable.
			InputTensors.emplace_back(Ort::Value::CreateTensor<D>(mem_info, Inputs[i].data(), Inputs[i].size(), InputShapes[i].data(), InputShapes[i].size())
			);
			
		
		}

		return Forward(InputTensors, InputNames, OutputNames);
	}


	/*
	Forward pass through the model

	Inputs:

	- InputTensors: Tensors.
	- InputNames: Names of the input tensors. Optional; pass an empty one and they will be auto-fetched
	- OutputNames: Names of the output tensors. Optional; pass an empty one and they will be auto-fetched

	*/
	std::vector<Ort::Value> Forward(std::vector<Ort::Value>& InputTensors,
		std::vector<std::string> InputNames = std::vector<std::string>{}, std::vector<std::string> OutputNames = std::vector<std::string>{})
	{

		// Autogen output names if we don't have them
		if (!OutputNames.size())
			OutputNames = GetOutputNames();

		// Ditto for input names
		if (!InputNames.size())
			InputNames = GetInputs().second;


		// Create input names
		auto InputOutNames = GetInputOutputNamesChar(InputNames, OutputNames);
		std::vector<const char*> InputNamesChar = InputOutNames.first;
		std::vector<const char*> OutputNamesChar = InputOutNames.second;

		if (!Sess)
			throw std::runtime_error("Session is NULL!");

		auto OutputTensors = Sess->Run(Ort::RunOptions{ nullptr }, InputOutNames.first.data(), InputTensors.data(),
			InputOutNames.first.size(), InputOutNames.second.data(), InputOutNames.second.size());


		return OutputTensors;
	}

	// Get a reference to the unique ptr of the session.
	std::unique_ptr<Ort::Session>& GetSession() { return Sess; }

	std::vector<std::string> GetOutputNames();
	std::vector<std::string> GetInputNames();

	std::vector<ONNXTensorElementDataType> GetInputTypes();
	// Returns the input shapes and names
	std::pair<std::vector<std::vector<int64_t>>, std::vector<std::string>> GetInputs();
};

