# Simple ASR decoder

What I want to do is to give a simple static decoder without dependency on Kaldi & OpenFST, which could be deployed on both mobile device or cloud server.

The decoder now I use here comes from Kaldi's faster-decoder, which maintains one-best tokens. 

I want to make it framework-insensitive because acoustic models now could be trained with kinds of tools, such as Kaldi, pytorch, tensorflow, caffe2. Before deploying models, you should implement inference of network based on your platform and training tools(This is not diffcult, using feature interface here to extract acoustic features, feeding them into neural networks, and finally decoding from log-likelihoods).

