//Sudoku validator
//By Shirisha Vadlamudi & Peter Kemper

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

typedef struct{
	int row;
	int col;
	int (*grid)[9];
}sudoku;

void *validateSubgrid(void *);
void *validateRows(void *);
void *validateCols(void *);
int readSudoku(int (*grid)[9], FILE *);

#define totalThreads 11

int main(int argc, char *argv[])
{
	int grid[9][9];
	FILE *fp = fopen(argv[1], "r");

	fseek(fp, 0, SEEK_CUR);

	// Initalize parameters for subgrid evaluation threads
	sudoku *data[9];
	int row, col, i = 0;
	for(row = 0; row < 9; row += 3)
	{
		for(col = 0; col < 9; col += 3, ++i)
		{
			data[i] = (sudoku *)malloc(sizeof(sudoku));
			if(data[i] == NULL){
				int err = errno;
				puts(strerror(err));
				exit(EXIT_FAILURE);
			}
			data[i]->row = row;
			data[i]->col = col;
			data[i]->grid = grid;
		}
	}

	// Validate all sudoku grids in file
	pthread_t tid[totalThreads];
	int p, j, h, retCode, isValid = 1, t_status[totalThreads];

		if(readSudoku(grid, fp)){
			puts("Something happened while reading the grid from the file");
			exit(EXIT_FAILURE);
		}

		// Create threads for subgrid validation 
		for(p = 0; p < 9; ++p){
			retCode = pthread_create(&tid[p], NULL, validateSubgrid, (void *)data[p]);
			if (retCode != 0){
				fprintf(stderr, "Error in creating threads for subgrid validation: %d\n", retCode);
				exit(EXIT_FAILURE);
			}
		}


		// Create thread for row validation
		retCode = pthread_create(&tid[9], NULL, validateRows, (void *)data[0]);
		if (retCode != 0){
			fprintf(stderr, "Error in creating thread for row validation: %d\n", retCode);
			exit(EXIT_FAILURE);
		}

		// Create thread for column validation
		retCode = pthread_create(&tid[10], NULL, validateCols, (void *)data[0]);
		if (retCode != 0){
			fprintf(stderr, "Error in creating thread for column validation: %d\n", retCode);
			exit(EXIT_FAILURE);
		}

		// Join all threads
		for(j = 0; j < totalThreads; ++j){
			retCode = pthread_join(tid[j], (void *)&t_status[j]);
			if(retCode != 0){
				fprintf(stderr, "Error in joining threads: %d\n", retCode);
				exit(EXIT_FAILURE);
			}
		}

		// Check the status returned by each thread
		for(h = 0; h < totalThreads; ++h){
			if(t_status[h] != 0){
				isValid = 0;
				break;
			}
		}

		if(isValid){
			printf("VALID Sudoku Puzzle\n");
		}else{
			printf("INVALID Sudoku Puzzle\n");
		}

	// Free the memory
	int k;
	for(k = 0; k < 9; ++k){
		free(data[k]);
	}
	fclose(fp);

	return 0;
}

// Validates the subgrids of sudoku
void *validateSubgrid(void *data)
{
	int gridDigits[10] = {0};
	sudoku *params = (sudoku *)data;
	int i, j;
	for(i = params->row; i < params->row + 3; ++i){
		for(j = params->col; j < params->col + 3; ++j){
			if(gridDigits[params->grid[i][j]] == 1){
				return (void *)-1;
			}
			gridDigits[params->grid[i][j]] = 1;
		}
	}
	return (void *)0;
}

// Validates the rows of sudoku
void *validateRows(void *data)
{
	int rowDigits[10] = {0};
	sudoku *params = (sudoku *)data;
	int i, j;
	for(i = 0; i < 9; ++i){
		for(j = 0; j < 9; ++j){
			if(rowDigits[params->grid[i][j]] == 1){
				return (void *)-1;
			}
			rowDigits[params->grid[i][j]] = 1;
		}
		memset(rowDigits, 0, sizeof(int)*10);
	}
	return (void *)0;
}

// Validates the columns of sudoku
void *validateCols(void *data)
{
	int columnDigits[10] = {0};
	sudoku *params = (sudoku *)data;
	int i, j;
	for(i = 0; i < 9; ++i){
		for(j = 0; j < 9; ++j){
			if(columnDigits[params->grid[j][i]] == 1){
				return (void *)-1;
			}
			columnDigits[params->grid[j][i]] = 1;
		}
		memset(columnDigits, 0, sizeof(int)*10);
	}
	return (void *)0;
}

//Reads sudoku grid from a file.
int readSudoku(int (*grid)[9], FILE *fp)
{
	fseek(fp, 0, SEEK_CUR);

	char entry;
	int i = 0, j = 0, totalValues = 0;
	while((fread(&entry, 1, 1, fp)) > 0 && totalValues < 81){ // Read 81 digits from file: sudoku grid 9x9 = 81
		if(entry != '\n'){
			if(isdigit(entry)){
				++totalValues;
				grid[i][j] = entry - '0';
				++j;
				if(j == 9){
					j = 0;
					++i;
				}
			}
			else{
				return -1;
			}
		}
	}

	return 0;
}
