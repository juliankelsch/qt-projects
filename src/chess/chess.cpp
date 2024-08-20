#include "chess.h"

#include <cassert>

#include <QLabel>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>

#include <QPainter>
#include <QDialog>

#include <QPixmapCache>
#include <QMouseEvent>
#include <QMessageBox>

#include <QRadioButton>
#include <QDialogButtonBox>

using namespace Chess;

Board::Board() : Board(8, 8)
{

}

Board::Board(size_t width, size_t height)
    : m_width(width),
    m_height(height),
    m_squares(width * height, std::nullopt)
{

}

Board Board::standardSetup()
{
    Board board;

    board.setPiece(0, 0, Black, Rook);
    board.setPiece(1, 0, Black, Knight);
    board.setPiece(2, 0, Black, Bishop);
    board.setPiece(3, 0, Black, Queen);
    board.setPiece(4, 0, Black, King);
    board.setPiece(5, 0, Black, Bishop);
    board.setPiece(6, 0, Black, Knight);
    board.setPiece(7, 0, Black, Rook);

    board.setPiece(0, 1, Black, Pawn);
    board.setPiece(1, 1, Black, Pawn);
    board.setPiece(2, 1, Black, Pawn);
    board.setPiece(3, 1, Black, Pawn);
    board.setPiece(4, 1, Black, Pawn);
    board.setPiece(5, 1, Black, Pawn);
    board.setPiece(6, 1, Black, Pawn);
    board.setPiece(7, 1, Black, Pawn);

    board.setPiece(0, 6, White, Pawn);
    board.setPiece(1, 6, White, Pawn);
    board.setPiece(2, 6, White, Pawn);
    board.setPiece(3, 6, White, Pawn);
    board.setPiece(4, 6, White, Pawn);
    board.setPiece(5, 6, White, Pawn);
    board.setPiece(6, 6, White, Pawn);
    board.setPiece(7, 6, White, Pawn);

    board.setPiece(0, 7, White, Rook);
    board.setPiece(1, 7, White, Knight);
    board.setPiece(2, 7, White, Bishop);
    board.setPiece(3, 7, White, Queen);
    board.setPiece(4, 7, White, King);
    board.setPiece(5, 7, White, Bishop);
    board.setPiece(6, 7, White, Knight);
    board.setPiece(7, 7, White, Rook);

    return board;
}

void Board::setPiece(size_t x, size_t y, Piece piece)
{
    m_squares[index(x, y)] = piece;
}

void Board::setPiece(size_t x, size_t y, Color color, PieceType type)
{
    setPiece(x, y, Piece{ .color = color, .type = type });
}

void Board::setEmptyAt(size_t x, size_t y)
{
    m_squares[index(x, y)].reset();
}

bool Board::isEmptyAt(size_t x, size_t y) const
{
    return !hasPieceAt(x, y);
}

bool Board::hasPieceAt(size_t x, size_t y) const
{
    return m_squares[index(x, y)].has_value();
}

bool Board::isValid(size_t x, size_t y) const
{
    return x < m_width && y < m_height;
}

bool Board::isValid(QPoint pos) const
{
    return pos.x() >= 0 && pos.y() >= 0 && isValid(pos.x(), pos.y());
}

std::optional<Piece> Board::pieceAt(size_t x, size_t y) const
{
    return m_squares[index(x, y)];
}

bool Board::tryMovePiece(size_t x0, size_t y0, size_t x1, size_t y1)
{
    std::optional<Piece> piece = pieceAt(x0, y0);
    if(piece)
    {
        setEmptyAt(x0, y0);
        setPiece(x1, y1, *piece);
        return true;
    }

    return false;
}

bool Board::tryMovePiece(Move move)
{
    return tryMovePiece(move.from.x(), move.from.y(), move.to.x(), move.to.y());
}

void Board::clearPieces()
{
    for (std::optional<Piece>& square : m_squares)
    {
        square.reset();
    }
}

size_t Board::width() const
{
    return m_width;
}

size_t Board::height() const
{
    return m_height;
}

size_t Board::index(size_t x, size_t y) const
{
    assert(x < m_width);
    assert(y < m_height);

    size_t index = y * m_width + x;
    assert(index < m_squares.size());

    return index;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(720, 720);
    setWindowTitle("Chess");

    m_boardView = new BoardView(&m_gameState.board());
    connect(m_boardView, &BoardView::squareClicked, this, &MainWindow::onSquareClicked);

    setCentralWidget(m_boardView);
}

