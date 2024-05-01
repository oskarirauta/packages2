#include "mpro.h"

static ssize_t partial_read(struct device* dev, struct device_attribute *attr, char *buf) {

	struct mpro_device *mpro = dev_get_drvdata(dev);

	if ( mpro -> partial < 0 )
		return sprintf(buf, "not supported by this model, mpro chipset is required for partial updates\n");

	return sprintf(buf, "%d\n", mpro -> partial);
}

static struct device_attribute partial_attr = {
	.attr = {
		.name = "partial_updates",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = partial_read,
};

int mpro_sysfs_init(struct mpro_device *mpro) {

	int ret;

	ret = sysfs_create_file(&mpro -> dev.dev -> kobj, &partial_attr.attr);
        if ( ret )
		return ret;

	return 0;
}
