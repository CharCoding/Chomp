/*
  Chomp 16x16 board solver
  - runs in about 4 minutes 20 seconds on a 2.4 GHz Intel Broadwell cpu (420 blazeit)
  - Outputs a binary file "16x" (12.66 MiB), where every int represents a losing position.
  Specifically:
    There are always 16 1's and 0's in each int. 1's represent a horizontal edge, 0's represent a vertical edge of the bottom right sides of the chomp board.
  - 3317972 such losing positions
  - Highly optimized, but still O(n!) ish
  - Possible optimizations:
    - Might be faster to use a hashmap, O(1) lookup but potentially more memory (who cares, 26 MiB memory isn't much)
      - Sort it afterwards with radix sort, O(n). But notice that this can spike peak memory usage to 40 ish MiB
    - When encountering a losing position, also store its mirror image somewhere.
      - Then we have to sort the final results somehow. Mirror images aren't sorted in the order they appear.
      - Kind of hard for non-square boards
    - Dynamically reduce the range of binary search (probably not worth it, O(log n) already)
    - Skip "height = width" without using a while loop (barely any improvement)
    - Include dynamic pattern seeking using ultimate-periodicity (only proven for height = 3, tiny advantage)
    - Solve Chomp in polynomial time (uhh)
*/

#include <cstdio> // fastest to write memory to file using fwrite_unlocked
#include <vector>
#include <algorithm>

#define LEN 3317972
// Note: currently, chomp.html only accepts uncompressed 16x losing positions dictionary
// #define COMPRESS 1

using namespace std;

#ifdef COMPRESS

unsigned arr[256]; // n choose r memo

unsigned nCr(unsigned i, unsigned j) {
  j = min(j, i - j);
  if(j == 0) {
    return 1;
  }
  if(j == 1) {
    return i;
  }
  --i;
  return arr[(i * i >> 2) + j - 1];
}

void initnCr(unsigned* arr) {
  *arr = 2;
  ++arr;
  for(unsigned k = 2; k <= 16; ++k) {
    const unsigned l = k - 1;
    const unsigned * const ptr = arr + l;
    *arr = k + l;
    while(++arr < ptr) {
      *arr = *(arr - k) + *(arr - l);
      *(arr + l) = *arr + *(arr - 1);
    }
    *arr = 2 * k;
    arr += l;
    *arr = 2 * *(arr - k);
    ++arr;
  }
}

unsigned perm_rank(unsigned pos) { // hash a 16 1's, 16 0's bit permutation to an integer between 0 and nCr(32, 16) - 1
  unsigned ones = __builtin_ctz(~pos); // this makes it easy to squeeze 32-bit positions into unique, strictly increasing integers
  pos &= pos + 1;
  unsigned rank = 0;
  unsigned total = __builtin_ctz(pos);
  pos >>= total;
  while(pos) {
    if(pos & 1) {
      rank += nCr(total, ++ones);
    }
    pos >>= 1;
    ++total;
  }
  return rank;
}

unsigned perm_unrank(unsigned rank) { // inverse of said hash
  unsigned pos = 0;
  unsigned total = 31;
  unsigned ones = 16;
  while(rank) {
    int temp = rank - nCr(total, ones);
    if(temp >= 0) {
      rank = temp;
      --ones;
      pos |= 1 << total;
    }
    --total;
  }
  return pos | ((1 << ones) - 1);
}

void printBin(unsigned pos) { // debug fn
  for(int i = 31; i >= 0; --i) {
    putchar_unlocked((pos >> i & 1) ? '1' : '.');
  }
}

#endif

inline constexpr unsigned bsr(const unsigned pos) {
  return 31 ^ __builtin_clz(pos); // force g++ to generate a single bsr instruction
}

