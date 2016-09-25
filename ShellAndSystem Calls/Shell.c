/*
 ============================================================================
 Name        : shell_monica.c
 Author      : Monica
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_SIZE 513

char*PATH[50];
int PATH_SIZE = 0;
char* LOG_FILE;
char* HISTORY_FILE;

char* append(char* str1, char* str2);    //append two strings
void batchMode(char* fileName); //reads file and execute each command
void myScan(char* ans[], char str[MAX_SIZE], int *size);  //parsing
int executeCommand(char*ans[], int size);  //check for comments / exit / environment a
int setEnv(char*ans[], int size); //set env like x=5;
void handleEnv(char *ans[], int size); // replace $tokens with it's value or empty string if it does not exist
int handleCd(char*ans[], int size); //change directory
void handleEcho(char*ans[], int size); //remove quotes
void getPath(); //get all possible paths for commands
void history(char* line); //saves in file
void print_history(); //prints history
void handler(); //handler for log file

void handler(){
     while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
     FILE *fp;
     fp=fopen(LOG_FILE,"a");
     fputs("Child process was terminated\n",fp);
     fclose(fp);
}
void print_history() {
	FILE *fp;
	fp = fopen(HISTORY_FILE, "r");
	char str[MAX_SIZE];
	if (fp == NULL) {
		printf("No previous History\n");
		return;
	}
	while (fgets(str, MAX_SIZE, fp) != NULL) {
		printf("%s", str);
	}
	fclose(fp);
}

void history(char* line) {
	FILE *fp;
	fp = fopen(HISTORY_FILE, "a");
	fprintf(fp, "%s\n", line);
	fclose(fp);
}
void getPath() {
	char* str;
	str = getenv("PATH");
	int j, i = 0;
	PATH[0] = malloc(MAX_SIZE);
	for (j = 0; str[j] != '\0'; j++) {
		if (str[j] == ':') {
		    /////////////////
		    PATH[PATH_SIZE][i++]='/';
		    ///////////////////
			PATH[PATH_SIZE][i] = '\0';
			PATH_SIZE++;
			PATH[PATH_SIZE] = malloc(MAX_SIZE);
			i = 0;

		} else {
			PATH[PATH_SIZE][i] = str[j];
			i++;
		}

	}
	PATH_SIZE++;

}


void newProcess(char*ans[], int size, int background) {   ///////////////
	char* temp;
	temp = malloc(MAX_SIZE);
	ans[size] = NULL;
	signal(SIGCHLD, handler);
	int pid = fork();
	int i = 0;
	if (pid == 0) {
		if(execv(ans[0], ans) == -1){
		while (ans[0][i] != '\0')
			temp[i] = ans[0][i++];
		temp[i++] = '\0';
		i = 0;
		while (i < PATH_SIZE) {
		    ans[0] = append(PATH[i] , temp);
			if (execv(ans[0], ans) != -1)
				break;
			i++;

		}
		if (i == PATH_SIZE) {
			perror("Error");
			// sleep(1);
		}
	}
		 _exit(0);

	} else {
		if (!background) {
	    	waitpid(pid, NULL, 0);

		}
	}


}
char* removeDoubleQuotes(char* str){
	char* newStr = malloc(MAX_SIZE);
	int j , i = 0;
	for (j = 0; str[j] != '\0'; j++) {
				if (str[j] != '"'){
					newStr[i] = str[j];
					i++;
		      }
	}
	newStr[i] = '\0';
	return newStr;
}

void handleEcho(char*ans[], int size) {
	int i, j;
	for (i = 1; i < size; i++) {
		if (strcmp(ans[i], "") != 0) {
			printf("%s " , removeDoubleQuotes(ans[i]));
		}
	}
	puts("");
}

int handleCd(char*ans[], int size) {
	int error = 0;
	if (size == 1 || strcmp(ans[1], "~") == 0) {
		error = chdir(getenv("HOME"));
	} else {
		error = chdir(ans[1]);
	}
	return error;

}

void executeProcess(char* ans[], int size) {
	int background = 0;
	if (strcmp(ans[size - 1], "&") == 0) {
		background = 1;
		size--;
	}else if(ans[size-1][strlen(ans[size-1])-1] == '&'){
		background = 1;
		ans[size-1][strlen(ans[size-1])-1] = '\0';
	}
	if (strcmp(ans[0], "cd") == 0) {
		int error = handleCd(ans, size);
		if(error == -1)
			perror("Error");

	} else if (strcmp(ans[0], "echo") == 0) {

		handleEcho(ans, size);

	} else {
		newProcess(ans, size, background);
	}

}
char* replaceEnv(char* str) {
	int j;

	for (j = 0; str[j] != '\0'; j++) {
		str[j] = str[j + 1];
	}
	if (getenv(str) != NULL) {
		str = getenv(str);
		if (str[0] == '$') {
			return removeDoubleQuotes(replaceEnv(str));
		}
		return removeDoubleQuotes(str);
	} else
		return "";
}

void handleEnv(char *ans[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (ans[i][0] == '$' && ans[i][1] != '\0') {
			ans[i] = replaceEnv(ans[i]);
		}

	}
}

int setEnv(char*ans[], int size) {   ///missing
	char* var[size];
	char* val[size];
	int i, j;
	for (i = 0; i < size; i++) {
		var[i] = malloc(MAX_SIZE);
		val[i] = malloc(MAX_SIZE);
		int flag = 0;
		int k = 0;
		for (j = 0; ans[i][j] != '\0'; j++) {
			if (ans[i][j] == '=' && !flag) {
				flag = 1;
				var[i][k] = '\0';
				k = 0;
			} else if (!flag) {
				var[i][k] = ans[i][j];
				k++;
			} else {
				val[i][k] = ans[i][j];
				k++;
			}
		}

		if (!flag)
			return i;

		val[i][k] = '\0';

	}
	for (i = 0; i < size; i++) {
		if (setenv(var[i], val[i], 1) == -1) {
			perror("Error:");
			//sleep(1);
		}
	}
	return -1;

}

int executeCommand(char*ans[], int size) {
	if (size == 0 || ans[0][0] == '#')
		return 1;
	if (strcmp(ans[0], "exit") == 0)
		return 0;
	if (strcmp(ans[0], "history") == 0) {
		print_history();
		return 1;
	}
	int x = setEnv(ans, size);
	if (x != -1) {
		handleEnv(ans, size);
		executeProcess(ans, size);
	}
	return 1;

}
void myScan(char* ans[], char str[MAX_SIZE], int *size) {
	int i = 0;
	int index = 0;
	ans[0] = malloc(MAX_SIZE);
	int j = 0;
	if(strlen(str) >= MAX_SIZE){
			printf("Too long command\n");
			*size = 0;
			return;
	}
	if(strcmp(str, "") == 0){
		*size = 0;
		return;
	}
	while (str[i] != '\0' && (str[i] == ' ' || str[i] == '\t'))
		i++;
	if (str[i] == '\0' || str[i] == '\n' ) {
		*size = 0;
		return;
	}
	while (str[i] != '\0') {
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '"') {
			ans[index][j] = str[i];
			j++;
			i++;
		}else if(str[i] == '"'){
			ans[index][j] = str[i];
			i++;
			j++;
			int f = 1;
			while(str[i] != '\0' && f){
					ans[index][j] = str[i];
					if(str[i] == '"')
						f=0;
			        j++;
					i++;
					
			}
		}else {
			while (str[i] == ' ' || str[i] == '\t')
				i++;
			if (str[i] != '\0') {
				ans[index][j] = '\0';
				index++;
				ans[index] = malloc(MAX_SIZE);
				j = 0;
			}
		}
	}
	*size = index + 1;
}
void batchMode(char* fileName) {
	FILE *fp;
	char str[MAX_SIZE];
	char *ans[MAX_SIZE];
	int size = 0;

	fp = fopen(fileName, "r");
	if (fp == NULL) {
		perror("Error opening file");
		//sleep(1);
		return;
	}
	while (fgets(str, MAX_SIZE, fp) != NULL) {
		printf("Command: ");
		if (str[0] == '\n') {
			puts("");
			continue;
		}
		size_t len = strlen(str) - 1;
		if (str[len] == '\n')
			str[len] = '\0';
		puts(str);
		myScan(ans, str, &size);
		history(str);
		if (!executeCommand(ans, size))
			break;
	}
	fclose(fp);

}

char* append(char* str1, char* str2) {
	char* res = malloc(strlen(str1) + strlen(str2) - 1);
	int indx, i = 0;
	for (indx = 0; str1[indx] != '\0'; indx++)
		res[i++] = str1[indx];
	for (indx = 0; str2[indx] != '\0'; indx++) {
		res[i++] = str2[indx];
	}
	res[i] = '\0';
	return res;
}


int main(int argc, char *argv[]) {
	char* home = getenv("HOME");
	LOG_FILE = append(home, "/monica_log");
	fclose(fopen(LOG_FILE, "w"));
	HISTORY_FILE = append(home, "/monica_history");
	char *ans[MAX_SIZE];
	char str[MAX_SIZE+500];
	int size = 0;
	getPath();
	if (argc >= 2) {
		batchMode(argv[1]);
		return 0;
	}
	while (1) {
		printf("Shell> ");
		if (gets(str) == NULL)
			break;
		myScan(ans, str, &size);
		if (size != 0) {
			history(str);
			if (!executeCommand(ans, size))
			 	break;
		
		}
	}
	return 0;
}
