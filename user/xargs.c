#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/param.h"
#include "../user/user.h"


int
main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(2,"xargs [command] [params...]\n");
		exit(1);
	} else if(argc + 1 > MAXARG) {
		fprintf(2,"Too many arguments\n");
		exit(1);
	}

	char * params[MAXARG],buf[512];

	for(int i = 1;i < argc; ++i) {
		params[i-1] = argv[i];
	}

	params[argc] = 0;

	while(1) {
		int i = 0;
		while(1) {
			int n = read(0,&buf[i],1);
			if(n == 0 || buf[i] == '\n') break;
			++i;
		}

		if(0 == i)break;
		buf[i] = 0;
		params[argc - 1] = buf;

		int pid = fork();
		if(0 == pid) {
			exec(params[0],params);
			exit(0);
		} else if(0 < pid) {
			wait((int *) 0);
		} else {
			fprintf(2,"fork error\n");
		}
	}
	exit(0);
}