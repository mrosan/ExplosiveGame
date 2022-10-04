#include "gameview.h"
#include <QDebug>

GameView::GameView(QWidget *parent)
    : QWidget(parent)
{
    //Setting basic properties for the gamewindow
    setMinimumSize(1200, 900);
    //this->setWindowState(Qt::WindowMaximized);
    setWindowTitle(tr("Bomber game"));
    this->setStyleSheet("background-color: rgb(153, 255, 186);");
    int infoPanelWidth = this->width() - this->height();

    //Creating the interface, setting default values
    mapSizeLabel = new QLabel("Size of the map: 20x20");
    mapSizeSlider = new QSlider(Qt::Horizontal);
    mapSizeSlider->setMinimum(10);
    mapSizeSlider->setMaximum(30);
    mapSizeSlider->setValue(20);
    mapSizeSlider->setMaximumWidth(infoPanelWidth);
    mapSizeSlider->setFocusPolicy(Qt::NoFocus);
    wallNumberLabel = new QLabel("Number of walls: 20");
    wallNumberSlider = new QSlider(Qt::Horizontal);
    wallNumberSlider->setMinimum(0);
    wallNumberSlider->setMaximum(120);
    wallNumberSlider->setValue(20);
    wallNumberSlider->setMaximumWidth(infoPanelWidth);
    wallNumberSlider->setFocusPolicy(Qt::NoFocus);
    enemyNumberLabel = new QLabel("Number of enemies: 5");
    enemyNumberSlider = new QSlider(Qt::Horizontal);
    enemyNumberSlider->setMinimum(1);
    enemyNumberSlider->setMaximum(20);
    enemyNumberSlider->setValue(5);
    enemyNumberSlider->setMaximumWidth(infoPanelWidth);
    enemyNumberSlider->setFocusPolicy(Qt::NoFocus);
    enemySpeedLabel = new QLabel("Enemy speed: 2");
    enemySpeedSlider = new QSlider(Qt::Horizontal);
    enemySpeedSlider->setMinimum(1);
    enemySpeedSlider->setMaximum(7);
    enemySpeedSlider->setValue(3);
    enemySpeedSlider->setMaximumWidth(infoPanelWidth);
    enemySpeedSlider->setFocusPolicy(Qt::NoFocus);
    destroyWallLabel = new QLabel("     Walls are destructible: ");
    destroyWallButton = new QRadioButton();
    destroyWallButton->setFocusPolicy(Qt::NoFocus);
    destroyWallButton->setChecked(true);

    newGameButton = new QPushButton(trUtf8("Launch mission"));
    QFont font = newGameButton->font();
    font.setPointSize(16);
    newGameButton->setFont(font);
    newGameButton->setFixedSize(infoPanelWidth, 60);
    newGameButton->setFocusPolicy(Qt::NoFocus);

    enemyCounterLabel = new QLabel("\n\n Greetings, soldier! \n");
    enemyCounterLabel->setAlignment(Qt::AlignHCenter);
    enemyCounterLabel->setFont(QFont("Times New Roman", 20, QFont::Bold));
    enemyCounterLabel->setMaximumWidth(infoPanelWidth);
    infoLabel = new QLabel("To begin your career, \n click the 'Launch mission' \n button above! \n\n Use W,A,S,D to move, and SPACE \n to call an airstrike!");
    infoLabel->setAlignment(Qt::AlignHCenter);
    infoLabel->setFont(QFont("Times New Roman", 14, QFont::Bold));
    infoLabel->setMaximumWidth(infoPanelWidth);
    timeCounter = new QLCDNumber(5);
    timeCounter->display("00:00");
    timeCounter->setFixedSize(infoPanelWidth,80);
    pauseButton = new QPushButton(trUtf8("Freeze time!"));
    pauseButton->setFixedSize(infoPanelWidth, 40);
    pauseButton->setFocusPolicy(Qt::NoFocus);

    //Organizing everything with layouts
    tableLayout = new QGridLayout();
    QVBoxLayout* optionsLayout = new QVBoxLayout();
    optionsLayout->addWidget(mapSizeLabel);
    optionsLayout->addWidget(mapSizeSlider);
    optionsLayout->addWidget(wallNumberLabel);
    optionsLayout->addWidget(wallNumberSlider);
    optionsLayout->addWidget(enemyNumberLabel);
    optionsLayout->addWidget(enemyNumberSlider);
    optionsLayout->addWidget(enemySpeedLabel);
    optionsLayout->addWidget(enemySpeedSlider);
    QHBoxLayout* radioButtonLayout = new QHBoxLayout();
    radioButtonLayout->addWidget(destroyWallLabel);
    radioButtonLayout->addWidget(destroyWallButton);
    optionsLayout->addLayout(radioButtonLayout);
    optionsLayout->addWidget(newGameButton);
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->addLayout(optionsLayout);
    menuLayout->addWidget(enemyCounterLabel);
    menuLayout->addWidget(infoLabel);
    menuLayout->addWidget(timeCounter);
    menuLayout->addWidget(pauseButton);
    menuLayout->setAlignment(Qt::AlignRight);
    menuLayout->setMargin(10);
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout(tableLayout);
    mainLayout->addLayout(menuLayout);
    setLayout(mainLayout);

    //connecting event handlers
    connect(mapSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(setSliderMaxValues()));
    connect(wallNumberSlider, SIGNAL(valueChanged(int)), this, SLOT(setWallNumberText()));
    connect(enemyNumberSlider, SIGNAL(valueChanged(int)), this, SLOT(setEnemyNumberText()));
    connect(enemySpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(setEnemySpeedText()));
    connect(newGameButton, SIGNAL(clicked()), this, SLOT(generateTable()));
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(pauseGame()));

    gameBegan = false;

}

