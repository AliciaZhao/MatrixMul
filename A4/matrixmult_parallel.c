/**
 * Description:Compute array multiplication in a parallel fashion using multiple processes.
 * For example, the ith child process will compute dot products of the ith row of Ai against W to return the vector Ai * W .
 * Thus it will return the ith row of result R. We will assume they're square: 8x8 for A, and 8x8 for W.
 * We will compute A*W.
 *
 * Author names: Luisa Arias Barajas and Alicia Zhao
 * Author emails:luisa.ariasbarajas@sjsu.edu and alicia.zhao@sjsu.edu
 * Last modified date: 11/5/2023
 * Creation date: 9/15/2023
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>


//defining values
#define matrixSize 8

/**
 * @param file_name
 * @param message
 *
 * this function serves to make writing in logs a bit easier.
 * This function will open and close the files and write logs into the .err or .out file
 */
void logMessage(const char *file_name, char *message) {
    FILE *file = fopen(file_name, "a");
    if(file == NULL){
        perror("Failed to open log file");
        exit(1);
    }

    fprintf(file, "%s\n", message);
    fflush(file);
    fclose(file);
}

/**
 * This unction is a new function that adds on to send the matrix to the log file
 */
void logMatrix(const char *file_name, int matrix[matrixSize][matrixSize]){

    FILE *file  = fopen(file_name, "a");
    if(file == NULL){
        perror("Failed to open file");
        exit(1); //exit with error
    }

    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            fprintf(file, "%2d", matrix[i][j]);
            fprintf(file, " ");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "]\n");
    fclose(file);
}

/**
 * This function reads a file line by line and stores it in a matrix.
 * Assumption: there is a matrix inside the file and the file exists.
 * Input parameters: rows, cols, matrix(to store in), filename
 * Returns: a matrix
**/
int readMatrix(int rows, int cols, int matrix[rows][cols], char* filename){
    //open file
    char output_file[50];
    sprintf(output_file, "%d.out", getpid());

    FILE *fp = fopen(filename, "r");

    //initialize variables
    char line[100];
    int i = 0, j = 0;

    //read file line by line
    while (fgets(line, sizeof(line), fp)) {
        char *token;
        token = strtok(line, " ");

        //store line in matrix
        while (token != NULL) {
            //convert string to int
            matrix[i][j] = atoi(token);
            token = strtok(NULL, " ");
            j++;
        }
        j = 0;
        i++;
    }
    //NEW: call logMatrix to add the read matrix to log
    logMessage( output_file,strcat(filename, "=["));
    logMatrix(output_file, matrix);

    //close file
    fclose(fp);
    return 0;
}

/**
 * This function performs an array multiplication in a parallel fashion.
 * Assumption: matrices have fixed size: A is 8x8, W is 8x8, are not empty, and are passed in the correct order.
 * Input parameters: matrixA, matrixW, result matrix
 * Returns: a matrix
**/
void matrixCalculation(int row, int matrixA[matrixSize][matrixSize], int matrixW[matrixSize][matrixSize], int result[matrixSize]){

    // Multiplying matrixA and matrixW and storing in array result.
    for (int i = 0; i < matrixSize; i++) {
        result[i] = 0;
        for (int j = 0; j < matrixSize; j++) {
            result[i] += matrixA[row][j] * matrixW[j][i];
        }
    }
}

