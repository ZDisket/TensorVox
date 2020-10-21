from tqdm import tqdm
import os
import argparse
import tensorflow as tf
import yaml
import shutil
global_max = 0
cumodel = None

def safemkdir(dirn):
  if not os.path.isdir(dirn):
    os.mkdir(dirn)


def preprocess(in_fname):
  words = list()
  phn = list()
  print("Opening file...")
  with open(in_fname,"r",encoding="utf-8") as f:
    for li in tqdm(f.readlines()):
      spl = li.strip().split("\t")
      if len(spl) > 1:
        words.append(spl[0].lower()) #convert to lowercase for re-exporting later
        phn.append(spl[1])

  phntok = tf.keras.preprocessing.text.Tokenizer(lower=False)
  txttok = tf.keras.preprocessing.text.Tokenizer(char_level=True)
  
  print("Fitting on texts...")
  phntok.fit_on_texts(phn)
  txttok.fit_on_texts(words)

  print("Converting to sequences")
  txtseqs = txttok.texts_to_sequences(words)
  phnseqs = phntok.texts_to_sequences(phn)

  txt_max = len(max(txtseqs, key=len))
  phn_max = len(max(phnseqs,key=len))

  global global_max
  global_max = max(txt_max,phn_max)
  print("Common padding index is " + str(global_max))

  txtpadded = tf.keras.preprocessing.sequence.pad_sequences(txtseqs,padding="post",maxlen=global_max)
  phnpadded =  tf.keras.preprocessing.sequence.pad_sequences(phnseqs,padding="post",maxlen=global_max)
  
  txtsize = len(txttok.word_index)
  phnsize = len(phntok.word_index)

  return txtpadded, phnpadded, txtsize, phnsize, phntok.word_index, txttok.word_index, words, phn


def getmodel(input_shape, in_vocab_size, out_vocab_size,gru_size,in_lr):
    model = tf.keras.models.Sequential([tf.keras.layers.Embedding(in_vocab_size, gru_size, input_length=input_shape[1], input_shape=input_shape[1:]),
                                        tf.keras.layers.Bidirectional(tf.keras.layers.GRU(gru_size,input_shape=input_shape[1:],return_sequences=True)),
                                        tf.keras.layers.TimeDistributed(tf.keras.layers.Dense(1024,activation="relu")),
                                        tf.keras.layers.Dropout(0.5),
                                        tf.keras.layers.TimeDistributed(tf.keras.layers.Dense(out_vocab_size,activation="softmax"))])
    
    model.compile(loss='sparse_categorical_crossentropy',
                  optimizer=tf.keras.optimizers.Adam(in_lr),
                  metrics=['accuracy'])
    return model

@tf.function(
    experimental_relax_shapes=True,
    input_signature=[
        tf.TensorSpec([None], dtype=tf.int32, name="input_ids"),
        tf.TensorSpec([1,], dtype=tf.int32, name="input_len"),
        tf.TensorSpec([1,], dtype=tf.float32, name="input_temperature"),
    ],
)
def callg2p(input_ids,input_len,input_temperature):
  #Generate padding
  pad = tf.zeros([global_max - input_len[0]],dtype=tf.int32)
  #Add padding to input_ids and reshape
  input_ids = tf.concat([input_ids,pad],0)
  input_ids = tf.reshape(input_ids,[-1,global_max])
  #Predict
  pred = cumodel(input_ids)
  #Apply temperature
  predx = tf.squeeze(pred, 0)
  predx /= input_temperature

  #Select IDs
  retids = tf.random.categorical(predx, 1)

  #Remove padding
  bool_mask = tf.not_equal(retids, 0)
  phn_ids = tf.boolean_mask(retids, bool_mask)

  return tf.cast(phn_ids,tf.int32)

def exportdict(indict,outf):
  f = open(outf,"w")
  for de in indict:
    f.write(de + "\t" + str(indict[de]) + "\n")
  
  f.close()


def export_model(folname,in_model,in_phnwi,in_charwi):
  safemkdir(folname)


  exportdict(in_phnwi,os.path.join(folname,"phn2id.txt"))
  exportdict(in_charwi,os.path.join(folname,"char2id.txt"))
  
  print("Exporting model...")
  in_model.save(os.path.join(folname,"model"),save_format="tf",signatures=callg2p)


def main():
    parser = argparse.ArgumentParser(description="Train and export a G2P model")
    parser.add_argument(
        "--config-path",
        default="config/default.yaml",
        type=str,
        help="Path of config",
    )    
    parser.add_argument(
        "--dict-path",
        default="dict.txt",
        type=str,
        help="Path of dictionary",
    )
    parser.add_argument(
        "--out-path",
        default="model1",
        type=str,
        help="Output path of model",
    )

    args = parser.parse_args()
    
    txtpadded, phnpadded, txtsize, phnsize, phn_wi, txt_wi, words, phns = preprocess(args.dict_path)
    
    yf = open(args.config_path,"r")
    config = yaml.load(yf)
    yf.close()

    print("Finished preprocessing. Getting model")
    global cumodel
    cumodel = getmodel(txtpadded.shape,txtsize + 1,phnsize + 1,config["gru_dims"],config["learning_rate"])

    x_train = txtpadded
    y_train = phnpadded

    print("Starting training...")
    cumodel.fit(x_train, y_train, batch_size=config["batch_size"], epochs=config["epochs"],validation_split=config["val_per"])
    
    print("Starting export...")
    export_model(args.out_path,cumodel,phn_wi,txt_wi)

    print("Re-exporting dict...")
    outdict = open(os.path.join(args.out_path,"dict.txt"),"w",encoding="utf-8")

    for idx, w in enumerate(words):
      outdict.write(w + "\t" + phns[idx] + "\n")
    
    outdict.close()





    
    print("Done!")






        

if __name__ == "__main__":
    main()

