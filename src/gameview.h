#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QLCDNumber>
#include <QKeyEvent>
#include <QSlider>
#include "gamemodel.h"

class GameView : public QWidget
{
    Q_OBJECT

public:
    GameView(QWidget *parent = 0);
    ~GameView();

private:
    //properties for the menu/settings bar
    QLCDNumber* timeCounter;
    QLabel* enemyCounterLabel;
    QLabel* infoLabel;
    QLabel* mapSizeLabel;
    QLabel* wallNumberLabel;
    QLabel* enemyNumberLabel;
    QLabel* enemySpeedLabel;
    QLabel* destroyWallLabel;
    QRadioButton* destroyWallButton;
    QSlider* mapSizeSlider;
    QSlider* wallNumberSlider;
    QSlider* enemyNumberSlider;
    QSlider* enemySpeedSlider;
    QPushButton* newGameButton;
    QPushButton* pauseButton;

    //properties for the game table
    QGridLayout* tableLayout;
    QVector<QVector<QLabel*> > gameTable;

    //other properties
    GameModel* model;
    int mapSize;
    bool gameBegan;

private slots:
    //slots responsible for creating new game
    void setSliderMaxValues();
    void setWallNumberText();
    void setEnemyNumberText();
    void setEnemySpeedText();
    void generateTable();

    //slots responsible for gameplay
    void keyPressEvent(QKeyEvent*);
    void gameModel_refreshTable(QVector<QVector<GameModel::TileType> > tiles, GameModel::Position p, QList<GameModel::Position> e);
    void gameModel_refreshStatus(int bombedEnemies, int gameTime, bool airstrike,  int countdown);
    void gameModel_gameEnded(bool playerWon);
    void pauseGame();

    void resizeEvent(QResizeEvent*);
};

#endif // GAMEVIEW_H
