// decoder/io.h
// wujian@2018

// IO interface

#ifndef IO_H
#define IO_H

#include <fstream>
#include <iostream>

#include "decoder/type.h"

class BinaryInput {
 public:
  BinaryInput(const std::string &filename) : filename_(filename) {
    is_.open(filename_.c_str(), std::ios::in | std::ios::binary);
    if (!is_.is_open()) LOG_FAIL << "Open " << filename_ << " failed";
  }

  ~BinaryInput() {
    if (is_.is_open()) is_.close();
  }

  std::istream &Stream() { return is_; }

 private:
  std::string filename_;
  std::ifstream is_;
};

class BinaryOutput {
 public:
  BinaryOutput(const std::string &filename) : filename_(filename) {
    os_.open(filename_.c_str(), std::ios::out | std::ios::binary);
    if (!os_.is_open()) LOG_FAIL << "Open " << filename_ << " failed";
  }

  ~BinaryOutput() {
    if (os_.is_open()) os_.close();
  }

  std::ostream &Stream() { return os_; }

 private:
  std::string filename_;
  std::ofstream os_;
};

void WriteBinary(std::ostream &os, const char *ptr, Int32 num_bytes);

void ReadBinary(std::istream &is, char *ptr, Int32 num_bytes);

template <class T>
void ReadBinaryBasicType(std::istream &is, T *t);

template <class T>
void WriteBinaryBasicType(std::ostream &os, T t);

void WriteToken(std::ostream &os, const char *token);

void ReadToken(std::istream &is, std::string *token);

void WriteToken(std::ostream &os, const std::string &token);

void ExpectToken(std::istream &is, const char *token);

void ExpectToken(std::istream &is, const std::string &token);

void Seek(std::istream &is, Int64 off, std::ios_base::seekdir way);

void Seek(std::ostream &os, Int64 off, std::ios_base::seekdir way);

#endif