#include "gamemodel.h"
#include <QDebug>


//-----PUBLIC METHODS-----

//The constructor creates the table, adds walls and enemies on random positions,
//and also starts up the timers (for enemies, explosions, and time-counting).
GameModel::GameModel(int size, int wallnum, int enemynum, int enemyspd, bool destroywalls):
    _size(size), _wallnum(wallnum), _enemynum(enemynum), _enemyspd(enemyspd), _destroywalls(destroywalls)
{
    //initialize table
    table.resize(_size);
    for (int i = 0; i < _size; ++i)
    {
        table[i].resize(_size);
        for (int j = 0; j < _size; ++j)
        {
            if (i==0 || j == 0 || i == _size-1 || j == _size-1) table[i][j] = Wall;
            else table[i][j] = Floor;
        }
    }

    //TODO: check whether the incoming parameters are correct (ex.: enemynum + wallnum < size ; minSize > 3)
    qsrand(QTime::currentTime().msec()); //needed for random walls and enemies
    //initialize walls
    createWalls(_size, _wallnum);
    //initialize player
    player.x = 1;
    player.y = 1;
    player.facing = Right;
    playerDied = false;
    //initialize enemies
    createEnemies(_size, _enemynum);

    //setting up timers
    enemyStepTimer = new QTimer();
    enemyStepTimer->setInterval(1000 / _enemyspd);
    timeCounter = new QTimer();
    timeCounter->setInterval(1000);
    connect(enemyStepTimer, SIGNAL(timeout()), this, SLOT(moveEnemies()));
    connect(timeCounter, SIGNAL(timeout()), this, SLOT(timerTimeout()));

    paused = false;
    waitingForExplosion = false;
    explosionDelay = 4;
    gameTime = 0;

}

// GameModel::~GameModel(){ }


//This method is called when the game starts. It could have been part of this class's constructor,
//but for unit testing purposes it was separated.
//Starts the timers for enemies and the time counter.
void GameModel::startTimers(){
    enemyStepTimer->start();
    timeCounter->start();
}


//For unit testing purposes.
//Advances the game, without relying on timeCounter
void GameModel::advanceGame(){
    timerTimeout();
}


//This method is called right after the table is created in the View.
void GameModel::requestUpdate(){
    //sends signal to View in order to show the initial state of the game
    emit tableChanged(table,player,enemies);
}


//This method is called when the user tries to move in a direction.
//It checks whether that direction is valid, and that the player is alive afterwards or not.
//In case of a valid step it changes the position and the facing of the player.
void GameModel::playerMoved(Direction dir){
    if (!paused){
        Position newPos;
        switch (dir){
            case Up:
                newPos.x = player.x - 1;
                newPos.y = player.y;
                break;
            case Right:
                newPos.x = player.x;
                newPos.y = player.y + 1;
                break;
            case Down:
                newPos.x = player.x + 1;
                newPos.y = player.y;
                break;
            case Left:
                newPos.x = player.x;
                newPos.y = player.y - 1;
                break;
        }

        if (checkPlayerNewPos(newPos.x, newPos.y)) {
            player.x = newPos.x;
            player.y = newPos.y;
            player.facing = dir;
        }

        if (playerDied){
            pauseGame();
            emit gameEnded(false);
        }

        emit tableChanged(table,player,enemies);
    }
}


//if a game is ongoing, this method pauses it
//if a game is paused, this method continues it
void GameModel::pauseGame(){
    if (paused && !playerDied){
        paused = false;
        enemyStepTimer->start();
        timeCounter->start();
    } else {
        paused = true;
        enemyStepTimer->stop();
        timeCounter->stop();
    }
}


//stores the focus point (target) of the airstrike
void GameModel::airstrikeCalled(){
    if( !waitingForExplosion){
        target.x = player.x;
        target.y = player.y;
        table[target.x][target.y] = TargetFloor;
        waitingForExplosion = true;
    }
}



//-----PRIVATE METHODS-----

