#include <QTest>

#include "../chess.h"

class BoardTest : public QObject
{
    Q_OBJECT

private slots:
    void testSetPiece() {
        Chess::Board board;
        board.setPiece(9, 0, Chess::White, Chess::Pawn);

        QVERIFY(false);
        QVERIFY(board.hasPieceAt(0, 0));
    }

};

QTEST_MAIN(BoardTest)
#include "test_board.moc"
