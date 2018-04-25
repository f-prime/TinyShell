#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 1024

int countOccurances(char *string, char item) {
    int found = 0;
    int i;
    for(i = 0; i < strlen(string); i++ ) {
        char on = string[i];
        if(on == item)
            found++;
    }
    return found;
}

void replace(char *string, char replace, char with) {
    int i = 0;
    while(string[i] != '\0') {
        if(string[i] == replace)
            string[i] = with;
        i++;
    }
}

char  ** split(char *string, char at) {
    int occurances = countOccurances(string, at); 
    int occuranceOn = 0;
    char **output = malloc(occurances * sizeof(char*)); 
    char atstring[2];
    atstring[0] = at;
    char *tok = strtok(string, atstring);
    while(tok != NULL) {
        output[occuranceOn] = tok;
        occuranceOn++;
        tok = strtok(NULL, atstring);
    }
    return output;
}

void execute(char **command, int size, int inbg, char *outFile) {
    char *cmds[size + 1];
    int x = 0;
    for(; x <= size; x++)
        cmds[x] = command[x];
    cmds[x] = NULL;
    pid_t pid = fork();
    if(pid == 0) {
        if(outFile) {
            int fd = open(outFile, O_CREAT | O_RDWR | O_TRUNC, 0666);
            dup2(fd, 1);
        }
        if(execvp(cmds[0], cmds)) {
            fprintf(stderr, "Invalid Command: %s\n", command[0]);
            exit(1);
        } else {
            exit(0);
        }
    } else {
        int status;
        if(inbg)
            waitpid(pid, &status, WNOHANG);
        else {
            waitpid(pid, &status, 0);
        }
    }
}

char *getOutFile(char *command) {
    int on = 0;
    for(; on < strlen(command); on++) {
        if (command[on] == '>')
            break;
    }
    on ++;
    char *fileName = malloc(sizeof(command) * sizeof(char));
    int i;
    for(i = 0; on < strlen(command); i++) {
        fileName[i] = command[on];
        on++;
    }
    if(fileName[0] == ' ') {
        char *out = malloc(sizeof(fileName) * sizeof(char));
        for(i = 1; i < strlen(fileName); i++)
            out[i - 1] = fileName[i];
        return out;
    }
    return fileName;
}   

void parseCommand(char *command) {
    int inbg = 0;
    char *outFile = NULL;
    replace(command, '\n', '\0');
    if(countOccurances(command, '&')) {
        replace(command, '&', '\0');
        inbg = 1;
    }
    
    if(countOccurances(command, '>')) {
        outFile = getOutFile(command);
        replace(command, '>', '\0');
    }

    int occurances = countOccurances(command, ' ');
    char **tokenized;
    int size = 0; // Number of elements in array - 1
    if(occurances > 0) {
        size = occurances;
        tokenized = split(command, ' ');
    } else if(strlen(command) > 0) {
        tokenized = malloc(2 * sizeof(char*));
        tokenized[0] = command;
    }
  
    if(strcmp(tokenized[0], "quit") == 0 || strcmp(tokenized[0], "quit ") == 0 || strcmp(tokenized[0], "quit\n") == 0) {
        while(1) // Ensures that it quits
            exit(0);
    } else if(strcmp(tokenized[0], "barrier") == 0 || strcmp(tokenized[0], "barrier ") == 0 || strcmp(tokenized[0], "barrier\n") == 0) {
        int status;
        pid_t wid;
        while((wid = wait(&status)) > 0);
    } else {
        execute(tokenized, size, inbg, outFile);
    }
}

int isEmpty(char *command) {
    int empty = 1;
    int i = 0;
    for(; i < strlen(command); i++) {
        if(command[i] != ' ') {
            empty = 0;
            break;
        }
    }

    return empty;
            
}

void startShell() {
    while(1) {
        char command[MAX_INPUT_SIZE];
        printf("prompt> ");
        fgets(command, MAX_INPUT_SIZE, stdin);
        replace(command, '\n', '\0');
        if(strlen(command) > 0 && !isEmpty(command)) {
            parseCommand(command);
        }
    }
}

void executeBatch(char *batch_file) {
    FILE *fp;
	char *line;
	size_t length = 0;
	ssize_t read_size;
 
	fp = fopen(batch_file, "r");
	if (fp == NULL) {
        fprintf(stderr, "Invalid File Name: %s\n", batch_file);
		exit(1);
    }

	while ((read_size = getline(&line, &length, fp)) != -1) {
        if(strlen(line) <= 0 || isEmpty(line) || strcmp(line, "\n") == 0 || strcmp(line, " ") == 0) {
            continue;
        }
		printf("%s", line);
        parseCommand(line);
	}
}   

int main(int argc, char *argv[]) {
    if (argc == 1) {
        startShell();
    } else if (argc == 2) {
        char *batch_file = argv[1];
        executeBatch(batch_file);
    } else {
        printf("Invalid number of arguments\n");
        exit(1);
    }
    return 0;
}
