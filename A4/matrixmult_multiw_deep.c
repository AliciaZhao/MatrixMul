/**
 * Description: this program takes n number of files from the command line and stdin.
 * The program will pass each line of files to matrixmult_parallel.c and will return the result matrix.
 * Then the program will add all the result matrices and store it in R.txt.
 *
 * Author names: Luisa Arias Barajas and Alicia Zhao
 * Author emails:luisa.ariasbarajas@sjsu.edu and alicia.zhao@sjsu.edu
 * Last modified date: 11/5/2023
 * Creation date: 10/25/2023
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define matrixSize 8
#define lineLength 1048576
#define fileLength 256
/**
 * @param file_name
 * @param message
 *
 * this function serves to make writing in logs a bit easier.
 * This function will open and close the files and write logs into the .err or .out file
 * Input parameters: file_name: file name that we will be writing to, message: message that we will write.
 * returns: return to main function if successful, otherwise returns exit(1)
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
 * This function reads a file line by line and stores it in a matrix.
 * @param R_SUM
 * @param row
 * @param result
 */
void readAddMatrix(int R_SUM[matrixSize][matrixSize], int row, char result[fileLength]){
    char *token;
    token = strtok(result, " ");
    int j = 0;

    //store line in matrix
    while (token != NULL) {
        //convert string to int
        R_SUM[row][j] += atoi(token);
        token = strtok(NULL, " ");
        j++;
    }

}

//load into R into file (overwrite)
/**
 * This function replaces the result matrix in the file with the new matrix.
 * @param filename
 * @param R_SUM
 */
void replaceMatrixInFile(char *filename, int R_SUM[matrixSize][matrixSize]){
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening the file");
    }

    // Write the new matrix content to the file
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            fprintf(file, "%d ", R_SUM[i][j]);
        }
        fprintf(file, "\n"); // Newline at the end of each row
    }

    // Close the file
    fclose(file);
}

/**
 * This function parses a line and stores it in an array of files.
 * @param line
 * @param files
 * @return fileC
 */
int parse_line(char *line, char *files[]) {
    size_t input_length = strcspn(line, "\n");
    if (line[input_length] == '\n') {
        line[input_length] = '\0';  // Replace '\n' with '\0' to truncate the string
    }
    int fileC = 0;
    char *token;
    token = strtok(line, " ");//tokenize
    //files[fileC]=(char*)malloc(256*sizeof(char));
    //strcpy(files[fileC],token );

    while(token != NULL){ //counts files per line
        files[fileC]=(char*)malloc(fileLength*sizeof(char));
        strcpy(files[fileC],token );
        fileC++;
        token = strtok(NULL, " ");
    }
    return fileC;
}

