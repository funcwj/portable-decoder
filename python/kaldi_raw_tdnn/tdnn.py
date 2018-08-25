#!/usr/bin/env python

# wujian@2018

import operator
import os

import torch as th
import torch.nn.functional as F

from .iobase import read_common_mat, read_common_float_vec, read_token, read_ark

debug = False


def splice_feats(x, context):
    T, D = x.shape
    if not T:
        raise ValueError("Could not splice for empty features")
    C = (context[0] + 1 + context[1])
    s = th.zeros(T, D * C, dtype=x.dtype)
    for t in range(T):
        for c in range(C):
            idx = min(max(t + c - context[0], 0), T - 1)
            s[t, c * D:c * D + D] = x[idx]
    return s


class Linear(object):
    def __init__(self, weight, bias):
        self.weight = weight
        self.bias = bias
        self.output_dim, self.input_dim = weight.shape

    def __call__(self, x):
        if x.shape[-1] != self.input_dim:
            raise ValueError("Input feature dim mismatch, {:d} vs {:d}".format(
                x.shape[-1], self.input_dim))
        # x * W^T + b
        # same as kaldi
        return F.linear(x, self.weight, self.bias)


class BatchNorm1D(object):
    def __init__(self, scale, offset):
        self.scale, self.offset = scale, offset
        self.dim = scale.shape[0]

    def __call__(self, x):
        input_dim = x.shape[-1]
        if input_dim != self.dim:
            raise ValueError("Input feature dim mismatch: {:d} vs {:d}".format(
                input_dim, self.dim))
        # same as kaldi
        return x * self.scale + self.offset


class NonLinear(object):
    def __init__(self, token):
        supported_nonlinear = {
            "ReLU": F.relu,
            "Sigmoid": F.sigmoid,
            "Tanh": F.tanh,
        }
        if token not in supported_nonlinear:
            raise TypeError("Unsupport non-linear type: {}".format(token))
        self.f = supported_nonlinear[token]

    def __call__(self, x):
        return self.f(x)


class LogSoftmax(object):
    def __init__(self):
        pass

    def __call__(self, x):
        if x.dim() != 2:
            dim = x.shape[-1]
            x = x.view(-1, dim)
        return F.log_softmax(x, dim=-1)


class TDNN(object):
    def __init__(self, tdnn_conf="", param="final.param", feature_dim=40):
        self.context = self._parse_tdnn_config(tdnn_conf)
        if debug:
            print("Parsed context: {}".format(self.context))
        self.component = self.from_param(param)
        self.check()
        self.feature_dim = feature_dim

    def check(self):
        num_layers = len(self.context)
        linear = [c for c in self.component if isinstance(c, Linear)]
        if (len(linear) != num_layers):
            raise RuntimeError(
                "TDNN configure do not match with parameters, please check")
        for index in range(1, num_layers):
            check_input_dim = len(
                self.context[index]) * linear[index - 1].output_dim
            if linear[index].input_dim != check_input_dim:
                raise RuntimeError(
                    "Check input dim for {:d}th Linear failed: expected {:d}, get {:d}".
                    format(index, check_input_dim, linear[index].input_dim))
        return

    def compute_context(self):
        left_context = right_context = 0
        for c in self.context:
            left_context += c[0]
            right_context += c[-1]
        return (-left_context, right_context)

    def make_context(self, x, feature_dim, context):
        """
            x: shape of [N x C x D]
            feature_dim: D
            context: sample context
        """
        context_width = -context[0] + context[-1] + 1
        batch_size, num_frames, feature_dim = x.shape
        subsamp_frames = num_frames - context_width + 1
        if debug:
            print("Reduce {:d} frames to {:d} frames".format(
                num_frames, subsamp_frames))
        samp_size = len(context)
        if samp_size == 1:
            return x
        y = th.zeros(
            [batch_size, subsamp_frames, samp_size * feature_dim],
            dtype=th.float32)

        context_base = [c - context[0] for c in context]
        for t in range(subsamp_frames):
            for i, c in enumerate(context_base):
                assert (t + c <= num_frames)
                y[:, t, i * feature_dim:(i + 1) * feature_dim] = x[:, t + c, :]
        return y

    def compute_output(self, feats):
        if not isinstance(feats, th.FloatTensor):
            feats = th.tensor(feats, dtype=th.float32)
        tdnn_ctx = self.compute_context()
        tdnn_in = splice_feats(feats, tdnn_ctx)
        loglikes = self(tdnn_in)
        return loglikes.numpy()

    def __call__(self, x):
        ctx = self.compute_context()
        if x.shape[-1] != (sum(ctx) + 1) * self.feature_dim:
            raise ValueError(
                "Input dim mismatch: expected {:d}, get {:d}".format(
                    (sum(ctx) + 1) * self.feature_dim, x.shape[-1]))
        linear_context_index = 0
        linear = [c for c in self.component if isinstance(c, Linear)]
        batch_size = x.shape[0]
        x = x.view(batch_size, -1, self.feature_dim)
        for c in self.component:
            if isinstance(c, Linear):
                # 2D/3D
                input_context = self.make_context(
                    x, self.feature_dim if linear_context_index == 0 else
                    linear[linear_context_index - 1].output_dim,
                    self.context[linear_context_index])
                linear_context_index += 1
                x = c(input_context)
            else:
                x = c(x)
        return th.squeeze(x)

    def _parse_tdnn_config(self, conf):
        tokens = conf.split(";")
        context = []
        for token in tokens:
            ctx = list(map(int, token.split(",")))
            if not operator.eq(ctx, sorted(ctx)):
                raise ValueError(
                    "Context configure in TDNN must in ascend order: {}".
                    format(token))
            context.append(ctx)
        return context

    def from_param(self, param):
        def to_tensor(x):
            return th.tensor(x, dtype=th.float32)

        if not os.path.exists(param):
            raise FileNotFoundError(
                "Not found TDNN's parameter file: {}".format(param))

        component = []
        with open(param, "rb") as fd:
            while True:
                token = read_token(fd)
                if not token:
                    break
                if token == "Linear":
                    weight = read_common_mat(fd)
                    bias = read_common_float_vec(fd)
                    if debug:
                        print(
                            "(Linear) weight: {:d} x {:d}, bias: {:d}".format(
                                weight.shape[0], weight.shape[1], bias.size))
                    component.append(
                        Linear(to_tensor(weight), to_tensor(bias)))
                elif token == "BatchNorm":
                    scale = read_common_float_vec(fd)
                    offset = read_common_float_vec(fd)
                    if debug:
                        print("(BatchNorm) scale: {:d}, offset: {:d}".format(
                            scale.size, offset.size))
                    component.append(
                        BatchNorm1D(to_tensor(scale), to_tensor(offset)))
                elif token == "ReLU":
                    if debug:
                        print("(ReLU)")
                    component.append(NonLinear("ReLU"))
                elif token == "LogSoftmax":
                    if debug:
                        print("LogSoftmax")
                    component.append(LogSoftmax())
                else:
                    print("Unknown token: {}".format(token))
        return component


def run_demo():
    tdnn_conf = "-2,-1,0,1,2;0;-1,0,2;-3,0,3;-7,0,2;-3,0,3;0;0"
    param = "596.param"
    feature_dim = 40
    tdnn = TDNN(tdnn_conf, param, feature_dim=feature_dim)
    x = th.rand(100, (sum(tdnn.compute_context()) + 1) * feature_dim)
    print(tdnn(x).shape)


if __name__ == "__main__":
    run_demo()
