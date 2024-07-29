# Multithreaded Knight's Tour
In this implementation of the Knight's Tour problem, a chess knight is placed on an m x n chessboard at a specified starting position (r, c). The algorithm will find a valid path for the knight that allows it to visit every square on the board exactly once. Both closed and open tours are acceptable outcomes: a closed tour is when the knight returns to its starting square, while an open tour visits every square without returning to the starting point.

Various techniques exist for solving the Knight's Tour problem. This algorithm uses multithreading to efficiently explore potential paths and optimize the search.

![gif](https://upload.wikimedia.org/wikipedia/commons/c/ca/Knights-Tour-Animation.gif)

_This gif is taken from [wikipedia](https://en.wikipedia.org/wiki/Knight%27s_tour)._

## Usage
**Input:** 

a.out \<m> \<n> \<r> \<c>

Where:
* m: Number of columns on the board
* n: Number of rows on the board
* r: Knight's starting row position
* c: Knight's starting column position
