#include "chess.h"

#include <cassert>
#include <random>

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

#include <QMenuBar>
#include <QApplication>
#include <QStyle>
#include <QGroupBox>
#include <QComboBox>
#include <QFormLayout>

#include <QTimer>

using namespace Chess;

Board Board::standardSetup()
{
    Board board{};

    board.setPiece(QPoint(0, 0), Color::Black, PieceType::Rook);
    board.setPiece(QPoint(1, 0), Color::Black, PieceType::Knight);
    board.setPiece(QPoint(2, 0), Color::Black, PieceType::Bishop);
    board.setPiece(QPoint(3, 0), Color::Black, PieceType::Queen);
    board.setPiece(QPoint(4, 0), Color::Black, PieceType::King);
    board.setPiece(QPoint(5, 0), Color::Black, PieceType::Bishop);
    board.setPiece(QPoint(6, 0), Color::Black, PieceType::Knight);
    board.setPiece(QPoint(7, 0), Color::Black, PieceType::Rook);

    for (int x = 0; x < board.width(); ++x) {
        board.setPiece(QPoint(x, 1), Color::Black, PieceType::Pawn);
    }

    for (int x = 0; x < board.width(); ++x) {
        board.setPiece(QPoint(x, 6), Color::White, PieceType::Pawn);
    }

    board.setPiece(QPoint(0, 7), Color::White, PieceType::Rook);
    board.setPiece(QPoint(1, 7), Color::White, PieceType::Knight);
    board.setPiece(QPoint(2, 7), Color::White, PieceType::Bishop);
    board.setPiece(QPoint(3, 7), Color::White, PieceType::Queen);
    board.setPiece(QPoint(4, 7), Color::White, PieceType::King);
    board.setPiece(QPoint(5, 7), Color::White, PieceType::Bishop);
    board.setPiece(QPoint(6, 7), Color::White, PieceType::Knight);
    board.setPiece(QPoint(7, 7), Color::White, PieceType::Rook);

    return board;
}

void Board::setPiece(QPoint pos, Piece piece)
{
    pieceAt(pos) = piece;
}

void Board::setPiece(QPoint pos, Color color, PieceType type)
{
    pieceAt(pos) = Piece{ .color = color, .type = type };
}

void Board::setEmptyAt(QPoint pos)
{
    pieceAt(pos).reset();
}

bool Board::isEmptyAt(QPoint pos) const
{
    return !hasPieceAt(pos);
}

bool Board::hasPieceAt(QPoint pos) const
{
    return pieceAt(pos).has_value();
}

bool Board::isValid(QPoint pos) const
{
    return pos.x() >= 0
           && pos.y() >= 0
           && pos.x() < width()
           && pos.y() < height();
}

std::optional<Piece> Board::pieceAt(QPoint pos) const
{
    if(!isValid(pos))
    {
        return std::nullopt;
    }

    return m_squares[pos.y()][ pos.x()];
}

bool Board::tryMovePiece(QPoint from, QPoint to)
{
    std::optional<Piece> piece = pieceAt(from);
    if(piece)
    {
        setEmptyAt(from);
        setPiece(to, *piece);
        return true;
    }

    return false;
}

void Board::clearPieces()
{
    for (auto& row : m_squares)
    {
        for(auto& square: row)
        {
            square.reset();
        }
    }
}

constexpr size_t Board::width() const
{
    return WIDTH;
}

constexpr size_t Board::height() const
{
    return HEIGHT;
}

