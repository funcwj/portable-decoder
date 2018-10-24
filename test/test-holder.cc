// wujian@2018

#include "decoder/holder.h"

int main(int argc, char const *argv[]) {
  struct Token {
    Int32 key;
    Float32 cost;
  };

  Holder<Token> token_holder;
  const Int32 num_object = 100;
  std::vector<Token *> to_free;
  for (Int32 i = 0; i < num_object; i++) {
    Token *tok = token_holder.New();
    tok->cost = -1;
    if (i % 2 == 0) to_free.push_back(tok);
  }
  for (Int32 i = 0; i < to_free.size(); i++) token_holder.Free(to_free[i]);
  return 0;
}
