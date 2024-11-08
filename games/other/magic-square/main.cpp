
#include <iostream>
using namespace std;

int main() {

    int squareSize;
    cout << "Enter the width of the magic square: ";
    cin >> squareSize;

    int **square= new int*[squareSize];
    for (int i = 0; i < squareSize; i++) {
        square[i] = new int[squareSize];
        for(int j = 0; j < squareSize; j++) {
            square[i][j] = 0;
        }
    }

    int col = squareSize/2;
    int row = 0;
    square[row][col] = 1;

    for(int i = 2; i <= squareSize*squareSize; i++) {
        col++;
        row--;
        if(row == -1 && col == squareSize) {
            col--;
            row += 2;
        }
        else if(row < 0) {row = squareSize-1;}
        else if(col >= squareSize) {col= 0;}

        if(square[row][col] != 0) {
            row += 2;
            col--;
        }

        cout << row << ", " << col << endl;

        square[row][col] = i;
    }

    for (int i = 0; i < squareSize; i++) {
        for(int j = 0; j < squareSize; j++) {
            int bufferSpaces = to_string(squareSize*squareSize).length() - to_string(square[i][j]).length();
            for(int k = 0; k < bufferSpaces; k++) {
                cout << " ";
            }
            cout << square[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}