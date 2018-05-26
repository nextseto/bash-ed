# bash-ed

An alternative shell for UNIX-based systems.

The implementation for this project is located in main.c in this directory. A written breakdown of what it implements is detailed in this document below.

## Part 1

**bash-ed** takes in a string from the keyboard and executes the command on behalf of the user. In this program, the program first configures the console window into **non-canonical mode** which will continuously read in keystrokes from the keyboard. In the main function, it infinitely loops to retrieve the characters entered from the keyboard and then split into an array of strings. Once the command is split up into its individual components, the program checks if the command needs to be executed concurrently or sequentially.
 
- If the program detects an ampersand (`&`) within the input then the parent creates a child to process the command but does not wait for the child to complete. 

- When a program does not detect an ampersand, then the program will create a child and then wait for it to finish before another command can be executed

The program will exit when the user inputs `exit`.

## Part 2

**bash-ed** needs to store and retrieve up to **ten commands** that the user previously entered into the program. For every processed input in this program, the program will appropriately store the command. The user can enter the `history` command to see the ten most recent commands in **descending order from most recent to least recent**. 

- If there are less than ten commands, then the program will show up to the number of commands entered. 

- If the user entered more than ten commands, then the program will still show the ten most recent commands from the total number of entered commands. 

In addition, the history feature also provides shortcuts to let users quickly execute the most recent commands. 

- Whenever a user enters `!!`, the program will find the most recent command and execute that command. 

- Whenever a user enters `!<number>`, where n is an integer, then the n-th command from the history will be executed. 

While this feature is useful when using the shell, the program does have checks in place that prevents the user from trying to run commands that are either not there or out of bounds. When a user types in the shortcuts, the command that is executed is added to the history, not the shortcut command.

## Part 3

**bash-ed** needs to detect if the user presses the up or down arrow key and updates the console window with the command accordingly. In this program, with non-canonical mode turned on, the user can press either the up or down arrow key and cause the console window to show recent commands in the prompt. 

- When the user presses the up key, the least recent command in the history is shown on the screen. The user can continue to press the up arrow key until the console window shows the 10th most recent command. 

- When the user presses the down key, the most recent command in the history is shown on the screen until the user reaches the current input. 

- If the user tries to press up past the 10th command, the program will only show the 10th most recent command. 

- If the user tries to press down during the first prompt, the program will show remain and show the recently entered input. 

- If there are less than ten recent commands in history, the console window will only show the least recent command in history.

## Part 4

**bash-ed** provides the user with a range of additional commands that provide directory manipulation features. In this program, each custom function either masks existing bash commands or uses C library functions to perform each task. In addition, each function has its own way to handle errors and provides text to the user when the input is incorrect. The following commands were implemented:

- create : This will create a folder within the current directory.
  - Usage: create `<name of the directory>`
  
- cd : This will move the current working directory to a given path 
 - Usage: `cd <directory path>`

- rename/move : This will rename a directory from one name to another or move a directory from one path to another.
 - Usage 1: `rename <current directory name> <new directory name>`
 - Usage 2: `move <source directory> <destination directory>`

- copy : This will copy the contents of one directory to the contents of another directory.
 - Usage: `copy <source directory> <destination directory>`

- delete : This will delete a given directory and all of its contents.
  - Usage: `delete <name of the directory>`

## Part 5

**bash-ed** needs to be able to read in a given script and execute it if it is 'bashed' compatible. For a script to be compatible, the first line of the script should include the string `#! bashed` to let the program know that it can be executed with support with normal bash or additional commands that were implemented in part three of this project. 

In this program, it first detects if the user’s input is the path or name of a script and then tries to read its contents. The contents of the script are then split by new lines to get each string of commands. Then each command is executed until there are no more commands. Along with support for `#! bashed` compatibility, the program also supports bash scripts by checking if the script is for bash. If the script is not compatible with `#! bashed` then it gets passed and executed in the system’s default bash.

## License

All **source code** in this repository is released under the MIT license. See LICENSE for details.