#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include <memory>
#include <sstream>
#include <algorithm>


// Util functions for ONNX stuff
namespace ONNXUtil {
	// pretty prints a shape dimension vector
	std::string print_shape(const std::vector<std::int64_t>& v) {
		std::stringstream ss("");
		for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
		ss << v[v.size() - 1];
		return ss.str();
	}

	template <typename T>
	Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {

		Ort::MemoryInfo mem_info =
			Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
		auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
		return tensor;
	}

	std::vector<const char*> StringVectoCharVec(const std::vector<std::string>& inputStrings) {
		std::vector<const char*> outputChars(inputStrings.size(), nullptr);
		std::transform(std::begin(inputStrings), std::end(inputStrings), std::begin(outputChars),
			[](const std::string& str) { return str.c_str(); });

		return outputChars;
	}

}