std::optional<Piece> &Board::pieceAt(QPoint pos)
{
    assert(isValid(pos));

    return m_squares[pos.y()][pos.x()];
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_currentPosition(),
    m_history(m_currentPosition)
{
    setFixedSize(1600, 900);
    setWindowTitle("Chess");

    // Menu Bar

    auto menuBar = new QMenuBar();
    auto fileMenu = new QMenu("&File");

    auto style = QApplication::style();

    auto newGameAction = new QAction("&New");
    auto saveAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "&Save");
    auto loadAction = new QAction(style->standardIcon(QStyle::SP_DialogOpenButton), "&Load");
    auto exitAction = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "&Exit");

    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewAction);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);


    fileMenu->addAction(newGameAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(loadAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    menuBar->addMenu(fileMenu);

    setMenuBar(menuBar);

    // Board

    m_boardView = new BoardView(&m_currentPosition.board());

    m_boardView->setFixedSize(720, 720);

    connect(m_boardView, &BoardView::squareClicked, this, &MainWindow::onSquareClicked);

    // History
    auto historyBox = new QGroupBox("History");
    historyBox->setFixedSize(360, 720);

    auto historyLayout = new QVBoxLayout();

    m_historyView = new MoveHistoryView();
    m_historyView->setHistory(&m_history);

    historyLayout->addWidget(m_historyView);

    historyBox->setLayout(historyLayout);

    // Central

    auto centralWidget = new QWidget();
    auto centralLayout = new QHBoxLayout();

    centralLayout->addWidget(m_boardView);
    centralLayout->addWidget(historyBox);

    centralWidget->setLayout(centralLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::onSquareClicked(QPoint pos)
{
    if(!isHumansTurn())
    {
        return;
    }

    if(!m_selectedPos)
    {
        selectPieceAt(pos);
        return;
    }

    selectAndPlayHumanMove(*m_selectedPos, pos);
}

void MainWindow::onNewAction()
{
    NewGameDialog newGameDialog;
    auto code = newGameDialog.exec();
    if(code == QDialog::Accepted)
    {
        MatchSettings settings = newGameDialog.getMatchSettings();
        MainWindow::startNewGame(settings);
    }
}

void MainWindow::startNewGame(const MatchSettings &settings)
{
    m_matchSettings = settings;

    m_currentPosition = Position();

    m_boardView->clearHighlights();
    m_boardView->clearMoveIndicators();
    m_boardView->setBoard(&m_currentPosition.board());

    Color startingPlayer = m_currentPosition.currentPlayer();
    m_boardView->setViewForPlayer(startingPlayer);

    m_history.clear();
    m_historyView->setHistory(&m_history);

    if(!isHumansTurn())
    {
        doAiMove();
    }
}

void MainWindow::selectPieceAt(QPoint pos)
{
    std::optional<Piece> piece = m_currentPosition.board().pieceAt(pos);
    if(piece)
    {
        m_selectedPos = pos;

        m_boardView->clearHighlights();
        m_boardView->clearMoveIndicators();

        QVector<Move> legalMoves = m_currentPosition.getLegalMoves(pos);
        for (const Move& move : legalMoves) {
            m_boardView->addMoveIndicator(move);
        }

        m_boardView->addHighlight(pos, Qt::white);

        showCheckIndicator();
    }
}

void MainWindow::playMove(Move move)
{
    m_currentPosition.doMove(move);
    m_boardView->update();

    m_history.addMove(move);
    m_historyView->setHistory(&m_history);

    m_boardView->clearHighlights();
    m_boardView->clearMoveIndicators();

    showCheckIndicator();

    if(isGameOver())
    {
        showGameResult();
        return;
    }

    if(!isHumansTurn())
    {
        doAiMove();
        return;
    }

    Color currentPlayer = m_currentPosition.currentPlayer();
    m_boardView->setViewForPlayer(currentPlayer);
}

std::optional<Move> MainWindow::trySelectMove(QPoint from, QPoint to)
{
    QVector<Move> legalMoves = m_currentPosition.getLegalMoves(from);

    auto moveIter = std::find_if(std::begin(legalMoves), std::end(legalMoves), [to](const Move& move) {
        return move.to == to;
    });

    if(moveIter == std::end(legalMoves))
    {
        return std::nullopt;
    }

    Move move = *moveIter;
    if(move.flags & PromotionAny)
    {
        PromotionDialog promotionDialog;
        promotionDialog.exec();

        move.flags &= ~PromotionAny;

        switch(promotionDialog.getPieceType())
        {
        case PieceType::Knight: move.flags |= PromotionKnight; break;
        case PieceType::Bishop: move.flags |= PromotionBishop; break;
        case PieceType::Rook: move.flags |= PromotionRook; break;
        case PieceType::Queen: move.flags |= PromotionQueen; break;

        case PieceType::Pawn:
        case PieceType::King:
            break;
        }
    }

    return move;
}


void MainWindow::selectAndPlayHumanMove(QPoint firstClick, QPoint secondClick)
{
    m_selectedPos.reset();

    std::optional<Move> move = trySelectMove(firstClick, secondClick);
    if(!move)
    {
        selectPieceAt(secondClick);
        return;
    }

    playMove(*move);
}

void MainWindow::showCheckIndicator()
{
    if(m_currentPosition.isKingInCheck())
    {
        auto kingPos = findPiece(m_currentPosition.board(), Piece{m_currentPosition.currentPlayer(), PieceType::King});
        m_boardView->addHighlight(*kingPos, Qt::red);
    }
}

void MainWindow::showGameResult()
{
    m_boardView->clearHighlights();
    m_boardView->clearMoveIndicators();
    m_boardView->repaint();

    if(m_currentPosition.isKingInCheck(Color::White))
    {
        showGameResult(GameResult{EndReason::CheckMate, Color::Black});
    }
    else if(m_currentPosition.isKingInCheck(Color::Black))
    {
        showGameResult(GameResult{EndReason::CheckMate, Color::White});
    }
    else
    {
        showGameResult(GameResult{EndReason::StaleMate});
    }
}

static QString playerText(Color color) {
    switch (color) {
    case Color::White: return "White";
    case Color::Black: return "Black";
    default: return "";
    }
}

void MainWindow::showGameResult(GameResult gameResult)
{
    auto msgBox = new QMessageBox();
    msgBox->setWindowTitle("Game Over");
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);

    QString winner = gameResult.winner ? playerText(*gameResult.winner).toUpper() : "";

    switch(gameResult.endReason)
    {
    case EndReason::CheckMate:
        msgBox->setText(QString("%1 wins by CHECKMATE").arg(winner));
        break;
    case EndReason::StaleMate:
        msgBox->setText(QString("The game ends in a DRAW by STALEMATE."));
        break;
    case EndReason::InsufficientMaterial:
        msgBox->setText("The game ends in a DRAW by INSUFFICIENT MATERIAL.");
        break;
    case EndReason::ThreefoldRepetition:
        msgBox->setText("The game ends in a DRAW by THREEFOLD REPETITION.");
        break;
    case EndReason::FiftyMoveRule:
        msgBox->setText("The game ends in a DRAW by FIFTY MOVE RULE.");
        break;
    case EndReason::OutOfTime:
        msgBox->setText(QString("%1 wins by TIME").arg(winner));
        break;
    case EndReason::Resignation:
        msgBox->setText(QString("%1 wins by RESIGNATION").arg(winner));
        break;
    }

    msgBox->show();
}

bool MainWindow::isGameOver() const
{
    return m_currentPosition.getLegalMoves().empty();
}

PlayerType MainWindow::getCurrentPlayerType() const
{
    Color currentPlayer = m_currentPosition.currentPlayer();
    return m_matchSettings.getPlayerByColor(currentPlayer);
}

bool MainWindow::isHumansTurn() const
{
    return getCurrentPlayerType() == PlayerType::Human;
}

Move calculateMove_EasyAI(Position position)
{
    QVector<Move> legalMoves = position.getLegalMoves();
    assert(!legalMoves.empty());

    // Easy AI just picks a random move for now
    std::random_device device;
    std::mt19937 gen(device());
    std::uniform_int_distribution<> distrib(0, legalMoves.size() - 1);

    return legalMoves[distrib(gen)];
}

void MainWindow::doAiMove()
{
    switch(getCurrentPlayerType())
    {
    case PlayerType::Human:
    {
        assert(false);
    }
    break;
    case PlayerType::EasyBot:
    {
        QTimer::singleShot(1, this, [this]() {
            Move move = calculateMove_EasyAI(m_currentPosition);
            playMove(move);
        });

    }
    break;
    }
}

static QString getPieceKey(Piece piece)
{
    switch(piece.color)
    {
    case Color::White:
        switch(piece.type)
        {
        case PieceType::Pawn: return "white_pawn";
        case PieceType::Knight: return "white_knight";
        case PieceType::Bishop: return "white_bishop";
        case PieceType::Rook: return "white_rook";
        case PieceType::Queen: return "white_queen";
        case PieceType::King: return "white_king";
        }
        break;
    case Color::Black:
        switch(piece.type)
        {
        case PieceType::Pawn: return "black_pawn";
        case PieceType::Knight: return "black_knight";
        case PieceType::Bishop: return "black_bishop";
        case PieceType::Rook: return "black_rook";
        case PieceType::Queen: return "black_queen";
        case PieceType::King: return "black_king";
        }
        break;
    }
    return "";
}

QPixmap BoardView::getPiecePixmap(Piece piece)
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

void BoardView::setViewForPlayer(Color color)
{
    m_viewForPlayer = color;

    if(m_hoveredPos)
    {
        int flippedX = (m_board->width() - 1) - m_hoveredPos->x();
        int flippedY = (m_board->height() - 1) - m_hoveredPos->y();
        m_hoveredPos = QPoint(flippedX, flippedY);
    }

    update();
}

void BoardView::setBoard(const Board *board)
{
    m_board = board;

    update();
}

void BoardView::addHighlight(QPoint pos, QColor color)
{
    m_highlights.append(Highlight{pos, color});

    update();
}

void BoardView::addMoveIndicator(const Move& move)
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
    m_highlights.clear();

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
    if(!m_board)
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    paintSquares(painter);
    paintHoveredPos(painter);
    paintHighlights(painter);
    paintPieces(painter);
    paintMoveIndicators(painter);
}

void BoardView::mousePressEvent(QMouseEvent *event)
{
    m_hoveredPos = getPosFromCursor(event->pos());

    emit squareClicked(*m_hoveredPos);
}

void BoardView::mouseMoveEvent(QMouseEvent *event)
{
    m_hoveredPos = getPosFromCursor(event->pos());

    update();
}

void BoardView::leaveEvent(QEvent *event)
{
    m_hoveredPos.reset();

    update();
}

QPoint BoardView::getViewAdjustedPos(QPoint pos) const
{
    if(m_viewForPlayer == Color::White)
    {
        return pos;
    }

    if(!m_board)
    {
        return pos;
    }

    int x = m_board->width() - pos.x() - 1;
    int y = m_board->height() - pos.y() - 1;

    return QPoint(x, y);
}

QPointF BoardView::getSquarePos(QPoint pos) const
{
    QSizeF size = getSquareSize();
    QPoint viewPos = getViewAdjustedPos(pos);
    float x = viewPos.x() * size.width();
    float y = viewPos.y() * size.height();
    return QPointF(x, y);
}

QSizeF BoardView::getSquareSize() const
{
    float squareWidth = (float)width() / m_board->width();
    float squareHeight = (float)height() / m_board->height();
    return QSizeF(squareWidth, squareHeight);
}

QRectF BoardView::getSquareRect(QPoint pos) const
{
    return QRectF(getSquarePos(pos), getSquareSize());
}

QPoint BoardView::getPosFromCursor(QPoint cursor) const
{
    int boardWidth = m_board ? m_board->width() : Board::WIDTH;
    int boardHeight = m_board ? m_board->height() : Board::HEIGHT;

    int x = cursor.x() / (width() / boardWidth);
    int y = cursor.y() / (height() / boardHeight);

    return getViewAdjustedPos(QPoint(x, y));
}

float BoardView::strokeWidth() const
{
    return width() / 180.0f;
}

void BoardView::paintSquares(QPainter &painter) const
{
    for (int y = 0; y < m_board->height(); ++y) {
        for (int x = 0; x < m_board->width(); ++x) {
            QRectF square = getSquareRect(QPoint(x, y));

            bool isLight = (x + y) % 2 == 0;
            QColor squareColor = isLight ? LIGHT_COLOR : DARK_COLOR;
            painter.fillRect(square, squareColor);
        }
    }
}

void BoardView::paintHoveredPos(QPainter &painter) const
{
    if(!m_hoveredPos)
    {
        return;
    }

    QRectF square = getSquareRect(*m_hoveredPos);

    QPen pen;
    pen.setWidth(strokeWidth());
    pen.setColor(Qt::yellow);

    painter.setPen(pen);

    float inset = pen.widthF() / 2;
    painter.drawRect(square.adjusted(inset, inset, -inset, -inset));
}

void BoardView::paintHighlights(QPainter &painter) const
{
    QPen pen;
    pen.setWidth(strokeWidth());

    for (Highlight highlight : m_highlights) {
        QRectF square = getSquareRect(highlight.pos);

        pen.setColor(highlight.color);
        painter.setPen(pen);

        float inset = pen.widthF() / 2;
        painter.drawRect(square.adjusted(inset, inset, -inset, -inset));
    }
}

void BoardView::paintPieces(QPainter &painter) const
{
    for (int y = 0; y < m_board->height(); ++y) {
        for (int x = 0; x < m_board->width(); ++x) {
            QRectF square = getSquareRect(QPoint(x, y));

            auto piece = m_board->pieceAt(QPoint(x, y));
            if(piece)
            {
                QPixmap piecePixmap = getPiecePixmap(*piece);
                painter.drawPixmap(square, piecePixmap, piecePixmap.rect().toRectF());
            }
        }
    }
}

void BoardView::paintMoveIndicators(QPainter &painter) const
{

    QColor indicatorColor(20, 60, 40);

    QBrush brush(indicatorColor);

    QPen pen;
    pen.setWidth(strokeWidth());
    pen.setColor(indicatorColor);

    for (const Move& move : m_moves) {
        QRectF square = getSquareRect(move.to);

        if(move.isCapture())
        {
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            float inset = pen.widthF() / 2.0f;
            painter.drawRect(square.adjusted(inset, inset, -inset, -inset));
        }
        else
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(brush);
            float inset = square.width() / 3.0f;
            painter.drawEllipse(square.adjusted(inset, inset, -inset, -inset));
        }
    }
}

