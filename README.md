# TensorVox


TensorVox is an application designed to enable user-friendly and lightweight neural speech synthesis in the desktop, aimed at increasing accessibility to such technology. 

Being able to load a variety of AI TTS models, it is written in pure C++/Qt, using the ONNX Runtime, and supporting TensorFlow and LibTorch as legacy backends.

![Interface with Tac2 model loaded](https://i.imgur.com/wtPzzNh.png)


### Try it out
**System requirements:** Windows 10 64-bit and a CPU that supports the AVX instruction set (pretty much anything made after 2010). As for GPU, to use it you need one that supports DirectX 12 (only with ONNX models)


[Detailed guide in Google Docs](https://docs.google.com/document/d/1OS1kfb19bvpPPkF71Vbak_b735mi7epjUanIfPG671M/edit?usp=sharing)

Grab a copy from the releases, extract the .zip and check [the Google Drive folder](https://drive.google.com/drive/folders/1atUyxBbstKZpMqQEZMdNmRF2AKrlahKy?usp=sharing) for models and installation instructions

If you're interested in using your own model, first you need to train then export it. 


## Supported architectures

TensorVox supports several models by various authors

### Current-gen models
These are models under active development by their authors and support by me. These use the ONNX backend, with GPU support under DirectML
 - **ZDisket/VITS Evolution:** This is my fully upgraded version of VITS
 - **Supertone/[Supertonic](https://github.com/supertone-inc/supertonic) 2:** Supertone's small, fast model made for fast on-device inference

More models are to be added soon.
### Legacy models
Compatibility for these models is kept but not maintained, as they're obsolete and have been superseded by newer ones. These run under the LibTorch and Tensorflow backends, supporting only CPU.
 - **jaywalnut310/VITS:** VITS, which is a fully E2E model. (Stressed IPA as phonemes) Export notebook: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/1BSGE5DQYweXBWrwPOmb6CRPUU8H5mBvb?usp=sharing)
 - **TensorFlowTTS**: FastSpeech2, Tacotron2, both char and phoneme based and Multi-Band MelGAN. Here's a Colab notebook demonstrating how to export the LJSpeech pretrained, char-based Tacotron2 model: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/1KLqZ1rkD4Enw7zpTgXGL6if7e5s0UeWa?usp=sharing) 
 - **Coqui-TTS:** Tacotron2 (phoneme-based IPA) and Multi-Band MelGAN, after converting from PyTorch to Tensorflow. Here's a notebook showing how to export the LJSpeech DDC model: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/15CdGEAu_-KezV1XxwzVfQiFSm0tveBkC?usp=sharing)

More support of modern TTS models is being actively worked on!

These examples should provide you with enough guidance to understand what is needed. If you're looking to train a model specifically for this purpose, then stay tuned... 
*Or if you’d rather skip the training and export work, you can also get a TensorVox-ready model directly from me. (see contact details at the bottom of this)*

As for languages, out-of-the-box support is provided for English, German and Spanish (only TensorFlowTTS); that is, you won't have to do anything. You can add languages without modifying code, as long as the phoneme set are IPA (stressed or nonstressed), ARPA, or GlobalPhone, (open an issue and I'll explain it to you)

## Backends
TensorVox currently supports multiple inference backends.

LibTorch (TorchScript) and TensorFlow backends are maintained for compatibility with older models and projects created before ONNX export was refined enough.

New development and active support are focused on ONNX Runtime, with DirectML used for GPU acceleration on Windows. This backend provides the best portability, long-term stability, and hardware coverage.


## Build instructions
Currently, only Windows 10 x64 (although I've heard reports of it running on 8.1) is supported.

**Requirements:**
 1. Qt Creator
 2. MSVC 2017 (v141) compiler

**Primed build (with all provided libraries):**

 1. Download [precompiled binary dependencies and includes](https://drive.google.com/file/d/1N6IxSpsgemS94z_v82toXhiNs2tLXkz6/view?usp=sharing)
 2. Unzip it so that the `deps` folder is in the same place as the .pro and main source files.
 3. Open the project with Qt Creator, add your compiler and compile

Note that to try your shiny new executable you'll need to download a release of program as described above and replace the executable in that release with your new one, so you have all the DLLs in place.

TODO: Add instructions for compile from scratch.

## Externals (and thanks)

 - **ONNX Runtime** :https://onnxruntime.ai/

 - **Tensorflow C API**: [https://www.tensorflow.org/install/lang_c](https://www.tensorflow.org/install/lang_c)
 - **CppFlow** (TF C API -> C++ wrapper): [https://github.com/serizba/cppflow](https://github.com/serizba/cppflow) 
 - **AudioFile** (for WAV export): [https://github.com/adamstark/AudioFile](https://github.com/adamstark/AudioFile)
 - **Frameless Dark Style Window**: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
 - **JSON for modern C++**: https://github.com/nlohmann/json
 - **r8brain-free-src** (Resampling): https://github.com/avaneev/r8brain-free-src
 - **rnnoise** (CMake version, denoising output): https://github.com/almogh52/rnnoise-cmake
 - **Logitech LED Illumination SDK** (Mouse RGB integration): https://www.logitechg.com/en-us/innovation/developer-lab.html
 - **QCustomPlot** : https://www.qcustomplot.com/index.php/introduction
 - **libnumbertext** : https://github.com/Numbertext/libnumbertext


## Contact
You can open an issue here or join the [Discord server](https://discord.gg/B9fGwXgz) and discuss/ask anything there

Custom model training, fine-tuning, and compatible exports are available on request (not free). Use email or DM me on Xitter

Follow me on X (formerly Twitter): [ZD1908 (@ZDi____) / X](https://x.com/ZDi____)
For any formal inquiries, send to this email: nika109021@gmail.com

## Note about licensing

This program itself is MIT licensed, but for the models you use, their license terms apply. For example, if you're in Vietnam and using TensorFlowTTS models, you'll have to check [here](https://github.com/TensorSpeech/TensorFlowTTS#license) for some details
