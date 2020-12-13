# Freecell

The FreeCell is a single-player complete information puzzle game. The
game starts with a random distribution of a standard 52-card deck in 8
piles (called *cascade*) and ends when the player completes the 4 suits
in the foundation.

The rules of the game are as follow: (1) the player can only move one
card at a time, (2) the player can only move cards from the cascades or
the freecells, (3) the player can only move cards to an empty freecell,
an empty cascade, a cascade if it's bottom-most card is of opposite
color and of next value, or it's corresponding suit foundation if its
last card is of previous value.

This repository hosts the author's various attempts to write a solver.

## Solver

So far the following algorithms have been tested:

* uninformed depth-first
* uninformed breadth-first
* A* (using naive heuristic)
* uninformed iterative deeping
* iterative deeping A* (using naive heuristic)


## See also

* <https://freecellgamesolutions.com/>