bool Move::operator==(const Move &other) const
{
    return piece == other.piece
           && capture == other.capture
           && from == other.from
           && to == other.to
           && flags == other.flags;
}

bool Move::isCapture() const
{
    return capture || flags & EnPassant;
}

Move Move::withFlags(uint8_t flags) const
{
    Move newMove = *this;
    newMove.flags |= flags;
    return newMove;
}

Position::Position()
    : m_currentPlayer{Color::White},
    m_board{Board::standardSetup()}
{

}

PieceType getPromotionPiece(uint8_t moveFlags)
{
    if(moveFlags & PromotionQueen)
    {
        return PieceType::Queen;
    }
    else if(moveFlags & PromotionRook)
    {
        return PieceType::Rook;
    }
    else if(moveFlags & PromotionKnight)
    {
        return PieceType::Knight;
    }
    else if(moveFlags & PromotionBishop)
    {
        return PieceType::Bishop;
    }

    return PieceType::Queen;
}

Position Position::nextPosition(const Move& move) const
{
    Position nextState = *this;
    nextState.doMove(move);
    return nextState;
}

void Position::doMove(const Move& move)
{
    if(move.flags & TwoSquareAdvance)
    {
        m_twoSquareAdvance = move;
    }
    else
    {
        m_twoSquareAdvance.reset();
    }

    if(move.flags & EnPassant)
    {
        assert(m_twoSquareAdvance);

        QPoint captureSquare = m_twoSquareAdvance->to;

        m_board.setEmptyAt(captureSquare);
    }

    std::optional<Piece> piece = m_board.pieceAt(move.from);
    assert(piece);

    if(piece->type == PieceType::King)
    {
        m_canCastleKingSide[indexOfColor(piece->color)] = false;
        m_canCastleQueenSide[indexOfColor(piece->color)] = false;
    }

    int baseRank = piece->color == Color::White ? m_board.height() - 1 : 0;
    if(piece->type == PieceType::Rook)
    {
        if(move.from == QPoint(0, baseRank))
        {
            m_canCastleQueenSide[indexOfColor(piece->color)] = false;
        }

        if(move.from == QPoint(m_board.width(), baseRank))
        {
            m_canCastleKingSide[indexOfColor(piece->color)] = false;
        }
    }

    // TODO: Maybe should not call tryMovePiece() directly
    // and rather to avoid setting a piece twice on promotion
    // if adding signals on change to the Board later.
    m_board.tryMovePiece(move.from, move.to);

    if(move.flags & PromotionAny)
    {
        PieceType pieceType = getPromotionPiece(move.flags);
        m_board.setPiece(move.to, m_currentPlayer, pieceType);
    }

    if(move.flags & CastleKingSide)
    {
        m_board.tryMovePiece(QPoint(7, baseRank), QPoint(5, baseRank));
    }

    if(move.flags & CastleQueenSide)
    {
        m_board.tryMovePiece(QPoint(0, baseRank), QPoint(3, baseRank));
    }

    m_currentPlayer = oppositeColor(m_currentPlayer);
}

