// wujian@2018

#include "io.h"

void Seek(std::istream &is, Int64 off, std::ios_base::seekdir way) { is.seekg(off, way); }

void Seek(std::ostream &os, Int64 off, std::ios_base::seekdir way) { os.seekp(off, way); } 

void WriteBinary(std::ostream &os, const char *ptr, Int32 num_bytes) {
    os.write(ptr, num_bytes);
    ASSERT(!os.fail() && "WriteBinary Failed");
}

void ReadBinary(std::istream &is, char *ptr, Int32 num_bytes) {
    is.read(ptr, num_bytes);
    ASSERT(!is.fail() && "ReadBinary Failed");
}

template<class T> 
void ReadBinaryBasicType(std::istream &is, T *t) {
    char num_bytes = is.get();
    if (abs(num_bytes) != sizeof(*t))
        LOG_FAIL << "Expect " << sizeof(*t) << " bytes, but get " << static_cast<Int32>(num_bytes);
    ReadBinary(is, reinterpret_cast<char*>(t), sizeof(*t));
    ASSERT(!is.fail() && "ReadBinaryBasicType Failed");
}

template<class T> 
void WriteBinaryBasicType(std::ostream &os, T t) {
    char num_bytes = sizeof(t);
    os.put(num_bytes); 
    WriteBinary(os, reinterpret_cast<const char*>(&t), sizeof(t));
    ASSERT(!os.fail() && "WriteBinaryBasicType Failed");
}


bool IsToken(const char *token) {
    if (token == NULL || *token == END_OF_STRING)
        return false;
    const char *token_ptr = token;
    while (*token_ptr != END_OF_STRING) {
        if (isspace(*token_ptr)) return false;
        token_ptr++;
    }
    return true;
}

void WriteToken(std::ostream &os, const char *token) {
    ASSERT(IsToken(token));
    os << token << " ";
    ASSERT(!os.fail() && "WriteToken Failed!");
}

void WriteToken(std::ostream &os, const std::string &token) {
    ASSERT(IsToken(token.c_str()));
    os << token << " ";
    ASSERT(!os.fail() && "WriteToken Failed!");
}

void ReadToken(std::istream &is, std::string *token) {
    ASSERT(token);
    is >> *token;
    ASSERT(!is.fail() && "ReadToken Failed!");
    ASSERT(isspace(is.peek()) && "ReadToken: Expect space after each token");
    is.get();
}

void ExpectToken(std::istream &is, const char *token) {
    ASSERT(IsToken(token));
    std::string read_token;
    is >> read_token; is.get();
    ASSERT(!is.fail() && "ExpectToken Failed!");
    if (strcmp(read_token.c_str(), token) != 0)
        LOG_FAIL << "Expect token \'" << token << "\', but got " << read_token;
}

void ExpectToken(std::istream &is, const std::string& token) {
    ASSERT(IsToken(token.c_str()));
    std::string read_token;
    is >> read_token; is.get();
    ASSERT(!is.fail() && "ExpectToken Failed!");
    if (strcmp(read_token.c_str(), token.c_str()) != 0)
        LOG_FAIL << "Expect token \'" << token << "\', but got " << read_token;
}

template
void ReadBinaryBasicType<Int32>(std::istream &is, Int32 *t);

template
void ReadBinaryBasicType<Int64>(std::istream &is, Int64 *t);

template
void ReadBinaryBasicType<UInt64>(std::istream &is, UInt64 *t);

template
void ReadBinaryBasicType<Float32>(std::istream &is, Float32 *t);

template
void WriteBinaryBasicType<Int32>(std::ostream &os, Int32 t);

template
void WriteBinaryBasicType<Int64>(std::ostream &os, Int64 t);

template
void WriteBinaryBasicType<UInt64>(std::ostream &os, UInt64 t);

template
void WriteBinaryBasicType<Float32>(std::ostream &os, Float32 t);
