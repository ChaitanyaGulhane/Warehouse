#include<stdio.h>
#include<sys/stat.h>
#include<ftw.h>
#include<errno.h>
#include<string.h>

# define MAX_SIZE 1024

long size = 0;

int 
sum(const char *filepath, const struct stat *sb, int typeflag) {
	size += sb->st_size;
	return 0;
}


int 
main(void) {

	char dir_path[MAX_SIZE];
	
	if(getcwd(dir_path, MAX_SIZE) == '\0') {
		fprintf(stderr, "Error in getcwd() = %s", strerror(errno));
		return -1;
	}
	
	if(ftw(dir_path, &sum, 1) == -1) {
		fprintf(stderr, "Error in fwt() = %s", strerror(errno));
		return -1;
	}
		
	printf("Size of %s directory = %ld", dir_path, size);

	return 0;
}
