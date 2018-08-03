// wujian@2018

#include "wave.h"

bool CheckHeader(WaveHeader &header, Int32 byte_per_sample) {
    if ((strncmp(reinterpret_cast<char*>(header.chunk_id), "RIFF", 4) != 0)
        || (strncmp(reinterpret_cast<char*>(header.format), "WAVE", 4) != 0)
        || (strncmp(reinterpret_cast<char*>(header.format_id), "fmt ", 4) != 0))
        return false;
    if ((header.byte_rate != header.sample_rate * header.num_channels * byte_per_sample)
        || (header.block_align != header.num_channels * byte_per_sample) 
        || (header.format_size - 16 < 0))
        return false;
    return true;
}

WaveHeader GenWavHeader(Int32 num_samples, Int32 num_channels, 
                        Int32 sample_rate, Int32 byte_per_sample) {
    UInt32 num_bytes  = num_samples * byte_per_sample;
    WaveHeader header = {
        'R', 'I', 'F', 'F',
        static_cast<UInt32>(num_bytes + sizeof(WaveHeader)),
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        16, 1, static_cast<UInt16>(num_channels),
        static_cast<UInt32>(sample_rate),
        static_cast<UInt32>(sample_rate * num_channels * byte_per_sample),
        static_cast<UInt16>(num_channels * byte_per_sample),
        static_cast<UInt16>(byte_per_sample * 8)
    };
    if (!CheckHeader(header, byte_per_sample))
        LOG_ERR << "Check generated wave header failed, please check GenWavHeader()";
    return header;
}

void Wave::Read(std::istream &is) {
    ReadBinary(is, reinterpret_cast<char*>(&header_), sizeof(header_));
    byte_per_sample_ = header_.bits_per_sample / 8;
    if(!CheckHeader(header_, byte_per_sample_))
        LOG_ERR << "Check wave header failed";
    // Skip other parameters between format part and data part
    is.seekg(header_.format_size - 16, std::ios::cur);
    char data_id[4];
    ReadBinary(is, data_id, sizeof(data_id));
    ASSERT(strncmp(data_id, "data", sizeof(data_id)) == 0);
    UInt32 num_bytes;
    ReadBinary(is, reinterpret_cast<char*>(&num_bytes), sizeof(num_bytes));
    num_samples_ = num_bytes / byte_per_sample_;
    // check num_samples_ here
    data_ = new Float32[num_samples_];
    if (byte_per_sample_ != 2)
        LOG_ERR << "Now only support for Int16 encode, get int" << header_.bits_per_sample;
    Int16 *cache = new Int16[num_samples_];
    ReadBinary(is, reinterpret_cast<char*>(cache), sizeof(Int16) * num_samples_);
    for (Int32 i = 0; i < num_samples_; i++)
        data_[i] = static_cast<Float32>(cache[i]);
    delete[] cache;
    num_channels_ = header_.num_channels;
    sample_rate_ = header_.sample_rate;
    ASSERT(header_.chunk_size == header_.format_size + 20 + num_bytes);
}


void Wave::Write(std::ostream &os) {
    // check header
    if(!CheckHeader(header_, byte_per_sample_))
        LOG_ERR << "Check wave header failed, could not dump to disk";
    Int32 num_bytes  = num_samples_ * byte_per_sample_;
    const char *data_id = "data";
    WriteBinary(os, reinterpret_cast<char*>(&header_), sizeof(header_));
    WriteBinary(os, data_id, 4);
    WriteBinary(os, reinterpret_cast<char*>(&num_bytes), sizeof(num_bytes));
    Int32 num_clipped = 0;
    for (Int32 n = 0; n < num_samples_; n++) {
        Int32 sample32 = static_cast<Int32>(truncf(data_[n]));
        Int16 sample16 = static_cast<Int16>(sample32);
        if (sample16 < MIN_INT16 || sample16 > MAX_INT16)
            num_clipped++;
        sample16 = std::max(std::min(sample16, MAX_INT16), MIN_INT16);
        WriteBinary(os, reinterpret_cast<char*>(&sample16), sizeof(sample16));
    }
    if (num_clipped)
        LOG_INFO << "Clipped " << num_clipped << " samples, total " << num_samples_;
}


void ReadWave(std::string filename, Wave *wave) {
    std::fstream fs(filename.c_str(), std::ios::binary | std::ios::in);
    ASSERT(wave);
    wave->Read(fs); fs.close();
}


void WriteWave(std::string filename, Wave &wave) {
    std::fstream fs(filename.c_str(), std::ios::binary | std::ios::out);
    wave.Write(fs); fs.close();
}