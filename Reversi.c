#include <stdio.h>
#include <stdlib.h>
#include "ColorPaletteForTerminal.h"

typedef enum
{
    EMPTY_CELL = '.',
    COMPUTER_CELL = 'X',
    USER_CELL = 'O'
}CellSymbol;

typedef struct
{
    int x, y; /* they may not be necessary */
    CellSymbol cellSymbol;
}Cell;

/* taking piece functions */
int ComputeBestMove(Cell**, int);
void CountPossibleRivalPieces(Cell**, int, int, int, int[], int*, CellSymbol);
int CheckBoardForInput(Cell**, int, int, int, CellSymbol);
void ApplyInput(Cell**, Cell*, int, int, CellSymbol);

int CanPlayerMove(Cell**, int);
void CalculateResult(Cell**, int, int*, int*);
void StartGame(Cell**, int); /* RETURNS player's number cell */
void InputCellPosition(int, int*, int*); /* input for user */
void SetCell(Cell*, CellSymbol, int, int);
Cell** CreateBoard(int*);
int IsGameOver(Cell**, int);
void PrintBoard(Cell**, int, int);
void SetMemoryFree(Cell**, int);

int main()
{
    int boardSize, playersCell = 0, computersCell = 0;
    Cell **board = CreateBoard(&boardSize);

    StartGame(board, boardSize);

    /* calculate result */
    CalculateResult(board, boardSize, &playersCell, &computersCell);

    printf("GAME OVER!\n");
    if(computersCell > playersCell)
        printf("Computer won the game!\n");
    else
        printf("Player won the game!\n");

    printf("Player's cell number: %d\n", playersCell);
    printf("Computer's cell number: %d\n", computersCell);
    
    /* free memory - game over */
    SetMemoryFree(board, boardSize);
    return 0;
}

void StartGame(Cell** board, int boardSize)
{
    int isUsersTurn = 1, couldPlayerMove = 1;
    int inputX, inputY;
    int isGameOver = 0;
    PrintBoard(board, boardSize, isUsersTurn);

    while(!isGameOver && IsGameOver(board, boardSize)) 
    {
        if(isUsersTurn)
        {
            if(CanPlayerMove(board, boardSize))
            {
                InputCellPosition(boardSize, &inputX, &inputY);
                if(CheckBoardForInput(board, boardSize, inputX, inputY, USER_CELL))
                {
                    isUsersTurn = 0;
                    couldPlayerMove = 1;
                }
                else
                    printf("Selected invalid cell!\n");
            }
            else
            {
                printf("Player could not play!\n");
                isUsersTurn = 0;
                couldPlayerMove = 0;
            }
        }
        else /* computer's turn */
        {
            printf("Computer played:\n");
            if(ComputeBestMove(board, boardSize) == 0)
            {
                printf("Computer could not play!\n");
                if(couldPlayerMove == 0)
                    isGameOver = 1;
            }

            isUsersTurn = 1;
        }
        PrintBoard(board, boardSize, isUsersTurn);
    }
}

int IsGameOver(Cell **board, int size)
{
    int i, j;
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            if(board[i][j].cellSymbol == EMPTY_CELL)
            {
                return 1;
            }
        }
    }
    return 0;
}

int CanPlayerMove(Cell **board, int size)
{
    int i, j;
    int maxPieceCounter = 0; /* how many pieces computer can take */
    int bestPos[2]; /* y,x - this is not necessary (for function) */

    for(i = 0; i < size; i++) /* inputX */
    {
        for(j = 0; j < size; j++) /* inputY */
        {
            CountPossibleRivalPieces(board, size, i, j, bestPos, &maxPieceCounter, USER_CELL);
        }
    }
    
    if(maxPieceCounter == 0)
        return 0;

    return 1;
}

int ComputeBestMove(Cell **board, int size)
{
    int i, j;
    int maxPieceCounter = 0; /* how many pieces computer can take */
    int bestPos[2]; /* y,x */

    for(i = 0; i < size; i++) /* inputX */
    {
        for(j = 0; j < size; j++) /* inputY */
        {
            CountPossibleRivalPieces(board, size, i, j, bestPos, &maxPieceCounter, COMPUTER_CELL);
        }
    }
    
    if(maxPieceCounter > 0)
        CheckBoardForInput(board, size, bestPos[1], bestPos[0], COMPUTER_CELL);
    else /* computer could not play! */
        return 0;
    
    return 1;
}

