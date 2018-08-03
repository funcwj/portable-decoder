// wujian@2018

// First implemented in https://github.com/funcwj/asr-utils/blob/master/wave/wave.h

#ifndef WAVE_H
#define WAVE_H


#include "type.h"
#include "math.h"
#include "io.h"

struct WaveHeader {
    char chunk_id[4];       // "RIFF"
    UInt32 chunk_size;      // total bit size exclude chunk_id & chunk_size
    char format[4];         // "WAVE"
    char format_id[4];      // "fmt "
    UInt32 format_size;     // format header total bit size exclude format_id & format_size
    UInt16 audio_format;    // 1
    UInt16 num_channels;    // 1/2/..
    UInt32 sample_rate;     // 16000
    UInt32 byte_rate;       // sample_rate * num_channel * sizeof(T)
    UInt16 block_align;     // num_channels * sizeof(T)
    UInt16 bits_per_sample; // sizeof(T) * 8
    // ...data part
};

bool CheckHeader(WaveHeader &header, Int32 byte_per_sample);

WaveHeader GenWavHeader(Int32 num_samples, Int32 num_channels, 
                        Int32 sample_rate, Int32 byte_per_sample);

class Wave {
public:
    // egs:
    // Wave wave;
    // ReadWave("demo.wav", &wave);
    Wave(): data_(NULL), num_channels_(-1), num_samples_(-1), 
        sample_rate_(-1), byte_per_sample_(-1) { }

    // egs:
    // Wave wave(data, 23456);
    // WriteWave("demo.wav", wave);
    Wave(Float32 *data, Int32 num_samples, Int32 num_channels = 1, 
        Int32 sample_rate = 16000): data_(data), num_samples_(num_samples),
        num_channels_(num_channels), sample_rate_(sample_rate) {
        // default Int16
        byte_per_sample_ = 2;
        header_ = GenWavHeader(num_samples, num_channels, sample_rate, byte_per_sample_);
    }

    ~Wave() { 
        if (data_)
            delete[] data_;
    }

    void Read(std::istream &is);

    void Write(std::ostream &os);

    Int32 NumChannels() { return num_channels_; }

    Int32 NumSamples() { return num_samples_; }

    Int32 SampleRate() { return sample_rate_; }

    Float32 *Data() { return data_; }

private:
    Int32 num_channels_, num_samples_, sample_rate_;
    Int32 byte_per_sample_;
    Float32 *data_;
    WaveHeader header_;

};


void ReadWave(std::string filename, Wave *wave);

void WriteWave(std::string filename, Wave &wave);


#endif 