QVector<Move> Position::getLegalMoves(QPoint pos) const
{
    std::optional<Piece> piece = m_board.pieceAt(pos);
    if(!piece)
    {
        return {};
    }

    if(piece->color != m_currentPlayer)
    {
        return {};
    }

    QVector<Move> candidateMoves;
    addPossibleMoves(candidateMoves, pos);

    removeKingInCheckMoves(candidateMoves, piece->color);

    return candidateMoves;
}

QVector<Move> Position::getLegalMoves() const
{
    QVector<Move> moves;
    for (size_t y = 0; y < m_board.height(); ++y) {
        for (size_t x = 0; x < m_board.width(); ++x) {
            std::optional<Piece> piece = m_board.pieceAt(QPoint(x,y));
            if(!piece)
            {
                continue;
            }

            if(piece->color != m_currentPlayer)
            {
                continue;
            }

            addPossibleMoves(moves, QPoint(x, y));
        }
    }

    removeKingInCheckMoves(moves, m_currentPlayer);

    return moves;
}

bool Position::isLegalMove(const Move &move)
{
    return getLegalMoves().contains(move);
}

QVector<Move> Position::getCurrentThreats(Color color) const
{
    QVector<Move> moves;
    for (size_t y = 0; y < m_board.height(); ++y) {
        for (size_t x = 0; x < m_board.width(); ++x) {
            std::optional<Piece> piece = m_board.pieceAt(QPoint(x,y));
            if(!piece || piece->color != color)
            {
                continue;
            }
            bool onlyAttackingMoves = true;
            addPossibleMoves(moves, QPoint(x, y), onlyAttackingMoves);
        }
    }

    return moves;
}

