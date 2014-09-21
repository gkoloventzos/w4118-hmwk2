#include <stdio.h>
#include "prinfo.h"

int main(char *argsv, int argsc)
{
	struct prinfo p;
	int nr = 1;

	syscall(223, &p, &nr);

	printf("%s,%d,%ld,%d,%d,%d,%d\n", p.comm, p.pid, p.state,
				p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
}