/**
 * This function executes multiple matrix multiplications in parallel.
 * It reads all matrices for all executions from files and writes the result matrix to a file.
 * Input parameters: argc and argv, Files: matrixA, matrixW
 * returns: a matrix if successful, otherwise exit(1) if failed
**/
int main(int argc, char* argv[]) {
    //initialize variables
    #define messageLen 100
    int Rsum[matrixSize][matrixSize];
    char* rsum_filename;
    int command = -1;

    //set Rsum to 0s
    for(int i = 0; i < matrixSize; i++){
        for(int j = 0; j < matrixSize; j++){
            Rsum[i][j] = 0;
        }
    }

    //record start time
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //checking if there are more or equal to 2 files
    if(argc < 3) {
        //if there are less than 2 files, then print to stderr error messages and terminate with code 1

        fprintf(stderr, "Error - Command 0: \nReceived %d arguments, expecting more or equal to 2 files as input. \nTerminating with exit code 1\n", argc - 1);
        //exit failure
        exit(1);
    }

    //entire A3 code from command line
    //for loop to execute matrixmult_parallel.c
    for(int i = 2; i  < argc; i++) {

        //create pipe
        int pipefd[2];
        if(pipe(pipefd) == -1){
            perror("pipe");
            exit(1);
        }

        //create child process
        command++;
        pid_t pid = fork();
        //fork failed
        if(pid == -1){
            perror("fork");
            exit(1);
        }

        //child process
        if(pid == 0){

            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
            close(pipefd[0]); // Close read end of the pipe
            close(pipefd[1]); // Close the original stdout

            //creating output and error files named after their pid
            char output_filen[fileLength];
            char error_filen[fileLength];
            sprintf(output_filen, "%d.out", getpid());
            sprintf(error_filen, "%d.err", getpid());
            char message[messageLen];

            sprintf(message, "Starting command %d: child PID %d of parent PPID %d\n", command, getpid(), getppid());
            logMessage(output_filen, message);

            //matrix A and W will be printed in matrix_parallel

            //the execution of matrixmult_parallel.c
            execlp("./matrixmult_parallel", "./matrixmult_parallel", argv[1], argv[i], (char *)NULL);
            perror("execlp failed");
            exit(1);
        }else{
            //log file
            char output_file [fileLength];
            sprintf(output_file, "%d.out", pid);
            char err_file [fileLength];
            sprintf(err_file, "%d.err", pid);
            //parent process
            close(pipefd[1]);  //close read end of pipe

            int status = 0; //status of child

            //message
            pid_t finished = waitpid(pid, &status, 0);

            if (finished == -1) {
                perror("waitpid");
                exit(1);
            }

            char output_msg[messageLen];
            if(WIFEXITED(status)){  //normal termination
                sprintf(output_msg, "Finished child %d pid of parent %d", pid, getpid());
                logMessage(output_file, output_msg);
                //exit signal code
                int exit_signal = WEXITSTATUS(status);
                sprintf(output_msg, "Exited with exit code = %d", exit_signal);
                logMessage(output_file, output_msg);
            } else if(WIFSIGNALED(status)){     //abnormal termination
                int exit_signal = WTERMSIG(status);
                sprintf(output_msg, "Killed with signal %d", exit_signal);
                logMessage(err_file, output_msg);
            }

            //create a buffer to read from pipe
            ssize_t bytes_read;
            int buffer_size = (matrixSize * matrixSize) * sizeof(int);
            int* buffer = malloc(buffer_size);


            // Read the data size (assuming it's an int)
            int dataSize;
            read(pipefd[0], &dataSize, sizeof (int));

            //read from pipe
            while((bytes_read = read(pipefd[0], buffer, dataSize)) > 0) {
                //add the result matrix from the buffer to Rsum
                for (int r = 0; r < matrixSize; r++) {
                    for (int j = 0; j < matrixSize; j++) {
                        Rsum[r][j] += buffer[r * matrixSize + j];
                    }
                }

            }
            free(buffer);  //free buffer
            close(pipefd[0]); // Close the read end of the pipe in the parent process

        }
    }

    //add and initialize R.txt
    rsum_filename = "Rsum.txt";
    replaceMatrixInFile(rsum_filename, Rsum); //replace the matrix in R.txt

    //command line done

    //currently in stdin
    //initialize variables
    size_t buffer_size2 = lineLength;
    // Read input lines from stdin
    char *input_line = malloc(buffer_size2);
    // Check for allocation failure
    if (input_line == NULL) {
        perror("malloc");
        exit(1);
    }

    //currently in stdin
    while (fgets(input_line, buffer_size2, stdin) != NULL) {   //reading a line
        //make Rsum have all zeros
        for(int i = 0; i < matrixSize; i++){
            for(int j = 0; j < matrixSize; j++){
                Rsum[i][j] = 0;
            }
        }

        // Truncate input_line at the first newline character
        size_t input_length = strcspn(input_line, "\n");
        if (input_line[input_length] == '\n') {
            input_line[input_length] = '\0';
        }

        //initialize variable to keep track of the number of children finished
        int childFinished = 0;

        //parse line and store in array of files
        char *files[fileLength];
        int fileC = parse_line(input_line, files);

        for(int i = 0; i < fileC; i++){ //sending array of file to execl
            //create pipe
            int pipes[2];
            if(pipe(pipes) == -1){
                perror("pipe");
                exit(1);
            }

            //create child process
            command++;
            pid_t child_pid = fork();
            if (child_pid == -1) {
                perror("fork");
                return 1;
            } else if (child_pid == 0) {
                // Child process
                dup2(pipes[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
                close(pipes[0]); // Close read end of the pipe
                close(pipes[1]); // Close the original stdout

                //creating output and error files named after their pid
                char output_filen[fileLength];
                char error_filen[fileLength];
                sprintf(output_filen, "%d.out", getpid());
                sprintf(error_filen, "%d.err", getpid());
                char message[messageLen];

                sprintf(message, "Starting command %d: child PID %d of parent PPID %d\n", command, getpid(), getppid());
                logMessage(output_filen, message);

                // Execute matrix multiplication with the appropriate files
                execl("./matrixmult_parallel", "./matrixmult_parallel", "Rsum.txt", files[i], (char *)NULL);
                perror("execl");
                exit(1);
            }else{
                // Parent process
                //log file
                // Wait for the child to finish
                close(pipes[1]); // Close write end of the pipe
                int status = 0;
                pid_t finished_pid = waitpid(child_pid, &status, 0);

                char output_file [fileLength];
                sprintf(output_file, "%d.out", child_pid);
                char err_file [fileLength];
                sprintf(err_file, "%d.err", child_pid);

                //check if waitpid failed
                if (finished_pid == -1) {
                    perror("waitpid");
                    exit(1);
                }

                //check if child finished successfully
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    //message
                    char output_msg[messageLen];
                    if(WIFEXITED(status)){  //normal termination
                        sprintf(output_msg, "Finished child %d pid of parent %d", child_pid, getpid());
                        logMessage(output_file, output_msg);

                        //exit signal code
                        int exit_signal = WEXITSTATUS(status);
                        sprintf(output_msg, "Exited with exit code = %d", exit_signal);
                        logMessage(output_file, output_msg);
                        //childFinished++;
                    } else if(WIFSIGNALED(status)){     //abnormal termination
                        int exit_signal = WTERMSIG(status);
                        sprintf(output_msg, "Killed with signal %d", exit_signal);
                        logMessage(err_file, output_msg);
                    }

                    //increment childFinished if child finished successfully
                    childFinished++;

                    //create a buffer to read from pipe
                    ssize_t bytes_read;
                    int buffer_size = (matrixSize * matrixSize) * sizeof(int);
                    int* buffer = malloc(buffer_size);

                    // Read the data size (assuming it's an int)
                    int dataSize;
                    read(pipes[0], &dataSize, sizeof (int));

                    //read from pipe
                    while((bytes_read = read(pipes[0], buffer, dataSize)) > 0) {
                        //add the result matrix from the buffer to Rsum
                        for (int r = 0; r < matrixSize; r++) {
                            for (int j = 0; j < matrixSize; j++) {
                                Rsum[r][j] += buffer[r * matrixSize + j];

                            }
                        }
                    }
                    free(buffer);
                    close(pipes[0]); // Close the read end of the pipe in the parent process
                }
            }
            //replace matrix in R.txt
            if(childFinished == fileC){
                replaceMatrixInFile(rsum_filename, Rsum);
            }
        }
        //free files
        for(int i = 0; i < fileC; i++){
            free(files[i]);
        }
    }

    //free fgets
    free(input_line);

    printf("Rsum = [ \n");
    //print RSum
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            printf("%4d ", Rsum[i][j]); // Adjust the width (4) for formatting
        }
        printf("\n"); // Move to the next row
    }
    printf(" ]\n");
    fflush(stdout);

    //record end time
    gettimeofday(&end, NULL);
    double runtime = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
    fprintf(stdout, "Parent runtime = %.2f seconds\n", runtime);

    //exit success
    exit(0);
}