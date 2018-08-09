// wujian@2018

#include "decoder/common.h"

class TransitionTable {
public:
    TransitionTable(): num_tids_(-1), num_pdfs_(-1), table_(NULL) { }

    ~TransitionTable() {
        if (table_)
            delete[] table_;
    }

    void Read(std::istream &is) {
        ReadBinaryBasicType(is, &num_tids_);
        ReadBinaryBasicType(is, &num_pdfs_);
        table_ = new Int32[num_tids_];
        ReadBinary(is, reinterpret_cast<char*>(table_), sizeof(Int32) * num_tids_);
    }

    void Write(std::ostream &os) {
        WriteBinaryBasicType(os, num_tids_);
        WriteBinaryBasicType(os, num_pdfs_);
        WriteBinary(os, reinterpret_cast<const char*>(table_), sizeof(Int32) * num_tids_);
    }

    Int32 TransitionIdToPdf(Int32 tid) {
        ASSERT(tid < num_tids_ && "Index out of total number of transition ids");
        return table_[tid];
    }

    Int32 NumTransitionIds() { return num_tids_; }

    Int32 NumPdfs() { return num_pdfs_; }

private:
    Int32 num_tids_, num_pdfs_;
    // index by transition-id, mapping to pdfid
    Int32 *table_;
};

void ReadTransitionTable(const std::string &filename, TransitionTable *table) {
    BinaryInput bi(filename);
    ASSERT(table);
    table->Read(bi.Stream());
}