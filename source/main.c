/** Warren Seto
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <errno.h>

/* Macros */
#define INPUT_BUFFER_SIZE 1024
#define HISTORY_BUFFER_SIZE 10

/* Custom Struct that imitates a String array */
typedef struct Array {
    int length;
    char** values;
} StringArray;


/* Custom Struct that buffers input from the user */
typedef struct Buffer {
    int position;
    char buffer[INPUT_BUFFER_SIZE];
} InputBuffer;


/* Prototypes for Project 1 functionality */
void coreProcess(char* input);
void executeCommand(char** input, int asyncFlag);
void runScript(char* filePath);
void addHistory(char* input);
void showHistory();


/* Prototypes for Custom Function Operations */
void oscCreate(StringArray* input);
void oscRename(StringArray* input);
void oscCopy(StringArray* input);
void oscDelete(StringArray* input);


/* Prototypes for Helper Functions */
StringArray stringSplit(char* inputString, const char* seperatorString);
char* stringCopy(const char* input);
char** _stringAppend2D(char* frontPointer, char* backPointer, char** appendingArray, int* currentSize, int* alertSize);
void freeStringArray(StringArray* input);
char* readFileContents(char *filename);


/* Variables */
char* history[HISTORY_BUFFER_SIZE]; // Holds 10 of the most recent commands
int historyCount = 0; // The total number of commands that the user entered
int scriptMode = 0; // Flag to indicate that custom #! bashed script is running


/* Main function that focuses on user input and passes the command to other functions */
int main(int argc, const char * argv[])
{
    /* 
        Before the user interacts with the terminal,
        the console window must be set to non-canonical mode
        which allows this program to capture and process every command/keystroke 
     */
    
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio);
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON;
    newtio.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &newtio);
    
    /* 
        Once non-canonical mode is turned on, the program will need to keep track of three things:
        1. What the user inputted into the terminal
        2. Where the user's last position was when inputting
        3. And, when the command on screen needs to be replaced with a string from history
     */
    
    char input;
    int historyIndex = -1;
    char* tempString;
    InputBuffer keyboard = { .position = 0 };
    
    printf("bashed>");
    
    while (1)
    {
        input = getchar();

        /* When an arrow key is pressed */
        if (input == 27)
        {
            input = getchar();
            
            switch (getchar())
            {
                case 'A': // Up Arrow Key
                    
                    /* Whenever the user first presses the up arrow key,
                     copy what the user entered into a temporary place for later */
                    if (historyIndex == -1)
                    {
                        keyboard.buffer[keyboard.position] = '\0';
                        tempString = stringCopy(keyboard.buffer);
                    }
                    
                    historyIndex++;
                    
                    /* Limits the user to the most recent command under 10 */
                    if (historyIndex >= historyCount)
                    {
                        historyIndex = historyCount - 1;
                    }
                    
                    /* Limits the user to the 10th most recent command */
                    else if (historyIndex > 9)
                    {
                        historyIndex = 9;
                    }
                    break;
                    
                case 'B': // Down Arrow Key
                    historyIndex--;
                    
                    /* Limits the user to the user's current command */
                    if (historyIndex < -1) {
                        historyIndex = -1;
                    }
                    break;

                default:
                    continue;
            }
            
            /* Remove what is in the buffer from the console window */
            size_t size = strlen(keyboard.buffer);
            while(size--) {
                printf("\b \b");
            }
            
            /* Replaces the string in the buffer with what the user typed in */
            if (historyIndex == -1)
            {
                strcpy(keyboard.buffer, tempString);
            }
            
            /* Replaces the string in the buffer with a previous command using the arrow key */
            else
            {
                strcpy(keyboard.buffer, history[historyIndex]);
            }

            keyboard.position = (int)strlen(keyboard.buffer);
            
            printf("%s", keyboard.buffer);
        }
        
        /* When the backspace key is pressed */
        else if (input == 0x7f)
        {
            if (!keyboard.position)
            {
                continue;
            }
            
            /* Delete the character from the console window */
            printf("\b \b");
            keyboard.buffer[--keyboard.position] = '\0';
        }
        
        /* When the user presses the ENTER key (simulates canonical mode) */
        else if (input == '\n')
        {
            keyboard.buffer[keyboard.position] = '\0';
            
            printf("\n");
            
            if (!strcmp(keyboard.buffer, "exit"))
            {
                break;
            }

            /* Takes the string that was inputted and executes it */
            coreProcess(keyboard.buffer);

            keyboard.position = 0;
            historyIndex = -1;
            
            fputs("bashed>", stdout);
            fflush(stdout);
        }
        
        /* When the user pressed any key that is not: an arrow, backspace, or enter */
        else
        {
            keyboard.buffer[keyboard.position++] = input;
            printf("%c", input);
        }
    }

    /* When the user enters 'exit', the program ends and this resets the console window back to the default canonical mode */
    tcsetattr(0, TCSANOW, &oldtio);
    
    return 0;
}


