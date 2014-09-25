#include <stdio.h>
#include <sys/types.h>

struct prinfo {
	pid_t parent_pid;/* process id of parent */
	pid_t pid;/* process id */
	pid_t first_child_pid;  	/* pid of youngest child */
	pid_t next_sibling_pid;  	/* pid of older sibling */
	long state;/* current state of process */
	long uid;/* user id of process owner */
	char comm[64];	/* name of program executed */
};

int main(char *argsv, int argsc)
{
	struct prinfo p[4];
	int nr = 4;
	int i;

	syscall(223, &p, &nr);

	for (i=0;i<4;i++)
		printf("%s,%d,%ld,%d,%d,%d,%d\n", p[i].comm, p[i].pid, p[i].state,\
				p[i].parent_pid, p[i].first_child_pid, p[i].next_sibling_pid, p[i].uid);

}
