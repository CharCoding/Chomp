#ifndef CHOMP_H
#define CHOMP_H
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

typedef unsigned u32;
using namespace std;
enum Player { human, AI, AI0 };

const char* rules = "Rules:\n  Two players each take a piece and all pieces to the right and/or below it.\n  Whoever takes the last piece wins.\n  Note that the top left piece is missing at the start.\n  To enter a position, type the column number (0-F), then the row number (0-F) and press Enter.\nAvailable in-game commands:\n  :h  Show this help\n  :u  Undo last move\n  :r  Redo last move\n  :!  Reset game\n  :l  Load position from uint32\n  :f  Flip board along the main diagonal\n  :q  Quit Game";

__attribute__((const)) inline constexpr u32 bsr(const u32 pos) {
  return 31 ^ __builtin_clz(pos);
}
__attribute__((const)) constexpr u32 reflect(u32 pos) {
  pos = (pos & 0x55555555) << 1 | (pos & 0xaaaaaaaa) >> 1;
  pos = (pos & 0x33333333) << 2 | (pos & 0xcccccccc) >> 2;
  pos = (pos >> 4 | 0xf0f0f0f0) ^ (pos << 4 | 0x0f0f0f0f);
  return __builtin_bswap32(pos);
}
void printBin(u32 pos) {
  for(u32 i = 32; i--;) {
    cout << (((pos >> i) & 1) ? '1' : '.');
  }
}

class Chomp {
  vector<u32> undo_stack;
  size_t undo_ptr = 0; // not actually a pointer... for correctly determining how many moves to undo
  u32* ppos = 0;
  u32 maxX;
  u32 maxY;
  u32 currPos;
  const char* alphabet = "0123456789ABCDEF";

