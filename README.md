# [Chomp](https://charcoding.github.io/Chomp/)
JS and C++ Implementation of the poset game, [Chomp](https://en.wikipedia.org/wiki/Chomp).

The AI is pretty strong.

### Notes:
This program uses at least 6.33 MiB of memory just to store the losing position dictionary. And this
is already storing the edges as individual bits as opposed to storing row widths.
The C++ solver can use [parallel hashmap](https://github.com/greg7mdp/parallel-hashmap) for a speed boost.  

## Usage:
Running `make` will make the game executable. This executable doesn't require parallel hashmap.  
To start the game, run `./chomp [width] [height] [player 1] [player 2]`.  
`width` and `height` are between 3 and 16.  
Player 1 and 2 can be: `human`, `AI` or `AI0`. `AI` will make mistakes; `AI0` will not.  
Note that when `width` equals `height`, the board is trivially won by taking the piece (1, 1).