void MainWindow::onSquareClicked(size_t x, size_t y)
{
    if(m_selectedPos)
    {
        QPoint from = *m_selectedPos;
        QPoint to = QPoint(x, y);

        QVector<Move> validMoves = m_gameState.getValidMoves(from.x(), from.y());

        auto moveIter = std::find_if(std::begin(validMoves), std::end(validMoves), [to](const Move& move) {
            return move.to == to;
        });

        if(moveIter != std::end(validMoves))
        {
            Move move = *moveIter;
            if(move.flags & PromotionAny)
            {
                PromotionDialog promotionDialog;
                promotionDialog.exec();

                move.flags &= ~PromotionAny;

                switch(promotionDialog.getPieceType())
                {
                case Knight: move.flags |= PromotionKnight; break;
                case Bishop: move.flags |= PromotionBishop; break;
                case Rook: move.flags |= PromotionRook; break;
                case Queen: move.flags |= PromotionQueen; break;

                case Pawn:
                case King:
                    break;
                }
            }

            std::optional<ChessState> nextState = m_gameState.nextState(move);
            if(nextState)
            {

                m_gameState = std::move(*nextState);
                m_boardView->setBoard(&m_gameState.board());

                if(m_gameState.getAllValidMoves().empty())
                {
                    QMessageBox gameOverMessageBox;
                    gameOverMessageBox.setWindowTitle("Game Over");
                    gameOverMessageBox.setIcon(QMessageBox::Information);
                    gameOverMessageBox.setStandardButtons(QMessageBox::Ok);

                    if(m_gameState.isKingInCheck(White))
                    {
                        gameOverMessageBox.setText("The winner is BLACK.");
                    }
                    else if(m_gameState.isKingInCheck(Black))
                    {
                        gameOverMessageBox.setText("The winner is WHITE.");
                    }
                    else
                    {
                        gameOverMessageBox.setText("The game resulted in a draw.");
                    }

                    gameOverMessageBox.exec();
                }
            }
        }

        m_selectedPos.reset();

        m_boardView->clearHighlights();
        m_boardView->clearMoveIndicators();

        return;
    }

    std::optional<Piece> piece = m_gameState.board().pieceAt(x, y);
    if(piece)
    {
        m_selectedPos = QPoint(x, y);

        QVector<Move> validMoves = m_gameState.getValidMoves(x, y);
        for (Move move : validMoves) {
            m_boardView->addMoveIndicator(move);
        }

        m_boardView->addHighlight(x, y);

        return;
    }
}

static QString getPieceKey(Piece piece)
{
    switch(piece.color)
    {
    case White:
        switch(piece.type)
        {
        case Pawn: return "white_pawn";
        case Knight: return "white_knight";
        case Bishop: return "white_bishop";
        case Rook: return "white_rook";
        case Queen: return "white_queen";
        case King: return "white_king";
        }
        break;
    case Black:
        switch(piece.type)
        {
        case Pawn: return "black_pawn";
        case Knight: return "black_knight";
        case Bishop: return "black_bishop";
        case Rook: return "black_rook";
        case Queen: return "black_queen";
        case King: return "black_king";
        }
        break;
    }
    return "";
}

static QPixmap getPiecePixmap(Piece piece)
{
    QString key = getPieceKey(piece);

    QPixmap pixmap;
    if(!QPixmapCache::find(key, &pixmap))
    {
        auto resourcePath = QString(":/resources/chess/%1.png").arg(key);
        pixmap = QPixmap(resourcePath);
        QPixmapCache::insert(key, pixmap);
    }

    return pixmap;
}

//static void paintSomething(QPaintEvent *event)
//{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);

//    QRectF widgetRect = rect().toRectF();

//    QColor lightColor = QColor(150, 120, 75);
//    QColor darkColor = QColor(100, 60, 50);
//    QColor backgroundColor = m_squareColor == Light ? lightColor : darkColor;
//    painter.fillRect(widgetRect, backgroundColor);


//    if(m_highlightColor)
//    {

//        QPen pen;
//        pen.setWidth(5);
//        pen.setColor(*m_highlightColor);

//        painter.setPen(pen);
//        painter.drawEllipse(widgetRect.center(), radius, radius);
//    }
//}

BoardView::BoardView(const Board *board, QWidget *parent)
    : QWidget(parent),
    m_board(board)
{
    setMouseTracking(true);
}

