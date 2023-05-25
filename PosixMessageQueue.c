#include <sys/types.h>     
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <time.h>

// FOR OS INFORMATION
#include <sys/utsname.h>

// USED ONLY for FREE() to release memory allocated by malloc
#include <stdlib.h>

// USED ONLY for REMOVE() system call to discard of BUFFERFILE used for pipe read concatenation
#include <stdio.h> 

/* -- HELPER FUNCTIONS --------------- HELPER FUNCTIONS -------------- HELPER FUNCTIONS -- */

int getStringLength(char * arr){
    int len = 0;
    int i = 0;
    //a character string is "null terminated"
    //count until it is reached
    while(arr[i] != '\0'){
        len = len + 1;
        i = i + 1;
    }
    return len;
}

//for our purposes, only accepts char arrays
size_t getSize(char * arr){
    return sizeof(arr) / sizeof(arr[0]);
}

// is char a int?
int isInt(char * arr){
    for(int i = 0; i < getStringLength(arr); i++){
        //if current char is not a char digit
        if(arr[i] < 48 || arr[i] > 57)
            return 0;
    }
    return 1;
}

//decorator
void printCharArray(char * arr){
    write(STDOUT_FILENO, arr, getStringLength(arr));
}

int getConstArrayLength(const char * arr){
    int len = 0;
    int i = 0;
    //a character array is "null terminated"
    //count until it is reached
    while(arr[i] != '\0'){
        len = len + 1;
        i = i + 1;
    }
    return len;
}

void printConstCharArray(const char * arr){
    write(STDOUT_FILENO, arr, getConstArrayLength(arr));
}

// *REVISE* crude approach to int length to avoid errors from math.h
int getIntLength(int num){
    //return avoids execution of other ifs
    if (num >= 1000000000000) return 13;
    if (num >= 100000000000) return 12;
    if (num >= 10000000000) return 11;
    if (num >= 1000000000) return 10;
    if (num >= 100000000) return 9;
    if (num >= 10000000) return 8;
    if (num >= 1000000) return 7;
    if (num >= 100000) return 6;
    if (num >= 10000) return 5;
    if (num >= 1000) return 4;
    if (num >= 100) return 3;
    if (num >= 10) return 2;
    return 1;
}

//assumes only positive values
int getIntDigit(int num, int index){
    int digit_len = getIntLength(num);
    if(index > digit_len - 1) return -48;

    int denominator = 1;
    //get exponent used in N//10**i % 10
    for(int i = 0; i < index; i++){
        denominator = denominator * 10;
    }

    return (num / denominator) % 10;
}

// assumes single digit num
char digitToChar(int num){
    return (char) num + 48;
}

char * myToString(int num){
    int len = getIntLength(num);
    //allocate array of chars
    char * number = malloc((len+1) * (sizeof(char)));
    for(int i = 0; i < len; i++){
        number[i] = digitToChar(getIntDigit(num, len-i-1)); //numbers are in inverse order of strings
    }
    number[len] = '\0'; //null terminator for safety
    return number;
}

// string to int based on pure arithmetic digit retrieval n//10**i%10
int myStoi(char * arr){
    int i = 0;
    int num = 0;

    while(arr[i] != '\0'){

        //reverse indicies of numbers
        int numIndex = (getStringLength(arr) - 1) - i;
        int integer = arr[i] - 48; //return to digit
        int base = 1;

        int numTracker = numIndex;
        while(numTracker > 0){ //exponent based on position 
            base = base * 10; 
            numTracker = numTracker - 1; 
        }
        
        num = num + (base * (integer));
        i = i + 1;
    }

    return num;
}

// use $ for substitution
void printIntString(char * arr, int value){

    //get the length of the digit
    int digit_len = getIntLength(value);
    int digits_left = digit_len;

    int arr_len = getStringLength(arr);
    int len = arr_len + digit_len - 1;

    //create a new array with enough space for the digits
    char newCharArray[len];
    
    //copy until we hit the $ or end 
    int i = 0;
    while(arr[i] != '$' && i < len){
        newCharArray[i] = arr[i];
        i = i + 1;
    }

    //if we hit the $ start copying over
    if(arr[i] == '$'){
        while(digits_left > 0 && i < len){
            int digit = getIntDigit(value, digits_left - 1);
            newCharArray[i] = digitToChar(digit); //insert into array
            i = i + 1;
            digits_left = digits_left - 1; //move onto the next digit
        }
    }

    while(i < len){
        //i is incremented by digit_len, may get out of bounds
        //decrement arr to match up, add 1 to exclude $
        newCharArray[i] = arr[i-digit_len+1]; 
        i = i + 1;
    }

    //use system calls to print, NOT PRINTF
    write(STDOUT_FILENO, newCharArray, len);
    write(STDOUT_FILENO, "\n", 1);
}

