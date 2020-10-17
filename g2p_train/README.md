#  G2P for TensorVox
TensorVox utilizes an RNN-based G2P model implemented in Tensorflow to convert text to phonemes before feeding the text2speech models.

## Training
In order to train a model, you need to prepare two things:
1. A dictionary in format `WORD \t PHONETIC SPELLING` as the dataset
2. A config file (optional, there is already one in `config/default.yaml`)

Tensorflow 2.0 or greater, is of course, required.

Since the training is very quick on GPU (Tesla T4), it's just one script that does preprocessing, training, and exporting. If you don't have one, just use Google Colab.

You can download my English dictionary (converted to tab-based from the LibriSpeech lexicon) [here](https://drive.google.com/file/d/19cnHM3-Zsc7uRJ2scUPNMNoSlyXuaGNe/view?usp=sharing).
Rename it from dict.d to dict.txt

The command to run it is as follows: 

    python3 train_and_export.py --dict-path dict.txt --config-path config/default.yaml --out-path English

Arguments should be self-explanatory.

## Structure

Once finished, the script will output all files required to use the model to the folder determined by the `--out-path` argument  (will be created if it doesn't exist). 

No further action is necessary, just drag it into  (executable file path)/g2p/`language name` and it will be used by the program to do phoneme conversion for all models it loads in that language. Make sure language name folder is capitalized.

The output consists of three things:

4. **char2id.txt, phn2id.txt**: Two text files in format TOKEN \t ID that indicate the IDs that first go into the model (char) and are returned (phn)
5. **dict.txt**: Dictionary in format WORD \t PHONETIC-SPELLING that is used to find phonetic spellings in
6. **model**: The actual G2P model, saved in Tensorflow SavedModel format.

Due to the unreliability of the network, we only want to use it to guess novel words, so first it does a dictionary lookup (semi-optimized with bucketed string search) then if not found, uses the model.

An example English model is zipped in the `models/` directory.
