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

// Define a structure to represent a Move
struct Move
{
    Piece piece;         // The piece being moved
    Color color;         // Color of the piece
    pair<int, int> from; // Starting position (row, col)
    pair<int, int> to;   // Ending position (row, col)

    Move(Piece p, Color c, pair<int, int> f, pair<int, int> t)
        : piece(p), color(c), from(f), to(t) {}

    // Optional: Define comparison operator for sorting/uniqueness
    bool operator<(const Move &other) const
    {
        if (from != other.from)
            return from < other.from;
        return to < other.to;
    }
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
    // Pseudo-legal moves generation for each piece type

    void generatePawnMoves(set<Move> &moves, int row, int col, Color color)
{
    int direction = (color == WHITE) ? -1 : 1;
    
    // Forward movement
    if (board[row + direction][col].piece == EMPTY)
    {
        moves.insert(Move(PAWN, color, {row, col}, {row + direction, col}));
        if ((color == WHITE && row == 6) || (color == BLACK && row == 1))
        {
            if (board[row + 2 * direction][col].piece == EMPTY)
            {
                moves.insert(Move(PAWN, color, {row, col}, {row + 2 * direction, col}));
            }
        }
    }

    // Capture diagonally
    if (col - 1 >= 0 && board[row + direction][col - 1].color != color && board[row + direction][col - 1].piece != EMPTY)
    {
        moves.insert(Move(PAWN, color, {row, col}, {row + direction, col - 1}));
    }
    if (col + 1 < 8 && board[row + direction][col + 1].color != color && board[row + direction][col + 1].piece != EMPTY)
    {
        moves.insert(Move(PAWN, color, {row, col}, {row + direction, col + 1}));
    }
}


    void generateKnightMoves(set<Move> &moves, int row, int col, Color color)
    {
        int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
        for (auto &move : knightMoves)
        {
            int newRow = row + move[0];
            int newCol = col + move[1];
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
            {
                if (board[newRow][newCol].color != color)
                {
                    moves.insert(Move(KNIGHT, color, {row, col}, {newRow, newCol}));
                }
            }
        }
    }

    void generateBishopMoves(set<Move> &moves, int row, int col, Color color)
    {
        // 4 diagonal directions: top-left, top-right, bottom-left, bottom-right
        int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
        for (auto &dir : directions)
        {
            int r = row;
            int c = col;
            while (true)
            {
                r += dir[0];
                c += dir[1];
                if (r < 0 || r >= 8 || c < 0 || c >= 8)
                    break;
                if (board[r][c].color == color)
                    break;
                moves.insert(Move(BISHOP, color, {row, col}, {r, c}));
                if (board[r][c].color != NONE)
                    break; // Stop at the first piece
            }
        }
    }

    void generateRookMoves(set<Move> &moves, int row, int col, Color color)
    {
        // 4 vertical and horizontal directions: up, down, left, right
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (auto &dir : directions)
        {
            int r = row;
            int c = col;
            while (true)
            {
                r += dir[0];
                c += dir[1];
                if (r < 0 || r >= 8 || c < 0 || c >= 8)
                    break;
                if (board[r][c].color == color)
                    break;
                moves.insert(Move(ROOK, color, {row, col}, {r, c}));
                if (board[r][c].color != NONE)
                    break; // Stop at the first piece
            }
        }
    }

    void generateQueenMoves(set<Move> &moves, int row, int col, Color color)
    {
        generateRookMoves(moves, row, col, color);
        generateBishopMoves(moves, row, col, color);
    }

    void generateKingMoves(set<Move> &moves, int row, int col, Color color)
    {
        int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
        for (auto &dir : directions)
        {
            int r = row + dir[0];
            int c = col + dir[1];
            if (r >= 0 && r < 8 && c >= 0 && c < 8)
            {
                if (board[r][c].color != color)
                {
                    moves.insert(Move(KING, color, {row, col}, {r, c}));
                }
            }
        }
    }

    set<Move> getPseudoLegalMoves(Color color)
    {
        set<Move> moves;
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                Square &square = board[row][col];
                if (square.color == color)
                {
                    switch (square.piece)
                    {
                    case PAWN:
                        generatePawnMoves(moves, row, col, color);
                        break;
                    case KNIGHT:
                        generateKnightMoves(moves, row, col, color);
                        break;
                    case BISHOP:
                        generateBishopMoves(moves, row, col, color);
                        break;
                    case ROOK:
                        generateRookMoves(moves, row, col, color);
                        break;
                    case QUEEN:
                        generateQueenMoves(moves, row, col, color);
                        break;
                    case KING:
                        generateKingMoves(moves, row, col, color);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        return moves;
    }
};

int main()
{
    ChessBoard board;
    board.printBoard();
    set<Move> whiteMoves = board.getPseudoLegalMoves(WHITE);
    cout << "\nWhite Pseudo-Legal Moves:\n";
    for (auto &move : whiteMoves)
    {
        cout << "(" << move.color << "," << move.piece << "," << move.from.first << "," << move.from.second << ") -> ("
             << move.to.first << "," << move.to.second << ")\n";
    }
    return 0;
}