//put M number of walls on a N*N matrix
//TODO: constraints could be added, in order to reduce the chance of walling off entire areas; or use path finding algorithms
void GameModel::createWalls(const int &N, const int &M){
    int xPos, yPos;
    int placedWallNum = 0;
    while (placedWallNum < M){
        xPos = qrand() % (N-2) + 1;
        yPos = qrand() % (N-2) + 1;
        //walls won't be generated in the immediate vicinity of the player's starting position
        if( !(xPos == 1 && yPos == 1) && table[xPos][yPos] == Floor && !(xPos < 6 && yPos < 6)){
            table[xPos][yPos] = Wall;
            placedWallNum++;
        }
    }

}


//generates M number of enemies on a N*N matrix
//TODO: performance could be improved by storing enemies in ordered list, making the 'unique position check' faster
void GameModel::createEnemies(const int &N, const int &M){
    int enemyNum = 0;
    Position newEnemy;
    int tmp;
    while (enemyNum < M){
        //enemies won't be generated in the immediate vicinity of the player's starting position
        newEnemy.x = qrand() % (N-2-(N/4)) + 1 + (N/4);
        newEnemy.y = qrand() % (N-2-(N/4)) + 1 + (N/4);
        //check all the other enemies, so that 2 won't start on the same tile
        //also making sure that we aren't placing them on walls
        if( checkEnemyNewPos(newEnemy.x, newEnemy.y) ){
            //finally, specifying a new random direction...
            tmp = qrand() % 4;
            switch (tmp){
                case 0 : newEnemy.facing = Up; break;
                case 1 : newEnemy.facing = Down; break;
                case 2 : newEnemy.facing = Left; break;
                case 3 : newEnemy.facing = Right; break;
            }
            //...and adding the new enemy to the others
            enemies.push_back(newEnemy);
            enemyNum++;
        }
    }
}


//This method moves each enemy one tile in their specified direction.
//If that new tile contains an explosion, the enemy is deleted;
//if that direction isn't valid (wall, or other enemy) they choose a new random direction instead.
void GameModel::moveEnemies(){
    int tmp;
    for(QList<Position>::iterator it = enemies.begin(); it < enemies.end(); it++){

        if (it->facing == Up){
            if(table[it->x-1][it->y] == FloorUnderExplosion) enemies.erase(it);
            else if ( checkEnemyNewPos(it->x - 1, it->y) ) {
                it->x--;
            } else {
                tmp = qrand() % 3;
                switch (tmp){
                    case 0 : it->facing = Right; break;
                    case 1 : it->facing = Down; break;
                    case 2 : it->facing = Left; break;
                }
            }
        } else if (it->facing == Right){
            if(table[it->x][it->y+1] == FloorUnderExplosion) enemies.erase(it);
            else if ( checkEnemyNewPos(it->x, it->y + 1) ) {
                it->y++;
            } else {
                tmp = qrand() % 3;
                switch (tmp){
                    case 0 : it->facing = Up; break;
                    case 1 : it->facing = Down; break;
                    case 2 : it->facing = Left; break;
                }
            }
        } else if (it->facing == Down){
            if(table[it->x+1][it->y] == FloorUnderExplosion) enemies.erase(it);
            else if ( checkEnemyNewPos(it->x + 1, it->y) ) {
                it->x++;
            } else {
                tmp = qrand() % 3;
                switch (tmp){
                    case 0 : it->facing = Right; break;
                    case 1 : it->facing = Up; break;
                    case 2 : it->facing = Left; break;
                }
            }
        } else if (it->facing == Left){
            if(table[it->x][it->y-1] == FloorUnderExplosion) enemies.erase(it);
            else if ( checkEnemyNewPos(it->x, it->y - 1) ) {
                it->y--;
            } else {
                tmp = qrand() % 3;
                switch (tmp){
                    case 0 : it->facing = Right; break;
                    case 1 : it->facing = Down; break;
                    case 2 : it->facing = Up; break;
                }
            }
        }

    }

    emit tableChanged(table,player,enemies);

    if(playerDied){
        pauseGame();
        emit gameEnded(false);
    } else if (enemies.size() == 0){
        pauseGame();
        emit gameEnded(true);
    }
}


