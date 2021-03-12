#include "chomp.h"
#include <bitset>
/*
#include "gtest/gtest.h"

namespace {
  Chomp game(33, 30);
  TEST(Util, width) {
    EXPECT_EQ(game.width(game.currPos), 33);
  }
  TEST(Util, height) {
    EXPECT_EQ(game.height(game.currPos), 30);
  }
  TEST(Util, squares) {
    EXPECT_EQ(game.squares(game.currPos), 30 * 33);
    game.currPos = 0x10ffffffff;
    EXPECT_EQ(game.squares(game.currPos), 4);
    game.currPos = 0x2ffffffff;
    EXPECT_EQ(game.squares(game.currPos), 1);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
*/

using namespace std;

constexpr double ncr(double n, double r) {
  double res = 1.0;
  while(r) {
    res *= n;
    res /= r;
    --n;
    --r;
  }
  return res;
}

#define gs 13

int main() {
  Chomp game(gs, gs - 1, 1);
  cout << game.ppos.size() << endl;
  game.draw();
  game.currPos = game.solve(game.currPos);
  game.draw();
  /*
  cout << game.ppos.size() << endl;
  for(int i = gs; i; --i) {
    for(int j = i; j; --j) {
      //cout << i << ' ' << j << ": " << bitset<64>((1UL << (14 + j)) - 1UL - (1UL << (14 + j - i)) + (1UL << (14 - i))) << '\n';
      cout << i << ' ' << j << ": ";
      u64 tmp = (1UL << (gs + j)) - 1UL - (1UL << (gs + j - i)) + (1UL << (gs - i));
      game.currPos = tmp;
      game.draw();
      cout << "=============\n";
      game.currPos = game.solve(tmp);
      game.draw();
      cout << "=============\n";
    }
  }
  cout << "done\n";
  */
  /*
  for(int i = 3; i < 15; ++i) {
    for(int j = i; j < 15; ++j) {
      //Chomp game(i, j, 1);
      //size_t s = game.ppos.size();
      //cout << i << ", " << j << ": " << s << " ratio: " << (double)s / ncr(i + j, min(i, j)) << '\n';
      cout << i << ", " << j << ": " << (i * j) * (i * j) << '\n';
    }
    cout << endl;
  }
  */
}