  bool load_dict() {
    ifstream dict("16x", ios::binary);
    if(!dict.is_open()) {
      return false;
    }
    ppos = new u32[1658994];
    if(!ppos) {
      cerr << "Couldn't allocate memory." << endl;
      return false;
    }
    dict.read((char*)ppos, 1658994 * 4);
    return ppos[1658993] == 0x8000fffe; // hopefully all correct
  }
  bool search_dict(u32 pos) const {
    if(__builtin_clz(pos) < __builtin_ctz(~pos)) {
      pos = reflect(pos);
    }
    long min = 0;
    long max = 1658993; // pitfall: can't use size_t, because middle - 1 can be negative and max underflows to 2**64 - 1
    while(min <= max) {
      long middle = (min + max) >> 1;
      if(ppos[middle] < pos) {
        min = middle + 1;
      } else if(ppos[middle] > pos) {
        max = middle - 1;
      } else {
        return true;
      }
    }
    return false;
    /* // smh can't copy the stl lower_bound() algorithm
    u32* min = ppos; // the problem might be you can't access the equivalent of ppos.end() (segfault)
    size_t length = 1658994;
    while(length > 0) {
      size_t half = length >> 1;
      u32* middle = min + half;
      if(*middle < pos) {
        min = middle;
        ++min;
        length = length - half - 1;
      } else {
        length = half;
      }
    }
    return *min == pos;
    */
  }
  bool take(u32 x, u32 y) { // might be possible to optimize but nah
    u32 mask = -1 << (16 - x + y); // I forgot how it worked anyway
    u32 head = currPos & mask;
    u32 tail = currPos ^ mask;
    int ones = __builtin_popcount(head) - x;
    if(ones <= 0) {
      return false;
    }
    do {
      head &= head - 1;
    } while(--ones);
    while(y) {
      --y;
      tail |= tail + 1;
    }
    ++turn;
    ++undo_ptr;
    undo_stack.resize(undo_ptr);
    undo_stack.back() = currPos;
    currPos ^= ~(head | tail);
    return true;
  }
  bool undo() { // seems to work, didn't test much
    if(undo_ptr > 0) {
      --undo_ptr;
      currPos = undo_stack[undo_ptr];
      --turn;
      return true;
    }
    return false;
  }
  bool redo() {
    if(undo_stack.size() > undo_ptr) {
      currPos = undo_stack[undo_ptr];
      ++undo_ptr;
      ++turn;
      return true;
    }
    return false;
  }
  public:
  u32 turn = 2;
  Chomp(const u32 x, const u32 y, const bool loadAI): maxX{x}, maxY{y} {
    if(x > 16 || y > 16) {
      cerr << "Board size too large" << endl;
      exit(1);
    }
    if(x < 3 || y < 3) {
      cerr << "Board size too small" << endl;
      exit(1);
    }
    if(x == y) {
      cout << "Warning: square boards are easily solved\n";
    }
    currPos = (0x10000 - (0x10000 >> x)) << y | (0xffff >> x);
    if(loadAI) {
      if(!load_dict()) {
        cerr << "Failed to load the losing positions dictionary '16x'.\
 If it has not been generated, run `make solve`." << endl;
        exit(1);
      }
    }
  }
  ~Chomp() {
    delete[] ppos;
  }
  void draw() {
cout << "\nPlayer " << ((turn & 1) + 1) << ", move " << (turn >> 1) << ", pos = ";
    printBin(currPos);
    cout << " (" << currPos << ")\n ";
    for(u32 i = 0; i < maxX; ++i) {
      cout << ' ' << alphabet[i];
    }
    cout << "\n0  ";
    u32 pos = currPos;
    u32 w = 16;
    while(pos & 1) {
      --w;
      pos >>= 1;
    }
    pos >>= 1;
    for(u32 i = w; --i;) {
      cout << " #";
    }
    cout << '\n';
    for(u32 j = 1; j < maxY; ++j) {
      cout << alphabet[j];
      while(pos & 1) {
        --w;
        pos >>= 1;
      }
      pos >>= 1;
      for(u32 i = w; i--;) {
        cout << " #";
      }
      cout << '\n';
    }
  }
  void AI() {
    if(search_dict(currPos)) {
      rand_move();
      return;
    }
    u32 ipos = ~currPos | -1U << bsr(currPos);
    u32 y = 0;
    do {
      u32 body = ipos & -ipos;
      ipos ^= body;
      u32 tail = currPos & (body - 1);
      u32 head = currPos ^ tail;
      do {
        head &= head - 1;
        tail |= body;
        if(search_dict(head | tail)) {
          u32 x = __builtin_popcount(head);
          cout << "\nAI move: " << alphabet[x] << ',' << alphabet[y] << '\n';
          take(x, y);
          return;
        }
        body += body;
      } while(head);
      ++y;
    } while(ipos);
    __builtin_unreachable(); // famous last words
  }
  void rand_move() { // deterministically select a piece at the edge
    u32 start = bsr(currPos);
    u32 end = __builtin_ctz(~currPos);
    u32 rand = currPos % (start - end + 1) + end; // "random"
    u32 mask = -2 << rand; // ok unless you're playing two AI's against each other, this won't matter
    u32 head = __builtin_popcount(currPos & mask) - 1; // because chances are you'll make a mistake after it does
    u32 tail = __builtin_popcount(~(currPos | mask)) - 1;
    if (head > 16) {
      head = 0;
    }
    if (tail > 16) {
      tail = 0;
    }
    cout << "\nAI move: " << alphabet[head] << ',' << alphabet[tail] << '\n';
    take(head, tail);
  }
  void read_move() {
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
              draw();
            } else {
              cout << "Nothing to undo\n";
            }
            continue;
          case 'r':
            if(redo()) {
              cout << "Redid last move\n";
              draw();
            } else {
              cout << "Nothing to redo\n";
            }
            continue;
          case '!':
            cout << "Resetting game\n";
            reset();
            draw();
            continue;
          case 'f':
            cout << "Flipping board\n";
            currPos = reflect(currPos);
            swap(maxX, maxY);
            draw();
            continue;
          case 'h':
            cout << rules;
            continue;
          case 'q':
            cout << "Quitting game\n";
            exit(0);
          case 'l':
            cout << "Enter a 32-bit position (decimal): ";
            u32 temp;
            cin >> temp;
            if(__builtin_popcount(temp) != 16 || temp == 0xffff) {
              cout << "Invalid position\n";
              continue;
            }
            currPos = temp;
            cout << "Loaded position ";
            printBin(currPos);
            cout << " (" << currPos << ")\n"; 
            return;
          default:
            cout << "Invalid command. Type :h for a list of commands.\n";
            continue;
        }
      } else if(cy == ' ' || cy == ',') { // allow a separator
        cin >> cy;
      }
      if('0' <= cx && cx <= '9') {
        x = cx - '0';
      } else if('A' <= cx && cx <= 'F') {
        x = cx - 'A' + 10;
      } else if('a' <= cx && cx <= 'f') {
        x = cx - 'a' + 10;
      }
      if('0' <= cy && cy <= '9') {
        y = cy - '0';
      } else if('A' <= cy && cy <= 'F') {
        y = cy - 'A' + 10;
      } else if('a' <= cy && cy <= 'f') {
        y = cy - 'a' + 10;
      }
      if(x <= maxX && y <= maxY) {
        cout << "Got move: " << cx << ", " << cy << '\n';
        if(x == 0 && y == 0) {
          cout << "Invalid move: top left piece does not exist.";
          continue;
        }
        if(take(x, y)) {
          return;
        }
        cout << "Invalid move: piece is already taken.";
      } else {
        cout << "Invalid move: no such position. Enter a position in the format 'x,y'.";
      }
    }
  }
  bool check_win() {
    return currPos <= 0x17fff;
  }
  void reset() {
    undo_stack.clear();
    undo_ptr = 0;
    turn = 2;
    currPos = (0x10000 - (0x10000 >> maxX)) << maxY | (0xffff >> maxX);
  }
};
#endif // CHOMP_H
