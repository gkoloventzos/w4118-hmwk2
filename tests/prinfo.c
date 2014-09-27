#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "prinfo.h"

/*
 * Helper function to print process info add necessary tabs.
 */
void print_process(struct prinfo p, int depth)
{
	int i;

	for (i = 0; i < depth; i++)
		printf("\t");
	printf("%s, %ld, %ld, %ld, %ld, %ld, %ld\n", p.comm, (long) p.pid,
	       p.state, (long) p.parent_pid, (long) p.first_child_pid,
	       (long) p.next_sibling_pid, p.uid);

}

int main(int argc, char **argv)
{
	int i;
	int rval;
	int nproc;
	int depth;
	pid_t parent_pid;
	struct prinfo *buf;

	if (argc != 2) {
		printf("Usage:%s  <number of processes>\n", argv[0]);
		goto error;
	}
	nproc = atoi(argv[1]);
	if (argv[1] < 0) {
		printf("Number of processes cannot be negative\n");
		goto error;
	}
	buf = calloc(nproc, sizeof(struct prinfo));
	if (buf == NULL) {
		perror("calloc:");
		goto error;
	}
	rval = syscall(223, buf, &nproc);
	if (rval < 0) {
		perror("ptree:");
		goto error_free_mem;
	}
	/*
	 * Your system call should return the total number of entries on
	 * success (this may be bigger than the actual number of entries copied)
	 * seg fault?
	 */
	print_process(buf[0], 0);
	parent_pid = -1;
	depth = 0;
	/* Printing the init_task */
	for (i = 1; i != nproc; i++) {
		/* If you have the same parent with previous process in array*/
		if (parent_pid == buf[i].parent_pid) {
			print_process(buf[i], depth);
			continue;
		}
		/*
		 * If your parent id is the same with the pid of the previous
		 * process this means that you are child of this process as
		 * this is a dfs structured array.
		 */
		if (buf[i].parent_pid == buf[i - 1].pid) {
			depth++;
			parent_pid = buf[i].parent_pid;
			print_process(buf[i], depth);
			continue;
		}
		/*
		 * If your parent id is not the same with the previous then it
		 * means that you are a sibling of the previous process
		 */
		--depth;
		parent_pid = buf[i].parent_pid;
		print_process(buf[i], depth);
	}
	return 0;
error_free_mem:
	free(buf);
error:
	return -1;
}

