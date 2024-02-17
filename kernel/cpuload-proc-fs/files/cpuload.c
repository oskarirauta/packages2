#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>


#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/time_namespace.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>

// Buffer enough for 64 cpus
#define BUFFER_SIZE 1024
#define PROC_NAME "cpuload"

static void cpuload_handler(struct work_struct *w);

static struct workqueue_struct *cpuload_workqueue = 0;
static DECLARE_DELAYED_WORK(Task, cpuload_handler);
static unsigned long onesec;
static int die = 0;

typedef struct cpu_t {
	u64 user, nice, system, idle, iowait, irq, softirq;
	u64 prev_user, prev_nice, prev_system, prev_idle, prev_iowait, prev_irq, prev_softirq;
	int major, minor;
} cpu_t;

static int cpu_count = 0;
static cpu_t* cpu = NULL;

u64 nsec_to_clock_t(u64 x) {
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	/*
         * max relative error 5.7e-8 (1.8s per year) for USER_HZ <= 1024,
         * overflow after 64.99 years.
         * exact for HZ=60, 72, 90, 120, 144, 180, 300, 600, 900, ...
         */
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

static void cpuload_handler(struct work_struct *w) {

	int i = 0;

	cpu[0].prev_user = cpu[0].user;
	cpu[0].prev_nice = cpu[0].nice;
	cpu[0].prev_system = cpu[0].system;
	cpu[0].prev_idle = cpu[0].idle;
	cpu[0].prev_iowait = cpu[0].iowait;
	cpu[0].prev_irq = cpu[0].irq;
	cpu[0].prev_softirq = cpu[0].softirq;

	cpu[0].user = cpu[0].nice = cpu[0].system = cpu[0].idle = cpu[0].iowait = cpu[0].irq = cpu[0].softirq = 0;

	for_each_possible_cpu(i) {

		cpu[i + 1].prev_user = cpu[i + 1].user;
		cpu[i + 1].prev_nice = cpu[i + 1].nice;
		cpu[i + 1].prev_system = cpu[i + 1].system;
		cpu[i + 1].prev_idle = cpu[i + 1].idle;
		cpu[i + 1].prev_iowait = cpu[i + 1].iowait;
		cpu[i + 1].prev_irq = cpu[i + 1].irq;
		cpu[i + 1].prev_softirq = cpu[i + 1].softirq;

		struct kernel_cpustat kcpustat;
		u64 *cpustat = kcpustat.cpustat;
		kcpustat_cpu_fetch(&kcpustat, i);

		if ( cpu_online(i)) {
			cpu[i + 1].idle = cpustat[CPUTIME_IDLE];
			cpu[i + 1].iowait = cpustat[CPUTIME_IOWAIT];
		} else {
			cpu[i + 1].idle = get_cpu_idle_time_us(i, NULL) * NSEC_PER_USEC;
			cpu[i + 1].iowait =  get_cpu_iowait_time_us(i, NULL) * NSEC_PER_USEC;
		}

		cpu[i + 1].user = cpustat[CPUTIME_USER];
		cpu[i + 1].nice = cpustat[CPUTIME_NICE];
		cpu[i + 1].system = cpustat[CPUTIME_SYSTEM];
		cpu[i + 1].irq = cpustat[CPUTIME_IRQ];
		cpu[i + 1].softirq = cpustat[CPUTIME_SOFTIRQ];

		cpu[0].user += cpu[i + 1].user;
		cpu[0].nice += cpu[i + 1].nice;
		cpu[0].system += cpu[i + 1].system;
		cpu[0].idle += cpu[i + 1].idle;
		cpu[0].iowait += cpu[i + 1].iowait;
		cpu[0].irq += cpu[i + 1].irq;
		cpu[0].softirq += cpu[i + 1].softirq;

		cpu[i + 1].user = nsec_to_clock_t(cpu[i + 1].user);
		cpu[i + 1].nice = nsec_to_clock_t(cpu[i + 1].nice);
		cpu[i + 1].system = nsec_to_clock_t(cpu[i + 1].system);
		cpu[i + 1].idle = nsec_to_clock_t(cpu[i + 1].idle);
		cpu[i + 1].iowait = nsec_to_clock_t(cpu[i + 1].iowait);
		cpu[i + 1].irq = nsec_to_clock_t(cpu[i + 1].irq);
		cpu[i + 1].softirq =nsec_to_clock_t(cpu[i + 1].softirq);
	}

	cpu[0].user = nsec_to_clock_t(cpu[0].user);
	cpu[0].nice = nsec_to_clock_t(cpu[0].nice);
	cpu[0].system = nsec_to_clock_t(cpu[0].system);
	cpu[0].idle = nsec_to_clock_t(cpu[0].idle);
	cpu[0].iowait = nsec_to_clock_t(cpu[0].iowait);
	cpu[0].irq = nsec_to_clock_t(cpu[0].irq);
	cpu[0].softirq =nsec_to_clock_t(cpu[0].softirq);

	for ( i = 0; i <= cpu_count; i++ ) {

		u64 n_total = cpu[i].user + cpu[i].nice + cpu[i].system + cpu[i].idle + cpu[i].iowait + cpu[i].irq + cpu[i].softirq;
		u64 p_total = cpu[i].prev_user + cpu[i].prev_nice + cpu[i].prev_system + cpu[i].prev_idle + cpu[i].prev_iowait + cpu[i].prev_irq + cpu[i].prev_softirq;
		u64 n_idle = cpu[i].idle + cpu[i].iowait;
		u64 p_idle = cpu[i].prev_idle + cpu[i].prev_iowait;
		u64 total = n_total - p_total;
		u64 idle = n_idle - p_idle;

		u64 scale = 100000 / total;
		u64 idle2 = idle * scale;
		u64 percent = 100000 - idle2;
		percent /= 10;

		cpu[i].major = (int)percent / 100;
		cpu[i].minor = (int)percent - ( cpu[i].major * 100);
	}

	if (die == 0)
		queue_delayed_work(cpuload_workqueue, &Task, onesec);
}

static ssize_t proc_read(struct file *file, char __user *usr_buf,size_t count, loff_t *pos) {

	int i = 0;
	int len = 0;
	static char buffer[BUFFER_SIZE];
	static int completed = 0;

	if ( completed ) {

		completed = 0;
		return 0;
	}

	completed = 1;
	for ( i = 0; i <= cpu_count; i++ )
		len += sprintf(buffer, "cpu%d: %d.%d\n", i, cpu[i].major, cpu[i].minor);

	copy_to_user(usr_buf, buffer, len);
	return len;
}

static const struct proc_ops cpuload_ops= {
	.proc_read = proc_read
};

static int proc_init(void) {

	int i = 0;
	cpu_count = num_online_cpus();
	cpu = (cpu_t*)kmalloc(sizeof(cpu_t) * cpu_count + 1, GFP_KERNEL);
	memset(cpu, 0, sizeof(cpu_t) * cpu_count + 1);

	for ( i = 0; i <= cpu_count; i++ ) {
		cpu[i].user = cpu[i].nice = cpu[i].system = cpu[i].idle = cpu[i].iowait = cpu[i].irq = cpu[i].softirq = 0;
		cpu[i].prev_user = cpu[i].prev_nice = cpu[i].prev_system = cpu[i].prev_idle = cpu[i].prev_iowait = cpu[i].prev_irq = cpu[i].prev_softirq = 0;
		cpu[i].major = cpu[i].minor = 0;
	}

	onesec = msecs_to_jiffies(1000);
	proc_create(PROC_NAME, 0666, NULL, &cpuload_ops);

	if ( !cpuload_workqueue )
		cpuload_workqueue = create_singlethread_workqueue("cpuload_kmod");
	if ( cpuload_workqueue )
		queue_delayed_work(cpuload_workqueue, &Task, onesec);

	printk(KERN_INFO "/proc/%s created\n", PROC_NAME);

	return 0;
}

static void proc_exit(void) {

	remove_proc_entry(PROC_NAME, NULL);

	if ( cpuload_workqueue ) {
		die = 1;
		cancel_delayed_work(&Task);
		flush_workqueue(cpuload_workqueue);
		destroy_workqueue(cpuload_workqueue);
	}

	kfree(cpu);
	cpu = NULL;

	printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_DESCRIPTION("CPULoad Module");
MODULE_AUTHOR("Oskari Rauta");
MODULE_LICENSE("GPL");