bool Position::isKingInCheck(Color color) const
{
    QVector<Move> attackingMoves = getCurrentThreats(oppositeColor(color));

    auto moveTargetsKing = [&](const Move& move) {
        auto piece = m_board.pieceAt(move.to);
        return piece && piece->color == color && piece->type == PieceType::King;
    };

    return std::any_of(std::begin(attackingMoves), std::end(attackingMoves), moveTargetsKing);
}

bool Position::isKingInCheck() const
{
    return isKingInCheck(m_currentPlayer);
}

const Board &Position::board() const
{
    return m_board;
}

Color Position::currentPlayer() const
{
    return m_currentPlayer;
}

bool Position::canCastleKingSide(Color color) const
{
    return m_canCastleKingSide[indexOfColor(color)];
}

bool Position::canCastleQueenSide(Color color) const
{
    return m_canCastleQueenSide[indexOfColor(color)];
}

void Position::addPossibleMoves(QVector<Move> &moves, QPoint pos, bool onlyAttackingMoves) const
{
    std::optional<Piece> piece = m_board.pieceAt(pos);
    if(!piece)
    {
        return;
    }

    switch(piece->type)
    {
    case PieceType::Pawn:
    {
        int dy = piece->color == Color::White ? -1 : 1;

        if(!onlyAttackingMoves)
        {
            // standard pawn move
            size_t beforeCount = moves.count();
            addDirectionalMoves(moves, pos, QPoint(0, dy), 1, false);
            size_t afterCount = moves.count();


            // two square advnance

            // whether or not the two square advance is blocked
            bool isBlocked = beforeCount == afterCount;
            // TODO: This check only works for a standard setup.
            bool hasNotMoved = piece->color == Color::White && pos.y() == 6 || piece->color == Color::Black && pos.y() == 1;
            if(hasNotMoved && !isBlocked)
            {
                addDirectionalMoves(moves, pos, QPoint(0, 2 * dy), 1, false);
                moves.last().flags |= TwoSquareAdvance;
            }
        }

        // Diagonal captures and en passant

        std::array<QPoint, 8> diagonals = {
            QPoint(1, dy),
            QPoint(-1, dy),
        };

        for (QPoint diagonal : diagonals) {
            QPoint target = pos + diagonal;
            if(!m_board.isValid(target))
            {
                continue;
            }

            std::optional<Piece> otherPiece = m_board.pieceAt(target);
            if(otherPiece && otherPiece->color != piece->color)
            {
                auto diagonalCapture = Move{
                    .piece = *piece,
                    .from = pos,
                    .to = target,
                    .capture = otherPiece
                };
                moves.append(diagonalCapture);
            }

            // Check for en passant
            if(m_twoSquareAdvance && target == getEnPassantSquare())
            {
                auto enPassantCapture = Move{
                    .piece = *piece,
                    .from = pos,
                    .to = target,
                    .capture = otherPiece,
                    .flags = EnPassant,
                };
                moves.append(enPassantCapture);
            }
        }

        // Promotion

        // !WARN! iterate using indices to avoid iterator invalidation

        size_t count = moves.count();
        for (size_t i = 0; i < count; i++) {
            int rank = moves[i].to.y();
            bool isPromotionRank = (piece->color == Color::White && rank == 0)
                                   || (piece->color == Color::Black && rank == m_board.height() - 1);
            if(isPromotionRank)
            {
                moves.append(moves[i].withFlags(PromotionKnight));
                moves.append(moves[i].withFlags(PromotionBishop));
                moves.append(moves[i].withFlags(PromotionRook));

                moves[i].flags |= PromotionQueen;
            }
        }
    }
    break;
    case PieceType::Knight:
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
            addDirectionalMoves(moves, pos, offset, 1);
        }
    }
    break;
    case PieceType::Bishop:
    {
        std::array<QPoint, 8> directions = {
            QPoint(1, 1),
            QPoint(-1, -1),
            QPoint(1, -1),
            QPoint(-1, 1),
        };

        for(QPoint direction : directions)
        {
            addDirectionalMoves(moves, pos, direction);
        }
    }
    break;
    case PieceType::Rook:
    {
        std::array<QPoint, 8> directions = {
            QPoint(+1, 0),
            QPoint(-1, 0),
            QPoint(0, +1),
            QPoint(0, -1),
        };

        for(QPoint direction : directions)
        {
            addDirectionalMoves(moves, pos, direction);
        }
    }
    break;
    case PieceType::Queen:
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
            addDirectionalMoves(moves, pos, direction);
        }
    }
    break;
    case PieceType::King:
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
            addDirectionalMoves(moves, pos, direction, kingDistance);
        }

        int baseRank = piece->color == Color::White ? m_board.height() - 1 : 0;


        if(onlyAttackingMoves)
        {
            break;
        }

        // Castling

        // King Side Castling

        auto squareIsEmpty = [this](QPoint square){ return m_board.isEmptyAt(square); };

        if(canCastleKingSide(piece->color))
        {
            std::array<QPoint, 2> path = {
                QPoint(5, baseRank),
                QPoint(6, baseRank),
            };

            bool isPathClear = std::all_of(std::begin(path), std::end(path), squareIsEmpty);

            QVector<Move> attackingMoves = getCurrentThreats(oppositeColor(piece->color));

            auto attacksPath = [&path](const Move& move) {
                return std::count(std::begin(path), std::end(path), move.to) > 0;
            };

            bool isPathSafe = std::none_of(std::begin(attackingMoves), std::end(attackingMoves), attacksPath);

            if(isPathClear && isPathSafe && !isKingInCheck(piece->color))
            {
                auto kingSideCastle = Move {
                    .piece = *piece,
                    .from = pos,
                    .to = QPoint(6, baseRank),
                    .flags = CastleKingSide,
                };

                moves.append(kingSideCastle);
            }
        }

        // Queen Side Castling

        if(canCastleQueenSide(piece->color))
        {
            std::array<QPoint, 3> path = {
                QPoint(1, baseRank),
                QPoint(2, baseRank),
                QPoint(3, baseRank),
            };

            bool isPathClear = std::all_of(std::begin(path), std::end(path), squareIsEmpty);

            QVector<Move> attackingMoves = getCurrentThreats(oppositeColor(piece->color));

            auto attacksPath = [&path](const Move& move) {
                return std::count(std::begin(path), std::end(path), move.to) > 0;
            };

            bool isPathSafe = std::none_of(std::begin(attackingMoves), std::end(attackingMoves), attacksPath);

            if(isPathClear && isPathSafe && !isKingInCheck(piece->color))
            {
                auto queenSideCastle = Move {
                    .piece = *piece,
                    .from = pos,
                    .to = QPoint(2, baseRank),
                    .flags = CastleQueenSide,
                };

                moves.append(queenSideCastle);
            }
        }
    }
    break;
    }
}

