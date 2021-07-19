/*
Chomp 16x16 board solver
  - runs in about 4 minutes 20 seconds on a 2.4 GHz Intel Broadwell cpu
    - New version: 3 minutes 45 seconds
      - Latest version: 1 minutes 7 seconds
        - Final version: 27 seconds
  - Outputs a binary file "16x" (6.33 MiB), where every int represents a losing position.
  Specifically:
    There are always 16 1's and 0's in each int. 1's represent a horizontal edge, 0's represent a vertical edge of the bottom right sides of the chomp board.
  - 3317972 such losing positions, 16 of them are symmetric. For the rest, we only store the smaller mirror reflection, resulting in 1658994 positions.
  - Highly optimized, but still O(n!) ish.
*/
#include <cstdio>
#include <cstring>
#include <algorithm>
#ifdef PHMAP
#include <parallel_hashmap/phmap.h>
typedef phmap::flat_hash_set<unsigned> hashset;
#else
// SLOW
#include <unordered_set>
typedef std::unordered_set<unsigned> hashset;
#endif

#define LEN 3317972

using namespace std;

__attribute__((const)) constexpr unsigned reflect(unsigned pos) { // flip a position along the main diagonal
  pos = (pos & 0x55555555) << 1 | (pos & 0xaaaaaaaa) >> 1; // only works for 16x16
  pos = (pos & 0x33333333) << 2 | (pos & 0xcccccccc) >> 2;
  pos = (pos >> 4 | 0xf0f0f0f0) ^ (pos << 4 | 0x0f0f0f0f);
  return __builtin_bswap32(pos);
}
/*
void sort(unsigned* before) { // we used to radix sort the losing positions because they weren't in increasing order
  unsigned count[256]; // but we can also remove the out of order positions, since their mirror images already exist
  unsigned* after = new unsigned[LEN]; // a.k.a "Stalin sort"
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
*/
hashset ppos(LEN - 2); // O(1) lookup
unsigned h = 3; // current position height and width
unsigned w = 4;

inline constexpr unsigned inc(const unsigned pos) { // cause the lowest bit to be moved up
  return pos + (pos & -pos);
}

__attribute__((pure)) bool isWinning(const unsigned pos) { // goes through each subposition
  unsigned ipos = ~pos ^ 0xffff0000 << h; // checking if they are losing
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
  unsigned* arr = new unsigned[1658994];
  *arr = 0x17fff;
  *++arr = 0x2bfff;
  for(unsigned window = 0xf << 13; window >= 0xf; window >>= 1) {
    pos ^= window;
    *++arr = pos;
    ppos.insert(pos);
    ppos.insert(reflect(pos));
  }
  *++arr = 0x4dfff;
  ppos.insert(0x4dfff);
  pos = 0x66fff;
  while(true) {
    if(w > h) { // never encountered
      if(isWinning(pos)) { // not losing
        if((pos >> (15 - w) & 7) == 1) { // skippable
          w = min(w + 2, 16U);
          pos ^= 0x50000 >> w;
          continue;
        }
        unsigned ripple = inc(pos);
        if(!(ripple & pos)) { // height increase
          w = h;
          ++h;
          if(h == 16) {
            break;
          }
          unsigned temp = ripple | (0xffff & (0xfffeffff >> h));
          *++arr = temp;
          ppos.insert(temp);
          pos = ripple | hmin;
          hmin ^= 0x18000 >> w;
          continue;
        }
        pos = ripple;
      } else { // losing
        *++arr = pos;
        ppos.insert(pos);
        ppos.insert(reflect(pos));
        pos &= pos + 1;
        pos = inc(pos);
      }
    } else if(w != h && ppos.contains(pos)) { // encountered; losing
      pos &= pos + 1;
      pos = inc(pos);
    } else if(!(pos & 0x30000 >> w)) { // not losing; skippable
      w = min(w + 2, 16U);
      pos ^= 0x50000 >> w;
      continue;
    } else { // not skippable
      pos = inc(pos);
    }
    w = __builtin_popcount(pos); // fill back appropriate number of 1's at the end
    pos |= 0xffff >> w;
  }
  printf("%d\n", ppos.size());
  ppos.clear();
  *++arr = 0x8000fffe; // all greater positions have mirror images smaller than itself
  arr -= 1658993;
  FILE* out = fopen("16x", "wb");
  fwrite_unlocked(arr, 4, 1658994, out);
  delete[] arr;
  fclose(out);
  return 0;
}