void BoardView::setBoard(const Board *board)
{
    m_board = board;

    update();
}

void BoardView::addHighlight(size_t x, size_t y)
{
    m_highlightedSquares.append(QPoint((int)x, (int)y));

    update();
}

void BoardView::addMoveIndicator(Move move)
{
    m_moves.append(move);

    update();
}

void BoardView::clearMoveIndicators()
{
    m_moves.clear();

    update();
}

void BoardView::clearHighlights()
{
    m_highlightedSquares.clear();

    update();
}

static std::optional<Move> findMoveWithEndPos(const QVector<Move>& moves, QPoint endPos)
{
    auto move = std::find_if(std::begin(moves), std::end(moves), [endPos](const Move& move) {
        return move.to == endPos;
    });

    if(move != std::end(moves))
    {
        return *move;
    }

    return std::nullopt;
}

void BoardView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::SmoothPixmapTransform);

    float squareWidth = (float)width() / m_board->width();
    float squareHeight = (float)height() / m_board->height();

    QColor lightColor = QColor(150, 120, 75);
    QColor darkColor = QColor(100, 60, 50);

    for (int y = 0; y < m_board->height(); ++y) {
        for (int x = 0; x < m_board->width(); ++x) {
            QRectF square(x * squareWidth, y * squareWidth, squareWidth, squareHeight);

            QColor squareColor = (x + y) % 2 == 0 ? lightColor : darkColor;
            painter.fillRect(square, squareColor);

            auto piece = m_board->pieceAt(x, y);
            if(piece)
            {
                QPixmap piecePixmap = getPiecePixmap(*piece);
                painter.drawPixmap(square, piecePixmap, piecePixmap.rect().toRectF());
            }

            if(m_hoveredPos && *m_hoveredPos == QPoint(x, y))
            {
                QPen pen;
                pen.setWidth(5);
                pen.setColor(Qt::darkBlue);

                painter.setPen(pen);
                painter.drawRect(square.adjusted(2.5, 2.5, -2.5, -2.5));
            }

            if(m_highlightedSquares.contains(QPoint(x, y)))
            {
                QPen pen;
                pen.setWidth(5);
                pen.setColor(Qt::blue);

                painter.setPen(pen);

                painter.drawRect(square.adjusted(2.5, 2.5, -2.5, -2.5));
            }

            std::optional<Move> move = findMoveWithEndPos(m_moves, QPoint(x, y));
            if(move)
            {
                QPen pen;
                pen.setWidth(5);
                QColor color = m_board->hasPieceAt(x, y) ? Qt::red : Qt::green;
                if(move->flags)
                {
                    color = Qt::cyan;
                }

                pen.setColor(color);

                painter.setPen(pen);

                float inset = squareWidth / 4.0f;
                painter.drawRect(square.adjusted(inset, inset, -inset, -inset));
            }

        }
    }
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    m_hoveredPos = calcSquarePos(event->pos());

    emit squareClicked(m_hoveredPos->x(), m_hoveredPos->y());
}

void BoardView::mouseMoveEvent(QMouseEvent *event)
{
    m_hoveredPos = calcSquarePos(event->pos());

    update();
}

QPoint BoardView::calcSquarePos(QPoint cursor)
{
    int x = cursor.x() / (width() / m_board->width());
    int y = cursor.y() / (height() / m_board->height());

    return QPoint(x, y);
}

bool Move::operator==(const Move &other) const
{
    return from == other.from
           && to == other.to
           && flags == other.flags;
}

Move Move::withFlags(uint8_t flags)
{
    Move newMove = *this;
    newMove.flags |= flags;
    return newMove;
}

ChessState::ChessState()
    : m_currentPlayer(White),
    m_board(Board::standardSetup())
{

}

PieceType getPromotionPiece(uint8_t moveFlags)
{
    if(moveFlags & PromotionQueen)
    {
        return Queen;
    }
    else if(moveFlags & PromotionRook)
    {
        return Rook;
    }
    else if(moveFlags & PromotionKnight)
    {
        return Knight;
    }
    else if(moveFlags & PromotionBishop)
    {
        return Bishop;
    }

    return Queen;
}

