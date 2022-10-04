#include <QString>
#include <QtTest>
#include <QList>
#include <QDebug>
#include "gamemodel.h"

class BomberTest : public QObject
{
    Q_OBJECT

private:
    GameModel* _model;
    GameModel* _model2;
    GameModel* _model3;
    GameModel* _model4;
private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitState();
    void testPlayerStep();
    void testExplosion();
    void dieToExplosion();
    void dieToExplosion2();
    void killEnemy();
    void dieToEnemy();
};


void BomberTest::initTestCase()
{
    //10*10 arena, 0 inner walls, 1 enemy
    _model = new GameModel(10,0,1,1,false);
    _model2 = new GameModel(10,0,1,1,true);
    _model3 = new GameModel(10,0,1,1,true);
    _model4 = new GameModel(10,0,1,1,true);
}


void BomberTest::testInitState(){
    //player should start in the upper right corner
    QCOMPARE(_model->getPlayer().x,1);
    QCOMPARE(_model->getPlayer().y,1);
    QVERIFY( ! _model->getPlayerDied() );
    //no walls near player
    QCOMPARE(_model->getTable()[1][2], GameModel::Floor);
    QCOMPARE(_model->getTable()[2][1], GameModel::Floor);
    QCOMPARE(_model->getTable()[2][2], GameModel::Floor);
    //no enemies near player
    foreach(GameModel::Position e, _model->getEnemies()){
        QVERIFY(e.x + e.y > 4);
    }
    //NOTE: these tests are made with the knowledge that the initial placement of
    //walls and enemies aren't sophisticated either
}

void BomberTest::testPlayerStep(){
    //step in all directions (all steps are legit)
    _model->playerMoved(GameModel::Right);
    QCOMPARE(_model->getPlayer().x,1);
    QCOMPARE(_model->getPlayer().y,2);
    _model->playerMoved(GameModel::Down);
    QCOMPARE(_model->getPlayer().x,2);
    QCOMPARE(_model->getPlayer().y,2);
    _model->playerMoved(GameModel::Left);
    QCOMPARE(_model->getPlayer().x,2);
    QCOMPARE(_model->getPlayer().y,1);
    _model->playerMoved(GameModel::Up);
    QCOMPARE(_model->getPlayer().x,1);
    QCOMPARE(_model->getPlayer().y,1);

    //step an invalid step (would step on wall)
    _model->playerMoved(GameModel::Up);
    QCOMPARE(_model->getPlayer().x,1);
    QCOMPARE(_model->getPlayer().y,1);
}

void BomberTest::testExplosion(){
    //call an aisstrike to the starting position, move out of the way
    _model->airstrikeCalled();
    if(_model->getEnemies()[0].x != 0){
        for (int i=0; i<5; i++){
            _model->playerMoved(GameModel::Right);
        }
    } else {
        for (int i=0; i<5; i++){
            _model->playerMoved(GameModel::Down);
        }
    }
    //wait for the airstrike
    for (int i=0; i<4; i++){
        _model->advanceGame();
    }
    QCOMPARE(_model->getTable()[1][1], GameModel::FloorUnderExplosion);
    //turn off explosion
    _model->advanceGame();
    QCOMPARE(_model->getTable()[1][1], GameModel::Floor);
    QCOMPARE(_model->getTable()[1][4], GameModel::Floor);
    QCOMPARE(_model->getTable()[4][1], GameModel::Floor);
    QCOMPARE(_model->getTable()[4][4], GameModel::Floor);
}

//call explosion to current location and wait it out
void BomberTest::dieToExplosion(){
    QVERIFY( !_model->getPlayerDied() );
    _model->airstrikeCalled();
    for (int i=0; i<4; i++){
        _model->advanceGame();
    }
    QVERIFY(_model->getPlayerDied());
}

//locate the second enemy, go next to them, call an airstrike, but stay alive
void BomberTest::killEnemy(){
    //there is 1 enemy
    QCOMPARE(_model2->getEnemies().length(),1);

    //target enemy
    int EX = _model2->getEnemies()[0].x;
    int EY = _model2->getEnemies()[0].y;
    //step history
    QList<GameModel::Direction> path;
    //next possible step
    int X = _model2->getPlayer().x + 1;
    int Y = _model2->getPlayer().y;

    //moving in position
    bool found = false;
    while ( !found ){

        if ( X < EX ){
            X++;
            if (EX == X && EY == Y) found = true;
            else {
                _model2->playerMoved(GameModel::Down);
                path.push_front(GameModel::Down);
            }
        } else {
            Y++;
            if (EX == X && EY == Y) found = true;
            else {
                _model2->playerMoved(GameModel::Right);
                path.push_front(GameModel::Right);
            }

        }
    }

    //placing the bomb
    _model2->airstrikeCalled();

    //retreating
    for(QList<GameModel::Direction>::iterator it = path.begin(); it < path.end(); it++){
        if(*it == GameModel::Up){
            _model2->playerMoved(GameModel::Down);
        } else if(*it == GameModel::Right){
            _model2->playerMoved(GameModel::Left);
        } else if(*it == GameModel::Down){
            _model2->playerMoved(GameModel::Up);
        } else {
            qDebug() << "ERROR";
        }
    }

    //making it explode
    for (int i=0; i<4; i++){
        _model2->advanceGame();
    }
    _model2->advanceGame(); //just for good measure

    //0 enemy remains
    QCOMPARE(_model2->getEnemies().length(),0);
}

//step onto an enemy
void BomberTest::dieToEnemy(){
    int X = _model3->getEnemies()[0].x;
    int Y = _model3->getEnemies()[0].y;

    QVERIFY( !_model3->getPlayerDied() );

    bool found = false;
    while (!found){
        if (_model3->getPlayer().y < Y){
            _model3->playerMoved(GameModel::Right);
        } else if (_model3->getPlayer().x < X){
            _model3->playerMoved(GameModel::Down);
        } else {
            found = true;
        }
    }

    QVERIFY( _model3->getPlayerDied() );
}

//step into the explosion
void BomberTest::dieToExplosion2(){
    _model4->airstrikeCalled();
    for (int i=0; i<4; i++){
        _model4->playerMoved(GameModel::Down);
    }
    for (int i=0; i<4; i++){
        _model4->advanceGame();
    }

    QVERIFY( !_model4->getPlayerDied() );
    _model4->playerMoved(GameModel::Up);
    QVERIFY(_model4->getPlayerDied());
}

void BomberTest::cleanupTestCase(){
    delete _model;
    delete _model2;
    delete _model3;
    delete _model4;
}


QTEST_APPLESS_MAIN(BomberTest)

#include "bombertest.moc"
