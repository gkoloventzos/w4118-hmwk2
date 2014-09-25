#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "prinfo.h"

void print_process(struct prinfo ptr, int depth);

int main(int argc, char **argv) {

	int number_of_tasks, out, depth, i, change_father;
	pid_t current_father;
	struct prinfo *buf;
	
	if (argc != 1){
		printf("Usage: prinfo_test <number of processes\n>");
		return 0;
	}
	buf = calloc(number_of_tasks, sizeof(struct prinfo));
	number_of_tasks = atoi(argv[1]);
	//out = prinfo(&buf,&number_of_tasks);
	out = syscall(223, &buf, &number_of_tasks);
	if (out < 0){
		perror("error: ");
		return 1;
	}
	depth = 0;
	change_father = 0;
	/*
	 *Your system call should return the total number of entries on
	 *success (this may be bigger than the actual number of entries copied)
	 * seg fault?
	 */
	current_father = buf[0].parent_pid;
	for(i = 0; i < out; i++) {
		if (change_father) {
			current_father = buf[i].parent_pid;
			change_father = 0;
		}
		if (buf[i].parent_pid != current_father) {
			depth++;
			current_father = buf[i].parent_pid;
		}
		print_process(buf[i],depth);
		if (buf[i].next_sibling_pid == 0) {
			depth--;
			change_father = 1;
		}
	}
	return 0;
}

void print_process(struct prinfo p, int depth){

	int i;
	for (i = 0; i < depth; i++)
		printf("\t");
	printf("%s,%d,%ld,%d,%d,%d,%ld\n", p.comm, p.pid, p.state,
	p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

}