void Position::removeKingInCheckMoves(QVector<Move> &moves, Color kingColor) const
{
        // removes all candidates sthat would leave the own color's king in check
    moves.removeIf([&](const Move& move){
        std::optional<Position> nextPos = nextPosition(move);
        return nextPos && nextPos->isKingInCheck(kingColor);
    });
}

void Position::addDirectionalMoves(QVector<Move> &moves,
                                     QPoint start,
                                     QPoint direction,
                                     size_t maxDistance,
                                     bool canCapture
                                     ) const
{
    std::optional<Piece> piece = m_board.pieceAt(start);
    assert(piece);

    size_t distance = 0;

    QPoint pos(start);
    while(distance < maxDistance)
    {
        pos += direction;

        // dont run off the board
        if(!m_board.isValid(pos))
        {
            break;
        }

        auto move = Move {
            .piece = *piece,
            .from = start,
            .to = pos
        };

        std::optional<Piece> otherPiece = m_board.pieceAt(pos);
        if(otherPiece)
        {
            bool isCapture = otherPiece->color != piece->color;
            if(isCapture && canCapture)
            {
                move.capture = otherPiece;
                moves.append(move);
            }

            break;
        }

        moves.append(move);

        distance++;
    }
}

QPoint Position::getEnPassantSquare() const
{
    assert(m_twoSquareAdvance);

    QPoint center = m_twoSquareAdvance->from + m_twoSquareAdvance->to;
    return QPoint(center.x() / 2, center.y() / 2);
}

