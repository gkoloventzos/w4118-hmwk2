/*
 *  linux/kernel/ptree.c
 *
 *  Copyright (C) 2014 V. Atlidakis, G. Koloventzos, A. Papancea
 */
#include <linux/prinfo.h>
#include <linux/init_task.h>
#include <linux/slab.h>
#include <asm-generic/errno-base.h>

#define EXTRA_SLOTS 15
#define MIN(a, b) ((a) < (b) ? (a) : (b))


static inline
struct list_head *get_head_list_children_depth(struct task_struct *tsk)
{
	return  &tsk->parent->children;
}

/*
 * store_node: Helper to store necessary info about visited nodes.
 */
static inline void store_node(struct prinfo *cur, struct task_struct *tsk)
{
	/* keep the real parent not SIGCHILD recipient*/
	cur->parent_pid = tsk->real_parent->pid;
	cur->pid = tsk->pid;
	cur->state = tsk->state;
	cur->uid = (long) tsk->cred->uid;
	cur->state = tsk->state;
	get_task_comm(cur->comm, tsk);
}

void print_task(struct task_struct *tsk)
{
	pr_err("%s, %d, %ld", tsk->comm, tsk->pid, tsk->state);
}

/*
 * get_num_of_processes: Helper to get the number of processes, i.e.,
 * thread group leaders in the task_struct list.
 */
static inline int get_num_of_processes(void)
{
	int count = 0;
	struct task_struct *cur;

	read_lock(&tasklist_lock);
	for_each_process(cur) {
		if (cur == cur->group_leader)
			++count;
	}
	read_unlock(&tasklist_lock);
	return count;
}

/*
 * dfs_try_add: Traverse task_struct tree in dfs fashion and maybe store info
 *              about visited nodes in buf.
 * @kbuf: Buffer to store info about visited nodes.
 * @nr: Maximum number of nodes to store
 *
 * Returns the actuall number of nodes stored in kbuf or an error.
 *
 * WARNING: The caller must hold task_list lock.
 */
int dfs_try_add(struct prinfo *kbuf, int knr)
{
	int iterations;
	struct list_head *list;
	struct task_struct *cur;

	iterations = 0;
	cur = &init_task;
	while (iterations < knr) {
		store_node(kbuf + iterations, cur);
		print_task(cur);
		iterations++;
		if (!list_empty(&cur->children)) {
			cur = list_first_entry(&(cur->children),
					       struct task_struct,
					       sibling);
			continue;
		}
		list = get_head_list_children_depth(cur);
		if (!list_is_last(&(cur->sibling), list)) {
			cur = list_first_entry(&(cur->sibling),
					       struct task_struct,
					       sibling);
			continue;
		}
		cur = cur->real_parent;
		list = get_head_list_children_depth(cur);
		while (list_is_last(&(cur->sibling), list) &&
		       cur != &init_task)
			cur = cur->real_parent;
		cur = list_first_entry(&(cur->sibling),
				       struct task_struct,
				       sibling);
		list = get_head_list_children_depth(&init_task);
		if (cur == &init_task)
			break;
	}
	return iterations;
}

int sys_ptree(struct prinfo __user *buf, int __user *nr)
{
	int knr;
	int nproc;
	int kslots;
	int errno;
	struct prinfo *kbuf;

	if (buf == NULL || nr == NULL) {
		errno = -EINVAL;
		goto error;
	}
	if (get_user(knr, nr) < 0) {
		errno = -EFAULT;
		goto error;
	}
	/*
	 * Check the total number of processes in the system and
	 * allocate min(nproc + EXTRA_SLOTS, nr). In that way if
	 * the user requests more processs than those currently
	 * in the system kmalloc will probably not fail.
	 */
	read_lock(&tasklist_lock);
	nproc = get_num_of_processes();
	read_unlock(&tasklist_lock);
	kslots = MIN(nproc + EXTRA_SLOTS, knr);
	kbuf = kmalloc_array(kslots, sizeof(struct prinfo), GFP_KERNEL);
	if (kbuf == NULL) {
		errno = -ENOMEM;
		goto error;
	}
	/*
	 * Traverse task_struct tree in a dfs fashion and store
	 * at most "kslots" processes.
	 */
	read_lock(&tasklist_lock);
	pr_err("dfs with %d slots\n", kslots);
	nproc = dfs_try_add(kbuf, kslots);
	read_unlock(&tasklist_lock);
	pr_err("FOUNd:%d nodes\n", nproc);

	if (copy_to_user(buf, kbuf, nproc * sizeof(struct prinfo)) < 0) {
		errno = -EFAULT;
		goto fail_free_mem;
	}
	if (put_user(nproc, nr) < 0) {
		errno = -EFAULT;
		goto fail_free_mem;
	}
	errno = nproc;
fail_free_mem:
	kfree(kbuf);
error:
	return errno;
}
