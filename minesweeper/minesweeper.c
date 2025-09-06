#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_MOVES 100
#define MAX_BOARD_SIZE 10
#define BOARD_SYMBOL '#'
#define HIT_SYMBOL 'X'
#define MINE_SYMBOL '*'
#define VISITED_ZERO 'Z' /*Marks checked zero in recursive function*/

#define MAP_TXT "map.txt"
#define MOVE_TXT "moves.txt"

int CheckWinState(char [][MAX_BOARD_SIZE], int);
void UpdateGameBoard(char [][MAX_BOARD_SIZE], int [][MAX_BOARD_SIZE], int, int, int);
int CheckSlot(char [][MAX_BOARD_SIZE], int, int, int);
void CheckSurrounding(char [][MAX_BOARD_SIZE], int[][MAX_BOARD_SIZE], int);

/*Starting functions*/
void LocateMines(char [][MAX_BOARD_SIZE], int, int);
void FillBoard(char [][MAX_BOARD_SIZE], int);
void PrintBoard(char [][MAX_BOARD_SIZE], int);

/*Saving and undoing steps*/
void Undo(int [], int [], int, char [][MAX_BOARD_SIZE], int [][MAX_BOARD_SIZE], int);
void UndoZero(char [][MAX_BOARD_SIZE], int [][MAX_BOARD_SIZE], int, int, int);
void CheckPreviousMoves(int [], int [], int, char [][MAX_BOARD_SIZE], int [][MAX_BOARD_SIZE]);
void PushMove(int [], int [], int *, int, int);
void PopMove(int *);
void SaveMovesToFile(int [], int [], int);

int main()
{
    srand(time(NULL));

    int isGameOver = 0;
    int moveCounter = 1;
    
    int boardSize = (rand() % (MAX_BOARD_SIZE - 1)) + 2;
    int mineAmount = boardSize * boardSize / 4;

    char gameBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int numberBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE];

    int inputRow, inputColumn;

    /*Saving steps*/
    int movesRow[MAX_MOVES], movesColumn[MAX_MOVES];
    int lastMoveIndex = 0;

    char buffer[50];

    /*gameBoard*/
    FillBoard(gameBoard, boardSize);
    LocateMines(gameBoard, boardSize, mineAmount);
    /*numberBoard*/
    CheckSurrounding(gameBoard, numberBoard, boardSize);

    while(!isGameOver && moveCounter < MAX_MOVES)
    {     
        PrintBoard(gameBoard, boardSize);

        printf("Enter move (row col) or 'undo': ");
        if(scanf(" %d %d", &inputRow, &inputColumn) != 2)
        {
            scanf(" %s", buffer);

            if(strcmp(buffer, "undo") == 0)
            {
                if(moveCounter > 1)
                {
                    Undo(movesRow, movesColumn, lastMoveIndex, gameBoard, numberBoard, boardSize);
                    PopMove(&lastMoveIndex);
                    moveCounter--;
                    printf("Successfully undone!\n");
                }
                else
                    printf("You cannot undo on the first move\n");
            }
            else
            {
                printf("Please enter valid input!\n");
            }

            continue;
        }

        if(gameBoard[inputRow][inputColumn] < '0' && gameBoard[inputRow][inputColumn] > '9')
        {
            printf("This cell is already uncovered!\n");
            continue;
        }
        else if(inputRow < 0 || inputRow >= boardSize || inputColumn < 0 || inputColumn >= boardSize)
        {
            printf("Please enter valid number!\n");
            continue;
        }

        if(gameBoard[inputRow][inputColumn] == BOARD_SYMBOL)
        {
            UpdateGameBoard(gameBoard, numberBoard, boardSize, inputRow, inputColumn);
            
            PushMove(movesRow, movesColumn, &lastMoveIndex, inputRow, inputColumn);
        }
        else if(gameBoard[inputRow][inputColumn] == MINE_SYMBOL)
        {
            gameBoard[inputRow][inputColumn] = HIT_SYMBOL;
            PushMove(movesRow, movesColumn, &lastMoveIndex, inputRow, inputColumn);
            printf("Boom, you hit a mine!\n");
            isGameOver = 1;
        }

        if(CheckWinState(gameBoard, boardSize))
        {
            printf("Congratulations, you have opened mine-free slots!\n");
            isGameOver = 1;
        }

        moveCounter++;
    }
    
    PrintBoard(gameBoard, boardSize);
    SaveMovesToFile(movesRow, movesColumn, lastMoveIndex);

    return 0;
}

void Undo(int movesRow[], int movesColumn[], int lastIndex, char board[][MAX_BOARD_SIZE], int numberBoard[][MAX_BOARD_SIZE], int boardSize)
{
    int lastRow = movesRow[lastIndex - 1];
    int lastColumn = movesColumn[lastIndex - 1];

    if(numberBoard[lastRow][lastColumn] != 0)
    {
        board[lastRow][lastColumn] = BOARD_SYMBOL;
    }
    else
    {
        UndoZero(board, numberBoard, boardSize, lastRow, lastColumn);
        CheckPreviousMoves(movesRow, movesColumn, lastIndex, board, numberBoard); /*for cells that open before and next to zero*/
    }
}

void CheckPreviousMoves(int movesRow[], int movesColumn[], int lastIndex, char board[][MAX_BOARD_SIZE], int numberBoard[][MAX_BOARD_SIZE])
{
    int i;
    for(i = 0; i < lastIndex - 1; i++)
    {
        if(board[movesRow[i]][movesColumn[i]] == BOARD_SYMBOL)
        {
            board[movesRow[i]][movesColumn[i]] = numberBoard[movesRow[i]][movesColumn[i]] + '0';
        }
    }
}

