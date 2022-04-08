#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

int 
getPrime(int *pipefd){
	close(pipefd[PIPE_WRITE]);
	int next_pipefd[2];
	pipe(next_pipefd);

	int filter;
	if(read(pipefd[PIPE_READ],&filter,sizeof(filter)) == sizeof(filter)) {
		printf("prime %d\n",filter);
		int pid = fork();
		if(0 == pid) {
			//children
			getPrime(next_pipefd);
			exit(0);
		} else if(0 < pid) {
			//parent
			close(next_pipefd[PIPE_READ]);
			int num;
			while(read(pipefd[PIPE_READ],&num,sizeof(num)) == sizeof(num)) {
				if(0 != num % filter) {
					write(next_pipefd[PIPE_WRITE],&num,sizeof(num));
				}
			}
			close(next_pipefd[PIPE_WRITE]);
			wait((int*)0);
		} else {
			fprintf(2,"fork error\n");
			exit(1);
		}
	}
	exit(0);
}

int
main(void) {
	int pipefd[2];
	pipe(pipefd);

	int pid = fork();

	if(0 == pid) {
		//children 
		getPrime(pipefd);
		exit(0);
	} else if(0 < pid) {
		//parent
		close(pipefd[PIPE_READ]);
		for(int i = 2; i <= 35; ++i) {
			write(pipefd[PIPE_WRITE],&i,sizeof(i));
		}
		close(pipefd[PIPE_WRITE]);
		wait((int*)0);

	} else {
		fprintf(2,"fork error\n");
		exit(1);
	}

	exit(0);
} 