// EEC50281EEC50281EEC50281EEC50281EEC50281
#include "chomp.h"

/* Problem: match a possible move with a losing position
let k = squares(currPos)
let j = |{losingPos | losingPos < currPos}|
Where < is squares comp
Is it possible to:
1. given currPos, return iterator to all losingPos *less than* it? (partial order)

Comp function?
a < b if:
uint(a) < uint(b) &&
popcount(a >> i) <= popcount(b >> i) for i = [0..63]
Directed Acyclic Graph: too many edges?

Option 1:
  - 2D vectors indexed by square count (lots of squares empty? maybe just store the size, then the positions?)
    - each row contains losing positions for that many squares, sorted by size
Pros:
  - Given a position, can look up 'squares smaller', then filter by 'number size',
Cons:
  - Need efficient algorithm for deciding whether two positions are 1 move away
  - Potentially very slow for large positions

Option 2:
  - Unordered set recording every losing position
    - O(1) amortized lookup time
Pros:
  - Given a position, can check all moves (1024 max for 64 bits) whether they are in losing set
    O(maxX * maxY)
Cons:
  - up to 2x space needed

Option 3:
  - 1024 bitmask?

Better square counting:
pos = 01001100011100001111: 20 bit, maxX = maxY = 10
naive algorithm: 1*2+3*3+6*4=35 squares, O(20)

WA exp fit: 0.0012656 e^(1.35537 x)
The estimates are really bad... exp model misses by like 32k
ppos size:
3x3: 5
4x4: 10
5x5: 23
6x6: 48
7x7: 129
8x8: 322
9x9: 867
10x10: 2612
11x11: 8037
12x12: 26838 (0.22s)
13x13: 85555 (1.02s)
14x14: 256366 (4.99s) (2 MiB)
15x15: 829383 (25.81s) (6 MiB)
16x16: 3317972 (4m20s using ./solver) (25.3 MiB)
*/

int main(int argc, char const *argv[]) {
  int x = 32, y = 32, d = 0;
  bool first = false; // AI goes 2nd by default
  if(argc > 1) {
    x = atoi(argv[1]);
    if(argc > 2) {
      y = atoi(argv[2]);
      if(argc > 3) {
        d = atoi(argv[3]);
        if(argc > 4 && argv[4][0] == 'A' && argv[4][1] == 'I') {
          first = true;
        }
      }
    }
  }
  Chomp game(x, y, d);
  if(d < 0) {
    cout << game.ppos.size() << " losing positions.\n";
    return 0;
  }
  if(d > 0 && first) {
    game.draw();
    game.AI();
  }
  do {
    game.draw();
    game.readMove();
    if(d > 0) {
      game.draw();
      game.AI();
    }
  } while(game.checkWin());
}