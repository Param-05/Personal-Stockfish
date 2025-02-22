#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cmath>
using namespace std;

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

enum Color
{
    NONE,
    WHITE,
    BLACK
};

struct Square
{
    Piece piece;
    Color color;
    Square() : piece(EMPTY), color(NONE) {}
    Square(Piece p, Color c) : piece(p), color(c) {}
};

struct Move
{
    Piece piece;         // The piece being moved
    Color color;         // Color of the piece
    pair<int, int> from; // Starting position (row, col)
    pair<int, int> to;   // Ending position (row, col)
    Piece promotedPiece; // In case of promotion

    Move(Piece p, Color c, pair<int, int> f, pair<int, int> t, Piece promo = EMPTY)
        : piece(p), color(c), from(f), to(t), promotedPiece(promo) {}

    bool operator<(const Move &other) const
    {
        if (from != other.from)
            return from < other.from;
        if (to != other.to)
            return to < other.to;
        return promotedPiece < other.promotedPiece;
    }
};

struct Bitboards
{
    uint64_t whitePawns;
    uint64_t whiteKnights;
    uint64_t whiteBishops;
    uint64_t whiteRooks;
    uint64_t whiteQueens;
    uint64_t whiteKing;
    uint64_t blackPawns;
    uint64_t blackKnights;
    uint64_t blackBishops;
    uint64_t blackRooks;
    uint64_t blackQueens;
    uint64_t blackKing;
};

class ChessBoard
{
private:
    vector<vector<Square>> board;
    vector<Move> moveHistory;
    pair<int, int> enPassantTarget; // (-1,-1) when none

    // Castling rights flags.
    bool whiteKingMoved, blackKingMoved;
    bool whiteRookAMoved, whiteRookHMoved;
    bool blackRookAMoved, blackRookHMoved;

public:
    ChessBoard()
    {
        board.resize(8, vector<Square>(8));
        enPassantTarget = { -1, -1 };
        whiteKingMoved = blackKingMoved = false;
        whiteRookAMoved = whiteRookHMoved = false;
        blackRookAMoved = blackRookHMoved = false;
        initializeBoard();
    }

    void initializeBoard()
    {
        // Initialize pawns.
        for (int i = 0; i < 8; i++)
        {
            board[1][i] = Square(PAWN, BLACK);
            board[6][i] = Square(PAWN, WHITE);
        }
        // Black pieces.
        board[0][0] = board[0][7] = Square(ROOK, BLACK);
        board[0][1] = board[0][6] = Square(KNIGHT, BLACK);
        board[0][2] = board[0][5] = Square(BISHOP, BLACK);
        board[0][3] = Square(QUEEN, BLACK);
        board[0][4] = Square(KING, BLACK);
        // White pieces.
        board[7][0] = board[7][7] = Square(ROOK, WHITE);
        board[7][1] = board[7][6] = Square(KNIGHT, WHITE);
        board[7][2] = board[7][5] = Square(BISHOP, WHITE);
        board[7][3] = Square(QUEEN, WHITE);
        board[7][4] = Square(KING, WHITE);
    }

    // A helper to let us access a square (used in user input processing).
    Square getSquare(int row, int col) const
    {
        return board[row][col];
    }

