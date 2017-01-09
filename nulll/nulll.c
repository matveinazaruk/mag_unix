#include <linux/init.h>		/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>		/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>	/* mutexes */
#include <linux/string.h>	/* memchr() function */
#include <linux/slab.h>		/* kzalloc() function */
#include <linux/sched.h>	/* wait queues */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matvei Nazaruk <matvei.nazaruk@gmail.com>");
MODULE_DESCRIPTION("/dev/nulll");

static unsigned long capacity = 0;
static unsigned long rest;
static struct mutex lock;
module_param(capacity, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(capacity, "device capacity");



static ssize_t nulll_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	ssize_t result = 0;
	unsigned long used = capacity - rest;

	if (cmd == BLKGETSIZE64) {
		if (copy_to_user((void *) arg, &used, sizeof(unsigned long)) != 0) {
			result = -EFAULT;
		}
	} else {
		result = -ENOIOCTLCMD;
	}

	return result;
}

static ssize_t nulll_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off) {
	ssize_t result = 0;

	if (mutex_lock_interruptible(&lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	if (capacity && size > rest) {
		result = -ENOSPC;
		rest = 0;
		goto out_unlock;
	}

	rest -= size;
	result = size;

out_unlock:
	mutex_unlock(&lock);
out:
	return result;
}

static struct file_operations nulll_fops = {
	.owner = THIS_MODULE,
	.write = nulll_write,
	.unlocked_ioctl= nulll_ioctl
};

static struct miscdevice nulll_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "nulll",
	.fops = &nulll_fops
};

static int __init nulll_init(void)
{
	int errno = 0;

	rest = capacity;
	errno = misc_register(&nulll_misc_device);
	if (errno < 0) {
		printk(KERN_INFO
		       "nazaruk nulll device has been registered with error");
	} else {
		printk(KERN_INFO
		       "nazaruk nulll device has been registered, capacity is %lu bytes\n",
		       capacity);
	}

	mutex_init(&lock);


	return errno;
}

static void __exit nulll_exit(void)
{
	mutex_destroy(&lock);
	misc_deregister(&nulll_misc_device);
	printk(KERN_INFO "nazaruk nulll device has been unregistered\n");
}

module_init(nulll_init);
module_exit(nulll_exit);