std::optional<ChessState> ChessState::nextState(Move move, bool validate)
{
    if(!validate || getValidMoves(move.from.x(), move.from.y()).contains(move))
    {
        ChessState nextState = *this;

        if(move.flags & TwoSquareAdvance)
        {
            nextState.m_twoSquareAdvance = move;
        }
        else
        {
            nextState.m_twoSquareAdvance.reset();
        }

        if(move.flags & EnPassant)
        {
            assert(m_twoSquareAdvance);

            QPoint captureSquare = m_twoSquareAdvance->to;

            nextState.m_board.setEmptyAt(captureSquare.x(), captureSquare.y());
        }


        // TODO check remaining move flags and update state accordingly

        assert(nextState.m_board.tryMovePiece(move));
        if(move.flags & PromotionAny)
        {
            PieceType pieceType = getPromotionPiece(move.flags);
            nextState.m_board.setPiece(move.to.x(), move.to.y(), m_currentPlayer, pieceType);
        }

        nextState.m_currentPlayer = m_currentPlayer == White ? Black : White;

        return nextState;
    }

    return std::nullopt;
}

QVector<Move> ChessState::getValidMoves(size_t x, size_t y, bool checkForKingSafety)
{
    QVector<Move> candidateMoves;

    std::optional<Piece> piece = m_board.pieceAt(x, y);
    if(!piece)
    {
        return candidateMoves;
    }

    if(piece->color != m_currentPlayer)
    {
        return candidateMoves;
    }

    switch(piece->type)
    {
    case Pawn:
    {
        int dy = piece->color == White ? -1 : 1;

        // standard pawn move
        size_t beforeCount = candidateMoves.count();
        addDirectionalMoves(candidateMoves, QPoint(x, y), QPoint(0, dy), 1, false);
        size_t afterCount = candidateMoves.count();


        // two square advnance

        // whether or not the two square advance is blocked
        bool isBlocked = beforeCount == afterCount;
        // TODO: This check only works for a standard setup.
        bool hasNotMoved = piece->color == White && y == 6 || piece->color == Black && y == 1;
        if(hasNotMoved && !isBlocked)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), QPoint(0, 2 * dy), 1, false);
            candidateMoves.last().flags |= TwoSquareAdvance;
        }


        // Diagonal captures and en passant

        std::array<QPoint, 8> diagonals = {
            QPoint(1, dy),
            QPoint(-1, dy),
        };

        for (QPoint diagonal : diagonals) {
            QPoint target = QPoint(x, y) + diagonal;
            if(!m_board.isValid(target))
            {
                continue;
            }

            std::optional<Piece> otherPiece = m_board.pieceAt(target.x(), target.y());
            if(otherPiece && otherPiece->color != piece->color)
            {
                candidateMoves.append(Move{QPoint(x, y), target});
            }

            // Check for en passant
            if(m_twoSquareAdvance && target == getEnPassantSquare())
            {
                candidateMoves.append(Move{QPoint(x, y), target, EnPassant});
            }

        }

        // Promotion

        // !WARN! iterate using indices to avoid iterator invalidation
        size_t count = candidateMoves.count();
        for (size_t i = 0; i < count; i++) {
            Move move = candidateMoves[i];

            int rank = move.to.y();
            bool isPromotionRank = rank == 0 || rank == m_board.height();
            if(isPromotionRank)
            {
                candidateMoves.append(move.withFlags(PromotionKnight));
                candidateMoves.append(move.withFlags(PromotionBishop));
                candidateMoves.append(move.withFlags(PromotionRook));

                candidateMoves[i].flags |= PromotionQueen;
            }
        }
    }
    break;
    case Knight:
    {
        std::array<QPoint, 8> offsets = {
            QPoint(+2, +1),
            QPoint(+2, -1),
            QPoint(-2, +1),
            QPoint(-2, -1),

            QPoint(+1, +2),
            QPoint(+1, -2),
            QPoint(-1, +2),
            QPoint(-1, -2),
        };

        for(QPoint offset : offsets)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), offset, 1);
        }
    }
    break;
    case Bishop:
    {
        std::array<QPoint, 8> directions = {
            QPoint(1, 1),
            QPoint(-1, -1),
            QPoint(1, -1),
            QPoint(-1, 1),
        };

        for(QPoint direction : directions)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), direction);
        }
    }
    break;
    case Rook:
    {
        std::array<QPoint, 8> directions = {
            QPoint(+1, 0),
            QPoint(-1, 0),
            QPoint(0, +1),
            QPoint(0, -1),
        };

        for(QPoint direction : directions)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), direction);
        }
    }
    break;
    case Queen:
    {
        std::array<QPoint, 8> directions = {
            QPoint(+1, 0),
            QPoint(-1, 0),
            QPoint(0, +1),
            QPoint(0, -1),

            QPoint(1, 1),
            QPoint(-1, -1),
            QPoint(1, -1),
            QPoint(-1, 1),
        };

        for(QPoint direction : directions)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), direction);
        }

    }
    break;
    case King:
    {
        std::array<QPoint, 8> directions = {
            QPoint(+1, 0),
            QPoint(-1, 0),
            QPoint(0, +1),
            QPoint(0, -1),

            QPoint(1, 1),
            QPoint(-1, -1),
            QPoint(1, -1),
            QPoint(-1, 1),
        };

        int kingDistance = 1;
        for(QPoint direction : directions)
        {
            addDirectionalMoves(candidateMoves, QPoint(x, y), direction, kingDistance);
        }
    }
    break;
    }

    // TODO: Having this flag in the methods signature is kind of ugly, think about different solution
    if(checkForKingSafety)
    {
        candidateMoves.removeIf([&](const Move& move){
            std::optional<ChessState> resultingState = nextState(move, false);
            return resultingState && resultingState->isKingInCheck(piece->color);
        });
    }

    return candidateMoves;
}