//The timecounter calls this method every second.
//This method is responsible for updating the 'elapsed time' counter and it also makes a countdown before an airstrike,
//calls the appropriate function for 'starting' and 1 sec later for 'finishing' the explosion.
void GameModel::timerTimeout(){
    if (waitingForExplosion){
        if (explosionDelay > 1) explosionDelay --;
        else if (explosionDelay == 1){
            bombTarget(false);
            explosionDelay --;
        } else {
            bombTarget(true);
            waitingForExplosion = false;
            explosionDelay = 4;
        }
    }

    gameTime++;
    emit statusChanged( _enemynum - enemies.size(), gameTime, waitingForExplosion, explosionDelay);
    if (playerDied) {
        emit gameEnded(false);
    }
}


//This method is responsible for updating the game table about the explosion.
//It has 2 different behaviour, depending upon the user's choice of being able to destroy walls or not.
//The parameter 'explosionFinished' determines whether the state of the explosion should be applied or removed.
void GameModel::bombTarget(bool explosionFinished){
     if (!paused){
        int x = target.x - 3;
        int y = target.y - 3;
        if( !explosionFinished ) //applies explosion status
        {
            for (int i = x; i < x+7; i++){
                for (int j = y; j < y+7; j++){
                    if (i > 0 && i < _size-1 && j > 0 && j < _size-1){
                        if ( _destroywalls || table[i][j] == Floor || table[i][j] == TargetFloor ) {
                            table[i][j] = FloorUnderExplosion;
                        }
                        else table[i][j] = WallUnderExplosion;
                    }
                }
            }

            //if the player is caught in the explosion, it is game over
            if( qAbs(player.x - target.x) < 3 && qAbs(player.y - target.y) < 3){
                playerDied = true;
                pauseGame();

                emit tableChanged(table,player,enemies);
            }
            //if an enemy is caught in the explosion, they are deleted
            for(QList<Position>::iterator it = enemies.begin(); it < enemies.end(); it++){
                if(qAbs(it->x - target.x) < 3 && qAbs(it->y - target.y) < 3){
                    enemies.erase(it);
                }
            }


        } else //removes explosion status
        {
            for (int i = x; i < x+7; i++){
                for (int j = y; j < y+7; j++){
                    if (i > 0 && i < _size && j > 0 && j < _size){
                        if ( table[i][j] == FloorUnderExplosion ) table[i][j] = Floor;
                        else if ( table[i][j] == WallUnderExplosion ) table[i][j] = Wall;
                    }
                }
            }
        }
    }
}


//this function checks whether an enemy can step (or be created) to the specified coordinate:
//it returns false if it would step on a wall or an enemy
//it also checks the player's position and turns on a flag if the game is over
bool GameModel::checkEnemyNewPos(const int x, const int y){

    if (x == player.x && y == player.y){
        playerDied = true;
    }


    if (table[x][y] == Wall || table[x][y] == WallUnderExplosion) return false;
    foreach(Position e, enemies){
        if (e.x == x && e.y == y) return false;
    }

    return true;

}


//this function checks whether the player can step to the specified coordinate:
//it returns false if it would step on a wall or a wall under explosion
//it returns true otherwise, but turns on a flag if the game is over (stepped into explosion or other enemy)
bool GameModel::checkPlayerNewPos(const int &x, const int &y){

    if (table[x][y] == Wall || table[x][y] == WallUnderExplosion) return false;

    if (table[x][y] == FloorUnderExplosion) {
        playerDied = true;
        return true;
    }

    foreach(Position e, enemies){
        if ((e.x == x && e.y == y)) {
            playerDied = true;
            return true;
        }
    }

    return true;
}







