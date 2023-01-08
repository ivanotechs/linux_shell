#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#define MAX_LINE 80

/* a function to find the command separator in the input*/
int find_charact(char input[MAX_LINE]){
	int i=0;
	for(i=0; i<MAX_LINE; i++){
		if(input[i]=='|'){
			return 1;
		}
		
		if(input[i]=='>'){
			return 2;
		}
		if(input[i]=='<'){
			return 3;
		}
		
	}
	return 0; // the separator is a space
}

void addHistory(char* input[MAX_LINE], FILE *f, FILE *fi, int *numHis){
	f=fopen("history.data", "wb");
	fi=fopen("numberHis.data", "wb");
	fwrite(input, MAX_LINE, 1, f);
	fwrite(numHis, sizeof(int), 1, fi);
	fclose(f);
	fclose(fi);
}

void getHistory(char* input[MAX_LINE], FILE *f, FILE *fi, int *numHis){
	f=fopen("history.data", "rb");
	fi=fopen("numberHis.data", "rb");
	fread(input, MAX_LINE, 1, f);
	fread(numHis, sizeof(int), 1, fi);
	fclose(f);
	fclose(fi);
}

/*The function that will parse the entered commands, evaluate its type and execute the command with execvp. */

void parse_execute(char* args[MAX_LINE]){
int shouldRun = 1;
char *history[MAX_LINE] = {};
time_t tm;      //time variable that wille give us the system time
time(&tm);
int numHis[1];
numHis[0] = 0;
int i = 0;
FILE *f, *fi;
char input[MAX_LINE];
char *his;
getHistory(history, f, fi, &numHis[0]);
while(shouldRun){
pid_t pid = fork(); //creating a new process to execute the parsing
if(pid == -1){
	perror("fork");
}

// child process where commands will be parsed an executed
if(pid == 0){
printf("osh>");
fgets(input, MAX_LINE, stdin); // collecting input from stdin
input[strcspn(input, "\n")]=0;
history[numHis[0]]=input;
numHis[0]++;

addHistory(history, f, fi, &numHis[0]);
getHistory(history, f, fi, &numHis[0]);

if(strcmp(input, "history")==0){
int c;
printf("COMMAND                                      TIME\n");
for(c=0; c<MAX_LINE; c++){
	printf("%s\n", history[c]);
	if(history[c]==NULL){
	break;
	}
}
return;
} 
//else{

char *token;   //will store the separate tokens of the users commands
if(find_charact(input)==0){ //the delimiter here is space
char delim[2]=" ";
token = strtok(input, delim);
	while(token!=NULL){
			args[i]=token;
			//printf("%s\n", args[i]);
		
		token = strtok(NULL, delim);
		i++;
	}
	if(execvp(args[0], args)==-1){
		perror("exec");//print any error if execvp fails
		}
	}
else if(find_charact(input)==1){ // enter if the parser see "|" in command
	char* arg2[MAX_LINE]={};  // stores the right command of the pipe
	char *token;
	token = strtok(input, "|"); //strtok will seperate the commands
	args[0] = token;
	token = strtok(NULL, " ");
	arg2[0]=token;

	int pipefd[2], status, done = 0;
	
	pid_t cpid;   // initialising process id
	
	pipe(pipefd);
	
	cpid = fork();		//creating a child process to execute the   first command
	if(cpid == 0){//left child for left command in the pipe
		dup2(pipefd[1], STDOUT_FILENO); //set output to pipe
		execvp(args[0], args);
	}
	cpid = fork();   // creating another process for the right cmd
	
	if(cpid == 0){
		close(pipefd[1]);//close unuse end of pipe
		dup2(pipefd[0], STDIN_FILENO);//set input from pipe
		if(execvp(arg2[0], arg2)==-1){
			perror("exec");
		}
		continue;
	}
	if(cpid >0){
		close(pipefd[0]);//close pipes
		close(pipefd[1]);
		waitpid(-1, &status, 0);//waite for childres to terminate
		waitpid(-1, &status, 0);
	}
	
}
else if(find_charact(input) == 2){// parser sees a > in the command
	char *token, *file, *nexttok;
	token = strtok(input, ">"); //seprating the command from the file
	
	file = strtok(NULL, ">"); // keeping the file name
	printf("%s\n", file);
	nexttok = strtok(token, " ");
	int i = 0;
	while(nexttok!=NULL){
		args[i]=nexttok;
		nexttok = strtok(NULL," ");
		i++;
	}
	int f= fileno(fopen(file, "wb"));//getting the filenumber of f
	dup2(f, STDOUT_FILENO);//set the output to file f
	execvp(args[0], args);
}
else if(find_charact(input) == 3){// parser sees a < in the comand
	char *token, *file, *nexttok;
	token = strtok(input, "<"); //seprating the command from the file
	file = strtok(NULL, "<"); // keeping the file name
	nexttok = strtok(token, " ");
	int i = 0;
	while(nexttok!=NULL){
		args[i]=nexttok;
		nexttok = strtok(NULL," ");
		i++;
	}
	int f= fileno(fopen(file, "rb"));
	dup2(f, STDIN_FILENO);
	execvp(args[0], args);
}
//}
}

	if(pid >0){
	if(wait(0)==-1){
		perror("wait");
	}
	continue;
}


	}
	}

int main(){
	pid_t pid = fork();
	if(pid == -1){
		perror("fork");
	}
	if(pid == 0){
		//child
		int i = 0;
		char* args[MAX_LINE];
		parse_execute(args);
	}
	if(pid > 0){
		//parent process
		if(wait(0)==-1){
			perror("wait");
		}
			int i = 0;
		char* args[MAX_LINE];
		parse_execute(args);
	
		//while(1);
	}

return 0;
}