int main() {
  FILE* out = fopen("16x", "wb");
  vector<unsigned> ppos;
  ppos.reserve(3317972); // we know this ahead of time
  const unsigned hmin = 0x16fff; // hmin is the default start position after incrementing height. No smaller losing positions are possible.
  unsigned pos = 0x17fff; // single block
  ppos.push_back(pos);
  for(unsigned window = 0xf << 14; window >= 0xf; window >>= 1) {
    pos ^= window; // all height = 2 positions
    ppos.push_back(pos);
  }
  ppos.push_back(0x4dfff);
  auto cb = ppos.cbegin();
  cb += 2;
  pos = 0x53fff; // this is the next losing position
  unsigned h = 3; // keep track of height
  unsigned mask; // mask will help with move generation
  loop: // exit condition is when height > 16, no need to check every time
    mask = 0xffff8000 << h; // mask for the highest bit of pos
    const unsigned end = (pos + 1) & ~pos; // the lowest 0 bit of pos, inverted
    do { // begin to loop through all possible moves
      unsigned lpos = pos & mask; // bottom left portion of edge path
      unsigned rpos = ~(pos | mask); // top right portion of edge path, inverted
      do { // loop through all possible take indices
        lpos &= lpos - 1; // set the lowest 1 bit to 0
        rpos ^= 1 << bsr(rpos); // set the highest 1 bit to 0
        if(binary_search(cb, ppos.cend(), lpos | ~(rpos | mask))) { // reform the subposition by combining lpos and rpos. If the subposition is losing, the current position is winning
          unsigned smallest, ripple, ones; // calculate next position
          do {
            smallest = pos & -pos; // get last horizontal edge
            ripple = pos + smallest; // make that edge vertical, adding 1 horizontal edge before it
            if((ripple & pos) == 0) { // the entire horizontal edge disappeared, must have been a full rectangle
              ++h; // increase height
              if(h == 17) { // done; write to file
                printf("%d\n", ppos.size());
#ifndef COMPRESS
                fwrite_unlocked(ppos.data(), sizeof ppos[0], ppos.size(), out);
                return 0;
#else
                goto compress;
#endif
              }
              ppos.push_back(ripple | 0xffff ^ (0x10000 >> h)); // new height means a new losing position for height = width. We are skipping all other positions where height = width. It is the smallest losing position at height h.
              pos = ripple | hmin; // advance the position by 2 blocks on column 2
              goto loop; // try next position
            }
            ones = pos ^ ripple; // how many 1's are deleted from pos (this gives 2 more)
            smallest = ones + 1; // reuse for 2 row winning skip
            ones >>= 2 + __builtin_ctz(pos); // shift so that pos has a new last horizontal edge
            pos = ripple | ones; // form next position
          } while(__builtin_popcount(ripple) == h || (smallest & pos) == 0); // ripple contains the width; when width = height the position can't be losing (or we already added it when height increased)
          // 2 row winning skip: skip pos when it ends with 0101*
          goto loop; // try next position
        }
      } while(lpos && rpos); // finished with current take index.
      mask = (unsigned)((int)mask >> 1); // include 1 more bit in lpos and exclude 1 bit from rpos
    } while(mask + end); // have we reached the lowest 0 bit?
    ppos.push_back(pos); // if so, no moves lead to a losing position, thus the current position is losing
    pos &= pos + 1; // remove last horizontal edge
    pos += pos & -pos; // increase the width on the second last horizontal edge. If there is none, make a new one.
    pos |= 0xffff >> __builtin_popcount(pos); // pos contains the width; add back the last horizonal edge
    goto loop; // try next position

#ifdef COMPRESS
  compress:
  initnCr(arr);
  ppos[LEN - 1] = perm_rank(ppos[LEN - 1]);
  for(int i = LEN - 2; i >= 0; --i) { // use differential encoding on the permutation ranks
    ppos[i] = perm_rank(ppos[i]); // stores about 0.79 positions per byte
    ppos[i + 1] -= ppos[i];
  }
  unsigned char* comp = new unsigned char[4180552];
  unsigned char* ptr = comp;

  //unsigned saved = 0;
  for(const unsigned pos : ppos) {
    if(pos <= 0x7f) {
      /* // save on patterns where the increment decreases by 1 for several positions
      if(*ptr - *(ptr + 1) == 1) { // "runs" are encoded like so: 0x00 0x17 0x03
        int run = 1; // denotes 0x17 0x16 0x15 0x14 ... 0x04 0x03
        unsigned* fwd = ptr + 1; // this happens pretty rarely though... only saves around 80 bytes
        while(*fwd - *(++fwd) == 1) {
          ++run;
        }
        if(run >= 3) {
          saved += run - 3;
          *ptr = 0;
          *(ptr + 1) = *ptr;
          *(ptr + 2) = *(fwd - 1);
          ptr = fwd;
          ptr += 3;
          continue;
        }
      }
      */
      *ptr = pos;
    } else if(pos <= 0x3fff) {
      *ptr = 0x80 | pos >> 7;
      ++ptr;
      *ptr = pos & 0x7f;
    } else if(*ptr <= 0x1fffff) {
      *ptr = 0x80 | pos >> 14;
      ++ptr;
      *ptr = 0x80 | pos >> 7;
      ++ptr;
      *ptr = pos & 0x7f;
    } else {
      *ptr = 0x80 | pos >> 21;
      ++ptr;
      *ptr = 0x80 | pos >> 14;
      ++ptr;
      *ptr = 0x80 | pos >> 7;
      ++ptr;
      *ptr = pos & 0x7f;
    }
    ++ptr;
  }
  FILE* out = fopen("c16", "wb");
  fwrite_unlocked(comp, 1, ptr - comp, out);
  fclose(out);
  //printf("Saved: %d\n", saved);
  delete[] comp;
#endif
}