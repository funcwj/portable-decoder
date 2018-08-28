#!/usr/bin/env python

# wujian@2018

from _pydecoder import PyDecoder
from _pydecoder import PyFeatureExtractor


class Decoder(object):
    def __init__(self,
                 graph="graph.fst",
                 table="trans.tab",
                 decode_conf="decode.conf",
                 words="words.txt"):
        self.decoder = PyDecoder(graph, table, decode_conf)
        self.symb = self._cache_sym(words)

    def _cache_sym(self, fname):
        symb = []
        with open(fname, "r") as f:
            for line in f:
                tokens = line.split()
                if len(tokens) != 2:
                    raise RuntimeError("Format error: {}".format(line))
                symb.append(tokens[0])
        return symb

    def decode(self, loglikes):
        self.decoder.reset()
        self.decoder.decode(loglikes)
        best_sequence = self.decoder.best_sequence()
        word_sequence = [self.symb[idx] for idx in best_sequence]
        return " ".join(word_sequence)


class LogLikeDecoder(Decoder):
    def __init__(self,
                 graph="graph.fst",
                 table="trans.tab",
                 decode_conf="decode.conf",
                 words="words.txt"):
        super(LogLikeDecoder, self).__init__(
            graph=graph, table=table, decode_conf=decode_conf, words=words)

