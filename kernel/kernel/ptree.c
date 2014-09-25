/*
 *  linux/kernel/ptree.c
 *
 *  Copyright (C) 2014 V. Atlidakis, G. Koloventzos, A. Papancea
 */

#include <linux/prinfo.h>
#include <linux/init_task.h>
#include <linux/slab.h>


#include <asm-generic/errno-base.h>

//static void dfs(struct prinfo *, struct task_struct *, int *, int *);
static void full_prinfo(struct prinfo *, struct task_struct *);


asmlinkage int sys_ptree(struct prinfo *buf, int *nr)
{
	int errno;
	struct task_struct *ega_task, *cur;
	struct prinfo *kbuf;
        int iterations, store;

	if (buf == NULL || nr == NULL) {
		errno = -EINVAL;
		goto error;
	}
	if (*nr < 1) {
		errno = -EINVAL;
		goto error;
	}
        kbuf = kmalloc(*nr * sizeof(struct prinfo), GFP_KERNEL);
        if (!kbuf) {
                errno = -ENOMEM;
                goto error;
        }
        printk(KERN_ERR "Here I am: %d\n", *nr);
        read_lock(&tasklist_lock);
	ega_task = &init_task;
	iterations = 0;
	store = 1;
	while (iterations <= *nr) { 
		if (ega_task == &init_task && store == 0)
			break; /*More iterations than processes*/
		cur = ega_task;
		if (store) {
			full_prinfo(buf + iterations, cur);
			iterations++;
		}
		store = 1;
		if (list_empty(&cur->children)) {
			ega_task = cur->real_parent;
			store = 0;
			continue;
		} else {
			ega_task = list_entry(cur->children.next,\
				   struct task_struct, children);
			continue;
		}
		if (&cur->sibling.next == cur->real_parent) {
			ega_task = list_entry(cur->real_parent->sibling.next, \
					      struct task_struct, children);
			continue;
		}
	}
	read_unlock(&tasklist_lock);
	if (copy_to_user(buf, kbuf, *nr * sizeof(struct prinfo))) {
 		errno = -EFAULT;
		goto error;
	}
	errno = 0;
        kfree(kbuf);
error:
	return errno;
}


void full_prinfo(struct prinfo *cur, struct task_struct *tsk)
{

	cur->parent_pid = tsk->real_parent->pid;
	cur->pid = tsk->pid;
	cur->state = tsk->state;
	cur->uid = (long) tsk->cred->uid;
	cur->state = tsk->state;
	get_task_comm(cur->comm, tsk);
        /*                                                                      
         * cur->next_sibling_pid = tsk->p_ysptr; younger sibling                
         * cur->first_child_pid = tsk->p_cptr; youngest child                   
         * cur->uid = (long) tsk->cred->uid;                                    
         */ 
	return ;

}

/*void dfs(struct prinfo *buf, struct task_struct *tsk, int *iter, int *nr)
{
        struct task_struct *child;
        struct prinfo *cur;

        if (*iter >= *nr)
                return;*/
        /* Register info for current node */
//        cur = buf + *iter;
//        cur->parent_pid = tsk->real_parent->pid;
//        cur->pid = tsk->pid;
        /*
         * cur->next_sibling_pid = tsk->p_ysptr; younger sibling
         * cur->first_child_pid = tsk->p_cptr; youngest child
         * cur->uid = (long) tsk->cred->uid;
         */
//        cur->state = tsk->state;
//        printk(KERN_ERR "4-Here I am: %d\n", *iter);
//        get_task_comm(cur->comm, tsk);
//        printk(KERN_ERR "6-Here I am: %d\n", *iter);
        /* Continue in a dfs fashion */
        /*(*iter)++;
        list_for_each_entry(child, &tsk->children, children) {
                dfs(buf, child, iter, nr);
        }
        return;
}*/
