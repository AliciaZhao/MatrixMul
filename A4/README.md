# cs149_A4



## Authors

 * Author names: Luisa Arias Barajas and Alicia Zhao
 * Author emails: luisa.ariasbarajas@sjsu.edu and alicia.zhao@sjsu.edu
 * Last modified date: 11/05/2023
 * Creation date: 10/25/2023

## Purpose of the code

 - Expand your Assgt3 to create a matrixmult_multiw_deep.c program. Your command-line arguments (argc/argv) are your one A matrix along with one or more W matrices 
 - (the command line arguments are the same as Assgt3). 
 - Following that on each line from stdin, you will get one or more new W matrices.

 - Given the command line arguments (argc/argv) we will do the same as Assgt3, which is to multiply A with all W arrays in children processes. 
 - Same as we did in Assgt3, we will use fork for the children and exec matrixmult_parallel (Assgt2, but modified a bit). 
 - Using exec is a requirement so we will reuse code from Assgt2 and run things in parallel.
 
## How the code works
 - For a given line of input, the parent will receive the resulting R matrices back through pipes from the children, and sum them up position-wise into the Rsum matrix, which is also 8x8. The parent will receive the R matrices from all the children for a given line and sum them up into Rsum. Since you don't know how many W's you will get at each line, you will need to parse the W filenames in a line from stdin so each child can inherit its W filename like in Assgt3 (you can store the W filenames in a dynamically allocated array with malloc, or you might not use malloc - malloc isn't required by the rubric). 1 A child will inherit the index i when it is forked and will know which Wi filename corresponds to it.

 - When the parent receives the next line of W filenames from stdin, matrixmult_multiw_deep will fork/spawn new children and will exec the Assgt2 code (matrixmult_parallel, but modified a bit), but the difference is that child i will use Rsum and Wi (instead of A and Wi). Thus, Rsum from the previous input line is multiplied by all input weight matrices W in child processes, and the results are returned to the parent to be summed position-wise and replace Rsum.

 - When the parent receives the next line of W arrays from stdin, Rsum from the previous input line is multiplied by the new weight matrices W in forked child processes and the results are returned to the parent to replace Rsum.

 - Thus, at each input line of W's from stdin you will compute in child processes that exec Assgt2 code: the product of Rsum from the previous line times all W's, send the Ri results back to the parent to sum up into a new Rsum=R1+R2+...; and that will be your input to the next line of Ws, which will again multiply Rsum from the previous layer with the W matrices.

## How to run each test

 - To compile this program outside of cLion, you will have to compile both maraixmult_parallel and matrixmult_multiw_deep. 
 Type the following in the terminal:
 
```
	$ gcc -o matrixmult_parallel matrixmult_parallel.c -Wall -Werror
	$ gcc -o matrixmult_multiw matrixmult_multiw_deep.c -Wall -Werror
```

 - Then write the following in the terminal (Note: test/A.txt and others does not have to be the same if you are using other tests): 
 - ere is an example of running matrixmult_multiw_deep on A1.txt and eight W[1-8].txt weight files (using these test files, they are the same as Assgt1 plus five more W[4-8].txt):
 
```
 	./matrixmult_multiw_deep test/A1.txt test/W1.txt test/W2.txt test/W3.txt
	test/W4.txt test/W5.txt test/W6.txt test/W7.txt 
	test/W8.txt
	....
	^D <--Ctrl-D is the EOF value, which terminates your input
```

## Expected output

 - Sample test files are under folder A4/test. Your program takes as its command-line arguments the file names. Then it prints the result.

 - $ ./matrixmult_multiw_deep A1.txt W1.txt ... (that's if your test files are under the same dir as your code, but if they're under test/ then run as $ ./matrixmult_multiw_deep test/A1.txt test/W1.txt test/W2.txt test/W3.txt)
 which will output to the .out file of each corresponding pair of A and W matrix:
**Example 1:**

```
Rsum = [ 
71350 54770 54770 54770 54770 54770 54770 97080 
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
]
```
**If you had your commands in a text file cmds.txt**
 - you could also run the above with redirection or with a pipe: 
 
```
$ ./matrixmult_multiw_deep A1.txt W1.txt W2.txt W3.txt < cmds.txt
$ cat cmds.txt | ./matrixmult_multiw_deep A1.txt W1.txt W2.txt W3.txt
```

**If files cannot be opened / does not exist:**
 - If any of your input files A or W did not exist, then an error message should be sent to the corresponding .err file.

```
Error - cannot open file A.txt
Error - cannot open file W.txt
Terminating with exit code 1
```

 - If anything less than 2 files are passed as arguments (which means the child does not have the correct argument requirements), comment below will be printed and code will then terminate with exit code 1:

```
Error - Command 0:
Received 0 arguments, expecting more or equal to 2 files as input.
Terminating with exit code 1
```

## Our test runs (this depends on when the user decides to terminate the program)
 - Test 1 runtime: 11.44 seconds
 - Test 2 runtime: 13.33 seconds
 - Test 3 runtime: 14.19 seconds
 - Average: 12.99 seconds