void clearBuffer(char * buf){
    int len = getStringLength(buf);
    for(int i = 0; i < len; i++){
        buf[i] = 0;
    }
}

int count(char * buf, char ch){
    int len = getStringLength(buf);
    int count = 0;
    for(int i = 0; i < len; i++){
        if(buf[i] == (int) ch){
            count = count + 1;
        }
    }
    return count;
}

int isWhiteSpace(char ch){
    if(ch == ' ' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f'){
        return 1;
    }
    return 0;
}

//size should be entire size of file (number of characters)
int countWords(char * path, int size){
    char buf[size]; 
    int fd, n;

    //open file to read characters
    if((fd = open(path, O_RDONLY, 0)) == -1){
        printCharArray(path);
        printCharArray("\nCountsWords: Could not open file!\n");
        return -1;
    }

    //read file all at ONCE
    if((n = read(fd, buf, size)) <= 0){
        printCharArray(path);
        printCharArray("\nCountWords: Could not read anything from file!\n");
        return -1;
    }

    close(fd);

    int count = 0;
    int i = 0;

    //iterates through entire buffer
    //skipping whitespace and counting characters
    for(;i < n;){

        //traverse through all whitespace 
        while(i < n && isWhiteSpace(buf[i])){
            i++;
        }

        //have we traversed whitespace until the edge?
        if(i == n) break;

        // otherwise we've found a character
        // skip to next whitespace or end of arr
        while(i < n && !isWhiteSpace(buf[i])){
            i++;
        }
        
        if(!isWhiteSpace(buf[i-1])) count++;

    }
    return count;
}

int getMaxLine(char * path, int size){
    char buf[size]; 
    int fd, n;

    //open file to read characters
    if((fd = open(path, O_RDONLY, 0)) == -1){
        printCharArray("GetMaxLine: Could not open file!\n");
        return -1;
    }

    //read file all at ONCE
    if((n = read(fd, buf, size)) <= 0){
        printCharArray("GetMaxLine: Could not read anything from file!\n");
        return -1;
    }

    close(fd);

    int max = 0;
    int count = 0;
    int i = 0;

    //iterates through entire buffer
    for(;i < n;){

        while(i < n && buf[i] != '\n'){
            i++;
            count++;
        }

        //if we stopped at a newline character or space
        if(buf[i] == '\n'){
            if(count > max){
                max = count;
                count = 0; //reset
            }
            i++;
        }
    }

    //if we never stopped at a newline and 
    //finished reading the entire file
    if(max == 0) return count; 
    return max;
}

int countNewLines(char * path, int size){
    char buf[size]; 
    int fd, n;

    //open file to read characters
    if((fd = open(path, O_RDONLY, 0)) == -1){
        printCharArray("CountNewLines: Could not open file!\n");
        return -1;
    }

    //read file all at ONCE
    if((n = read(fd, buf, size)) <= 0){
        printCharArray("CountNewLines: Could not read anything from file!\n");
        return -1;
    }

    close(fd);

    int count = 0;

    //iterates through entire buffer
    for(int i = 0; i < n; i++){

        if(buf[i] == '\n') count++;

    }
    return count;
}

int contains(char * arr, char find){
    int len = getStringLength(arr);
    for(int i = 0; i < len; i++){
        if(arr[i] == find) return 1;
    }
    return 0;
}

//our options will ignore -llwwccLL
int isValidOptions(char * opt){
    int len = getStringLength(opt);

    if(len < 2 || len > 5) return 0;

    //ignore -
    for(int i = 1; i < len; i++){
        //if not a valid option
        if(opt[i] != 'l' && opt[i] != 'w' && opt[i] != 'c' && opt[i] != 'L'){
            return 0;
        }
        //is valid character, count if there are more
        else if(count(opt, opt[i]) > 1){
            return 0;
        }
    }
    return 1;
}

