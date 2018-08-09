// wujian@2018

#include "decoder/io.h"
#include "decoder/transition-table.h"


int main(int argc, char const *argv[]) {
    TransitionTable table;
    ReadTransitionTable("tid.map", &table);
    LOG_INFO << "Read TransitionTable, num_tids = " << table.NumTransitionIds() 
            << ", num_pdfs = " << table.NumPdfs();
    return 0;
}

