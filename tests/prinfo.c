#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "prinfo.h"
#include "list.h"

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
	struct node **head;

	if (argc != 2) {
		printf("Usage:%s  <number of processes>\n", argv[0]);
		goto error;
	}

	nproc = atoi(argv[1]);
	if (nproc < 0) {
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
	/* Printing the init_task */
	print_process(buf[0], 0);
	parent_pid = -1;
	depth = 0;
	for (i = 1; i != nproc; i++) {
		/*
         * If you have the same parent with the previous
         * process keep the same identation depth.
         */
		if (parent_pid == buf[i].parent_pid) {
			print_process(buf[i], depth);
			continue;
		}
		/*
		 * If the previous process is your parent
         * increase identation level.
		 */
		if (buf[i].parent_pid == buf[i - 1].pid) {
			depth++;
			prepend(&head,(pid_t) buf[i].parent_pid);
			parent_pid = buf[i].parent_pid;
			print_process(buf[i], depth);
			continue;
		}
		/*
		 * If none of the above applies, then you are
         * a sibling of the previous process's parent.
		 */
		--depth;
		while (buf[i].parent_pid != get_data_from_start(&head)) {
			--depth;
			remove_from_start(&head);	
		}
		parent_pid = buf[i].parent_pid;
		print_process(buf[i], depth);
	}
	return 0;
error_free_mem:
	free(buf);
error:
	return -1;
}

