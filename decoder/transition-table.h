// wujian@2018

#include "decoder/common.h"

// This class maintains relation between tid and pdfid
class TransitionTable {
public:
    TransitionTable(): num_tids_(-1), num_pdfs_(-1), table_(NULL) { }

    TransitionTable(const TransitionTable &table): num_tids_(table.NumTransitionIds()),
                    num_pdfs_(table.NumPdfs()) { 
        table_ = new Int32[num_tids_];
        memcpy(table_, table.Table(), sizeof(Int32) * num_tids_);
    }
    
    TransitionTable(const std::string &fname) {
        BinaryInput bi(fname);
        Read(bi.Stream());
    }

    ~TransitionTable() {
        if (table_)
            delete[] table_;
    }

    void Read(std::istream &is) {
        ReadBinaryBasicType(is, &num_tids_);
        ReadBinaryBasicType(is, &num_pdfs_);
        LOG_INFO << "Read " << num_tids_ << " TransitionIds and " << num_pdfs_ << " Pdfs"; 
        table_ = new Int32[num_tids_];
        ReadBinary(is, reinterpret_cast<char*>(table_), sizeof(Int32) * num_tids_);
    }

    void Write(std::ostream &os) {
        WriteBinaryBasicType(os, num_tids_);
        WriteBinaryBasicType(os, num_pdfs_);
        WriteBinary(os, reinterpret_cast<const char*>(table_), sizeof(Int32) * num_tids_);
    }

    Int32 TransitionIdToPdf(Int32 tid) {
        ASSERT(tid >= 1 && "TransitionId could not be negative");
        if (tid > num_tids_) {
            LOG_FAIL << "Index out of total number of transition ids, " 
                     << tid << "/" << num_tids_;
        }
        return table_[tid - 1];
    }

    Int32 NumTransitionIds() const { return num_tids_; }

    Int32 NumPdfs() const { return num_pdfs_; }

    Int32* Table() const { return table_; }

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
