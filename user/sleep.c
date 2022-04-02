#include "kernel/types.h"	
#include "user/user.h" 		//atoi

int 
main(int argc, char *argv[]) {
	int n;

	//If the user forgets to pass an argument, sleep should print an error message. 
	if (argc < 2) {
		fprintf(2, "Usage: sleep ticks\n");
    	exit(1);
	}

	if((n = sleep(atoi(argv[0]))) < 0) {
		fprintf(2, "sleep returned with error %d", n);
		exit(1);
	}
	exit(0);
}