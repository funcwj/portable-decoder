// wujian@2018

#include "io.h"


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
    char c = is.peek();
    ASSERT(c == sizeof(*t));
    is.get(); 
    ReadBinary(is, reinterpret_cast<char*>(t), sizeof(*t));
}

template<class T> 
void WriteBinaryBasicType(std::ostream &os, T t) {
    char c = sizeof(t); 
    os.put(c); 
    WriteBinary(os, reinterpret_cast<const char*>(&t), sizeof(t));
}


bool IsToken(const char *token) {
    if (token == NULL || *token == END_OF_STRING)
        return false;
    const char *token_ptr = token;
    while (token_ptr != END_OF_STRING) {
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

void ExpectToken(std::istream &is, const char *token) {
    ASSERT(IsToken(token));
    std::string read_token;
    is >> read_token; is.get();
    ASSERT(!is.fail() && "ReadToken Failed!");
    if (strcmp(read_token.c_str(), token) != 0)
        LOG_ERR << "Expect token \'" << token << "\', but got " << read_token;
}

void ExpectToken(std::istream &is, const std::string& token) {
    ASSERT(IsToken(token.c_str()));
    std::string read_token;
    is >> read_token; is.get();
    ASSERT(!is.fail() && "ReadToken Failed!");
    if (strcmp(read_token.c_str(), token.c_str()) != 0)
        LOG_ERR << "Expect token \'" << token << "\', but got " << read_token;
}

template
void ReadBinaryBasicType<Int32>(std::istream &is, Int32 *t);

template
void ReadBinaryBasicType<Float32>(std::istream &is, Float32 *t);

template
void WriteBinaryBasicType<Int32>(std::ostream &os, Int32 t);

template
void WriteBinaryBasicType<Float32>(std::ostream &os, Float32 t);
