// wujian@2018.8

#include "decoder/decoder.h"

const Int32 traceback_interval = 400;   // 4s

Bool ReadLoglikeInArchive(std::istream &is, std::string *token, Int32 *num_rows, Int32 *num_cols) {
    is >> *token;
    if (is.eof()) return false;
    ASSERT(isspace(is.peek()) && "Expect space after each token");
    is.get();
    ASSERT(is.get() == '\0' && is.get() == 'B' && "Expect binary header(\\0B)");
    ExpectToken(is, "FM");
    ReadBinaryBasicType(is, num_rows);
    ReadBinaryBasicType(is, num_cols);
    LOG_INFO << "Get matrix " << *token << ": " << *num_rows << " x " << *num_cols;
    return true;
}

void TestOfflineDecode(FasterDecoder &decoder, Float32 *loglikes, Int32 num_frames, 
                        Int32 num_pdfs, std::vector<Int32> *word_ids) {
    word_ids->clear();
    decoder.Reset();
    decoder.Decode(loglikes, num_frames, num_pdfs, num_pdfs);
    // for (Int32 t = 0; t < num_frames; t++)
    // decoder.DecodeFrame(loglikes + t * num_pdfs, num_pdfs);
    ASSERT(decoder.NumDecodedFrames() == num_frames);
    decoder.GetBestPath(word_ids);
}

void TestOnlineDecode(FasterDecoder &decoder, Float32 *loglikes, Int32 num_frames, 
                        Int32 num_pdfs, std::vector<Int32> *word_ids) {
    word_ids->clear();
    decoder.Reset();
    Int32 decode_frames = 0, num_chunks = 0;
    for (Int32 t = 0; t < num_frames; t++) {
        decoder.DecodeFrame(loglikes + t * num_pdfs, num_pdfs);
        if (t % traceback_interval == 0) {
            decoder.GetBestPath(word_ids);
            decode_frames += decoder.NumDecodedFrames(), num_chunks += 1;
            decoder.Reset();
        }
    }
    decoder.GetBestPath(word_ids);
    decode_frames += decoder.NumDecodedFrames();
    ASSERT(decode_frames == num_frames);
    LOG_INFO << "Decode in " << num_chunks << " chunks(" << traceback_interval 
             << " frames per chunk, total " << num_frames << " frames";
}

// Run command:
// ../bin/test-decoder 2>/dev/null | ./int2sym.pl -f 2- words.txt | ./wer_output_filter | sort -k1 > 50.asr
int main(int argc, char const *argv[]) {
    TransitionTable table;
    ReadTransitionTable("trans.tab", &table);
    SimpleFst fst;
    ReadSimpleFst("graph.fst", &fst);
    DecodeOpts opts("decode.conf");
    std::cerr << "Decode options: \n" << opts.Configure();
    FasterDecoder decoder(fst, table, opts);

    BinaryInput bo("posts.ref.ark");
    Int32 count = 0, num_frames, num_pdfs;
    std::string utt_id;
    std::vector<Float32> loglikes;
    std::vector<Int32> word_ids;
    
    while (true) {
        utt_id.clear();
        Bool go = ReadLoglikeInArchive(bo.Stream(), &utt_id, &num_frames, &num_pdfs);
        if (!go) break;
        loglikes.resize(num_frames * num_pdfs);
        // Float32 *loglikes = new Float32[num_frames * num_pdfs];
        ReadBinary(bo.Stream(), reinterpret_cast<char*>(loglikes.data()), sizeof(Float32) * loglikes.size());
        Timer timer;
        TestOfflineDecode(decoder, loglikes.data(), num_frames, num_pdfs, &word_ids);
        LOG_INFO << "Decode utterance(offline) " << utt_id << ": cost " << timer.Elapsed() << "s";
        for (Int32 i = 0; i < word_ids.size(); i++)
            std::cout << (i == 0 ? utt_id : "") << " " << word_ids[i] << (i == word_ids.size() - 1 ? "\n": "");
        timer.Reset();
        TestOnlineDecode(decoder, loglikes.data(), num_frames, num_pdfs, &word_ids);
        LOG_INFO << "Decode utterance(online)  " << utt_id << ": cost " << timer.Elapsed() << "s";
        for (Int32 i = 0; i < word_ids.size(); i++)
            std::cout << (i == 0 ? utt_id : "") << " " << word_ids[i] << (i == word_ids.size() - 1 ? "\n": "");
        count++;
        // delete[] loglikes;
    }
    return 0;
}
