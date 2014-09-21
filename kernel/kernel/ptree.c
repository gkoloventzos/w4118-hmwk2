/*
 *  linux/kernel/ptree.c
 *
 *  Copyright (C) 2014 Vaggelis Atlidakis, Georgios Koloventzos, Andrei Papancea
 */

#include <linux/prinfo.h>
#include <linux/init_task.h>
#include <asm-generic/errno-base.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

extern rwlock_t tasklist_lock;
void dfs(struct prinfo *, struct task_struct *, int, int *);

int ptree(struct prinfo *buf, int *nr)
{
	int errno;
	struct prinfo *kbuf;

	if (buf == NULL || nr == NULL) {
		errno = -EINVAL;
		goto error;
	}
	if (*nr < 1) {
		errno = -EINVAL;
		goto error;
	}
        kbuf = kmalloc( *nr * sizeof(struct prinfo), GFP_KERNEL);
        if (!kbuf) {
                errno = -ENOMEM;
                goto error;
        }
	read_lock(&tasklist_lock);
        dfs(kbuf, &init_task, 0, nr);
	read_unlock(&tasklist_lock);
	if (copy_to_user(buf, &kbuf, *nr * sizeof(struct prinfo))) {
 		errno = -EFAULT;
		goto error;
	}
	errno = 0;
        kfree(kbuf);
error:
	return errno;
}


void dfs(struct prinfo *buf, struct task_struct *tsk, int iter, int *nr)
{
        struct task_struct *child;
        struct prinfo *cur;

        if (iter == *nr)
                return;

        /* Register info for current node */
        cur = (struct prinfo *)(buf + iter);
        cur->parent_pid = tsk->real_parent->pid;
        cur->pid = tsk->pid;
//        cur->next_sibling_pid = tsk->
        cur->state = tsk->state;
        cur->uid = tsk->cred->uid;
        strcpy(cur->comm, tsk->comm);

        /* Continue in a dfs fashion */
        list_for_each_entry(child, &tsk->children, children) {
                dfs(buf, child, iter + 1, nr);
        }
        return;
}
