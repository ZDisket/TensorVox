# TensorVox

TensorVox is an application designed to enable user-friendly and lightweight neural speech synthesis in the desktop, aimed at increasing accessibility to such technology. 

Powered by [TensorflowTTS](https://github.com/TensorSpeech/TensorFlowTTS), it is written in pure C++/Qt, using the Tensorflow C API for interacting with the models. This way, we can perform inference without having to install gigabytes worth of pip libraries, just a 100MB DLL.

### Try it out
Grab it from the releases and check [the Google Drive folder](https://drive.google.com/drive/folders/1atUyxBbstKZpMqQEZMdNmRF2AKrlahKy?usp=sharing) for models and installation instructions
TODO: Add instructions for training and exporting models

## Supported architectures

Currently, only FastSpeech2 (phoneme-based) and Multi-Band MelGAN from TensorflowTTS are supported. 


## Build instructions
Currently, only Windows x64 is supported.

**Requirements:**
 1. Qt Creator
 2. MSVC 2017 (v141) compiler

**Primed build (with all provided libraries):**

 1. Download [precompiled binary dependencies and includes](https://drive.google.com/file/d/1ufLQvH-Me2NLmzNBkjcyD13WTyHb35aB/view?usp=sharing)
 2. Unzip it so that the `deps` folder is in the same place as the .pro and main source files.
 3. Open the project with Qt Creator, add your compiler and compile

Note that to try your shiny new executable you'll need to download the program as described above and insert the `models` folder where your new build is output.

TODO: Add instructions for compile from scratch.

## Externals (and thanks)

 - **Tensorflow C API**: [https://www.tensorflow.org/install/lang_c](https://www.tensorflow.org/install/lang_c)
 - **CppFlow** (TF C API -> C++ wrapper): [https://github.com/serizba/cppflow](https://github.com/serizba/cppflow) 
 - **AudioFile** (for WAV export): [https://github.com/adamstark/AudioFile](https://github.com/adamstark/AudioFile)
 - **Frameless Dark Style Window**: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
 - **JSON for modern C++**: https://github.com/nlohmann/json
 - **r8brain-free-src** (Resampling): https://github.com/avaneev/r8brain-free-src
 - **rnnoise** (CMake version, denoising output): https://github.com/almogh52/rnnoise-cmake
 - **Logitech LED Illumination SDK** (Mouse RGB integration): https://www.logitechg.com/en-us/innovation/developer-lab.html
 - **QCustomPlot** : https://www.qcustomplot.com/index.php/introduction

## Contact
You can open an issue here or join the [Discord server](https://discord.gg/yqFDAWH) and discuss/ask anything there

## Note about licensing

This project is MIT licensed almost everywhere except for Vietnam, where, due to using TensorflowTTS models as backend, it cannot be used without permission from the TensorflowTTS authors. See [here](https://github.com/TensorSpeech/TensorflowTTS#license) for details
