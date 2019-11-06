#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <unistd.h>

#define MAX_NAME 256
#define MAX_PATH 256

int main(){
	
	char cwd[MAX_PATH];
	getcwd(cwd, sizeof(cwd));
	
	char fullname[MAX_NAME]="Nguyen Tien Phat";
	FILE *fp;
	fp = fopen(strcat(cwd, "profile.txt"), "w+");
	fprintf(fp, "%s", fullname);
	fclose(fp); 
	
	return 0;
}
