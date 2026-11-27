#include <iostream>
#include <vector>
#include <conio.h>     // For _kbhit(), _getch()
#include <windows.h>   // For Sleep()
#include <ctime>       // For srand(), time()
using namespace std;

const int WIDTH = 25;
const int HEIGHT = 15;
int board[HEIGHT][WIDTH] = {0};  // 0 = empty, 1 = filled

// Tetromino shapes (7 total, 4x4 grids)
vector<vector<string>> tetromino = {
    { "....","****","....","...." }, // Horizontal I
    { ".*..",".*..",".**.","...." }, // Right L
    { ".*..",".*..","**..","...." }, // Left L
    { "....",".**.",".**.","...." }, // Square(4x4)
    { "....",".**.","**..","...." }, // Left Z
    { "....","**..",".**.","...." }, // Right Z
    { ".*..","***.","....","...." }, // Short T(downward)
};

//Rotate tetromino 90° clockwise
vector<string> rotate(const vector<string>& shape) {
    vector<string> r(4, "....");
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r[j][3 - i] = shape[i][j];
    return r;
}

// Check if shape fits at given position
bool isValid(const vector<string>& shape, int x, int y) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shape[i][j] == '*') {
                int nx = x + j;
                int ny = y + i;
                if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return false;
                if (ny >= 0 && board[ny][nx]) return false;
            }
    return true;
}

// Place shape permanently on the board
void place(const vector<string>& shape, int x, int y) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shape[i][j] == '*') {
                int nx = x + j;
                int ny = y + i;
                if (ny >= 0) board[ny][nx] = 1;
            }
}

// Clear completed lines and return number of cleared lines
int clearLines() {
    int cleared = 0;
    for (int i = HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < WIDTH; j++)
            if (!board[i][j]) full = false;

        if (full) {
            cleared++;
            for (int r = i; r > 0; r--)
                for (int j = 0; j < WIDTH; j++)
                    board[r][j] = board[r - 1][j];
            for (int j = 0; j < WIDTH; j++) board[0][j] = 0;
            i++; // re-check this row after shifting down
        }
    }
    return cleared;
}

// Draw the board + falling piece
void draw(const vector<string>& shape, int x, int y, int score) {
    system("cls");
    cout << "   WELCOME TO TETRIS GAME\n";
    cout << "   Score: " << score << "\n\n";

    for (int i = 0; i < HEIGHT; i++) {
        cout << "|";
        for (int j = 0; j < WIDTH; j++) {
            char c = (board[i][j] ? '*' : ' ');
            for (int si = 0; si < 4; si++)
                for (int sj = 0; sj < 4; sj++)
                    if (shape[si][sj] == '*') {
                        int nx = x + sj, ny = y + si;
                        if (i == ny && j == nx) c = '*';
                    }
            cout << c;
        }
        cout << "|\n";
    }

    // Draw bottom border
    cout << "|";
    for (int i = 0; i < WIDTH; i++) cout << "_";
    cout << "|\n";

    cout << " How to Play: \n A = Left | D = Right \n S = Down | W = Rotate \n Q = Quit\n";
}

int main() {
    srand(time(0));
    int score = 0;

    // Pick first random tetromino
    int current = rand() % tetromino.size();
    vector<string> shape = tetromino[current];
    int x = WIDTH / 2 - 2;
    int y = 0;

    while (true) {
        // Handle user input
        if (_kbhit()) {
            char c = _getch();
            if (c == 'a' && isValid(shape, x - 1, y)) x--;
            if (c == 'd' && isValid(shape, x + 1, y)) x++;
            if (c == 's' && isValid(shape, x, y + 1)) y++;
            if (c == 'w') {
                auto r = rotate(shape);
                if (isValid(r, x, y)) shape = r;
            }
            if (c == 'q' || c == 'Q') break;  // quit
        }

        // Move shape down
        if (isValid(shape, x, y + 1)) {
            y++;
        } else {
            // Lock shape, add score for placing piece
            place(shape, x, y);
            score += 10;

            // Clear lines and add bonus score
            int cleared = clearLines();
            score += cleared * 100;

            // Spawn a new shape
            current = rand() % tetromino.size();
            shape = tetromino[current];
            x = WIDTH / 2 - 2;
            y = 0;

            // Check game over
            if (!isValid(shape, x, y)) {
                system("cls");
                cout << "**** GAME OVER ****\n";
                cout << "Final Score: " << score << "\n";
                break;
            }
        }

        // Draw the updated board
        draw(shape, x, y, score);
        Sleep(300); // Delay for falling speed
    }

    return 0;
}