GameView::~GameView()
{
 delete model;
}


//After deleting the old one, creates a new GameModel with the specified parameters.
void GameView::generateTable(){
    //if there is already a game started, first it has to be cleared
    if ( gameBegan ) {
    //if (model != NULL) {
        delete model;
        foreach(QVector<QLabel*> rows, gameTable){
            foreach(QLabel* tile, rows){
                tableLayout->removeWidget(tile);
                delete tile;
            }
        }
        gameTable.clear();
    }

    gameBegan = true;


    //creating new model for the new game
    mapSize = mapSizeSlider->value();
    //possible improvement: don't delete and recreate the model each time when a new game is started
    model = new GameModel(mapSize, wallNumberSlider->value(), enemyNumberSlider->value(), enemySpeedSlider->value(), destroyWallButton->isChecked());

    connect(model, SIGNAL(tableChanged(QVector<QVector<GameModel::TileType> >,GameModel::Position,QList<GameModel::Position>)),
            this, SLOT(gameModel_refreshTable(QVector<QVector<GameModel::TileType> >,GameModel::Position,QList<GameModel::Position>)));
    connect(model, SIGNAL(statusChanged(int,int,bool,int)), this, SLOT(gameModel_refreshStatus(int,int,bool,int)));
    connect(model,SIGNAL(gameEnded(bool)),this,SLOT(gameModel_gameEnded(bool)));

    model->startTimers();

    //creating a label grid as the game table
    gameTable.resize(mapSize);
    for (int i = 0; i < mapSize; ++i)
    {
        gameTable[i].resize(mapSize);
        for (int j = 0; j < mapSize; ++j)
        {
            gameTable[i][j] = new QLabel(this);
            gameTable[i][j]->setStyleSheet("border: 1px solid grey");
            //gameTable[i][j]->setFixedHeight(20);
            //gameTable[i][j]->setFixedWidth(20);
            gameTable[i][j]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
            gameTable[i][j]->setMinimumWidth(this->height() / mapSize);
            gameTable[i][j]->setMinimumHeight(this->height() / mapSize);
            gameTable[i][j]->setScaledContents( true );
            tableLayout->addWidget(gameTable[i][j], i, j);
        }
    }
    //this->resize(sizeHint()); //automatically resize application window
    model->requestUpdate();

    infoLabel->setText("");
    infoLabel->setFont(QFont("Times New Roman", 25, QFont::Bold));
    enemyCounterLabel->setText("");
    pauseButton->setDisabled(false);
}


//Updates the appearance of the table.
//parameters: gametable (floor, wall, or explosion), player, enemies
void GameView::gameModel_refreshTable(QVector<QVector<GameModel::TileType> > tiles, GameModel::Position p, QList<GameModel::Position> e){
    for (int i = 0; i < mapSize; ++i)
    {
        for (int j = 0; j < mapSize; ++j)
        {
            gameTable[i][j]->setPixmap(QPixmap()); //clears any images on the label
            if (tiles[i][j] == GameModel::Floor) gameTable[i][j]->setStyleSheet("background-color: rgb(249,255,175);");
            else if (tiles[i][j] == GameModel::Wall) gameTable[i][j]->setStyleSheet("background-color: rgb(139,139,139);");

            if(tiles[i][j] == GameModel::TargetFloor) {
                gameTable[i][j]->setStyleSheet("background-color: rgb(249,255,175);");
                QPixmap pix(":/crosshair.png");
                gameTable[i][j]->setPixmap(pix);
            }
            else if (tiles[i][j] == GameModel::FloorUnderExplosion || tiles[i][j] == GameModel::WallUnderExplosion) {
                QPixmap pix(":/explosion.png");
                gameTable[i][j]->setPixmap(pix);
            }
        }
    }

    gameTable[p.x][p.y]->setStyleSheet("background-color: rgb(0,70,197);");

    foreach(GameModel::Position enemy, e){
        gameTable[enemy.x][enemy.y]->setStyleSheet("background-color: rgb(193,0,0);");
    }

}

