#ifndef CHESS_H
#define CHESS_H

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QDialog>
#include <QComboBox>

// # TODO
//
// ## General
//
// [x] Refactor all x and y coordinates to use QPoint instead
// [ ] Refactor MoveFlags to MoveType, because the flags are mutually exclusive
// [ ] Split into different files
// [ ] Look for opportunities to refactor and clean up code and collect them in this TODO
// [ ] Implement a history with undo and redo
// [ ] Implement save game
// -> [ ] Clean up move checking routines
// -> [ ] Add a check when castling to not allow castling when squares are under attack
// -> [ ] Fix isKingInCheck on GameState
// -> [ ] Implement valid move checking for colors separately
//
// ## UI
//
// [x] Extend BoardView to be able to view from black perspective
// [ ] Build UI for move history
// [ ] Build UI for players and timers
// [ ] Build UI menu bar for loading, saving and starting a new game
//
// ## Bugfix
//
// [x] Fix promotion bug
// [x] Fix castle bug
// [x] Check castle swap positions

namespace Chess
{

enum class Color : uint8_t
{
    White,
    Black,
};

static constexpr size_t COLOR_COUNT = 2;

std::underlying_type<Color>::type indexOfColor(Color color);
Color oppositeColor(Color color);

enum class PieceType : uint8_t
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

    bool operator==(const Piece& other) const;
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

    PromotionAny = PromotionKnight | PromotionBishop | PromotionRook | PromotionQueen,
    CastleAny = CastleKingSide | CastleQueenSide
};

// Right now this struct is used all over the application, also for undo / redo.
// It needs to hold all information to display all information and
// undo or redo a move without additional data about the previous or current state of the chess board
// Thus it is not a packed move struct you may see in other chess programs
struct Move
{
    Piece piece;

    QPoint from;
    QPoint to;

    std::optional<Piece> capture;

    uint8_t flags;

    bool operator==(const Move& other) const;

    bool isCapture() const;

    Move withFlags(uint8_t flags) const;
};


class Board
{
public:
    static constexpr size_t WIDTH = 8;
    static constexpr size_t HEIGHT = 8;

    static Board standardSetup();

    void setPiece(QPoint pos, Piece piece);
    void setPiece(QPoint pos, Color color, PieceType type);

    void setEmptyAt(QPoint pos);

    bool isEmptyAt(QPoint pos) const;
    bool hasPieceAt(QPoint pos) const;

    // Checks whether or not the given position are a valid square
    bool isValid(QPoint pos) const;

    std::optional<Piece> pieceAt(QPoint pos) const;
    std::optional<Piece>& pieceAt(QPoint pos);

    bool tryMovePiece(QPoint from, QPoint to);

    void clearPieces();

    constexpr size_t width() const;
    constexpr size_t height() const;

    void clear();
private:
    std::array<std::array<std::optional<Piece>, WIDTH>, HEIGHT> m_squares;
};

std::optional<QPoint> findPiece(const Board& board, Piece piece);

class Position
{
public:
    Position();

    // NOTE: Move needs to be legal. Validate with isLegalMove or call getLegalMoves to obtain a list of legal moves.
    Position nextPosition(const Move &move) const;

    // NOTE: Move needs to be legal. Validate with isLegalMove or call getLegalMoves to obtain a list of legal moves.
    void doMove(const Move& move);
    void undoMove(const Move& move);

    // Returns a list of legal moves only for the piece at the given location.
    // If there is no piece at the given location an empty list is returned.
    // This is a special case of getLegalMoves, which returns all legal moves
    QVector<Move> getLegalMoves(QPoint pos) const;

    // Returns a list of all legal moves of the current positio.
    // This respects all chess rules, i.e
    // which player's turn it is, pinned pieces can't move, a king is checked or checkmated, 50-move-rule etc.
    QVector<Move> getLegalMoves() const;

    bool isLegalMove(const Move& move);

    bool isKingInCheck(Color color) const;
    bool isKingInCheck() const;

//    bool isCheckmate() const;
//    bool isStalemate() const;
//    bool isInsufficientMaterial() const;
//    bool isFiftyMoveRule() const;

    const Board& board() const;
    Color currentPlayer() const;

    bool canCastleKingSide(Color color) const;
    bool canCastleQueenSide(Color color) const;
private:
    // Gets the moves regardless of whether or not its the current player's turn.
    // This doesn't respect pins or moves that leave the king in check.
    // It returns a list of all threats of the player's given color
    QVector<Move> getCurrentThreats(Color color) const;

    void addPossibleMoves(QVector<Move> &moves, QPoint pos, bool onlyAttackingMoves = false) const;

    // Removes all the moves that would leave the king with the given color in check
    void removeKingInCheckMoves(QVector<Move> &moves, Color kingColor) const;

    void addDirectionalMoves(QVector<Move> &moves,
                             QPoint start,
                             QPoint direction,
                             size_t maxDistance = std::numeric_limits<size_t>::max(),
                             bool canCapture = true
                             ) const;

