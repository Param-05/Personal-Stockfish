#include <iostream>
#include <vector>
#include <string>
#include <set>
using namespace std;

/*
Chess peices -
1. Pawn
2. Knight
3. Bishop
4. Rook
5. Queen
6. King

enum is best
0. for empty cell
*/

enum Piece
{
    EMPTY,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

// Enum for colors
enum Color
{
    NONE,
    WHITE,
    BLACK
};

// Structure to represent a square on the board
struct Square
{
    Piece piece;
    Color color;

    Square() : piece(EMPTY), color(NONE) {} // Default constructor

    // Parameterized constructor
    Square(Piece p, Color c) : piece(p), color(c) {}
};

// Create a ChessBoard
class ChessBoard
{
private:
    vector<vector<Square>> board;

public:
    ChessBoard()
    {
        //  Initaliaze a 8x8 board
        board.resize(8, vector<Square>(8));
        initializeBoard();
    };
    void initializeBoard()
    {
        // Initialize the board with pieces
        // Put pawns
        for (int i = 0; i < 8; i++)
        {
            board[1][i] = {PAWN, BLACK};
            board[6][i] = {PAWN, WHITE};
        }

        // Place other pieces for Black
        board[0][0] = board[0][7] = {ROOK, BLACK};
        board[0][1] = board[0][6] = {KNIGHT, BLACK};
        board[0][2] = board[0][5] = {BISHOP, BLACK};
        board[0][3] = {QUEEN, BLACK};
        board[0][4] = {KING, BLACK};

        // Place other pieces for White
        board[7][0] = board[7][7] = {ROOK, WHITE};
        board[7][1] = board[7][6] = {KNIGHT, WHITE};
        board[7][2] = board[7][5] = {BISHOP, WHITE};
        board[7][3] = {QUEEN, WHITE};
        board[7][4] = {KING, WHITE};
    };
    void printBoard()
    {
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                const Square &square = board[row][col];
                char pieceChar = '.';
                if (square.piece != EMPTY)
                {
                    pieceChar = getPieceChar(square);
                }
                std::cout << pieceChar << " ";
            }
            std::cout << std::endl;
        }
    }
    char getPieceChar(const Square &square) const
    {
        char pieceChar = '.';
        switch (square.piece)
        {
        case PAWN:
            pieceChar = 'P';
            break;
        case KNIGHT:
            pieceChar = 'N';
            break;
        case BISHOP:
            pieceChar = 'B';
            break;
        case ROOK:
            pieceChar = 'R';
            break;
        case QUEEN:
            pieceChar = 'Q';
            break;
        case KING:
            pieceChar = 'K';
            break;
        default:
            pieceChar = '.';
            break;
        }
        return square.color == BLACK ? tolower(pieceChar) : pieceChar;
    }
};

int main()
{
    ChessBoard board;
    board.printBoard();
    return 0;
}
