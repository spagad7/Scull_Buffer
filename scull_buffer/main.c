#include <linux/configfs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	// printk(), min()
#include <linux/slab.h>		// kmalloc()
#include <linux/fs.h>		// everything...
#include <linux/proc_fs.h>
#include <linux/errno.h>	// error codes
#include <linux/types.h>	// size_t
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include "scull.h"


#define init_MUTEX(_m) sema_init(_m, 1);

// parameters
int scull_major =   SCULL_MAJOR;
int scull_minor =   0;
int scull_nr_devs = 1;
int scull_nr_items = SCULL_NR_ITEMS;
int scull_sz_item = SCULL_SZ_ITEM;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_items, int, S_IRUGO);
module_param(scull_sz_item, int, S_IRUGO);

MODULE_AUTHOR("Shishir Pagad");
MODULE_LICENSE("Dual BSD/GPL");

dev_t dev_num;
static struct scull_buffer *scull_device;

static int spacefree(struct scull_buffer *dev);


/*
 * scull_open(): called by producer/consumer to gain access to the buffer
 */
int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;
    int size = scull_nr_items * scull_sz_item;

    // get pointer to scull_buffer from inode
	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);
	filp->private_data = dev;

    // allocate buffer if it does not exist
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (!dev->buffer)
    {
		dev->buffer = kmalloc(size, GFP_KERNEL);
		if (!dev->buffer)
        {
			up(&dev->sem);
			return -ENOMEM;
		}
        dev->rp = dev->wp = dev->buffer;
        dev->items_available = 0;
	}

    // set read & write pointers
	dev->buffersize = size;
	dev->end = dev->buffer + dev->buffersize;


	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	up(&dev->sem);

	return nonseekable_open(inode, filp);
}


/*
 * scull_release(): called by producer/consumer to detach from the buffer
 */
int scull_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = filp->private_data;

    // manage number of reader and writers
	down(&dev->sem);
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
    // no reader or writer, clear buffer
	if (dev->nreaders + dev->nwriters == 0) {
		kfree(dev->buffer);
		dev->buffer = NULL; /* the other fields are not checked on open */
	}
	up(&dev->sem);
	return 0;
}


/*
 * scull_read(): used for consumer to read data from the scull_buffer
 */
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

    // nothing to read
	while (dev->items_available == 0) {
        // release lock
        up(&dev->sem);

        // handle O_NONBLOCK
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		PDEBUG("Consumer: \"%s\": nothing to read, going to sleep\n", current->comm);
        // wait till there is some data available
		//if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
        if (wait_event_interruptible(dev->inq, (dev->items_available > 0)))
            // sleep interrupted by some signal, signal restart
			return -ERESTARTSYS;
		// reacquire the lock and reiterate the loop to ensure data is present
		if (down_interruptible(&dev->sem))
            // unable to get lock on semaphore
			return -ERESTARTSYS;
	}

	// data present; copy data from the scull_buffer to user buffer
	if (copy_to_user(buf, dev->rp, scull_sz_item))
    {
		up (&dev->sem);
		return -EFAULT;
	}
    // update read pointer and items_available
	dev->rp += scull_sz_item;
	if (dev->rp == dev->end)
        // wraped around circular buffer
		dev->rp = dev->buffer;
    dev->items_available--;
	up (&dev->sem);

	// wake up any waiting writers
	wake_up_interruptible(&dev->outq);
	PDEBUG("Consumer: \"%s\" read \"%s\" of size: %d bytes from the scull buffer\n",
            current->comm, buf, scull_sz_item);

    // return size of data read
    return (size_t)strlen(buf);
}


/*
 * spacefree(): function to check amount of free space left in the buffer
 * in terms of items
 */
static int spacefree(struct scull_buffer *dev)
{
	return (scull_nr_items - dev->items_available);
}


/*
 * Wait for space for writing; caller must hold device semaphore.
 * On error the semaphore will be released before returning.
 */
