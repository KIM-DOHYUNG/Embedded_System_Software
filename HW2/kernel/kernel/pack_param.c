#include <linux/kernel.h>
#include <linux/uaccess.h>

#define SYSCALL_PACK_PARAM 376

struct param_info
{
	int start_location;
	int start_value;
	int time_interval;
	int timer_count;
};

long sys_pack_param(struct param_info *param, unsigned long *packed_value)
{
	int i;
	struct param_info temp_param;
	unsigned long temp_packed_value = 0;

	copy_from_user(&temp_param, param, sizeof(struct param_info));

	// Make 4byte stream packed_value using bitwise operation
	temp_packed_value |= (temp_param.start_location  << 24);
	temp_packed_value |= (temp_param.start_value     << 16);
	temp_packed_value |= (temp_param.time_interval   <<  8);
	temp_packed_value |= (temp_param.timer_count     <<  0);

	copy_to_user(packed_value, &temp_packed_value, sizeof(unsigned long));

	return SYSCALL_PACK_PARAM;
}