QVector<Move> ChessState::getAllValidMoves(bool checkForKingSafety)
{
    QVector<Move> moves;
    for (size_t y = 0; y < m_board.height(); ++y) {
        for (size_t x = 0; x < m_board.height(); ++x) {
            // TODO: Slightly inefficient.
            // we could just directly add to the moves instead of creating new vectors
            moves.append(getValidMoves(x, y, checkForKingSafety));
        }
    }
    return moves;
}

bool ChessState::isKingInCheck(Color color)
{
    QVector<Move> validMoves = getAllValidMoves(false);

    auto moveTargetsKing = [&](const Move& move) {
        auto piece = m_board.pieceAt(move.to.x(), move.to.y());
        return piece && piece->color == color && piece->type == King;
    };

    auto checkMove = std::find_if(std::begin(validMoves), std::end(validMoves), moveTargetsKing);
    return checkMove != std::end(validMoves);
}

const Board &ChessState::board() const
{
    return m_board;
}

void ChessState::addDirectionalMoves(QVector<Move> &moves,
                                     QPoint start,
                                     QPoint direction,
                                     size_t maxDistance,
                                     bool canCapture
                                     )
{
    std::optional<Piece> piece = m_board.pieceAt(start.x(), start.y());
    if(!piece)
    {
        return;
    }

    if(piece->color != m_currentPlayer)
    {
        return;
    }

    size_t distance = 0;

    QPoint pos(start);
    while(distance < maxDistance)
    {
        pos += direction;

        if(!m_board.isValid(pos))
        {
            break;
        }

        std::optional<Piece> otherPiece = m_board.pieceAt(pos.x(), pos.y());
        if(otherPiece)
        {
            bool isCapture = otherPiece->color != piece->color;
            if(isCapture && canCapture)
            {
                moves.append(Move{start, pos});
            }

            break;
        }

        moves.append(Move{start, pos});

        distance++;
    }
}

QPoint ChessState::getEnPassantSquare()
{
    assert(m_twoSquareAdvance);

    QPoint center = m_twoSquareAdvance->from + m_twoSquareAdvance->to;
    return QPoint(center.x() / 2, center.y() / 2);
}

PromotionDialog::PromotionDialog(QWidget *parent)
    : m_pieceType(Queen)
{
    setWindowTitle("Choose Promotion Piece");

    auto queenButton = new QRadioButton("Queen");
    auto rookButton = new QRadioButton("Rook");
    auto bishopButton = new QRadioButton("Bishop");
    auto knightButton = new QRadioButton("Knight");

    queenButton->setChecked(true);

    auto layout = new QVBoxLayout;
    layout->addWidget(queenButton);
    layout->addWidget(rookButton);
    layout->addWidget(bishopButton);
    layout->addWidget(knightButton);

    setLayout(layout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PromotionDialog::accept);

    layout->addWidget(buttonBox);

    connect(queenButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = Queen;
    });

    connect(rookButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = Rook;
    });

    connect(bishopButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = Bishop;
    });

    connect(knightButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = Knight;
    });
}

PieceType PromotionDialog::getPieceType()
{
    return m_pieceType;
}
