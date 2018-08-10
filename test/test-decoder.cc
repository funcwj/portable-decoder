// wujian@2018.8

#include "decoder/decoder.h"

Bool ReadLoglikeInArchive(std::istream &is, std::string *token, Int32 *num_rows, Int32 *num_cols) {
    is >> *token;
    if (is.eof()) return false;
    ASSERT(isspace(is.peek()) && "Expect space after each token");
    is.get();
    ASSERT(is.get() == '\0' && is.get() == 'B' && "Expect binary header(\\0B)");
    ExpectToken(is, "FM");
    ReadBinaryBasicType(is, num_rows);
    ReadBinaryBasicType(is, num_cols);
    LOG_INFO << "Get matrix " << token << ": " << *num_rows << " x " << *num_cols;
    return true;
}

void TestDecode(FasterDecoder &decoder, Float32 *loglikes, Int32 num_frames, Int32 num_pdfs) {
    decoder.InitDecoding();
    std::vector<Int32> word_ids;
    for (Int32 t = 0; t < num_frames; t++) {
        decoder.DecodeFrame(loglikes + t * num_pdfs, num_pdfs);
    }
    ASSERT(decoder.NumDecodedFrames() == num_frames);
    decoder.GetBestPath(&word_ids);
    for (Int32 olabel: word_ids)
        std::cout << olabel << " ";
    std::cout << std::endl;
    decoder.FinalizeDecoding();
}

// ../bin/test-decoder 2>/dev/null | ./int2sym.pl -f 2- words.txt | sort -k1 > 50.asr
int main(int argc, char const *argv[]) {
    TransitionTable table;
    ReadTransitionTable("trans.tab", &table);
    SimpleFst fst;
    ReadSimpleFst("graph.fst", &fst);
    FasterDecoder decoder(fst, table);

    BinaryInput bo("50.ark");
    Int32 count = 0, num_frames, num_pdfs;
    std::string token;
    while (true) {
        token.clear();
        Bool go = ReadLoglikeInArchive(bo.Stream(), &token, &num_frames, &num_pdfs);
        if (!go) break;
        Float32 *loglikes = new Float32[num_frames * num_pdfs];
        ReadBinary(bo.Stream(), reinterpret_cast<char*>(loglikes), sizeof(Float32) * num_frames * num_pdfs);
        std::cout << token << " ";
        TestDecode(decoder, loglikes, num_frames, num_pdfs);
        count++;
        delete[] loglikes;
    }
    return 0;
}