// MARK : Prototypes for Project 1 functionality

/*  */
void coreProcess(char* input)
{
    /* First, split the input command into an array of Strings */
    StringArray tempInput = stringSplit(input, " ");

    /* Check if the input is a file (script) */
    if (access(input, F_OK) != -1)
    {
        runScript(input);
    }
    
    /* Check if the input is empty */
    else if (!strcmp(input, ""))
    {
        freeStringArray(&tempInput);
        return;
    }
    
    /* Check if the input executes 'history' */
    else if (!strcmp(input, "history"))
    {
        freeStringArray(&tempInput);
        
        addHistory(input);
        showHistory();
        
        return;
    }
    
    /* Check if the input executes 'cd' */
    else if (!strcmp(tempInput.values[0], "cd"))
    {
        chdir(tempInput.values[1]);
    }
    
    /* Check if the input executes 'create' */
    else if (!strcmp(tempInput.values[0], "create"))
    {
        oscCreate(&tempInput);
    }
    
    /* Check if the input executes 'rename' or 'move' */
    else if (!strcmp(tempInput.values[0], "rename") || !strcmp(tempInput.values[0], "move"))
    {
        oscRename(&tempInput);
    }
    
    /* Check if the input executes 'copy' */
    else if (!strcmp(tempInput.values[0], "copy"))
    {
        oscCopy(&tempInput);
    }
    
    /* Check if the input executes 'delete' */
    else if (!strcmp(tempInput.values[0], "delete"))
    {
        oscDelete(&tempInput);
    }
    
    /* Check if the input aims to execute a previous command */
    else if (input[0] == '!')
    {
        freeStringArray(&tempInput);
        
        /* Check if the input executes the most recent command */
        if (!strcmp(input, "!!"))
        {
            if (!history[0])
            {
                printf("No commands in history.\n");
                return;
            }

            coreProcess(history[0]);
        }
        
        /* Check if the input executes the n-th recent command */
        else
        {
            int historyIndex = historyCount - atoi(input + 1);
            
            /* Checks if the number is within 0 <= historyIndex <= 9 and a command exists */
            if (historyIndex < 0 || historyIndex > 9 || !history[historyIndex])
            {
                printf("No such command in history.\n");
                return;
            }
        
            coreProcess(history[historyIndex]);
        }
        
        return;
    }
    
    /* Execute a command */
    else
    {
        // Check if the input command needs to be run concurrently with the parent process
        int asyncFlag = tempInput.values[tempInput.length - 1][0] == '&';
        
        if (asyncFlag)
        {
            tempInput.values[tempInput.length - 1] = NULL;
            tempInput.values = realloc(tempInput.values, sizeof(char*) * (--tempInput.length));
        }
        
        executeCommand(tempInput.values, asyncFlag);
    }
    
    /* Bookkeeping: Freeing memory allocated on the heap and adding the command to the history */
    freeStringArray(&tempInput);
    addHistory(input);
}


/* Executes a command using execvp(...) and takes into account whether or not the command needs to be concurrently or not */
void executeCommand(char** input, int asyncFlag)
{
    int processID = fork();
    
    if (asyncFlag) // There is an &, then run the child concurrently
    {
        if (!processID) // Parent does not wait for the child
        {
            execvp(input[0], input);
            
            printf("-bashed: %s\n", strerror(errno));
        }
    }
    
    else // Waits for the child to finish
    {
        if (processID) // Parent block which waits for the child
        {
            wait(NULL);
        }
        
        else // Child executes the command
        {
            execvp(input[0], input);
            
            printf("-bashed: %s\n", strerror(errno));
        }
    }
}