static int scull_getwritespace(struct scull_buffer *dev, struct file *filp)
{
    // buffer full
	while (spacefree(dev) == 0)
    {
        // DEFINE_WAIT includes declaration and init of wait_queue
		DEFINE_WAIT(wait);

        // release sempahore before going to sleep
		up(&dev->sem);

        // don't wait if non-blocking or nreaders = 0
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
        if(dev->nreaders == 0)
        {
            PDEBUG("Producer: \"%s\" buffer full, no readers\n", current->comm);
            return BUFFER_FULL_NO_READER;
        }
        else
            PDEBUG("Producer: \"%s\" going to sleep\n", current->comm);

        // TASK_INTERRUPTIBLE sets process state to sleeping
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
        // if there is no free space, yield the CPU
        if (spacefree(dev) == 0)
			schedule();
        // cleanup after wait is finished
		finish_wait(&dev->outq, &wait);

        // after wake up, check if the condition for wait is true
        if(dev->nreaders == 0)
            return BUFFER_FULL_NO_READER;

		if (signal_pending(current))
            // signal fs layer to handle it
			return -ERESTARTSYS;
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	return 0;
}



ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_buffer *dev = filp->private_data;
    int result;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    // Make sure there's space to write
    result = scull_getwritespace(dev, filp);
    if(result)
    {
        if (result == BUFFER_FULL_NO_READER)
            return -1;
        else
    		return result;
    }

    // space available; copy data from user
    if (copy_from_user(dev->wp, buf, scull_sz_item))
    {
        up (&dev->sem);
        return -EFAULT;
    }
    // update write pointer
    dev->wp += scull_sz_item;
    if (dev->wp == dev->end)
        dev->wp = dev->buffer;
	dev->items_available++;
    up(&dev->sem);

	// wake up any reader
	wake_up_interruptible(&dev->inq);

    PDEBUG("Producer: \"%s\" wrote \"%s\" of size: %d bytes into the scull buffer\n",
            current->comm, buf, scull_sz_item);
	return scull_sz_item;
}


unsigned int scull_poll(struct file *filp, poll_table *wait)
{
	struct scull_buffer *dev = filp->private_data;
	unsigned int mask = 0;

	/*
	 * The buffer is circular; it is considered full
	 * if "wp" is right behind "rp" and empty if the
	 * two are equal.
	 */
	down(&dev->sem);
	poll_wait(filp, &dev->inq,  wait);
	poll_wait(filp, &dev->outq, wait);
	if (dev->rp != dev->wp)
		mask |= POLLIN | POLLRDNORM;	/* readable */
	if (spacefree(dev))
		mask |= POLLOUT | POLLWRNORM;	/* writable */
	up(&dev->sem);
	return mask;
}


/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations scull_buffer_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		scull_read,
	.write =	scull_write,
	.poll =		scull_poll,
	.open =		scull_open,
	.release =	scull_release,
};


/*
 *Set up a cdev entry.
 */
static void scull_setup_cdev(struct scull_buffer *dev)
{
	int err;
	cdev_init(&dev->cdev, &scull_buffer_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, dev_num, 1);
	// fail gracefully if need be
	if (err)
		printk(KERN_NOTICE "Error %d adding scull_buffer", err);
}


/*
 *Initialize the scull_buffer;
 *return number of scull buffers successfully created
 */
int scull_init(void)
{
	int result;

    // device number allocation
    if(scull_major)
    {
        dev_num = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev_num, scull_nr_devs, "scull_buffer");
    }
    else
    {
        result = alloc_chrdev_region(&dev_num, scull_minor, scull_nr_devs, "scull_buffer");
        scull_major = MAJOR(dev_num);
    }
    if(result < 0)
    {
        printk(KERN_NOTICE, "Unable to allocate device number for scull_buffer, error %d\n", result);
        return 0;
    }

    // allocate  memory in kernel for scull_buffer
	scull_device = kmalloc(scull_nr_devs * sizeof(struct scull_buffer),
                            GFP_KERNEL);
	if (scull_device == NULL)
    {
		unregister_chrdev_region(dev_num, scull_nr_devs);
		return 0;
	}
	memset(scull_device, 0, scull_nr_devs * sizeof(struct scull_buffer));

    // initialize wait_queues
	init_waitqueue_head(&(scull_device->inq));
	init_waitqueue_head(&(scull_device->outq));
    // initialize mutex sempahore
	init_MUTEX(&scull_device->sem);
	scull_setup_cdev(scull_device);

	return scull_nr_devs;
}


/*
 * This is called by cleanup_module or on failure.
 * It is required to never fail, even if nothing was initialized first
 */
void scull_cleanup(void)
{
	if (!scull_device)
		return;
	cdev_del(&scull_device->cdev);
	kfree(scull_device->buffer);
	kfree(scull_device);
	unregister_chrdev_region(dev_num, scull_nr_devs);
	scull_device = NULL;
}


module_init(scull_init)
module_exit(scull_cleanup)
