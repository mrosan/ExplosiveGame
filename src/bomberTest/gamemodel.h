#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QMetaEnum>
#include <QTimer>
#include <QTime>

class GameModel : public QObject
{
    Q_OBJECT
    Q_ENUMS(TileType)
public:
    enum TileType { Floor, Wall, FloorUnderExplosion, WallUnderExplosion, TargetFloor };
    enum Direction{ Up, Right, Down, Left };

    struct Position{
        int x;
        int y;
        Direction facing;
    };

    explicit GameModel(int size, int wallnum, int enemynum, int enemyspd, bool destroywalls);
    //~GameModel();
    void startTimers();
    void requestUpdate();
    void advanceGame();

    bool gamePaused() {return paused;}
    Position getPlayer() {return player;}
    QList<Position> getEnemies() {return enemies;}
    QVector< QVector<TileType> > getTable() {return table;}
    bool getPlayerDied(){return playerDied;}


public slots:
    void pauseGame();
    void airstrikeCalled();
    void playerMoved(Direction dir);


private:

    int _size;
    int _wallnum;
    int _enemynum;
    int _enemyspd;
    bool _destroywalls;

    Position player;
    QList<Position> enemies;
    QVector< QVector<TileType> > table;

    int gameTime;
    bool paused;
    QTimer* enemyStepTimer;
    QTimer* timeCounter;
    int explosionDelay;
    bool waitingForExplosion;
    Position target;
    bool playerDied;

    void createWalls(const int &N, const int &M);
    void createEnemies(const int &N, const int &M);
    bool checkEnemyNewPos(const int x, const int y);
    bool checkPlayerNewPos(const int &x, const int &y);
    void bombTarget(bool explosionFinished);

private slots:
    void moveEnemies();
    void timerTimeout();

signals:
    void tableChanged(const QVector< QVector<GameModel::TileType> > &tiles, const GameModel::Position &p, const QList<GameModel::Position> &e);
    void statusChanged(const int eNumber, const int tCounter, const bool airstrike, const int countdown);  
    void gameEnded(const bool playerWon);

};

#endif // GAMEMODEL_H