/**
 * This function initializes the matrices, reads the files, and calls the matrixCalculation function. It also detects
 * any errors with the files.
 * Assumption: the files contain numbers only.
 * Input parameters: argc and argv, Files: matrixA, matrixW
 * Returns: a matrix, exit(0) if successful, or exit(1) if there is an error.
**/
int main(int argc, char* argv[]) {

    // Record start time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    if(argc < 3) {
        //if there are less than 2 files, then print to stderr error messages and terminate with code 1
        fprintf(stderr, "Error - Command 0: \nReceived %d arguments, expecting more or equal to 2 files as input. \nTerminating with exit code 1\n", argc - 1);
        //exit failure
        exit(1);
    }

    //logs for R matrix
    char output_file[50];
    sprintf(output_file, "%d.out", getpid());

    //logs for error
    char output_err[50];
    sprintf(output_err, "%d.err", getpid());

    //initialize matrices
    int matrixA[matrixSize][matrixSize];
    int matrixW[matrixSize][matrixSize];
    int result[matrixSize][matrixSize];
    //initialize matrixA with 0s
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            matrixA[i][j] = 0;
        }
    }
    //initialize matrixW with 0s
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            matrixW[i][j] = 0;
        }
    }

    //matrix file names
    char matrixAtxt[50];
    char matrixWtxt[50];
    char test_file[100];

    //copy file names to variables
    strncpy(matrixAtxt, argv[1], 50);
    strncpy(matrixWtxt, argv[2], 50);

    //open files
    FILE* fpA = fopen(matrixAtxt, "r");
    FILE* fpW = fopen(matrixWtxt, "r");

    bool exitTrue = false;
    //check if file matrixA exists
    if (fpA == NULL ) {
        //file matrixA does not exist
        sprintf(test_file, "Error - cannot open file %s.", matrixAtxt);
        logMessage(output_err, test_file);

        exitTrue = true;
    }

    //check if file matrixW exists
    if(fpW == NULL){
        //file matrixW does not exist
        sprintf(test_file, "Error - cannot open file %s.", matrixWtxt);
        logMessage(output_err, test_file);

        exitTrue = true;

    }

    //if any of the files do not exist, then terminate with code 1
    if(exitTrue){
        logMessage(output_err, "Terminating, exit code 1.");
        exit(1);
    }

    //close files
    fclose(fpA);
    fclose(fpW);

    //read matrixA from file
    readMatrix(matrixSize, matrixSize, matrixA, matrixAtxt);

    //read matrixW from file
    readMatrix(matrixSize,matrixSize,matrixW, matrixWtxt);

    //compute array multiplication in a parallel fashion using multiple processes (fork)
    for(int i = 0; i < matrixSize; i++){
        int pid;
        int pipefd[2];

        // Create a pipe
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }

        // Fork a child process
        if ((pid = fork()) == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {  // Child process
            close(pipefd[0]);  // Close the read end of the pipe in the child process

            int row_result[matrixSize];
            matrixCalculation(i, matrixA, matrixW, row_result);

            // Write the result to the parent process through the pipe
            write(pipefd[1], row_result, sizeof(row_result));
            close(pipefd[1]);  // Close the write end of the pipe in the child process

            exit(0);
        } else {  // Parent process
            close(pipefd[1]);  // Close the write end of the pipe in the parent process
            wait(NULL);

            // Read the result from the child process through the pipe
            read(pipefd[0], result[i], sizeof(result[i]));

            close(pipefd[0]);  // Close the read end of the pipe in the parent process

        }

    }

    // Serialize the result matrix into a buffer
    int data_size = matrixSize * matrixSize;
    int serialized_data_size = data_size * sizeof(int);
    int* serialized_data = (int*)malloc(serialized_data_size);

    if (serialized_data == NULL) {
        perror("malloc");
        return 1;
    }

// Flatten the result matrix into a 1D array
   int k = 0;
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            serialized_data[k] = result[i][j];
            k++;
        }
    }

// Write the size of the serialized data to the pipe
    if (write(STDOUT_FILENO, &serialized_data_size, sizeof(int)) < 0) {
        perror("write");
        return 1;
    }

// Write the serialized data to the pipe
    if (write(STDOUT_FILENO, serialized_data, serialized_data_size) < 0) {
        perror("write");
        return 1;
    }

// Free dynamically allocated memory
    free(serialized_data);

    // Record end time
    gettimeofday(&end, NULL);

    //writing logs to .out
    char mat[100];
    sprintf(mat, "R = [");
    logMessage(output_file, mat);

    logMatrix(output_file, result);

    //exit success
    exit(0);
}