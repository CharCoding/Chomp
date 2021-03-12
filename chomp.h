// EEC50281EEC50281EEC50281EEC50281EEC50281
// WARNING: this code uses GNU C builtins. Will probably not compile using non-gcc compilers.
#ifndef CHOMP_H
#define CHOMP_H

#include <iostream>
#include <vector>
#include <cmath>
#include <bit> // requires c++20
#include <algorithm>
#include <parallel_hashmap/phmap.h> // !!!
//#include <unordered_set>
//#include <set>

typedef unsigned u32;
typedef unsigned long u64;
using namespace std;

inline constexpr int bsr(const u64 pos) {
  return 63 ^ __builtin_clzl(pos);
}

class Chomp {
  struct comp {
    constexpr bool operator()(const u64 a, const u64 b) const __attribute__((const)) { // partial order
      if(a >= b) {
        return false;
      }
      int i = bsr(b), j = __builtin_ctzl(~a);
      int oneDiff = 0;
      while(i > j) {
        oneDiff += (b >> i & 1) - (a >> i & 1);
        if(oneDiff < 0) return false;
        --i;
      }
      return true;
    }
  };
  struct comp_sq {
    int x;
    comp_sq(int maxX) : x(maxX) {}
    constexpr bool operator()(const u64 a, const u64 b) const __attribute__((const)) { // total order
      const int sa = a;//squares2(a);
      const int sb = b;//squares2(b);
      if(sa < sb) {
        return true;
      } else if(sa > sb) {
        return false;
      } else {
        return a < b;
      }
    }
  };
public:
  phmap::flat_hash_set<u64> ppos;
  //phmap::btree_set<u64> ppos2; // O(log n) lookup, not worth the memory save (just use vector instead!)
  vector<u64> undo_stack;
  vector<u64> redo_stack;
  u64 currPos;
  int maxX;
  int maxY;
  int diffy;
  int turn = 2;
  const char alphabet[63] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  inline constexpr int height(u64 pos) const __attribute__((const)) {
    //return 64 - maxX - countl_zero(pos);
    return 64 - maxX - __builtin_clzl(pos);
  }
  inline constexpr int width(u64 pos) const __attribute__((const)) {
    //return maxX - countr_one(pos);
    return maxX - __builtin_ctzl(~pos);
    //return __builtin_popcountl(pos & (pos + 1));
  }
  constexpr int squares(u64 pos) const __attribute__((const)) {
    int sum = 0;
    int ones = __builtin_ctzl(~pos);
    int width = maxX - ones;
    pos >>= ones;
    do {
      int bit = pos & 1;
      pos >>= 1;
      width -= bit;
      sum += width & (bit - 1);
    } while(pos);
    return sum;
  }
  constexpr int squares2(u64 pos) const __attribute__((const)) {
    int sum = 0;
    int run = __builtin_ctzl(~pos);
    int width = maxX - run;
    pos >>= run;
    while(pos) {
      run = __builtin_ctzl(pos);
      sum += width * run;
      pos >>= run;
      run = __builtin_ctzl(~pos);
      width -= run;
      pos >>= run;
    }
    return sum;
  }
  constexpr u64 canMove(u64 spos, u64 epos) const __attribute__((const)) {
    u64 diff = spos ^ epos;
    return 0;
  }
  bool undo() {
    if(undo_stack.empty()) {
      return false;
    }
    redo_stack.push_back(currPos);
    currPos = undo_stack.back();
    undo_stack.pop_back();
    --turn;
    return true;
  }
  bool redo() {
    if(redo_stack.empty()) {
      return false;
    }
    undo_stack.push_back(currPos);
    currPos = redo_stack.back();
    redo_stack.pop_back();
    ++turn;
    return true;
  }
  bool take(int x, int y) { // O(maxX + y)
    u64 rpos = -1UL << (maxX - x + y);
    u64 lpos = currPos & rpos;
    rpos ^= currPos;
    int ones = __builtin_popcountl(lpos) - x;
    if(ones <= 0) {
      return false;
    }
    do {
      lpos &= lpos - 1;
    } while(--ones);
    while(y) {
      --y;
      rpos |= rpos + 1;
    }
    undo_stack.push_back(currPos);
    redo_stack.clear();
    ++turn;
    currPos ^= ~(lpos | rpos);
    return true;
  }
  void reset() {
    currPos = ((1UL << maxX) - 1UL) << maxY;
    undo_stack.clear();
    redo_stack.clear();
    turn = 2;
  }
  constexpr u64 next(u64 pos) const __attribute__((const)) { // https://github.com/hcs0/Hackers-Delight/blob/master/snoob.c.txt
    u64 smallest, ripple, ones;
    smallest = pos & -pos;
    ripple = pos + smallest;
    ones = pos ^ ripple;
    ones >>= 2 + __builtin_ctzl(pos);
    return ripple | ones;
  }
  vector<u64> movesrand(u64 pos) const {
    vector<u64> mpos;
    mpos.reserve(squares2(pos));
    //int onepos = 63 ^ __builtin_clzl(pos);
    u64 mask = -1UL << bsr(pos);
    const u64 end = (pos + 1) & ~pos;
    //int zeropos = __builtin_ctzl(~pos);
    while(mask + end) {
      //u64 mask = -1UL << onepos;
      //--onepos;
      u64 lpos = pos & mask;
      u64 rpos = ~(pos | mask);
      do {
        lpos &= lpos - 1;
        rpos ^= 1UL << bsr(rpos);
        mpos.push_back(lpos | ~(rpos | mask));
      } while(lpos && rpos);
      mask = (u64)((long)(mask) >> 1);
    }
    //sort(mpos.begin(), mpos.end());
    return mpos;
  }
  vector<u64> movesord(u64 pos) {
    vector<u64> mpos;
    mpos.reserve(squares2(pos));
    u64 lpos = pos & pos + 1;
    int zeropos = __builtin_ctzl(~pos); // lowest 0
    int onepos = __builtin_ctzl(lpos); // lowest effective 1
    u64 xmask = 1 << zeropos | 1 << onepos;
    while(lpos) {
      mpos.push_back(pos ^ xmask);

    }
    return mpos;
  }
  vector<u64> movesinv(u64 pos) {
    vector<u64> mpos(squares(pos));
    // invariant: pos has 32 ones and 32 zeros
    //u32 onepos = 63 ^ countl_zero(pos); // highest 1 bit position
    u32 onepos = bsr(pos);
    //u32 zeropos = countr_one(pos); // lowest 0 bit position
    u32 zeropos = __builtin_ctzl(~pos);
    // valid horizontal edges = sizeof(u64) / 2 - number of trailing ones
    // first generated pos should be 0xffffffffUL, trivial, try to skip
    // second should be on range
    u32 i = onepos;
    u32 j = zeropos;
    u64 ones = 0xffffffffUL & (-1UL << j);

    auto iter = mpos.begin();
    const auto end = mpos.cend();
    while(iter != end) {
      u64 mask = ~((2UL << i) - (1UL << j));
      *iter = (pos & mask) | ones;
      ++iter;
      ++j; // move lowest 0 to the left
      ones <<= 1;
      while((pos >> j) & 1) {
        ones &= ones - 1; // remove least significant 1
        ++j;
      }
      if(j > i) {
        j = zeropos;
        --i;
        while(((pos >> i) & 1) == 0) {
          --i;
        }
      }
    }

    return mpos;
  }
  constexpr u64 flip(u64 pos) const __attribute__((const)) { // broken
    pos = ((pos >> 1) & 0x5555555555555555) | ((pos & 0x5555555555555555) << 1);
    pos = ((pos >> 2) & 0x3333333333333333) | ((pos & 0x3333333333333333) << 2);
    pos = ((pos >> 4) & 0x0F0F0F0F0F0F0F0F) | ((pos & 0x0F0F0F0F0F0F0F0F) << 4);
    /*
    pos = ((pos >> 8) & 0x00FF00FF00FF00FF) | ((pos & 0x00FF00FF00FF00FF) << 8);
    pos = ((pos >>16) & 0x0000FFFF0000FFFF) | ((pos & 0x0000FFFF0000FFFF) <<16);
    pos = pos >> 32 | pos << 32;
    /*/
    pos = ~__builtin_bswap64(pos);
    return pos;
  }
  void generate() {
    u64 mask = 3UL << (maxX - 1);
    u64 pos = mask - 1;
    const u64 hmin = (5UL << (maxX - 2)) - 1UL;
    ppos.insert(pos);
    for(int i = 1, j = min(maxX, maxY); i < j; ++i) {
      pos ^= mask << i | mask >> i;
      ppos.insert(pos);
    }
    pos = (mask << 1 | mask >> 2) - 1; // skip height = 1
    int h = 2;
    do {
      mask = -1UL << bsr(pos);
      const u64 end = (pos + 1) & ~pos;
      do {
        u64 lpos = pos & mask;
        u64 rpos = ~(pos | mask);
        do {
          lpos &= lpos - 1;
          rpos ^= 1UL << bsr(rpos);
          if(ppos.contains(lpos | ~(rpos | mask))) {
            u64 smallest, ripple, ones;
            do {
              smallest = pos & -pos;
              ripple = pos + smallest;
              if((ripple & pos) == 0) {
                ++h;
                pos = ripple | hmin;
                goto loop;
              }
              ones = pos ^ ripple;
              ones >>= 2 + __builtin_ctzl(pos);
              pos = ripple | ones;
            } while(__builtin_popcountl(ripple) == h); // popcnt of ripple is exactly the width
            goto loop;
          }
        } while(lpos && rpos);
        mask = (u64)((long)(mask) >> 1);
      } while(mask + end);
      ppos.insert(pos); // next(losing_pos) will never have to update h
      pos &= pos + 1;
      pos += pos & -pos;
      pos |= (1 << (maxX - __builtin_popcountl(pos))) - 1; // not bothering to check h == w. so few cases actually match that we can just check them
      loop:;
    } while(pos < currPos);
  }
//public:
  Chomp(int x, int y, int d = 0) : maxX{x}, maxY{y}, diffy{d}{
    if(x + y >= 64) { // >= prevents bit overflow (very annoying to deal with)
      cerr << "Board size too large" << endl;
      exit(1);
    }
    if(x <= 2 || y <= 2) {
      cerr << "Board size too small" << endl;
      exit(1);
    }
    if(x == y) {
      //cout << "Warning: square board is trivially won\n";
    }
    currPos = ((1UL << maxX) - 1UL) << maxY;
    if(d) {
      //generate_ppos();
      generate();
    }
  }
  u64 solve(u64 pos) const {
    u64 mask = -1UL << bsr(pos);
    const u64 end = (pos + 1) & ~pos;
    //int zeropos = __builtin_ctzl(~pos);
    while(mask + end) {
      //u64 mask = -1UL << onepos;
      //--onepos;
      u64 lpos = pos & mask;
      u64 rpos = ~(pos | mask);
      do {
        lpos &= lpos - 1;
        rpos ^= 1UL << bsr(rpos);
        //mpos.push_back(lpos | ~(rpos | mask));
        u64 move = lpos | ~(rpos | mask);
        if(ppos.contains(move)) {
          return move;
        }
      } while(lpos && rpos);
      mask = (u64)((long)(mask) >> 1);
    }
    return 0;
  }
  void AI() {

  }
  void printBin(u64 pos) const {
    for(int i = maxX + maxY; i--;) {
      cout << (char)(((pos >> i) & 1) ? '1' : '.');
    }
  }
  void draw() const {
    cout << "Player " << ((turn & 1) + 1) << ", move " << (turn >> 1) << ", pos = ";
    printBin(currPos);
    cout << " h = " << height(currPos) << ", w = " << width(currPos) << ", sq = " << squares2(currPos) << ", ppos = " << ppos.contains(currPos) << "\n ";
    for(int i = 0; i < maxX; ++i) {
      cout << ' ' << alphabet[i];
    }
    cout << "\n0 !";
    u64 pos = currPos;
    int w = width(pos);
    int i = w;
    while(--i > 0) {
      cout << " #";
    }
    cout << '\n';
    for(int j = 1; j < maxY; ++j) {
      cout << alphabet[j];
      if(pos & pos + 1) {
        pos |= 1 << (maxX - w);
        pos >>= 1;
        w = width(pos);
        i = w;
        while(i > 0) {
          --i;
          cout << " #";
        }
      }
      cout << endl;
    }
  }
  void readMove() {
    u32 x = -1U, y = -1U;
    char cx, cy;
    while(true) {
      cout << "\nYour move: ";
      cin >> cx >> cy;
      if(cx == ':') { // found the vim user
        switch(cy) {
          case 'u':
            if(undo()) {
              cout << "Undid last move\n";
              return;
            }
            cout << "Nothing to undo\n";
            continue;
          case 'r':
            if(redo()) {
              cout << "Redid last move\n";
              return;
            }
            cout << "Nothing to redo\n";
            continue;
          case '!':
            cout << "Resetting game\n";
            reset();
            return;
          /*
          case 'f':
            cout << "Flipping board\n";
            currPos = flip(currPos);
            swap(maxX, maxY);
            return;
          */
          case 'm': {
            const vector<u64> mpos = movesrand(currPos);
            const u64 temp = currPos;
            for(u64 pos : mpos) {
              currPos = pos;
              draw();
              cout << "--------\n";
            }
            cout << "Printed " << mpos.size() << " possible moves (should be " << squares2(temp) <<")\n";
            currPos = temp;
            return;
          }
          case 'x': {
            vector<u64> mpos = movesrand(currPos);
            /*
            for(u64& pos : mpos) {
              pos ^= currPos;
            }
            */
            //sort(mpos.begin(), mpos.end());
            for(u64 pos : mpos) {
              //u64 x = pos ^ currPos;
              printBin(pos);
              cout << '\n';
            }
            cout << "--------\n";
            return;
          }
          case 'd': {
            for(const auto pos : ppos) {
              printBin(pos);
              cout << ' ' << pos << ' ' << squares2(pos) << '\n';
            }
            cout << "--------\n";
            return;
          }
          case 'n': {
            currPos = next(currPos);
            return;
          }
          case 'q':
            cout << "Quitting game\n";
            exit(0);
          case 'l':
            cin >> currPos;
            return;
          default:
            cout << "Invalid command. :u for undo, :r for redo, :! for reset, :q for quit\n";
            continue;
        }
      } else if(cy == ' ' || cy == ',') { // allow a separator
        cin >> cy;
      }
      for(int i = max(maxX, maxY); i--;) {
        if(cx == alphabet[i]) {
          if(i >= maxX) {
            x = y = -1U;
            break;
          }
          x = i;
        }
        if(cy == alphabet[i]) {
          if(i >= maxY) {
            x = y = -1U;
            break;
          }
          y = i;
        }
      }
      if(x <= maxX && y <= maxY) {
        cout << "Got move: " << x << ", " << y << '\n';
        if(take(x, y)) {
          return;
        }
        cout << "Invalid move: piece is already taken.";
      } else {
        cout << "Invalid move: no such position.";
      }
    }
  }
  bool checkWin() { // returns "should we continue playing"
    if(currPos & currPos + 1) {
      return true; // still playing
    } else {
      cout << "Player " << ((turn & 1) + 1) << " won!\nNew game? (Y/N) ";
    }
    char opt;
    cin >> opt;
    if(opt == 'Y' || opt == 'y') {
      reset();
      return true; // technically won, but new game already started
    }
    return false; // no new game
  }
};
/*
class Chomp_old {
  static constexpr u64 const multipliers[16] = { 1, 0x11, 0x111, 0x1111, 0x11111, 0x111111, 0x1111111, 0x11111111, 0x111111111, 0x1111111111, 0x11111111111, 0x111111111111, 0x1111111111111, 0x11111111111111, 0x111111111111111, 0x1111111111111111 };
  vector<u64> ppos = { 1 };
  u64 currPos;
  u32 maxX;
  u32 maxY;
  u32 diff;
  [[gnu::const]] constexpr u32 height(u64 pos) const {
    return 16 - (countl_zero(pos) >> 2);
  }
  [[gnu::const]] constexpr u32 squares(u64 pos) const {
    pos = (pos >> 4 & 0xf0f0f0f0f0f0f0fUL) + (pos & 0xf0f0f0f0f0f0f0fUL);
    return pos * 0x101010101010101UL >> 56; // p a r a l l e l
  }
  [[gnu::const]] constexpr u64 reflect(u64 pos) const {
    if(pos & 0xf000000000000000UL) return 0;
    u32 h = height(pos), i = 0;
    u64 newPos = 0;
    while(h) {
      newPos += h << i;
      pos -= multipliers[h - 1];
      h = height(pos);
      i += 4;
    }
    return newPos;
  }
  void populate() {

  }
  bool isLosing(u64 pos, u32 depth) {
    return false;
  }
public:
  Chomp_old(u32 x, u32 y, u32 d) : maxX{x}, maxY{y}, diff{d} {
    currPos = multipliers[maxY - 1] * maxX;
  }
  void draw() const {
    cout << "  ";
    for(u32 i = 0; i < maxX; ++i) {
      cout << i << ' ';
    }
    cout << '\n';
    for(u32 j = 0; j < maxY; ++j) {
      cout << j << ' ';
      u32 w = (currPos >> (j << 2)) & 0xf;
      if(j == 0) {
        --w;
        cout << "X ";
      }
      while(w) {
        --w;
        cout << "# ";
      }
      cout << '\n';
    }
  }
  u32 readMove() {
    cout << "\nYour move: ";
    u32 x = 16, y = 16;
    cin >> x >> y;
    while(x >= 16 || y >= 16) {
      cout << "Error reading move. Enter the x and y coordinates separated by a space.\nYour move: ";
      cin >> x >> y;
    }
    return x | y << 4;
  }
  void take(u32 move) {
    const u32 x = move & 15, y = move >> 4, h = height(currPos);
    for(u32 i = y; i < h; ++i) {
      u64 row = currPos >> (i << 2) & 15;

    }
  }
  u32 findMove() {
    return 0;
  }
};*/

#endif // CHOMP