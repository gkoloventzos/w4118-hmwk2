/*
 * linux/kernel/ptree.c
 *
 * Copyright (C) 2014 V. Atlidakis, G. Koloventzos, A. Papancea
 *
 * COMS W4118 implementation of syscall ptree
 */
#include <linux/prinfo.h>
#include <linux/init_task.h>
#include <linux/slab.h>
#include <asm-generic/errno-base.h>

#define EXTRA_SLOTS 15
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Helper getting first sibling's list head */
static inline struct list_head *get_first_list_head(struct task_struct *tsk)
{
	return  &tsk->parent->children;
}

/* Helper getting real parent's pid - not SIGCHLD recipient*/
static inline pid_t get_ppid(struct task_struct *tsk)
{
	return task_pid_nr(tsk->real_parent);
}

/* Helper getting first child' pid */
static inline pid_t get_first_child_pid(struct task_struct *tsk)
{
	if (list_empty(&tsk->children))
		return (pid_t)0;

	tsk = list_first_entry(&tsk->children, struct task_struct, sibling);
	return task_pid_nr(tsk);
}

/* Helper getting first child' pid */
static inline
pid_t get_next_sibling_pid(struct task_struct *tsk)
{
	if (list_is_last(&tsk->sibling, &tsk->real_parent->children))
		return (pid_t)0;

	tsk = list_entry(tsk->sibling.next, struct task_struct, sibling);
	return task_pid_nr(tsk);
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
 * store_node: Helper to store necessary info about visited nodes.
 */
static void store_node(struct prinfo *cur, struct task_struct *tsk)
{
	cur->parent_pid = get_ppid(tsk);
	cur->pid = task_pid_nr(tsk);
	cur->first_child_pid = get_first_child_pid(tsk);
	cur->next_sibling_pid = get_next_sibling_pid(tsk);
	cur->state = tsk->state;
	cur->uid = (long) tsk->real_cred->uid;
	get_task_comm(cur->comm, tsk);
}

/*
 * dfs_try_add: Traverse task_struct tree in dfs fashion and store info
 *              about visited nodes in buf.
 * @kbuf: Buffer to store info about visited nodes.
 * @nr: Maximum number of nodes to store
 *
 * Returns the actuall number of nodes stored in kbuf or an error.
 *
 * WARNING: The caller must hold task_list lock.
 */
int dfs_add(struct prinfo *kbuf, int knr)
{
	int iterations;
	struct list_head *list;
	struct task_struct *cur;

	iterations = 0;
	cur = &init_task;
	while (iterations < knr) {
		store_node(kbuf + iterations, cur);
		iterations++;
		/* If you have children visit them first */
		if (!list_empty(&cur->children)) {
			cur = list_first_entry(&(cur->children),
					       struct task_struct,
					       sibling);
			continue;
		}
		/* If you do not have children visit your next sibling */
		list = get_first_list_head(cur);
		if (!list_is_last(&(cur->sibling), list)) {
			cur = list_first_entry(&(cur->sibling),
					       struct task_struct,
					       sibling);
			continue;
		}
		/*
		 * If you neither have children nor siblings iterate up to
		 * your father until you find siblings or init_task.
		 */
		cur = cur->real_parent;
		list = get_first_list_head(cur);
		while (list_is_last(&(cur->sibling), list) &&
		       cur != &init_task)
			cur = cur->real_parent;
		cur = list_first_entry(&(cur->sibling),
				       struct task_struct,
				       sibling);
		list = get_first_list_head(&init_task);
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
	nproc = dfs_add(kbuf, kslots);
	read_unlock(&tasklist_lock);

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
