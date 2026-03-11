Mini Linux Shell (C)
Overview

This project implements a mini Linux shell written in C. The shell mimics basic functionality of a standard Unix shell by allowing users to execute commands, change directories, run programs, redirect input/output, and run background processes.

The shell continuously prompts the user for input, parses the command, and executes it using system calls such as fork(), execvp(), and waitpid().

Features
1. Shell Prompt

The shell displays the current working directory followed by a > symbol as the command prompt.

Example:

/Users/username/projects/shell>

This is implemented using the getcwd() function.

2. Command Parsing

User input is read as a string and split into tokens using strtok().

The tokens are stored in an array of strings formatted for execvp():

args[0] = command
args[1] = argument
...
args[n] = NULL
3. Built-in Commands
exit

Terminates the shell program.

Example:

exit
cd

Changes the current working directory.

Example:

cd Documents

This uses the chdir() function. If the directory does not exist, the shell prints an error message.

4. Executing Programs

The shell can execute system programs such as:

ls
ls -l
ps
cat file.txt

Execution is done using:

fork() to create a child process

execvp() to run the program

waitpid() so the parent waits for the child to finish

If the command does not exist, the shell informs the user.

5. Input / Output Redirection

The shell supports redirection using:

>
<

Example:

ls -l > output.txt

Redirects program output to output.txt.

Example:

program < input.txt

Reads input from input.txt.

Redirection is implemented using the freopen() function to remap stdin or stdout.

6. Background Processes

Programs can run in the background using &.

Example:

sleep 10 &

The shell immediately returns to the prompt while the program continues running.

The shell handles terminated background processes using a SIGCHLD signal handler to prevent zombie processes.

Extra Credit Feature
Command History

The shell stores up to 100 previous commands.

history

Displays the list of previously executed commands.

Example:

history
!n

Executes a command from history.

Example:

!5

Runs the 5th command in the history list.

Compilation

Compile the program using gcc.

gcc shell.c -o shell
Running the Shell

Execute the shell with:

./shell

You will see the prompt showing the current directory and can begin entering commands.

Example Usage
/home/user/shell> ls
file1.txt file2.txt

/home/user/shell> cd Documents

/home/user/shell> ls -l > files.txt

/home/user/shell> sleep 5 &

/home/user/shell> exit
System Calls / Functions Used

getcwd()

strtok()

fork()

execvp()

waitpid()

chdir()

freopen()

signal()

Limitations

The shell does not support:

Job control (fg, bg)

Suspending processes (Ctrl+Z)

Interactive background programs

Listing background jobs (jobs)

These features were outside the scope of the assignment.