    QPoint getEnPassantSquare() const;
private:
    // keeps tracks of a potential last turns two square pawn advance to enable en passant
    std::optional<Move> m_twoSquareAdvance;

    static_assert(static_cast<std::underlying_type<Color>::type>(Color::White) == 0);
    static_assert(static_cast<std::underlying_type<Color>::type>(Color::Black) == 1);

    std::array<bool, COLOR_COUNT> m_canCastleKingSide = {true, true};
    std::array<bool, COLOR_COUNT> m_canCastleQueenSide = {true, true};

    Color m_currentPlayer;
    Board m_board;
};

class MoveHistory {

public:
    MoveHistory(Position position);

    void undo();
    void redo();

    void setCurrentIndex(size_t index);
    void addMove(const Move& move);

    const std::optional<Move> lastMove() const;
    const QVector<Move>& moves() const;

    const Position& basePosition() const;
    Position currentPosition() const;

    Position headPosition() const;

    void clear();
private:
    size_t m_nextMoveIndex = 0;
    QVector<Move> m_moves;
    Position m_basePosition;
};

class MoveHistoryView : public QWidget {
    Q_OBJECT
public:
    explicit MoveHistoryView(QWidget *parent = nullptr);

    void setHistory(const MoveHistory *history);
private:
    QListWidget* m_historyListWidget;
    const MoveHistory* m_history;
};

class BoardView : public QWidget
{
    Q_OBJECT
public:
    explicit BoardView(const Board *board, QWidget *parent = nullptr);

    static constexpr float SQUARE_SIZE = 90.0f;

    static constexpr QColor LIGHT_COLOR = QColor(150, 120, 75);
    static constexpr QColor DARK_COLOR = QColor(100, 80, 50);

    struct Highlight
    {
        QPoint pos;
        QColor color;
    };

signals:
    void squareClicked(QPoint pos);
public slots:
public:
    void setViewForPlayer(Color color);
    void setBoard(const Board *board);

    void addHighlight(QPoint pos, QColor color);
    void clearHighlights();

    void addMoveIndicator(const Move& move);
    void clearMoveIndicators();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    // Positioning
    QPoint getViewAdjustedPos(QPoint pos) const;
    QRectF getSquareRect(QPoint pos) const;
    QPointF getSquarePos(QPoint pos) const;
    QSizeF getSquareSize() const;
    QPoint getPosFromCursor(QPoint cursor) const;

    // Painting
    float strokeWidth() const;

    void paintSquares(QPainter& painter) const;
    void paintHoveredPos(QPainter &painter) const;
    void paintHighlights(QPainter& painter) const;
    void paintPieces(QPainter& painter) const;
    void paintMoveIndicators(QPainter &painter) const;

    static QPixmap getPiecePixmap(Piece piece);
private:
    const Board *m_board;

    // Determines from which side of the board the game is viewed
    Color m_viewForPlayer = Color::White;

    std::optional<QPoint> m_hoveredPos;
    QVector<Highlight> m_highlights;
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

enum class PlayerType
{
    Human,
    EasyBot,
};

struct MatchSettings
{
    PlayerType white = PlayerType::Human;
    PlayerType black = PlayerType::Human;

    PlayerType getPlayerByColor(Color color) const;
};

class NewGameDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewGameDialog(QWidget *parent = nullptr);

    MatchSettings getMatchSettings();

    std::optional<PlayerType> getPlayerTypeByName(QString name);
    std::optional<QString> getPlayerTypeName(PlayerType playerType);

    static const std::vector<std::pair<QString, PlayerType>>& playerTypes();
private:

    QComboBox* createPlayerComboBox();
private:
    MatchSettings m_matchSettings;
};

enum class EndReason
{
    CheckMate,
    StaleMate,
    InsufficientMaterial,
    ThreefoldRepetition,
    FiftyMoveRule,
    OutOfTime,
    Resignation,
};

struct GameResult
{
    EndReason endReason;
    std::optional<Color> winner;
};

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSquareClicked(QPoint pos);
    void onNewAction();
private:
    void startNewGame(const MatchSettings& settings);
    void selectPieceAt(QPoint pos);

    void playMove(Move move);

    std::optional<Move> trySelectMove(QPoint from, QPoint to);
    void selectAndPlayHumanMove(QPoint from, QPoint to);
    void showCheckIndicator();

    void setupUI();

    void showGameResult();
    void showGameResult(GameResult gameResult);

    bool isGameOver() const;

    PlayerType getCurrentPlayerType() const;

    bool isHumansTurn() const;

    void doAiMove();

private:
    std::optional<QPoint> m_selectedPos;


    MatchSettings m_matchSettings;

    Position m_currentPosition;
    MoveHistory m_history;

    BoardView* m_boardView;
    MoveHistoryView *m_historyView;
};

QString getAlgebraicNotation(const Move& move, const Position& resultingPosition);

}

#endif // CHESS_H