void UndoZero(char board[][MAX_BOARD_SIZE], int numberBoard[][MAX_BOARD_SIZE], int boardSize, int y, int x)
{
    board[y][x] = BOARD_SYMBOL;

    int i ,j;
    for(i = -1; i <= 1; i++)
    {
        for(j = -1; j <= 1; j++)
        {
            if(y + i < boardSize && y + i >= 0 && x + j < boardSize && x + j >= 0)
            {
                if(board[y + i][x + j] == VISITED_ZERO)
                    UndoZero(board, numberBoard, boardSize, y + i, x + j);

                if(board[y + i][x + j] <= '9' || board[y + i][x + j] > '0') /*Already it cannot be '0'*/
                    board[y + i][x + j] = BOARD_SYMBOL;
            }
        }
    }
}

void PushMove(int movesRow[], int movesColumn[], int *lastIndex, int newRow, int newColumn)
{
    movesRow[*lastIndex] = newRow;
    movesColumn[*lastIndex] = newColumn;
    (*lastIndex)++;
}

void PopMove(int *lastIndex)
{
    if(*lastIndex > 0)
        (*lastIndex)--;
}

void SaveMovesToFile(int movesRow[], int movesColumn[], int lastIndex)
{
    FILE *moveFile = fopen(MOVE_TXT, "w");

    fprintf(moveFile, "--- Game Moves ---\n");

    int i;
    for(i = 0; i < lastIndex; i++)
    {
        fprintf(moveFile, "Move %d: (Row: %d, Col: %d)\n", i + 1, movesRow[i], movesColumn[i]);
    }
    fprintf(moveFile, "...\nTotal Movess: %d\n", lastIndex);

    fclose(moveFile);
}

int CheckWinState(char board[][MAX_BOARD_SIZE], int boardSize)
{
    int i, j;
    for(i = 0; i < boardSize; i++)
    {
        for(j = 0; j < boardSize; j++)
        {
            if(board[i][j] == BOARD_SYMBOL)
                return 0; /*Game continue*/
        }
    }
    return 1;
}

void UpdateGameBoard(char board[][MAX_BOARD_SIZE], int numberBoard[][MAX_BOARD_SIZE], int boardSize, int y, int x)
{
    if(numberBoard[y][x] != 0)
    {
        board[y][x] = numberBoard[y][x] + '0';
    }
    else
    {
        board[y][x] = VISITED_ZERO; /*To prevent check it again*/

        int i ,j;
        for(i = -1; i <= 1; i++)
        {
            for(j = -1; j <= 1; j++)
            {
                if(y + i < boardSize && y + i >= 0 && x + j < boardSize && x + j >= 0)
                {
                    if(board[y + i][x + j] != MINE_SYMBOL && board[y + i][x + j] != VISITED_ZERO)
                        board[y + i][x + j] = numberBoard[y + i][x + j] + '0';
                    
                    if(numberBoard[y + i][x + j] == 0 && board[y + i][x + j] != VISITED_ZERO)
                        UpdateGameBoard(board, numberBoard, boardSize, y + i, x + j);
                }
            }
        }
    }
}

void CheckSurrounding(char board[][MAX_BOARD_SIZE], int numberBoard[][MAX_BOARD_SIZE], int boardSize)
{
    int i, j;
    
    for(i = 0; i < boardSize; i++)
    {
        for(j = 0; j < boardSize; j++)
        {
            numberBoard[i][j] = CheckSlot(board, boardSize, i, j);
        }
    } 
}

int CheckSlot(char board[][MAX_BOARD_SIZE], int boardSize, int row, int column)
{
    int mineCounter = 0, i ,j;
    for(i = -1; i <= 1; i++)
    {
        for(j = -1; j <= 1; j++)
        {
            if(row + i < boardSize && row + i >= 0 && column + j < boardSize && column + j >= 0)
                if(board[row + i][column + j] == MINE_SYMBOL)
                    mineCounter++;
        }
    }
    return mineCounter;
}

void LocateMines(char board[][MAX_BOARD_SIZE], int boardSize, int mineAmount)
{
    FILE *mapFile = fopen(MAP_TXT, "w");

    int i = 0, j, row, column;
    while(i < mineAmount)
    {
        row = rand() % boardSize; 
        column = rand() % boardSize;

        if(board[row][column] == MINE_SYMBOL)
            continue;

        board[row][column] = MINE_SYMBOL;

        i++;
    }

    for(i = 0; i < boardSize; i++)
    {
        for(j = 0; j < boardSize; j++)
        {
            if(board[i][j] == BOARD_SYMBOL)
                fprintf(mapFile, ". ");
            else if(board[i][j] == MINE_SYMBOL)
                fprintf(mapFile, "* ");
        }
        fprintf(mapFile, "\n");
    }

    fclose(mapFile);
}

void FillBoard(char board[][MAX_BOARD_SIZE], int size)
{
    int i, j;
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            board[i][j] = BOARD_SYMBOL;
        }
    }
}

void PrintBoard(char board[][MAX_BOARD_SIZE], int boardSize)
{
    int i, j;

    /*First line*/
    printf("\n  ");
    for(i = 0; i < boardSize; i++)
        printf("%-2d", i);
    printf("\n");

    for(i = 0; i < boardSize; i++)
    {
        printf("%-2d", i);

        for(j = 0; j < boardSize; j++)
        {
            if(board[i][j] == MINE_SYMBOL) 
                printf("# ");
            else if(board[i][j] == VISITED_ZERO)
                printf("0 ");
            else
                printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}