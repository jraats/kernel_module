#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#include "fpga_mapping.h"
#include "io.h"

// Some kernel module settings
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joost Raats");
MODULE_DESCRIPTION("A FPGA Linux module.");
MODULE_VERSION("0.01");

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

// Function prototype that is called every 2s
static int notify_thread_function(void* param);
// Thread pointer
static struct task_struct *notify_thread;

// number of file descriptors open
static int device_open_count = 0;
// pointer to the device class
static struct class *device_class;

static int readEnd = 0;

// kobject which contains attribute values
static struct kobject *mykobj;

// struct for storing attribute values
struct my_attr {
	struct attribute attr;
	int value;
};

// notify attribute value
static struct my_attr notify = {
	.attr.name=FPGA_SYSFS_ATTRIBUTE,
	.attr.mode = 0644,
	.value = 0,
};

// kobject attributes
static struct attribute * myattr[] = {
	&notify.attr,
	NULL
};

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
	if (readEnd) {
		readEnd = 0;
		return 0;
	}
	if (len >= 2) {
		put_user('a', buffer++);
		put_user('b', buffer++);
		readEnd = 1;
		return 2;
	}
	return 0;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
	/* This is a read-only device */
	printk(KERN_ALERT "This operation is not supported.\n");
	return -EINVAL;
}

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
	uint32_t status;
	switch (ioctl_num) {
	case FPGA_IOCTL_GET_VERSION:
		printk(KERN_INFO "Got get version: %zu", fpga_get_version());
		put_user(fpga_get_version(), (uint32_t *) ioctl_param);
		break;
	case FPGA_IOCTL_GET_STATUS:
		printk(KERN_INFO "Got get status: %zu", fpga_get_status());
		put_user(fpga_get_status(), (uint32_t *) ioctl_param);
		break;
	case FPGA_IOCTL_SET_STATUS:
		printk(KERN_INFO "Got set status: %zu", *((uint32_t *) ioctl_param));
		get_user(status, (uint32_t *) ioctl_param);
		fpga_set_status(status);
		break;
	default:
		printk(KERN_ALERT "unknown io control");
		break;
	}
	return 0;
}

int notify_thread_function(void* param) {
	unsigned long wakeupJiffies = jiffies + (2*HZ);
	// keep running until the kernel wants us to stop
	while(!kthread_should_stop()) {
		// wake up every 2 seconds
		if (time_before(jiffies, wakeupJiffies)) {
			schedule();
		} else {
			sysfs_notify(mykobj, NULL, FPGA_SYSFS_ATTRIBUTE);
			wakeupJiffies = jiffies + (2*HZ);
		}
	}
	do_exit(0);
	return 0;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
	/* If device is open, return busy, becuase only one file descriptor is allowed */
	if (device_open_count) {
		return -EBUSY;
	}
	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
	/* Decrement the open counter and usage count. Without this, the module would not unload. */
	device_open_count--;
	module_put(THIS_MODULE);
	return 0;
}

// Called when kobject attribute read
static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf) {
	struct my_attr *a = container_of(attr, struct my_attr, attr);
	return scnprintf(buf, PAGE_SIZE, "%s: %d\n", a->attr.name, a->value);
}

// Called when kobject attribute is writen
static ssize_t store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t len) {
	struct my_attr *a = container_of(attr, struct my_attr, attr);
	sscanf(buf, "%d", &a->value);
	notify.value = a->value;
	sysfs_notify(mykobj, NULL, "notify");
	return sizeof(int);
}

// kobject callbacks
static struct sysfs_ops myops = {
	.show = show,
	.store = store,
};

// kobject types
static struct kobj_type mytype = {
	.sysfs_ops = &myops,
	.default_attrs = myattr,
};

static int fpga_init(void) {
	char our_thread[12] = "fpga_notify";
	struct device *err_dev;
	/* Try to register character device */
	int major_num = register_chrdev(FPGA_MAJOR_NUM, FPGA_DEVICE_NAME, &file_ops);
	if (major_num < 0) {
		printk(KERN_ALERT "Could not register device: %d\n", FPGA_MAJOR_NUM);
		return major_num;
	}
	printk(KERN_INFO "%s module loaded with device major number %d\n", FPGA_DEVICE_NAME, FPGA_MAJOR_NUM);

	// create /dev/mymodule we use udev to make the file
	device_class = class_create(THIS_MODULE,FPGA_DEVICE_NAME);
	err_dev = device_create(device_class, NULL, MKDEV(FPGA_MAJOR_NUM,0), NULL, FPGA_DEVICE_NAME);

	// create a thread for notifying user space
	notify_thread = kthread_create(notify_thread_function,NULL,our_thread);
	if(notify_thread) {
		printk(KERN_INFO "%s thead created", FPGA_DEVICE_NAME);
		// start the thread
		wake_up_process(notify_thread);
	}

	mykobj = kzalloc(sizeof(*mykobj), GFP_KERNEL);
	if (mykobj) {
		kobject_init(mykobj, &mytype);
		if (kobject_add(mykobj, NULL, "%s", FPGA_SYSFS_NAME)) {
			printk(KERN_ALERT "%s kobject_add() failed\n", FPGA_DEVICE_NAME);
			kobject_put(mykobj);
			mykobj = NULL;
		}
	}
	return 0;
}

static void fpga_exit(void) {
	int ret;
	if (notify_thread) {
		ret = kthread_stop(notify_thread);
		if(ret < 0) {
			printk(KERN_ALERT "%s thread not stopped %d", FPGA_DEVICE_NAME, ret);
		}
	}

	device_destroy(device_class,MKDEV(FPGA_MAJOR_NUM,0));
	class_unregister(device_class);
	class_destroy(device_class);

	// Remember — we have to clean up after ourselves. Unregister the character device.
	unregister_chrdev(FPGA_MAJOR_NUM, FPGA_DEVICE_NAME);

	if (mykobj) {
		kobject_put(mykobj);
		kfree(mykobj);
	}
	printk(KERN_INFO "%s module unloaded!\n", FPGA_DEVICE_NAME);
}

/* Register module functions */
module_init(fpga_init);
module_exit(fpga_exit);
