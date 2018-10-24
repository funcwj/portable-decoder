// wujian@2018

#include "decoder/io.h"

Bool ReadMatrixInArchive(std::istream &is, Int32 *num_rows, Int32 *num_cols) {
  std::string token;
  is >> token;
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

int main(int argc, char const *argv[]) {
  BinaryInput bo("50.ark");
  Int32 count = 0, num_rows, num_cols;
  while (true) {
    Bool go = ReadMatrixInArchive(bo.Stream(), &num_rows, &num_cols);
    if (!go) break;
    LOG_INFO << "Read " << count << "-th...";
    Float32 *loglikes = new Float32[num_cols * num_cols];
    ReadBinary(bo.Stream(), reinterpret_cast<char *>(loglikes),
               sizeof(Float32) * num_cols * num_rows);
    count++;
    delete[] loglikes;
  }
  return 0;
}
