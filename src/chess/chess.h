#ifndef CHESS_H
#define CHESS_H

#include <QMainWindow>

#include <QLabel>

#include <QDialog>


namespace Chess
{

enum Color : uint8_t
{
    White,
    Black
};

enum PieceType : uint8_t
{
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
};

struct Piece
{
    Color color;
    PieceType type;
};

enum MoveFlags : uint8_t
{
    EnPassant = 1 << 0,

    TwoSquareAdvance = 1 << 1,

    PromotionKnight = 1 << 2,
    PromotionBishop = 1 << 3,
    PromotionRook = 1 << 4,
    PromotionQueen = 1 << 5,

    CastleKingSide = 1 << 6,
    CastleQueenSide = 1 << 7,

    PromotionAny = PromotionKnight | PromotionBishop | PromotionRook | PromotionQueen
};

struct Move
{
    QPoint from;
    QPoint to;

    uint8_t flags;

    bool operator==(const Move& other) const;

    Move withFlags(uint8_t flags);
};

class Board
{
public:
    // Creates a standard 8x8 empty board.
    Board();
    Board(size_t width, size_t height);

    static Board standardSetup();

    void setPiece(size_t x, size_t y, Piece piece);
    void setPiece(size_t x, size_t y, Color color, PieceType type);

    void setEmptyAt(size_t x, size_t y);

    bool isEmptyAt(size_t x, size_t y) const;
    bool hasPieceAt(size_t x, size_t y) const;

    // Checks whether or not the given position are a valid square
    bool isValid(size_t x, size_t y) const;
    bool isValid(QPoint pos) const;

    std::optional<Piece> pieceAt(size_t x, size_t y) const;

    bool tryMovePiece(size_t x0, size_t y0, size_t x1, size_t y1);
    bool tryMovePiece(Move move);

    void clearPieces();

    size_t width() const;
    size_t height() const;

    void clear();
private:
    size_t index(size_t x, size_t y) const;
private:
    std::vector<std::optional<Piece>> m_squares;

    size_t m_width;
    size_t m_height;
};

class ChessState
{

public:
    ChessState();

    std::optional<ChessState> nextState(Move move, bool validate = true);

    QVector<Move> getValidMoves(size_t x, size_t y, bool checkForKıngSafety = true);
    QVector<Move> getAllValidMoves(bool checkForKıngSafety = true);

    bool isKingInCheck(Color color);

    const Board& board() const;
private:
    void addDirectionalMoves(QVector<Move> &moves,
                             QPoint start,
                             QPoint direction,
                             size_t maxDistance = std::numeric_limits<size_t>::max(),
                             bool canCapture = true
                             );
    QPoint getEnPassantSquare();
private:
    // keeps tracks of a potential last turns two square pawn advance to enable en passant
    std::optional<Move> m_twoSquareAdvance;

    Color m_currentPlayer;
    Board m_board;
};

class BoardView : public QWidget
{
    Q_OBJECT
public:
    explicit BoardView(const Board *board, QWidget *parent = nullptr);

signals:
    void squareClicked(size_t x, size_t y);
public slots:
public:
    void setBoard(const Board *board);

    void addHighlight(size_t x, size_t y);
    void clearHighlights();

    void addMoveIndicator(Move move);
    void clearMoveIndicators();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    QPoint calcSquarePos(QPoint cursor);
private:
    const Board *m_board;

    std::optional<QPoint> m_hoveredPos;
    QVector<QPoint> m_highlightedSquares;
    QVector<Move> m_moves;
};

class PromotionDialog : public QDialog {
    Q_OBJECT
public:
    explicit PromotionDialog(QWidget *parent = nullptr);

    PieceType getPieceType();
private:
    PieceType m_pieceType;
};

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSquareClicked(size_t x, size_t y);
private:
    void setupUI();
private:
    std::optional<QPoint> m_selectedPos;

    ChessState m_gameState;

    BoardView* m_boardView;
};

}

#endif // CHESS_H
