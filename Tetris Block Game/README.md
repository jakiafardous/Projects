# **Console-Based Tetris Game Using C++**

## **Introduction**

Tetris is a classic block-stacking puzzle game in which different shapes fall from the top of a playing board. The player controls these falling shapes and tries to arrange them to complete horizontal lines. This project implements a simple console-based version of Tetris using the C++ programming language. Instead of graphical designs or colorful blocks, the game uses asterisks (`*`) to represent the blocks, making it simple and easy to understand.

## **How the Game Is Made**

The game is developed using C++ in the Code::Blocks IDE. A two-dimensional array is used to create the game board, which has 25 columns and 15 rows.

There are seven different types of block shapes, known as Tetrominoes. Each shape is represented using a 4×4 character grid, where `*` represents a block and `.` represents an empty space.

The program uses different functions to manage the game, such as:

- **rotate():** Rotates a block 90 degrees clockwise.
- **isValid():** Checks whether a block can move to a particular position without crossing the boundaries or overlapping existing blocks.
- **place():** Places a block permanently on the board when it reaches the bottom or another block.
- **clearLines():** Detects and removes completed horizontal lines.
- **draw():** Displays the game board, falling blocks and current score.

The program also uses random number generation to select different block shapes and keyboard input functions to control their movement.

## **How to Play**

When the game starts, a random block appears at the top of the board and begins falling automatically. The player can control its movement using the following keyboard keys:

 <ins>Key -- Function             

 A  --  Move the block left  
 D  --  Move the block right  
 S  --  Move the block down  
 W  --  Rotate the block     
 Q  --  Quit the game        

When a block can no longer move downward, it becomes a permanent part of the board, and a new random block appears.

## **Scoring System**

The game includes a simple scoring system to make it more interesting.

- 10 points are awarded whenever a block is successfully placed.
- 100 bonus points are awarded for each completed horizontal line.
- The current score is displayed at the top of the game board.

When a complete horizontal line is formed, it disappears, and the blocks above it move downward to fill the empty space.

## **Game Over Condition**

The game continues as long as new blocks can appear at the top of the board. When the existing blocks occupy the space required for a new block to appear, the game ends. The screen then displays **GAME OVER** along with the final score.

## **Technologies and Concepts Used**

- **Programming Language:** C++
- **IDE:** Code::Blocks
- **Header Files:** _iostream_, _vector_, _conio.h_, _windows.h_ and _ctime_.
- **Programming Concepts:**
  - Two-dimensional arrays
  - Implemented using functions.
  - Vectors and strings
  - Loops and conditional statements
  - User-defined functions
  - Random number generation
  - Keyboard input handling
  - Basic collision detection and game logic

## **Conclusion**

This project demonstrates how a simple interactive game can be developed using fundamental C++ programming concepts. It provides practical experience with arrays, vectors, functions, loops, conditional statements and keyboard input. By implementing block movement, rotation, collision detection, line clearing and score calculation, the project demonstrates the basic working principles of the classic Tetris game without requiring any advanced graphics libraries.