//Updates the informationpanel, displaying the number of enemies slain ("score") and the elapsed time.
//Indicates the time left before a detonation.
void GameView::gameModel_refreshStatus(int bombedEnemies, int gameTime, bool airstrike, int countdown){
    QDateTime time;
    time.setTime_t(gameTime);
    QString textTime = time.toString("mm:ss");
    timeCounter->display(textTime);

    enemyCounterLabel->setText("\nEnemies bombed: " + QString::number(bombedEnemies));

    if (airstrike){
        infoLabel->setText("\nAir Strike \n incoming in...\n" + QString::number(countdown));
    } else {
        infoLabel->setText("");
    }
}


//Updates the information panel about the status of the game: player won, or player lost
//Handles any other post-match interface changes.
void GameView::gameModel_gameEnded(bool playerWon){
    if (playerWon){
        infoLabel->setText("Misson Completed!");
    } else {
        infoLabel->setText("Misson Failed!");
    }
    pauseButton->setDisabled(true);
}


//this method handles keyboard input
void GameView::keyPressEvent(QKeyEvent* event){
    switch (event->key()) {
    case Qt::Key_Space:
        model->airstrikeCalled();
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        model->playerMoved(GameModel::Up);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        model->playerMoved(GameModel::Right);
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        model->playerMoved(GameModel::Down);
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        model->playerMoved(GameModel::Left);
        break;
    }
}


//Changes the appearance upon resizing the window.
void GameView::resizeEvent(QResizeEvent *){
    if (gameBegan){
        int infoPanelWidth = this->width() - this->height();
        mapSizeSlider->setMaximumWidth(infoPanelWidth);
        wallNumberSlider->setMaximumWidth(infoPanelWidth);
        enemyNumberSlider->setMaximumWidth(infoPanelWidth);
        enemySpeedSlider->setMaximumWidth(infoPanelWidth);
        newGameButton->setFixedSize(infoPanelWidth, 60);
        enemyCounterLabel->setMaximumWidth(infoPanelWidth);
        infoLabel->setMaximumWidth(infoPanelWidth);
        timeCounter->setFixedSize(infoPanelWidth,80);
        pauseButton->setFixedSize(infoPanelWidth, 40);

        for (int i = 0; i < mapSize; ++i)
        {
            for (int j = 0; j < mapSize; ++j)
            {
                gameTable[i][j]->setMinimumWidth(this->height() / mapSize);
                gameTable[i][j]->setMinimumHeight(this->height() / mapSize);
            }
        }
    }
}


//Upon clicking the 'pauseButton', this method is called.
//Responsible for calling the model's pauseGame method, and changing the interface accordingly.
void GameView::pauseGame(){
    if(gameBegan){
        model->pauseGame();
        //TODO: don't ask the model, let it emit a signal when the game has paused/continued!
        if (model->gamePaused()){
            pauseButton->setText("Unfreeze time!");
        } else {
            pauseButton->setText("Freeze time!");
        }
    }
}

//This method ensures that the user won't be able to set invalid values for enemies and walls.
//It also changes the text of 'mapSizeLabel'.
void GameView::setSliderMaxValues(){
    int size = mapSizeSlider->value();
    wallNumberSlider->setMaximum(size*size / 4 + size);
    enemyNumberSlider->setMaximum(size);

    mapSizeLabel->setText("Size of the map: " + QString::number(size) + "x" + QString::number(size));
}

//changes the label belonging to 'wallNumberSlider'
void GameView::setWallNumberText(){
    wallNumberLabel->setText("Number of walls: " + QString::number(wallNumberSlider->value()) );
}

//changes the label belonging to 'enemyNumberSlider'
void GameView::setEnemyNumberText(){
    enemyNumberLabel->setText("Number of enemies: " + QString::number(enemyNumberSlider->value()) );
}

//changes the label belonging to 'enemySpeedSlider'
void GameView::setEnemySpeedText(){
    enemySpeedLabel->setText("Enemy speed: " + QString::number(enemySpeedSlider->value()) );
}