/* Executes a given script. Detects if a given script is: empty, #! bashed compatible, or bash compatible */
void runScript(char* filePath)
{
    StringArray tempFileData = stringSplit(readFileContents(filePath), "\n");

    /* Check if the file is empty */
    if (!strcmp("", tempFileData.values[0]))
    {
        printf("This script has no text.\n");
    }
    
    /* Check if the script is not #! bashed compatible */
    else if (strcmp("#! bashed", tempFileData.values[0]))
    {
        char* tempRegularScript[1] = { filePath };
        
        executeCommand(tempRegularScript, 0);
    }
    
    /* Script is #! bashed compatible */
    else
    {
        scriptMode = 1;
        addHistory(filePath);
        
        int count;
        for (count = 1; count < tempFileData.length; count++)
        {
            coreProcess(tempFileData.values[count]);
        }
        
        scriptMode = 0;
    }
    
    freeStringArray(&tempFileData);
}


/* Function that adds a user's input to the history and updates the history's count */
void addHistory(char* input)
{
    if (scriptMode)
    {
        return;
    }
    
    int size = HISTORY_BUFFER_SIZE - 1;
    while (size--)
    {
        history[size + 1] = history[size];
    }
    
    history[0] = stringCopy(input);
    
    historyCount++;
}


/* Function that outputs the 10 most recent commands */
void showHistory()
{
    int count;
    for (count = 0; count < 10; count++)
    {
        if (history[count])
        {
            printf("%i %s\n", (historyCount - count), history[count]);
        }
    }
}


// MARK : Prototypes for Custom Function Operations

/* Custom function to handle creating a folder in a given directory */
void oscCreate(StringArray* input)
{
    if (input->length < 2) {
        printf("create: Please enter the name of the folder that you want to create.\n");
        return;
    }
    
    input->values[0] = stringCopy("mkdir");
    
    if (fork()) // Parent
    {
        wait(NULL);
    }
    
    else // Child
    {
        execvp(input->values[0], input->values);
        
        exit(0);
    }
}


/* Custom function to handle renaming/moving a folder in a given directory */
void oscRename(StringArray* input)
{
    if (input->length < 3) {
        printf("rename/move: Please enter the name of the folder that you want to rename/move.\n");
        return;
    }
    
    else if (!strcmp(input->values[1], "")) {
        printf("rename/move: Please enter the name of the first parameter to rename/move.\n");
        return;
    }
    
    else if (!strcmp(input->values[2], "")) {
        printf("rename/move: Please enter the name of the second parameter to rename/move.\n");
        return;
    }
    
    input->values[0] = stringCopy("mv");
    
    if (fork()) // Parent
    {
        wait(NULL);
    }
    
    else // Child
    {
        execvp(input->values[0], input->values);
        
        exit(0);
    }
}


/* Custom function to handle copying the contents from one folder to the other */
void oscCopy(StringArray* input)
{
    if (input->length < 3) {
        printf("copy: Please enter the name of the source and destination directories that you want to copy.\n");
        return;
    }
    
    else if (!strcmp(input->values[1], "")) {
        printf("copy: Please enter the name of the source directory that you want to copy.\n");
        return;
    }
    
    else if (!strcmp(input->values[2], "")) {
        printf("copy: Please enter the name of the destination directory that you want to copy to.\n");
        return;
    }
    
    size_t sourceLength = strlen(input->values[1]) - 1;
    
    /* Fixes the command to be executes to copy the contents of a folder to another */
    if (input->values[1][sourceLength] == '/') {
        input->values[1] = realloc(input->values[1], sourceLength + 3);
        input->values[1][sourceLength + 1] = '.';
        input->values[1][sourceLength + 2] = '\0';
    }
    
    else if (!(input->values[1][sourceLength] == '.')) {
        input->values[1] = realloc(input->values[1], sourceLength + 4);
        input->values[1][sourceLength + 1] = '/';
        input->values[1][sourceLength + 2] = '.';
        input->values[1][sourceLength + 3] = '\0';
    }
    
    char* formattedCopyString[5] = { "cp", "-rf", input->values[1], input->values[2], NULL};
    
    if (fork()) // Parent
    {
        wait(NULL);
    }
    
    else // Child
    {
        execvp(formattedCopyString[0], formattedCopyString);
        
        exit(0);
    }
}


