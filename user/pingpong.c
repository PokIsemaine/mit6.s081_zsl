#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../user/user.h"

int
main(void) {
	char buf[15];
	
	int ctp[2];//children->parent
	int ptc[2];//paretn->children

	pipe(ctp);
	pipe(ptc);

	int pid = fork();
	if(pid ==0) {	//children 
	//children->parent
		close(ctp[0]);
		close(ptc[1]);
		read(ptc[0],buf,4);
		printf("%d: received %s\n",getpid(),buf);
		write(ctp[1],"pong",4);
		close(ctp[1]);//close ctp write,write finish
	} else if(pid > 0){	//parent
	//paretn->children
		close(ctp[1]);	
		close(ptc[0]);
		write(ptc[1],"ping",4);
		close(ptc[1]);//close ptc write,write finish
		read(ctp[0],buf,4);
		printf("%d: received %s\n",getpid(),buf);
	} else {	//error
		fprintf(2,"fork error");
		exit(1);
	}

	exit(0);
} 