    void printBoard()
    {
        cout << "\n  0 1 2 3 4 5 6 7\n";
        for (int row = 0; row < 8; row++)
        {
            cout << row << " ";
            for (int col = 0; col < 8; col++)
            {
                char pieceChar = '.';
                if (board[row][col].piece != EMPTY)
                    pieceChar = getPieceChar(board[row][col]);
                cout << pieceChar << " ";
            }
            cout << endl;
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

    // --- Pseudo-Legal Move Generation Functions ---

    void generatePawnMoves(set<Move> &moves, int row, int col, Color color)
    {
        int direction = (color == WHITE) ? -1 : 1;
        int newRow = row + direction;
        if (newRow >= 0 && newRow < 8 && board[newRow][col].piece == EMPTY)
        {
            bool isPromotion = (color == WHITE && newRow == 0) || (color == BLACK && newRow == 7);
            if (isPromotion)
            {
                vector<Piece> promotions = { QUEEN, ROOK, BISHOP, KNIGHT };
                for (Piece promo : promotions)
                    moves.insert(Move(PAWN, color, { row, col }, { newRow, col }, promo));
            }
            else
            {
                moves.insert(Move(PAWN, color, { row, col }, { newRow, col }));
                if ((color == WHITE && row == 6) || (color == BLACK && row == 1))
                {
                    int doubleRow = row + 2 * direction;
                    if (doubleRow >= 0 && doubleRow < 8 && board[doubleRow][col].piece == EMPTY)
                        moves.insert(Move(PAWN, color, { row, col }, { doubleRow, col }));
                }
            }
        }
        // Captures.
        for (int dcol : { -1, 1 })
        {
            int targetCol = col + dcol;
            if (targetCol < 0 || targetCol >= 8)
                continue;
            if (newRow >= 0 && newRow < 8)
            {
                Square target = board[newRow][targetCol];
                if (target.color != color && target.color != NONE)
                {
                    bool isPromotion = (color == WHITE && newRow == 0) || (color == BLACK && newRow == 7);
                    if (isPromotion)
                    {
                        vector<Piece> promotions = { QUEEN, ROOK, BISHOP, KNIGHT };
                        for (Piece promo : promotions)
                            moves.insert(Move(PAWN, color, { row, col }, { newRow, targetCol }, promo));
                    }
                    else
                    {
                        moves.insert(Move(PAWN, color, { row, col }, { newRow, targetCol }));
                    }
                }
            }
        }
        // En passant.
        if (enPassantTarget.first != -1)
        {
            int epRow = enPassantTarget.first;
            int epCol = enPassantTarget.second;
            if ((color == WHITE && row == 3) || (color == BLACK && row == 4))
            {
                if (epRow == newRow && abs(epCol - col) == 1)
                    moves.insert(Move(PAWN, color, { row, col }, { epRow, epCol }));
            }
        }
    }

    void generateKnightMoves(set<Move> &moves, int row, int col, Color color)
    {
        int knightMoves[8][2] = { { -2, -1 }, { -2, 1 }, { -1, -2 }, { -1, 2 },
                                  { 1, -2 }, { 1, 2 }, { 2, -1 }, { 2, 1 } };
        for (auto &m : knightMoves)
        {
            int newRow = row + m[0], newCol = col + m[1];
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
                if (board[newRow][newCol].color != color)
                    moves.insert(Move(KNIGHT, color, { row, col }, { newRow, newCol }));
        }
    }

    void generateBishopMoves(set<Move> &moves, int row, int col, Color color)
    {
        int directions[4][2] = { { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };
        for (auto &dir : directions)
        {
            int r = row, c = col;
            while (true)
            {
                r += dir[0];
                c += dir[1];
                if (r < 0 || r >= 8 || c < 0 || c >= 8)
                    break;
                if (board[r][c].color == color)
                    break;
                moves.insert(Move(BISHOP, color, { row, col }, { r, c }));
                if (board[r][c].color != NONE)
                    break;
            }
        }
    }

    void generateRookMoves(set<Move> &moves, int row, int col, Color color)
    {
        int directions[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
        for (auto &dir : directions)
        {
            int r = row, c = col;
            while (true)
            {
                r += dir[0];
                c += dir[1];
                if (r < 0 || r >= 8 || c < 0 || c >= 8)
                    break;
                if (board[r][c].color == color)
                    break;
                moves.insert(Move(ROOK, color, { row, col }, { r, c }));
                if (board[r][c].color != NONE)
                    break;
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
        int directions[8][2] = { { -1, -1 }, { -1, 0 }, { -1, 1 },
                                 { 0, -1 },            { 0, 1 },
                                 { 1, -1 },  { 1, 0 }, { 1, 1 } };
        for (auto &dir : directions)
        {
            int r = row + dir[0], c = col + dir[1];
            if (r >= 0 && r < 8 && c >= 0 && c < 8)
                if (board[r][c].color != color)
                    moves.insert(Move(KING, color, { row, col }, { r, c }));
        }
        // --- Castling ---
        if (color == WHITE && !whiteKingMoved && row == 7 && col == 4)
        {
            // Kingside castling.
            if (!whiteRookHMoved && board[7][5].piece == EMPTY && board[7][6].piece == EMPTY)
            {
                if (!isSquareAttacked(7, 4, BLACK) &&
                    !isSquareAttacked(7, 5, BLACK) &&
                    !isSquareAttacked(7, 6, BLACK))
                    moves.insert(Move(KING, WHITE, { 7, 4 }, { 7, 6 }));
            }
            // Queenside castling.
            if (!whiteRookAMoved && board[7][1].piece == EMPTY &&
                board[7][2].piece == EMPTY && board[7][3].piece == EMPTY)
            {
                if (!isSquareAttacked(7, 4, BLACK) &&
                    !isSquareAttacked(7, 3, BLACK) &&
                    !isSquareAttacked(7, 2, BLACK))
                    moves.insert(Move(KING, WHITE, { 7, 4 }, { 7, 2 }));
            }
        }
        else if (color == BLACK && !blackKingMoved && row == 0 && col == 4)
        {
            // Kingside castling.
            if (!blackRookHMoved && board[0][5].piece == EMPTY && board[0][6].piece == EMPTY)
            {
                if (!isSquareAttacked(0, 4, WHITE) &&
                    !isSquareAttacked(0, 5, WHITE) &&
                    !isSquareAttacked(0, 6, WHITE))
                    moves.insert(Move(KING, BLACK, { 0, 4 }, { 0, 6 }));
            }
            // Queenside castling.
            if (!blackRookAMoved && board[0][1].piece == EMPTY &&
                board[0][2].piece == EMPTY && board[0][3].piece == EMPTY)
            {
                if (!isSquareAttacked(0, 4, WHITE) &&
                    !isSquareAttacked(0, 3, WHITE) &&
                    !isSquareAttacked(0, 2, WHITE))
                    moves.insert(Move(KING, BLACK, { 0, 4 }, { 0, 2 }));
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
                if (board[row][col].color == color)
                {
                    switch (board[row][col].piece)
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

    // --- Helpers for Move Legality Checks ---
    bool isSquareAttacked(int row, int col, Color attackerColor)
    {
        set<Move> enemyMoves = getPseudoLegalMoves(attackerColor);
        for (const auto &move : enemyMoves)
            if (move.to.first == row && move.to.second == col)
                return true;
        return false;
    }

    bool isKingInCheck(Color color)
    {
        int kingRow = -1, kingCol = -1;
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (board[i][j].piece == KING && board[i][j].color == color)
                {
                    kingRow = i;
                    kingCol = j;
                    break;
                }
            }
            if (kingRow != -1)
                break;
        }
        if (kingRow == -1)
            return false;
        Color enemy = (color == WHITE) ? BLACK : WHITE;
        return isSquareAttacked(kingRow, kingCol, enemy);
    }

    // --- Move Execution ---
    void applyMove(const Move &move)
    {
        int sr = move.from.first, sc = move.from.second;
        int dr = move.to.first, dc = move.to.second;
        Square &src = board[sr][sc];
        Square &dest = board[dr][dc];
        bool isCastling = (move.piece == KING && abs(dc - sc) == 2);
        bool isEnPassant = (move.piece == PAWN && sc != dc && dest.piece == EMPTY);

        moveHistory.push_back(move);

        if (isCastling)
        {
            board[dr][dc] = src;
            src = Square();
            if (dc > sc) // Kingside.
            {
                Square &rook = board[dr][7];
                board[dr][dc - 1] = rook;
                board[dr][7] = Square();
            }
            else // Queenside.
            {
                Square &rook = board[dr][0];
                board[dr][dc + 1] = rook;
                board[dr][0] = Square();
            }
            if (move.color == WHITE)
                whiteKingMoved = true;
            else
                blackKingMoved = true;
        }
        else
        {
            if (isEnPassant)
            {
                int capturedRow = (move.color == WHITE) ? dr + 1 : dr - 1;
                board[capturedRow][dc] = Square();
            }
            board[dr][dc] = src;
            src = Square();
            if (move.promotedPiece != EMPTY)
                board[dr][dc].piece = move.promotedPiece;
        }
        if (move.piece == PAWN && abs(dr - sr) == 2)
            enPassantTarget = { (sr + dr) / 2, sc };
        else
            enPassantTarget = { -1, -1 };

        if (move.piece == KING)
        {
            if (move.color == WHITE)
                whiteKingMoved = true;
            else
                blackKingMoved = true;
        }
        if (move.piece == ROOK)
        {
            if (move.color == WHITE)
            {
                if (sr == 7 && sc == 0)
                    whiteRookAMoved = true;
                if (sr == 7 && sc == 7)
                    whiteRookHMoved = true;
            }
            else
            {
                if (sr == 0 && sc == 0)
                    blackRookAMoved = true;
                if (sr == 0 && sc == 7)
                    blackRookHMoved = true;
            }
        }
    }

    set<Move> getLegalMoves(Color color)
    {
        set<Move> legal;
        set<Move> pseudo = getPseudoLegalMoves(color);
        for (const auto &move : pseudo)
        {
            ChessBoard temp = *this;
            temp.applyMove(move);
            if (!temp.isKingInCheck(color))
                legal.insert(move);
        }
        return legal;
    }

    // --- Bitboard Generation ---
    Bitboards getBitboards() const
    {
        Bitboards bb = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL,
                         0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                int index = row * 8 + col;
                uint64_t bit = 1ULL << index;
                const Square &sq = board[row][col];
                if (sq.piece != EMPTY)
                {
                    if (sq.color == WHITE)
                    {
                        switch (sq.piece)
                        {
                        case PAWN:
                            bb.whitePawns |= bit;
                            break;
                        case KNIGHT:
                            bb.whiteKnights |= bit;
                            break;
                        case BISHOP:
                            bb.whiteBishops |= bit;
                            break;
                        case ROOK:
                            bb.whiteRooks |= bit;
                            break;
                        case QUEEN:
                            bb.whiteQueens |= bit;
                            break;
                        case KING:
                            bb.whiteKing |= bit;
                            break;
                        default:
                            break;
                        }
                    }
                    else if (sq.color == BLACK)
                    {
                        switch (sq.piece)
                        {
                        case PAWN:
                            bb.blackPawns |= bit;
                            break;
                        case KNIGHT:
                            bb.blackKnights |= bit;
                            break;
                        case BISHOP:
                            bb.blackBishops |= bit;
                            break;
                        case ROOK:
                            bb.blackRooks |= bit;
                            break;
                        case QUEEN:
                            bb.blackQueens |= bit;
                            break;
                        case KING:
                            bb.blackKing |= bit;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
        return bb;
    }
};

int main()
{
    ChessBoard board;
    Color currentTurn = WHITE;
    string inputLine;

    while (true)
    {
        board.printBoard();
        cout << "\n" << ((currentTurn == WHITE) ? "White" : "Black")
             << " to move. Enter move as: fromRow fromCol toRow toCol (or type 'exit'): ";
        getline(cin, inputLine);
        if (inputLine == "exit")
            break;

        istringstream iss(inputLine);
        int sr, sc, dr, dc;
        if (!(iss >> sr >> sc >> dr >> dc))
        {
            cout << "Invalid input. Please enter four integers." << endl;
            continue;
        }
        // Retrieve the piece at the source square.
        Square sourceSquare = board.getSquare(sr, sc);
        if (sourceSquare.piece == EMPTY || sourceSquare.color != currentTurn)
        {
            cout << "No valid piece at the source square for the current turn." << endl;
            continue;
        }
        Move playerMove(sourceSquare.piece, sourceSquare.color, { sr, sc }, { dr, dc });
        set<Move> legalMoves = board.getLegalMoves(currentTurn);
        if (legalMoves.find(playerMove) == legalMoves.end())
        {
            cout << "Illegal move. Try again." << endl;
            continue;
        }
        board.applyMove(playerMove);
        // Switch turns.
        currentTurn = (currentTurn == WHITE) ? BLACK : WHITE;
    }
    cout << "Game over." << endl;
    return 0;
}