//pretty print used by child to format count string recieved from parent
void prettyPrint(char * file, char * countType, char * buf){
    printCharArray(file);
    printCharArray(":\t ");
    printCharArray(countType);
    printCharArray(":\t ");
    printCharArray(buf);
}

//needed to format incoming messages appropriately
const int getOptionIndex(char * buf, char find){
    int len = getStringLength(buf);
    //ignore -, start at 1
    for(int i = 1; i < len; i++){
        if(buf[i] == find){
            return i-1;
        }
    }
    return -1; //-1 fails if in loop
}

/* -- END OF HELPER FUNCTIONS --------------- END OF HELPER FUNCTIONS -------------- END OF HELPER FUNCTIONS -- */


int main(int argc, const char *argv[]) {

    /* CHECK ARGUMENTS AND CREATE FILE DESCRIPTORS */
    int BUFFER_SZ;
    const char * OPTIONS;

    int NUMFILES;
    int disregard = 2; //either 2 or 3 arguments to disregard when reading files
    const char * DEFAULTFILE = "prj1inp.txt";
    int useDefaultFile = 0;

    //file descriptor for our two processes
    int fd[2];

    /* If nothing specified in arguments */
    if(argc < 2) {
        printCharArray("You didn't enter enough arguments!\n");
        printCharArray("Program ending...try again.\n");
        return 1;
    }

    /* Only two arguments */
    else if(argc == 2){
        //is it the buffer size? (check if buffer)
        if(!isInt((char *) argv[1])){
            printCharArray("You didn't enter an integer. NEED BUFFER SIZE!\n");
            printCharArray("Program ending...try again.\n");
            return 1;
        }
        else {
            OPTIONS = "-lwc";
            BUFFER_SZ = myStoi((char *) argv[1]);
            if(BUFFER_SZ < 128 || BUFFER_SZ > 256){
                printCharArray("Buffer size should be 128-256! (inclusive)\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            NUMFILES = 1;
            useDefaultFile = 1;
        }
    }

    /* Three arguments */ 
    else if(argc == 3){
        char * second = (char*) argv[1];
        char * third = (char*) argv[2];
        
        //no options, buffer size & file
        if(isInt(second)){
            OPTIONS = "-lwc";
            BUFFER_SZ = myStoi((char *) argv[1]);
            if(BUFFER_SZ < 128 || BUFFER_SZ > 256){
                printCharArray("Buffer size should be 128-256! (inclusive)\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            NUMFILES = 1;
        }

        //options & buffer size
        else if(second[0] == '-' && isInt(third)){
            OPTIONS = second;
            if(!isValidOptions((char *) OPTIONS)){
                printCharArray("Wrong options! Expect -lwcL or some variation\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            BUFFER_SZ = myStoi((char *) argv[2]);
            if(BUFFER_SZ < 128 || BUFFER_SZ > 256){
                printCharArray("Buffer size should be 128-256! (inclusive)\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            NUMFILES = 1;
            useDefaultFile = 1;
        }

        //not a valid format
        else {
            printCharArray("Not valid format!\n");
            printCharArray("For 3 arguments, enter:\n");
            printCharArray("./file.exe OPTION BUFFER_SIZE\n");
            printCharArray("\nor\n\n");
            printCharArray("./file.exe BUFFER_SIZE FILENAME\n");
            printCharArray("Program ending...try again.\n");
            return 1;
        }

    }

    /* atleast 4 arguments */
    else {
        char * second = (char*) argv[1];
        char * third = (char*) argv[2];

        //has option
        if(second[0] == '-' && isInt(third)){
            OPTIONS = second;
            if(!isValidOptions((char *) OPTIONS)){
                printCharArray("Wrong options! Expect -lwcL or some variation\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            BUFFER_SZ = myStoi((char *) argv[2]);
            if(BUFFER_SZ < 128 || BUFFER_SZ > 256){
                printCharArray("Buffer size should be 128-256! (inclusive)\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
            disregard += 1; //additional argument to disregard
        }
        //doesn't have option
        else if(isInt(second)) {
            OPTIONS = "-lwc";
            BUFFER_SZ = myStoi((char *) argv[1]);
            if(BUFFER_SZ < 128 || BUFFER_SZ > 256){
                printCharArray("Buffer size should be 128-256! (inclusive)\n");
                printCharArray("Program ending...try again.\n");
                return 1;
            }
        }
        else {
            printCharArray("Not valid format!\n");
            printCharArray("For 4 or more arguments, enter:\n");
            printCharArray("./file.exe OPTION BUFFER_SIZE [FILES]\n");
            printCharArray("\nor\n\n");
            printCharArray("./file.exe BUFFER_SIZE [FILES]\n");
            printCharArray("Program ending...try again.\n");
            return 1;
        }

        //allocate array of char pointers for number of files recieved
        NUMFILES = argc - disregard;

    }

    /* INPUT INFORMATION */
    printCharArray("\nOPTIONS: ");
    printCharArray((char *) OPTIONS);
    printIntString("\nBUFFER SIZE: $", BUFFER_SZ);
    printIntString("NUMFILES: $", NUMFILES);
    printCharArray("USING FILE(S): ");

    if(useDefaultFile == 1){
        printCharArray((char *) DEFAULTFILE);
    }
    else {
        for(int i = disregard; i - disregard < NUMFILES; i++){
            printCharArray((char *) argv[i]);
            if(i - disregard + 1 < NUMFILES) printCharArray(", ");
        }
    }
    printCharArray("\n\n");


    /* PIPE = CHILD -> PARENT COMMUNICATION */
    /* SET UP PIPE BETWEEN CHILD AND PARENT PROCESS */
    if(pipe(fd) < 0){
        printCharArray("Pipe creation failed!\n");
        return 1;
    }


    /* MESSAGE QUEUE = PARENT -> CHILD COMMUNICATION */
    /* BEAR IN MIND, to compile on LINUX we must use -lrt at the end of our compile command */
    printCharArray("Parent: Before opening mq...\n");
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_curmsgs = 0, //queue starts with nothing
        .mq_maxmsg = 10, //CANNOT EXCEED SYS LIMIT found in sys/proc without root permission, our max is 10!
        .mq_msgsize = BUFFER_SZ
    };


    //initialize message queue
    //attributes is null, we will use default queue attributes
    //defining here gives both parent and child references to the message queue
    mqd_t msg_queue = mq_open("/ipc", O_CREAT | O_RDWR, 0644, &attr);
    if(msg_queue == (mqd_t) -1){
        printCharArray("Parent: Failed to open mq...\n");
        mq_unlink("/ipc");
        return 2;
    }
    printCharArray("Parent: Message queue has been opened...\n");

    // //list queue default attributes
    struct mq_attr def_attr;
    if(mq_getattr(msg_queue, &def_attr) == -1){
        printCharArray("Parent: Error retrieving queue attributes...\n");
        return 2;
    }

    //when using NULL (default) attributes for queue
    //we must turn blocking on, so our processes are synchronized
    //setattr can only change flags 
    // struct mq_attr new_attr = { //set attr will ignore other mv
    //     .mq_flags = 0 //BLOCKING
    // };

    // if(mq_setattr(msg_queue, &new_attr, &def_attr) == -1){
    //     printCharArray("Parent: Could not change message queue flag!\n");
    //     return 2;
    // };
    
    int blocking = (int) def_attr.mq_flags;
    int maxmsg = (int) def_attr.mq_maxmsg;
    int msgsize = (int) def_attr.mq_msgsize;
    int curmsgs = (int) def_attr.mq_curmsgs;

    printIntString("\nParent: Message queue flags: $", blocking);
    printIntString("Parent: Maximum # of messages on queue: $", maxmsg);
    printIntString("Parent: Maximum message size: $", msgsize);
    printIntString("Parent: # of messages currently on queue: $\n", curmsgs);
    

    /* PARENT & CHILD PROCESSES */
    pid_t pid = fork(); //create child process

    if(pid < 0){ //error
        printCharArray("Parent: Fork Failed...\n");
        return 2;
    }

/* CHILD PROCESS -------------------------------------------- CHILD PROCESS -------------------------- */
    else if(pid == 0){

        printCharArray("Child: Hello there!\n");

        printIntString("Child: Process ID is:\t$", (int) getpid());
        printIntString("Child: Parent Process ID is:\t$", (int) getppid());

        char cwd[8092];
        cwd[8091] = '\0';
        getcwd(cwd, 8091);
        printCharArray("Child: Current working directory:\t");
        printCharArray(cwd);
        printCharArray("\n");

        char hostname[8092];
        hostname[8091] = '\0';
        gethostname(hostname, 8091);
        printCharArray("Child: Hostname is:\t");
        printCharArray(hostname);
        printCharArray("\n\n");

        int file; //file descriptor
        char writeBuffer[BUFFER_SZ]; //buffer
        int n; //amount read

        //message queue recieve parameters
        char msgBuffer[8192];
        unsigned int priority;
        struct timespec timeout = {0, 0};

        //used for child to appropriately recieve messages from parent
        //formats option data accordingly
        const int newlineIndex = getOptionIndex((char *) OPTIONS, 'l');
        const int wordIndex = getOptionIndex((char *) OPTIONS, 'w');
        const int charIndex = getOptionIndex((char *) OPTIONS, 'c');
        const int maxLineIndex = getOptionIndex((char *) OPTIONS, 'L');

/* NO FILE ARGUMENTS GIVEN ---------------------------------------------------------------------------------------*/
        if(useDefaultFile == 1){
            //file failed
            if((file = open(DEFAULTFILE, O_RDONLY, 0444)) == -1){
                printCharArray("Failed to open default file!\n");
                return 2;
            }
            
            //close read end of pipe
            close(fd[0]); 

            //read file contents & leave room for eot in pipe
            while((n = read(file, writeBuffer, BUFFER_SZ - 1)) > 0){
                //write the read content (n bytes) to pipe
                //blocks until data has been read OFF from pipe
                write(fd[1], writeBuffer, n); 
            }

            //signal this is the end of file transmission
            char eot = (char) 4;
            write(fd[1], &eot, sizeof(char));

            // MESSAGE QUEUE - RECIEVER 
            ssize_t numRead;        
            /* important to correctly eval option length,
                will result in child blocking for an
                extra message it'll never recieve. */
            int num = getStringLength((char *) OPTIONS) - 1; // DISREGARD - 
            
            //at the end of EACH transmission recieve n messages, where n = amount of options
            // 1 file, meaning 1 message for each option
            for(int option = 0; option < num; option++){
                numRead = mq_receive(msg_queue, msgBuffer, def_attr.mq_msgsize, &priority);
                if(numRead == -1){
                    break; //stop trying to recieve
                }
                printCharArray("Child: Message *** ");

                // find the order of the option to format the integer string (count) appropriately
                if(option == newlineIndex)
                    prettyPrint((char *) DEFAULTFILE, "newline count", msgBuffer);
                else if(option == wordIndex)
                    prettyPrint((char *) DEFAULTFILE, "word count", msgBuffer);
                else if(option == charIndex) 
                    prettyPrint((char *) DEFAULTFILE, "char count", msgBuffer); 
                else if(option == maxLineIndex) 
                    prettyPrint((char *) DEFAULTFILE, "maximum line length", msgBuffer);

                printCharArray(" ***\n");
                //clear msgBuffer for next arriving msg
                clearBuffer(msgBuffer);
            }
            //release message queue and unlink name 
            if(mq_close(msg_queue) == -1){
                printCharArray("Child: Error closing message queue!\n");
                return 4;
            }
            // DONT close pipe, even though parent will read everything last written to buffer
            // we dont want to close pipe while parent is in execution just in case
            // The pipe will be discarded after the parent creating process has terminated
            // close(fd[1]); //close write end of pipe
            printCharArray("Child: Terminating.\n");
            return 0; //avoid execution
        }

/* FILE ARGUMENTS GIVEN ---------------------------------------------------------------------------------------*/
        close(fd[0]); //close read end
        //for each file send all contents from file into pipe, wait for messages
        for(int i = disregard; i - disregard < NUMFILES; i++){

            //current file failed
            if((file = open(argv[i], O_RDONLY, 0444)) == -1){
                printCharArray("Failed to open current file ");
                printCharArray((char *) argv[i]);
                printCharArray("!\n");
                return 2;
            }

            //read file contents, LEAVE ROOM FOR control character to signal end of file
            while((n = read(file, writeBuffer, BUFFER_SZ - 1)) > 0){
                //write the read content (n bytes) to pipe
                write(fd[1], writeBuffer, n); //blocks until data has been read OFF from pipe
            }

            //once done reading from current file, write control character to pipe
            //eot - end of transmission - 04
            char eot = (char) 4;
            write(fd[1], &eot, 1); //write a msg to pipe, rmr this small message will still BLOCK!
            //use so parent can distinguish each file

            //every time we send through the pipe and it is recieved
            // we wait for n messages, where n = len(OPTIONS)
            // numfiles * numoptions = nummessages

            //at the end of EACH transmission recieve n messages, where n = amount of options
            ssize_t numRead;
                        //assumes OPTIONS has been validated for duplicate letters
            
            /* important to correctly eval option length,
                will result in child blocking for an
                extra message it'll never recieve. */
            int num = getStringLength((char *) OPTIONS) - 1; // DISREGARD 
            
            for(int option = 0; option < num; option++){
                numRead = mq_receive(msg_queue, msgBuffer, def_attr.mq_msgsize, &priority);
                if(numRead == -1){
                    break; //stop trying to recieve
                }
                printCharArray("Child: Formatted Message *** ");

                if(option == newlineIndex)
                    prettyPrint((char *) argv[i], "newline count", msgBuffer);
                else if(option == wordIndex)
                    prettyPrint((char *) argv[i], "word count", msgBuffer);
                else if(option == charIndex) 
                    prettyPrint((char *) argv[i], "char count", msgBuffer); 
                else if(option == maxLineIndex) 
                    prettyPrint((char *) argv[i], "maximum line length", msgBuffer);

                printCharArray(" ***\n");
                //clear msgBuffer for next arriving msg
                clearBuffer(msgBuffer);
            }
            printCharArray("\n\n");

        }
        //close pipe when DONE sending everything to parent, pipes cannot be recreated
        close(fd[1]);
        //release message queue and unlink name 
        if(mq_close(msg_queue) == -1){
            printCharArray("Child: Error closing message queue!\n");
            return 4;
        }
        //no need to unlink, a duplicate unlink of the pipes name will gen an error
        printCharArray("Child: Terminating...\n");
        return 0;
    }

/* PARENT PROCESS -------------------------------------------- PARENT PROCESS -------------------------- */
    else { 
        
        /* OS INFORMATION */
        // struct of different strings with os information
        struct utsname uts; 

        // call uname to retrive os info and store in struct
        if(uname(&uts) < 0){
            printCharArray("Failed to retrieve O.S Information...\n");
            return 1;
        }
        printCharArray("Parent: OS name is:\t");
        printCharArray(uts.sysname); //os name
        printCharArray("\n");

        printCharArray("Parent: OS release is:\t");
        printCharArray(uts.release); //os release
        printCharArray("\n");

        printCharArray("Parent: OS version is:\t");
        printCharArray(uts.version); //os release
        printCharArray("\n\n");

        printIntString("Parent: Process ID is:\t$", (int) getpid());
        printIntString("Parent: Parent Process ID is:\t$", (int) getppid());

        char cwd[8092];
        cwd[8091] = '\0';
        getcwd(cwd, 8091);
        printCharArray("Parent: Current working directory:\t");
        printCharArray(cwd);
        printCharArray("\n");

        char hostname[8092];
        hostname[8091] = '\0';
        gethostname(hostname, 8091);
        printCharArray("Parent: Hostname is:\t");
        printCharArray(hostname);
        printCharArray("\n\n");

        char readBuffer[BUFFER_SZ]; //buffer
        int n; //amount read

        //used for counting aggregate pipe data
        int bufferFile;
        int fileSize = 0;

        //timeout for blocking termination
        struct timespec timeout = {0, 0};

        //attempt to create a file for simplistic word and line counts
        //basically used for read concatenation
        //cleared after each file read from pipe
        if((bufferFile = creat("bufferFile.txt", 0666)) == -1){
            printCharArray("\nParent: Could not create file tracking word count!\n");
            return 3;
        }

        close(fd[1]); //close write end of pipe
        //can return 0 once write end has been closed, even if  by other process (see screenshot 1031 2am)
        
        //keep reading from pipe until it is closed
        while((n = read(fd[0], readBuffer, BUFFER_SZ)) > 0){ //blocks until data is available in the pipe

            //debug information
            //printIntString("Parent: (read $ bytes from pipe)", n);
            //printIntString("Last char code read: $\n", (int) readBuffer[n-1]);
            //printIntString("File size: $\n", fileSize);

            //normal content
            if(readBuffer[n-1] != (char) 4){
                //push buffer contents into file
                if(write(bufferFile, readBuffer, n) != n){
                    printCharArray("Parent: Could not write to file from read buffer!\n");
                    close(bufferFile);
                    return 3;
                }
                //file must be able to contain each read from the pipe
                //used utility functions to read file all at once
                fileSize += n; 
            }

            // //is the last item read the eot character?
            else if(readBuffer[n-1] == (char) 4){
                    
                //write everything to bufferfile except last character
                if(write(bufferFile, readBuffer, n-1) != n-1){
                    printCharArray("Parent: Could not write to file from read buffer!\n");
                    close(bufferFile);
                    return 3;
                }
                fileSize += n-1; 
            
                int newlines = countNewLines("bufferFile.txt", fileSize);
                //send to child via msg queue
                //priority 1, use timeout value

                //can be sent in any order so make sure you send the CURRENT ONE IN ORDER!
                //so it aligns with index retrieved from start (via argv)
                for(int i = 1; i < getStringLength((char *) OPTIONS); i++){

                    if(OPTIONS[i] == 'l'){
                        char * lines = myToString(newlines);
                        if(mq_send(msg_queue, lines, getStringLength(lines), 1) == -1){
                            printCharArray("Parent: Error sending message on queue!\n");
                            return 3;
                        } 
                        //free memory provided by myToString, decrease heap size
                        free(lines);
                    }
                    else if(OPTIONS[i] == 'w'){
                        int words = countWords("bufferFile.txt", fileSize);
                        char * words_str = myToString(words);
                        if(mq_send(msg_queue, words_str, getStringLength(words_str), 1) == -1){
                            printCharArray("Parent: Error send sending message on queue!\n");
                            return 3;
                        }  
                        free(words_str);
                    }
                    else if(OPTIONS[i] == 'c'){
                        int characters = fileSize - newlines;
                        char * characters_str = myToString(characters);
                        if(mq_send(msg_queue, characters_str, getStringLength(characters_str), 1) == -1){
                            printCharArray("Parent: Error send sending message on queue!\n");
                            return 3;
                        }  
                        free(characters_str);
                    }
                    else if(OPTIONS[i] == 'L'){
                        int maxline = getMaxLine("bufferFile.txt", fileSize);
                        char * maxline_str = myToString(maxline);
                        if(mq_send(msg_queue, maxline_str, getStringLength(maxline_str), 1) == -1){
                            printCharArray("Parent: Error send sending message on queue!\n");
                            return 3;
                        } 
                        free(maxline_str);
                    }

                }

                //clear bufferFile
                // once finished reading from pipe, 
                // close buffer file and delete it via system call
                close(bufferFile);
                remove("bufferFile.txt");
                fileSize = 0;
                //recreate file
                if((bufferFile = creat("bufferFile.txt", 0666)) == -1){
                    printCharArray("\nParent: Could not create file tracking word count!\n");
                    return 3;
                }
            }
            
            // //buffer may still have data from previous read
            clearBuffer(readBuffer); 
        }

        //close(fd[0]); //close read end of pipe

        //release message queue and unlink name 
        if(mq_close(msg_queue) == -1){
            printCharArray("Parent: Error closing message queue!\n");
            return 3;
        }
        if(mq_unlink("/ipc") == -1){
            printCharArray("Parent: Error unlinking message queue!\n");
            return 3;
        }

        //once finished reading from pipe, 
        //close buffer file and delete it via system call
        close(bufferFile);
        remove("bufferFile.txt");

        //wait(NULL); //parent waits for child process to end
        wait(NULL); //parent waits for child process to end
        printCharArray("Parent: Terminating...\n");
        return 0;
    }

}
