
#include "signal.h"

void sigchild_handler(int sig)
{
	/* prevent zombie */
	pid_t child_pid;
	int status;

	while((child_pid = waitpid(-1,&status,WNOHANG)) < 0){
		/* nothing terminate child */
		break;
	}
	return;
}
