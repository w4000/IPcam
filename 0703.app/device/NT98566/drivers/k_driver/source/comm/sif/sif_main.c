#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <asm/signal.h>
#include <kwrap/dev.h>

#include "sif_drv.h"
#include "sif_reg.h"
#include "sif_main.h"
#include "sif_proc.h"
#include "sif_dbg.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int sif_debug_level = (NVT_DBG_INFO | NVT_DBG_WRN | NVT_DBG_ERR);
module_param_named(sif_debug_level, sif_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sif_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id sif_match_table[] = {
	{	.compatible = "nvt,nvt_sif"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_sif_open(struct inode *inode, struct file *file);
static int nvt_sif_release(struct inode *inode, struct file *file);
static long nvt_sif_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_sif_probe(struct platform_device * pdev);
static int nvt_sif_suspend( struct platform_device * pdev, pm_message_t state);
static int nvt_sif_resume( struct platform_device * pdev);
static int nvt_sif_remove(struct platform_device *pdev);
int __init nvt_sif_init(void);
void __exit nvt_sif_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_sif_open(struct inode *inode, struct file *file)
{
	SIF_DRV_INFO* pdrv_info;

	pdrv_info = container_of(inode->i_cdev, SIF_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_sif_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

    return 0;
}

static int nvt_sif_release(struct inode *inode, struct file *file)
{
	SIF_DRV_INFO* pdrv_info;
	pdrv_info = container_of(inode->i_cdev, SIF_DRV_INFO, cdev);

	nvt_sif_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
    return 0;
}

static long nvt_sif_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PSIF_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_sif_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_sif_fops = {
    .owner   = THIS_MODULE,
    .open    = nvt_sif_open,
    .release = nvt_sif_release,
    .unlocked_ioctl = nvt_sif_ioctl,
    .llseek  = no_llseek,
};

static int nvt_sif_probe(struct platform_device * pdev)
{
    SIF_DRV_INFO* pdrv_info;
	const struct of_device_id *match;
    int ret = 0;
	unsigned char ucloop;

    nvt_dbg(INFO, "%s\n", pdev->name);

	match = of_match_device(sif_match_table, &pdev->dev);
	if (!match){
        nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

    pdrv_info = kzalloc(sizeof(SIF_DRV_INFO), GFP_KERNEL);
    if (!pdrv_info) {
        nvt_dbg(ERR, "failed to allocate memory\n");
        return -ENOMEM;
    }

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
		if (pdrv_info->presource[ucloop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		nvt_dbg(INFO, "%d. resource:0x%x size:0x%x\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", ucloop);
			release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap_nocache(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
			if (ucloop == 0) {
				iounmap(pdrv_info->module_info.io_addr[ucloop]);
			}
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		nvt_dbg(INFO, "IRQ %d. ID%d\n", ucloop,pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//Get clock source
	for (ucloop = 0 ; ucloop < 1; ucloop++) {
		pdrv_info->module_info.pclk[ucloop] = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (IS_ERR(pdrv_info->module_info.pclk[ucloop])) {
			nvt_dbg(ERR, "faile to get clock%d source\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	nvt_dbg(INFO, "DevID Major:%d minor:%d\n" \
		        , MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_sif_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

    pdrv_info->pmodule_class = class_create(THIS_MODULE, "nvt_sif");
	if(IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
    ucloop = 0;
    pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
	                             , MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
                                 , MODULE_NAME);

    if(IS_ERR(pdrv_info->pdevice[ucloop])) {
        nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);
        ret = -ENODEV;
        goto FAIL_CLASS;
	}

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++) {
	}

	ret = nvt_sif_proc_init(pdrv_info);
	if(ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}

	ret = nvt_sif_drv_init(&pdrv_info->module_info);

    platform_set_drvdata(pdev, pdrv_info);
	if(ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;

FAIL_DRV_INIT:
	nvt_sif_proc_remove(pdrv_info);

FAIL_DEV:
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

FAIL_FREE_RES:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

FAIL_FREE_BUF:
    kfree(pdrv_info);
    return ret;
}

static int nvt_sif_remove(struct platform_device *pdev)
{
	PSIF_DRV_INFO pdrv_info;
	unsigned char ucloop;

    nvt_dbg(INFO, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_sif_drv_remove(&pdrv_info->module_info);

	nvt_sif_proc_remove(pdrv_info);

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

    kfree(pdrv_info);
	return 0;
}

static int nvt_sif_suspend( struct platform_device * pdev, pm_message_t state)
{
	PSIF_DRV_INFO pdrv_info;;

    nvt_dbg(INFO, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_sif_drv_suspend(&pdrv_info->module_info);

    nvt_dbg(INFO, "finished\n");
	return 0;
}


static int nvt_sif_resume( struct platform_device * pdev)
{
	PSIF_DRV_INFO pdrv_info;;

    nvt_dbg(INFO, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_sif_drv_resume(&pdrv_info->module_info);

    nvt_dbg(INFO, "finished\n");
	return 0;
}

static struct platform_driver nvt_sif_driver = {
    .driver = {
		        .name   = "nvt_sif",
				.owner = THIS_MODULE,
				.of_match_table = sif_match_table,
		      },
    .probe      = nvt_sif_probe,
    .remove     = nvt_sif_remove,
	.suspend = nvt_sif_suspend,
	.resume = nvt_sif_resume
};

#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#endif

int __init nvt_sif_init(void)
{
	int ret;

    nvt_dbg(WRN, "\n");
	ret = platform_driver_register(&nvt_sif_driver);

	return 0;
}

void __exit nvt_sif_exit(void)
{
    nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_sif_driver);
}

module_init(nvt_sif_init);
module_exit(nvt_sif_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("sif driver");
MODULE_LICENSE("GPL");
