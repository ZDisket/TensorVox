# TensorVox

[![](https://dcbadge.vercel.app/api/server/yqFDAWH)](https://discord.gg/yqFDAWH)

TensorVox is an application designed to enable user-friendly and lightweight neural speech synthesis in the desktop, aimed at increasing accessibility to such technology. 

Powered mainly by [TensorFlowTTS](https://github.com/TensorSpeech/TensorFlowTTS) and also by [Coqui-TTS](https://github.com/coqui-ai/TTS) and [VITS](https://github.com/jaywalnut310/vits), it is written in pure C++/Qt, using the Tensorflow C API for interacting with Tensorflow models (first two), and LibTorch for PyTorch ones. This way, we can perform inference without having to install gigabytes worth of Python libraries, just a few DLLs.

![Interface with Tac2 model loaded](https://i.imgur.com/wtPzzNh.png)


### Try it out

[Detailed guide in Google Docs](https://docs.google.com/document/d/1OS1kfb19bvpPPkF71Vbak_b735mi7epjUanIfPG671M/edit?usp=sharing)

Grab a copy from the releases, extract the .zip and check [the Google Drive folder](https://drive.google.com/drive/folders/1atUyxBbstKZpMqQEZMdNmRF2AKrlahKy?usp=sharing) for models and installation instructions

If you're interested in using your own model, first you need to train then export it. 


## Supported architectures

TensorVox supports models from three repos:

 - **TensorFlowTTS**: FastSpeech2, Tacotron2, both char and phoneme based and Multi-Band MelGAN. Here's a Colab notebook demonstrating how to export the LJSpeech pretrained, char-based Tacotron2 model: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/1KLqZ1rkD4Enw7zpTgXGL6if7e5s0UeWa?usp=sharing) 
 - **Coqui-TTS:** Tacotron2 (phoneme-based IPA) and Multi-Band MelGAN, after converting from PyTorch to Tensorflow. Here's a notebook showing how to export the LJSpeech DDC model: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/15CdGEAu_-KezV1XxwzVfQiFSm0tveBkC?usp=sharing)
 - **jaywalnut310/VITS:** VITS, which is a fully E2E model. (Stressed IPA as phonemes) Export notebook: [<img src="https://colab.research.google.com/assets/colab-badge.svg">](https://colab.research.google.com/drive/1BSGE5DQYweXBWrwPOmb6CRPUU8H5mBvb?usp=sharing)

Those two examples should provide you with enough guidance to understand what is needed. If you're looking to train a model specifically for this purpose then I recommend TensorFlowTTS, as it is the one with the best support, and VITS, as it's the closest thing to perfect
As for languages, out-of-the-box support is provided for English (Coqui and TFTTS, VITS), German and Spanish (only TensorFlowTTS); that is, you won't have to do anything. You can add languages without modifying code, as long as the phoneme set are IPA (stressed or nonstressed), ARPA, or GlobalPhone, (open an issue and I'll explain it to you)


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

 - **LibTorch**: https://pytorch.org/cppdocs/installing.html

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
You can open an issue here or join the [Discord server](https://discord.gg/yqFDAWH) and discuss/ask anything there

For media/licensing/any other formal stuff inquiries, send to this email: 9yba9c1y@anonaddy.me

## Note about licensing

This program itself is MIT licensed, but for the models you use, their license terms apply. For example, if you're in Vietnam and using TensorFlowTTS models, you'll have to check [here](https://github.com/TensorSpeech/TensorFlowTTS#license) for some details