PromotionDialog::PromotionDialog(QWidget *parent)
    : m_pieceType(PieceType::Queen)
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
        if(checked) m_pieceType = PieceType::Queen;
    });

    connect(rookButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = PieceType::Rook;
    });

    connect(bishopButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = PieceType::Bishop;
    });

    connect(knightButton, &QRadioButton::toggled, this, [this](bool checked) {
        if(checked) m_pieceType = PieceType::Knight;
    });
}

PieceType PromotionDialog::getPieceType()
{
    return m_pieceType;
}

Color Chess::oppositeColor(Color color)
{
    return color == Color::White ? Color::Black : Color::White;
}

std::optional<QPoint> Chess::findPiece(const Board &board, Piece target)
{
    for (int y = 0; y < board.height(); ++y) {
        for (int x = 0; x < board.width(); ++x) {
            std::optional<Piece> piece = board.pieceAt(QPoint(x, y));
            if(piece && piece == target)
            {
                return QPoint(x, y);
            }
        }
    }

    return {};
}

bool Piece::operator==(const Piece &other) const
{
    return color == other.color && type == other.type;
}

std::underlying_type<Color>::type Chess::indexOfColor(Color color)
{
    return static_cast<std::underlying_type<Color>::type>(color);
}

MoveHistory::MoveHistory(Position position)
    : m_basePosition(std::move(position))
{

}

void MoveHistory::undo()
{
    if(m_nextMoveIndex > 0)
    {
        m_nextMoveIndex--;
    }
}

void MoveHistory::redo()
{
    if(m_nextMoveIndex < m_moves.size() - 1)
    {
        m_nextMoveIndex++;
    }
}

void MoveHistory::setCurrentIndex(size_t index)
{
    if(index < m_moves.size())
    {
        m_nextMoveIndex = index;
    }
}

void MoveHistory::addMove(const Move& move)
{
    bool needsCutoff = !m_moves.empty() && m_nextMoveIndex < m_moves.count() - 1;
    if(needsCutoff)
    {
        m_moves.erase(std::cbegin(m_moves) + m_nextMoveIndex, std::cend(m_moves));
    }


    m_moves.push_back(move);
    m_nextMoveIndex++;
}

const QVector<Move> &MoveHistory::moves() const
{
    return m_moves;
}

const Position &MoveHistory::basePosition() const
{
    return m_basePosition;
}

Position MoveHistory::currentPosition() const
{
    Position position = m_basePosition;

    for (int i = 0; i < m_nextMoveIndex; ++i) {
        position.doMove(m_moves[i]);
    }

    return position;
}

Position MoveHistory::headPosition() const
{
    Position position = m_basePosition;

    for (const Move& move : m_moves) {
        position.doMove(move);
    }

    return position;
}

void MoveHistory::clear()
{
    m_moves.clear();
    m_nextMoveIndex = 0;
}

MoveHistoryView::MoveHistoryView(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout();
    m_historyListWidget = new QListWidget();
    layout->addWidget(m_historyListWidget);
    setLayout(layout);
}

