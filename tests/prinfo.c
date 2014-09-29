#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "prinfo.h"
#include "list.h"

#define MAX_ITER 15

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

/*
 * If the previous process is your parent increase identation level.
 */
int add_depth(pid_t parent, pid_t pid, struct node **head)
{
	int rlist;

	if (parent == pid) {
		rlist = prepend(head, (pid_t) parent);
		return rlist;
	}
	return 2;
}

int main(int argc, char **argv)
{
	int i;
	int rlist;
	int rval;
	int nproc;
	int depth;
	pid_t parent_pid;
	struct prinfo *buf;
	struct node *head;

	if (argc != 1) {
		printf("Usage:%s\n", argv[0]);
		goto error;
	}
        
        /*
         * Run this loop as many times as necessary in order
         * to allocate a big enough buffer to cover info
         * for the entire process tree.
         */
        for (i = 0, nproc = 1; i < MAX_ITER; i++) {
        	buf = calloc(nproc, sizeof(struct prinfo));
        	if (buf == NULL) {
        		perror("calloc:");
        		goto error;
        	}
                printf("Allocated a buffer of size: %d\n", nproc);
        	rval = syscall(223, buf, &nproc);
        	if (rval < 0) {
        		perror("ptree:");
        		goto error_free_mem;
        	}
                printf("Total number of processes running: %d\n", rval);
                if (rval <= nproc)
                        break;
                printf("Reallocating a larger buffer\n\n");
                free(buf);
                nproc <<= 1;
        }
        /* If we get here it means that the size
         * of buf was large enough to keep info
         * about the whole process tree.
         */
        printf("\n\n");
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
		rlist = add_depth(buf[i].parent_pid, buf[i - 1].pid, &head);
		if (rlist == 1)
			goto error_list;
		if (rlist == 0) {
			depth++;
			parent_pid = buf[i].parent_pid;
			print_process(buf[i], depth);
			continue;
		}
		/*
		 * If none of the above applies, then you are
		 * a sibling of the previous process's parent.
		 */
		while (buf[i].parent_pid != get_data_from_start(&head)) {
			--depth;
			remove_from_start(&head);
		}
		parent_pid = buf[i].parent_pid;
		print_process(buf[i], depth);
	}
	return 0;

error_list:
	free_all_nodes(&head);
error_free_mem:
	free(buf);
error:
	return -1;
}