void CountPossibleRivalPieces(Cell **board, int size, int inputX, int inputY, int bestPos[], int *maxPieceCounter, CellSymbol playerSymbol)
{
    int i;
    int dirX, dirY, coordinateX, coordinateY, count;
    int directions[][2] = 
    {
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}, /* cross */
        {1, 0}, {-1, 0}, {0, 1}, {0, -1} /* lineer */
    };
    CellSymbol rivalSymbol;

    if(board[inputY][inputX].cellSymbol != EMPTY_CELL)
        return;

    if(playerSymbol == COMPUTER_CELL)
        rivalSymbol = USER_CELL;
    else
        rivalSymbol = COMPUTER_CELL;

    for(i = 0; i < sizeof(directions) / sizeof(directions[0]); i++)
    {
        dirX = directions[i][0];
        dirY = directions[i][1];
        coordinateX = inputX + dirX;
        coordinateY = inputY + dirY;
        count = 0;

        /* count if rival piece exists */
        while(coordinateX >= 0 && coordinateX < size && coordinateY >= 0 && coordinateY < size && board[coordinateY][coordinateX].cellSymbol == rivalSymbol)
        {
            count++;
            coordinateX += dirX;
            coordinateY += dirY;
        }

        if(count > 0 && coordinateX >= 0 && coordinateX < size && coordinateY >= 0 && coordinateY < size && board[coordinateY][coordinateX].cellSymbol == playerSymbol)
        {
            if(count > *maxPieceCounter)
            {
                *maxPieceCounter = count;
                bestPos[0] = inputY;
                bestPos[1] = inputX;
            }
        }
        /*printf("%d\n", count);*/
    }
}

int CheckBoardForInput(Cell **board, int size, int inputX, int inputY, CellSymbol playerSymbol)
{
    int i, j, k;
    int isThereRival, isPieceTaken = 0;
    int directions[][2] = { {1, 1}, {-1, 1}, {1, -1}, {-1, -1}, /*cross section*/
                            {1, 0}, {-1, 0}, {0, 1}, {0, -1}    /*line section*/ }; /* x, y*/
    CellSymbol rivalSymbol;

    if(board[inputY][inputX].cellSymbol != EMPTY_CELL)
    {
        return isPieceTaken;
    }

    if(playerSymbol == COMPUTER_CELL)
        rivalSymbol = USER_CELL;
    else
        rivalSymbol = COMPUTER_CELL;

    
    for(i = 0; i < sizeof(directions) / sizeof(directions[0]); i++)
    {
        isThereRival = 0;
        for(j = inputX + directions[i][0], k = inputY + directions[i][1]; 
            k < size && k >= 0 && j < size && j >= 0 && board[k][j].cellSymbol != EMPTY_CELL; 
            j += directions[i][0], k += directions[i][1])
        {
            if(board[k][j].cellSymbol == rivalSymbol)
                isThereRival = 1;
            else if(isThereRival == 0 && board[k][j].cellSymbol == playerSymbol)
            {
                break;
            }
            else if(isThereRival && board[k][j].cellSymbol == playerSymbol)
            {
                ApplyInput(board, &board[k][j], inputX, inputY, playerSymbol);
                isPieceTaken = 1;
                break;
            }
        }
    }

    return isPieceTaken;
}

void ApplyInput(Cell **board, Cell *startCell, int endX, int endY, CellSymbol player)
{
    int i, j;
    int xDirection; /* 1 or - 1*/
    int yDirection;
    int stepCounter;

    /* X */
    if(endX == startCell->x) xDirection = 0;
    else xDirection = abs(endX - startCell->x) / (endX - startCell->x);
    /* Y */
    if(endY == startCell->y) yDirection = 0;
    else yDirection = abs(endY - startCell->y) / (endY - startCell->y);
    
    if(startCell->x == endX || startCell->y == endY) /* lineer direciton */
    {
        /* one of two directions is zero, thus I can sum them */
        stepCounter = abs(startCell->x - endX) + abs(startCell->y - endY); 
        
        for(i = startCell->y, j = startCell->x; stepCounter >= 0; i += yDirection, j += xDirection, stepCounter--)
        {
            board[i][j].cellSymbol = player;
        }
    }

    else /* cross direction */
    {
        xDirection = abs(endX - startCell->x) / (endX - startCell->x); /* they are not equal, so denumerator cannot be zero */
        yDirection = abs(endY - startCell->y) / (endY - startCell->y);

        for(i = 1; i <= abs(endX - startCell->x); i++)
        {
            board[startCell->y + i * yDirection][startCell->x + i * xDirection].cellSymbol = player;
        }
    }
}