/* Custom function to handle deleting a given folder and its contents */
void oscDelete(StringArray* input)
{
    if (input->length < 2) {
        printf("delete: Please enter the name of the folder that you want to delete.\n");
        return;
    }
    
    char* formattedDeleteString[4] = { "rm", "-r", input->values[1], NULL};
    
    if (fork()) // Parent
    {
        wait(NULL);
    }
    
    else // Child
    {
        execvp(formattedDeleteString[0], formattedDeleteString);
        
        exit(0);
    }
}


// MARK : Prototypes for Helper Functions

/* Returns a structure with: the array of strings split at each point where the separator occurs in the given input String and the number of strings in a 2D array. */
StringArray stringSplit(char* inputString, const char* seperatorString)
{
    int stringCount = 0, bufferSize = 10;
    char** output = malloc(bufferSize * sizeof(char*));
    
    char* frontPointer = inputString;
    char* backPointer = inputString;
    
    const size_t seperatorSize = strlen(seperatorString);
    
    while (*frontPointer)
    {
        if (*frontPointer == *seperatorString)
        {
            if (!strncmp(frontPointer, seperatorString, seperatorSize))
            {
                output = _stringAppend2D(frontPointer, backPointer, output, &stringCount, &bufferSize);
                
                frontPointer += seperatorSize;
                backPointer = frontPointer;
                continue;
            }
        }
        
        ++frontPointer;
    }
    
    output = _stringAppend2D(frontPointer, backPointer, output, &stringCount, 0);
    
    return (StringArray){ .length = stringCount, .values = output };
}


/* Returns the character values in the string */
char* stringCopy(const char* input)
{
    const size_t stringSize = (strlen(input) + 1) * sizeof(char);
    char* buffer = malloc(stringSize);
    
    if (buffer == NULL) { return NULL; }
    
    return (char *)memcpy(buffer, input, stringSize);
}


/* Helper function that adds a string into a StringArray struct */
char** _stringAppend2D(char* frontPointer, char* backPointer, char** appendingArray, int* currentSize, int* alertSize)
{
    if (alertSize)
    {
        if (*alertSize - *currentSize < 3)
        {
            appendingArray = realloc(appendingArray, sizeof(char*) * (*currentSize + *alertSize));
            *alertSize += *alertSize;
        }
    }
    
    else
    {
        appendingArray = realloc(appendingArray, sizeof(char*) * (*currentSize + 2));
        
        appendingArray[*currentSize + 1] = NULL;
    }
    
    const size_t stringSize = frontPointer - backPointer;
    appendingArray[*currentSize] = malloc((stringSize + 1) * sizeof(char));
    
    memcpy(appendingArray[*currentSize], backPointer, stringSize * sizeof(char));
    appendingArray[(*currentSize)++][stringSize] = '\0';
    
    return appendingArray;
}


/* Frees the memory used by a given StringArray input. Upon successful free-ing, num = -1. */
void freeStringArray(StringArray* input)
{
    while ((input->length)--)
    {
        free(input->values[input->length]);
    }
    
    free(input->values);
}


/* Function that reads in the content of a file and outputs a string of the contents of the file */
char* readFileContents(char *filename)
{
    char *buffer = NULL;
    FILE *handler = fopen(filename, "r");
    
    if (handler)
    {
        fseek(handler, 0, SEEK_END);
        size_t string_size = ftell(handler);

        rewind(handler);
        
        buffer = (char*) malloc(sizeof(char) * (string_size + 1));
        
        size_t read_size = fread(buffer, sizeof(char), string_size, handler);
        
        buffer[string_size] = '\0';
        
        if (string_size != read_size)
        {
            free(buffer);
            buffer = NULL;
        }
        
        fclose(handler);
    }
    
    return buffer;
}