void MoveHistoryView::setHistory(const MoveHistory *history)
{
    m_history = history;

    m_historyListWidget->clear();

    Position position = history->basePosition();

    for (size_t i = 0; i < m_history->moves().size(); ++i) {
        const Move& move = m_history->moves()[i];

        position.doMove(move);
        QString algebraicNotation = Chess::getAlgebraicNotation(move, position);

        m_historyListWidget->addItem(algebraicNotation);
    }
}

QString getFileCharacter(int fileIndex)
{
    return QString(QChar('a' + fileIndex));
}

QString getRankCharacter(int rankIndex)
{
    return QString(QChar('1' + rankIndex));
}

QString getSquareString(QPoint square)
{
    QString file = getFileCharacter(square.x());
    QString rank = getRankCharacter(square.y());

    return QString("%1%2").arg(file, rank);
}

QString getPieceCharacter(PieceType pieceType)
{
    switch(pieceType)
    {
    case PieceType::Pawn: return "";
    case PieceType::Knight: return "N";
    case PieceType::Bishop: return "B";
    case PieceType::Rook: return "R";
    case PieceType::Queen: return "Q";
    case PieceType::King: return "K";
    default:
        return "";
    }
}

QString getPiecePrefix(const Move& move)
{
    if(move.piece.type == PieceType::Pawn && move.isCapture())
    {
        return getFileCharacter(move.from.x());
    }

    return getPieceCharacter(move.piece.type);
}

QString getPromotionCharacter(const Move& move)
{
    if(move.flags & PromotionAny)
    {
        return getPieceCharacter(getPromotionPiece(move.flags));
    }

    return "";
}

QString Chess::getAlgebraicNotation(const Move& move, const Position& resultingPosition)
{
    if(move.flags & CastleKingSide)
    {
        return "O-O";
    }

    if(move.flags & CastleQueenSide)
    {
        return "O-O-O";
    }

    QString algebraicNotation;

    algebraicNotation.append(getPiecePrefix(move));

    if(move.isCapture())
    {
        algebraicNotation.append("x");
    }

    auto destination = getSquareString(move.to);
    algebraicNotation.append(destination);

    if(move.flags & PromotionAny)
    {
        algebraicNotation.append(getPromotionCharacter(move));
    }

    if(resultingPosition.isKingInCheck())
    {
        if(resultingPosition.getLegalMoves().empty())
        {
            algebraicNotation.append("#");
        }
        else
        {
            algebraicNotation.append("+");
        }
    }

    // TODO: Check and checkmate
    return algebraicNotation;
}

QComboBox* NewGameDialog::createPlayerComboBox() {

    auto playerComboBox = new QComboBox();

    for (const auto& [name, playerType] : playerTypes()) {
        playerComboBox->addItem(name);
    }

    playerComboBox->setEditable(false);

    return playerComboBox;
}

NewGameDialog::NewGameDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("New Game Settings");

    auto formLayout = new QFormLayout();

    QComboBox* white = createPlayerComboBox();
    formLayout->addRow("White:", white);

    QComboBox* black = createPlayerComboBox();
    formLayout->addRow("Black:", black);

    connect(white, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        m_matchSettings.white = *getPlayerTypeByName(text);
    });

    connect(black, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        m_matchSettings.black = *getPlayerTypeByName(text);
    });


    std::optional<QString> defaultWhite = getPlayerTypeName(m_matchSettings.white);
    if(defaultWhite)
    {
        white->setCurrentText(*defaultWhite);
    }

    std::optional<QString> defaultBlack = getPlayerTypeName(m_matchSettings.white);
    if(defaultBlack)
    {
        black->setCurrentText(*defaultBlack);
    }

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NewGameDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NewGameDialog::rejected);

    auto layout = new QVBoxLayout();
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

MatchSettings NewGameDialog::getMatchSettings()
{
    return m_matchSettings;
}

std::optional<PlayerType> NewGameDialog::getPlayerTypeByName(QString name)
{
    const auto players = playerTypes();
    auto entry = std::find_if(std::begin(players), std::end(players), [name] (const auto& entry){
        return entry.first == name;
    });

    if(entry != std::end(players))
    {
        return entry->second;
    }

    return std::nullopt;
}

std::optional<QString> NewGameDialog::getPlayerTypeName(PlayerType playerType)
{
    const auto players = playerTypes();
    auto entry = std::find_if(std::begin(players), std::end(players), [playerType] (const auto& entry){
        return entry.second == playerType;
    });

    if(entry != std::end(players))
    {
        return entry->first;
    }

    return std::nullopt;
}

const std::vector<std::pair<QString, PlayerType>> &NewGameDialog::playerTypes()
{
    static std::vector<std::pair<QString, PlayerType>> s_playerTypes({
        {"Human", PlayerType::Human},
        {"Easy Bot", PlayerType::EasyBot},
    });

    return s_playerTypes;
}

PlayerType MatchSettings::getPlayerByColor(Color color) const
{
    switch(color)
    {
    case Color::White: return white;
    case Color::Black: return black;
    }

    return white;
}