void InputCellPosition(int size, int *inputX, int *inputY)
{
    char pos[10], column;
    int row, inputFlag = 1;

    while(inputFlag)
    {
        printf("Enter next coordinates (e.g. 4C): ");
        fgets(pos, sizeof(pos), stdin);

        sscanf(pos, " %d%c", &row, &column);

        if(row <= 0 || row > size || column < 'A' || column >= 'A' + size)
        {
            printf("Invalid coordinates! ");
        }
        else
            inputFlag = 0;
    }

    *inputX = column - 'A';
    *inputY = row - 1;
}

void CalculateResult(Cell** board, int size, int *player, int *computer)
{
    int i, j;
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            if(board[i][j].cellSymbol == USER_CELL)
            {
                (*player)++;
            }
            else if(board[i][j].cellSymbol == COMPUTER_CELL)
            {
                (*computer)++;
            }
        }
    }
}

Cell** CreateBoard(int *size)
{
    Cell **board;
    int i, j;
    do
    {
        printf("Enter the length of the game board side (greater than two and even number): ");
        scanf(" %d", size);
        while (getchar() != '\n'); /* clear buffer for fgets() ! */
    } while(*size <= 2 || *size % 2 == 1);
    
    board = (Cell**)malloc(sizeof(Cell*) * (*size));
    if (board == NULL) 
    {
        printf("Could not allocate memory!\n");
        return NULL;
    }

    for(i = 0; i < (*size); i++)
    {
        board[i] = (Cell*)malloc(sizeof(Cell) * (*size));
        if (board[i] == NULL)
        {
            printf("Could not allocate memory!\n");

            while(i > 0) /* set free previous allocated memories */
            {
                free(board[--i]);
            }
            free(board);

            return NULL;
        }
    }

    /* board successfully created */
    /* start board */

    for(i = 0; i < *size; i++)
    {
        for(j = 0; j < *size; j++)
        {
            SetCell(&board[i][j], EMPTY_CELL, j, i);
        }
    }

    board[*size / 2 - 1][*size / 2 - 1].cellSymbol = COMPUTER_CELL;
    board[*size / 2][*size / 2].cellSymbol = COMPUTER_CELL;
    board[*size / 2 - 1][*size / 2].cellSymbol = USER_CELL;
    board[*size / 2][*size / 2 - 1].cellSymbol = USER_CELL;

    return board;
}

void SetCell(Cell *cell, CellSymbol symbol, int x, int y)
{
    cell->cellSymbol = symbol;
    cell->x = x;
    cell->y = y;
}

void PrintBoard(Cell **board, int size, int isUserTurn)
{
    int i, j;

    printf("   ");
    for(i = 0; i < size; i++) /* first line */
        printf("%c ", 'A' + i);
    printf("\n");
    for (i = 0; i < size; i++)
    {
        printf("%-2d ", i + 1);
        for(j = 0; j < size; j++)
        {
            if(board[i][j].cellSymbol == USER_CELL)
                isUserTurn == 1 ? printf(BGREEN "%c " RESET, board[i][j].cellSymbol) : printf(GREEN "%c " RESET, board[i][j].cellSymbol);
                
            else if(board[i][j].cellSymbol == COMPUTER_CELL)
                isUserTurn == 0 ? printf(BRED "%c " RESET, board[i][j].cellSymbol) : printf(RED "%c " RESET, board[i][j].cellSymbol);

            else
                printf("%c ", board[i][j].cellSymbol);
        }
        printf("\n");
    }   
}

void SetMemoryFree(Cell **board, int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        free(board[i]);
    }
    free(board);
}