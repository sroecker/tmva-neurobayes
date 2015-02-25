tmva-neurobayes
===============

NeuroBayes plugin for TMVA

Originally written by Daniel Martschei, Institut für Experimentelle Kernphysik, KIT.

## Usage
Compile with `make` and register the plugin in TMVA:

```
TMVA::Tools::Instance();
gSystem->Load("libTMVANeuroBayes.so");
TMVA::MethodNeuroBayes::RegisterNeuroBayes();
```

Setup your TMVA factory as usual and add the NeuroBayes method:

```
factory->BookMethod( TMVA::Types::kPlugins, "NeuroBayes", "!H:V:NTrainingIter=50:TrainingMethod=BFGS");
```

For more parameters have a look at [DeclareOptions()](src/MethodNeuroBayes.cxx#L145).
