#include <cstdio>
#include <cstring>
#include <algorithm>
#include <parallel_hashmap/phmap.h>

#define LEN 3317972

using namespace std;

__attribute__((const)) constexpr unsigned reflect(unsigned pos) {
  pos = (pos & 0x55555555) << 1 | (pos & 0xaaaaaaaa) >> 1;
  pos = (pos & 0x33333333) << 2 | (pos & 0xcccccccc) >> 2;
  pos = (pos >> 4 | 0xf0f0f0f0) ^ (pos << 4 | 0x0f0f0f0f);
  return __builtin_bswap32(pos);
}

void sort(unsigned* before) {
  unsigned count[256];
  unsigned* after = new unsigned[LEN];
  for(unsigned i = 0; i < 32; i += 8) {
    memset(count, 0, sizeof count);
    unsigned* ptr = before;
    unsigned* end = before + LEN;
    do {
      ++count[*ptr++ >> i & 255];
    } while(ptr != end);
    for(unsigned index = 1; index < 256; ++index) {
      count[index] += count[index - 1];
    }
    do {
      --ptr;
      after[--count[*ptr >> i & 255]] = *ptr;
    } while(ptr != before);
    before = after;
    after = ptr;
  }
  delete[] after;
}

phmap::flat_hash_set<unsigned> ppos(LEN - 2);
unsigned h = 3;
unsigned w = 4;

inline constexpr unsigned inc(const unsigned pos) {
  return pos + (pos & -pos);
}

__attribute__((pure)) bool isWinning(const unsigned pos) {
  unsigned ipos = ~pos ^ 0xffff0000 << h;
  do {
    unsigned body = ipos & -ipos;
    ipos ^= body;
    unsigned tail = pos & body - 1;
    unsigned head = pos ^ tail;
    do {
      head &= head - 1;
      tail |= body;
      if(ppos.contains(head | tail)) {
        return true;
      }
      body += body;
    } while(head);
  } while(ipos);
  return false;
}

int main() {
  unsigned hmin = 0x15fff;
  unsigned pos = 0x2bfff;
  for(unsigned window = 0xf << 13; window >= 0xf; window >>= 1) {
    pos ^= window;
    ppos.insert(pos);
    ppos.insert(reflect(pos));
  }
  ppos.insert(0x4dfff);
  pos = 0x66fff;
  while(true) {
    if(w > h) {
      if(isWinning(pos)) {
        if((pos >> (15 - w) & 7) == 1) {
          w = min(w + 2, 16U);
          pos ^= 0x50000 >> w;
          continue;
        }
        unsigned ripple = inc(pos);
        if(!(ripple & pos)) {
          w = h;
          ++h;
          if(h == 16) {
            break;
          }
          ppos.insert(ripple | (0xffff & (0xfffeffff >> h)));
          pos = ripple | hmin;
          hmin ^= 0x18000 >> w;
          continue;
        }
        pos = ripple;
      } else {
        ppos.insert(pos);
        ppos.insert(reflect(pos));
        pos &= pos + 1;
        pos = inc(pos);
      }
    } else if(w != h && ppos.contains(pos)) {
      pos &= pos + 1;
      pos = inc(pos);
    } else if(!(pos & 0x30000 >> w)) {
      w = min(w + 2, 16U);
      pos ^= 0x50000 >> w;
      continue;
    } else {
      pos = inc(pos);
    }
    w = __builtin_popcount(pos);
    pos |= 0xffff >> w;
  }
  printf("%d\n", ppos.size());
  unsigned* arr = new unsigned[LEN];
  *arr = 0x17fff;
  arr[1] = 0x2bfff;
  arr[2] = 0x8000fffe;
  arr += 3;
  for(const unsigned p : ppos) {
    *arr = p;
    ++arr;
  }
  ppos.clear();
  arr -= LEN;
  sort(arr);
  FILE* out = fopen("16x", "wb");
  fwrite_unlocked(arr, 4, LEN, out);
  delete[] arr;
  fclose(out);
  return 0;
}
