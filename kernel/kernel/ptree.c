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
//static void full_prinfo(struct prinfo *, struct task_struct *);
//static void print_task(struct task_struct *);
static inline struct list_head *get_head_list_children_depth(\
						struct task_struct * tsk)
{
	return &(tsk->parent->children);
}



int sys_ptree(struct prinfo *buf, int *nr)
{
	int errno;
	struct task_struct *cur, *pinit;
	struct prinfo *kbuf;
        int iterations, store;
	struct list_head *list;

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
	cur = &init_task;
	pinit = list_first_entry(&(cur->children), struct task_struct, sibling);
	iterations = 0;
	store = 1;
	while (iterations < *nr) { 
/*		if (cur == pinit && iterations > 1){
			cur = list_first_entry(&(cur->sibling), struct task_struct,sibling);
			if (cur == NULL) 
				break;
		}*/
		
        	printk(KERN_ERR "%p %p %p %d\n", &init_task, cur, pinit, iterations);
		//print_task(ega_task);
		//cur = ega_task;
		//full_prinfo(kbuf + iterations, cur);
		iterations++;
		if (!cur)	
        		printk(KERN_ERR "MAOTHERFUCKER\n");
		if (!list_empty(&cur->children)) {
        		printk(KERN_ERR "not empty children list");
			cur = list_first_entry(&(cur->children), struct task_struct,\
						sibling);
       	 		printk(KERN_ERR "1: <%ld>\n", (long)cur->pid);
			continue;
		}
		list = get_head_list_children_depth(cur);
		if (!list_is_last(&(cur->sibling), list)) {
        		printk(KERN_ERR "not empty sibling list");
			cur = list_first_entry(&(cur->sibling), struct task_struct,\
						sibling);
       	 		printk(KERN_ERR "2: <%ld>\n", (long)cur->pid);
			continue;
		}
		printk(KERN_ERR "both lists empty look to your father"); 
		cur = cur->real_parent;
		list = get_head_list_children_depth(cur);
			printk(KERN_ERR "before loop: <%ld>\n", (long)cur->pid);
		while (list_is_last(&(cur->sibling), list) && cur != &init_task){
			cur = cur->real_parent;
			printk(KERN_ERR "loop: <%ld>\n", (long)cur->pid);
		}
		cur = list_first_entry(&(cur->sibling), struct task_struct, sibling);
       	 		printk(KERN_ERR "3: <%ld>\n", (long)cur->pid);
		list = get_head_list_children_depth(&init_task);
		if (cur == &init_task)
			break;
		
	}
	read_unlock(&tasklist_lock);
       	printk(KERN_ERR "DId all iterations\n");
	if (copy_to_user(buf, kbuf, *nr * sizeof(struct prinfo))) {
 		errno = -EFAULT;
		goto error;
	}
       	printk(KERN_ERR "copied to user\n");
	errno = 0;
        kfree(kbuf);
error:
	
       	printk(KERN_ERR "exiting\n");
	return errno;
}


void full_prinfo(struct prinfo *cur, struct task_struct *tsk)
{

	cur->parent_pid = tsk->real_parent->pid;
	cur->pid = tsk->pid;
	cur->state = tsk->state;
	cur->uid = (long) tsk->cred->uid;
	cur->state = tsk->state;
        /*                                                                      
         * cur->next_sibling_pid = tsk->p_ysptr; younger sibling                
         * cur->first_child_pid = tsk->p_cptr; youngest child                   
         * cur->uid = (long) tsk->cred->uid;                                    
         */ 
	return ;

}

void print_task(struct task_struct *tsk){

	printk(KERN_ERR "%s,%d,%ld", tsk->comm, tsk->pid, tsk->state);

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
