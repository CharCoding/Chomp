#include "chomp.h"

int main(int argc, char const *argv[]) {
  ios_base::sync_with_stdio(false);
  u32 x = 8, y = 5;
  Player p1 = human, p2 = AI;
  if(argc > 1) {
    if(strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0) {
      cout << "Usage:  ./chomp [width] [height] [player 1] [player 2]\n  where width and height are between 3 and 16\n\
  player 1 and 2 can be: 'human', 'AI', 'AI0'. AI will make mistakes; AI0 will not.\n" << rules << endl;
      return 0;
    }
    x = atoi(argv[1]);
    if(argc > 2) {
      y = atoi(argv[2]);
      if(argc > 3) {
        if(strcmp("AI", argv[3]) == 0) {
          p1 = AI;
        } else if(strcmp("AI0", argv[3]) == 0) {
          p1 = AI0;
        }
        if(argc > 4) {
          if(strcmp("AI0", argv[4]) == 0) {
            p2 = AI0;
          } else if(strcmp("human", argv[4]) == 0) {
            p2 = human;
          }
        }
      }
    }
  }
  Chomp game(x, y, p1 != human || p2 != human);
  char response;
  while(true) {
    game.draw();
    if(p1 == human) {
      game.read_move();
    } else if(p1 == AI0 || game.turn & 6 || game.turn > (x + y + 1) * 2) {
      game.AI();
    } else {
      game.rand_move();
    }
    if(game.check_win()) {
      cout << "Player 1 wins! Restart game? [y|N]: ";
      cin >> response;
      if(response != 'y' && response != 'Y') {
        return 0;
      }
      game.reset();
      continue;
    }
    game.draw();
    if(p2 == human) {
      game.read_move();
    } else if(p2 == AI0 || game.turn & 6 || game.turn > (x + y + 1) * 2) {
      game.AI();
    } else {
      game.rand_move();
    }
    if(game.check_win()) {
      cout << "Player 2 wins! Restart game? [y|N]: ";
      cin >> response;
      if(response != 'y' && response != 'Y') {
        return 0;
      }
      game.reset();
    }
  }
}
