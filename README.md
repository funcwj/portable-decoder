# Simple ASR decoder

What I want to do is to give a simple static decoder without dependency on Kaldi & OpenFST, which could be deployed on both mobile device or cloud server.

The decoder now I use here comes from Kaldi's faster-decoder, which maintains one-best tokens. 

I want to make it framework insensitive because acoustic models now could be trained with kinds of tools, such as Kaldi, pytorch, tensorflow, caffe2, etc. Deploying models requires us to implement inference of network based on the platform and training tools first(I think it is not very diffcult, using feature interface here to extract acoustic features, feeding them into neural networks, and finally decoding from network's output log-likelihoods).

Many things to do with above.

### Compile & Install
```bash
# for test compile
ln -s /where/eigen/located third_party/eigen
mkdir build
cd build
# for kaldi-tools compile
export KALDI_ROOT=...
cmake ..
make -j 10
cd ../python
# check cython/numpy is installed
./install.sh
```
