// wujian@2018

#ifndef IO_H
#define IO_H

#include <iostream>
#include <fstream>

#include "decoder/type.h"


void WriteBinary(std::ostream &os, const char *ptr, Int32 num_bytes);

void ReadBinary(std::istream &is, char *ptr, Int32 num_bytes);


template<class T> 
void ReadBinaryBasicType(std::istream &is, T *t);


template<class T> 
void WriteBinaryBasicType(std::ostream &os, T t);


void WriteToken(std::ostream &os, const char *token);

void WriteToken(std::ostream &os, const std::string &token);

void ExpectToken(std::istream &is, const char *token);

void ExpectToken(std::istream &is, const std::string& token);



#endif 