#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "ctl_sen_proc.h"
#include "ctl_sen_dbg.h"
#include "ctl_sen_main.h"
#include "ctl_sen_api.h"
#include "sen_int.h"
#include "sen_dbg_infor_int.h"

static struct seq_file *g_seq_file;

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM     30

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PCTL_SEN_MODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static PCTL_SEN_DRV_INFO pdrv_info_data;
static int nvt_ctl_sen_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_ctl_sen_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_cmd_show, &pdrv_info_data->module_info);
}

static ssize_t nvt_ctl_sen_proc_cmd(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	} else if (len < 1) {
		nvt_dbg(ERR, "Command length is too short!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	CTL_SEN_DBG_IND("CMD:%s\n", cmd_line);

	argv[0] = CTL_SEN_PROC_NAME;
	// parse command string
	for (ucargc = 1; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ctl_sen_cmd_execute(ucargc, &argv[0]) == 0) {
		return size;
	}

	return size;

ERR_OUT:
	return -1;
}

static struct file_operations proc_cmd_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_ctl_sen_proc_cmd_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = nvt_ctl_sen_proc_cmd
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_ctl_sen_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000001 > /proc/ctl_sen/cmd ) to dump sensor (id) ctl sen get_cfg\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000002 > /proc/ctl_sen/cmd ) to dump sensor (id) sen ext info\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000004 > /proc/ctl_sen/cmd ) to dump sensor (id) sen drv info\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000008 > /proc/ctl_sen/cmd ) to dump sensor (id) vd\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000010 > /proc/ctl_sen/cmd ) to dump sensor (id) vd2\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000020 > /proc/ctl_sen/cmd ) to dump sensor (id) fmd\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000040 > /proc/ctl_sen/cmd ) to dump sensor (id) fmd2\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000080 > /proc/ctl_sen/cmd ) to dump sensor (id) map tbl\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000100 > /proc/ctl_sen/cmd ) to dump sensor (id) proc time\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00000400 > /proc/ctl_sen/cmd ) to dump sensor (id) ctl info\n");
	seq_printf(sfile, "Press ( echo dbg (id) 0x00008000 > /proc/ctl_sen/cmd ) to dump sensor (id) er\n");

	seq_printf(sfile, "Press ( echo w_reg (id) (addr) (length) (value0) (value1) > /proc/ctl_sen/cmd ) to write sensor register\n");
	seq_printf(sfile, "Press ( echo r_reg (id) (addr) (length) > /proc/ctl_sen/cmd ) to read sensor register\n");

	seq_printf(sfile, "Press ( echo dump_op_msg (id) (en) > /proc/ctl_sen/cmd ) to show sensor operation msg\n");
	seq_printf(sfile, "Press ( echo dbg_lv (lv) > /proc/ctl_sen/cmd ) set ctl sen dbg msg level\n");

	seq_printf(sfile, "=====================================================================\n");

	return 0;
}

static int nvt_ctl_sen_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_help_show, NULL);
}

static struct file_operations proc_help_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_ctl_sen_proc_help_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};

//=============================================================================
// proc "info" file operation functions
//=============================================================================

static int ctl_sen_seq_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	/* Initialize variable arguments. */
	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
	seq_printf(g_seq_file, buf);
	return 0;
}

static int nvt_ctl_sen_proc_info_show(struct seq_file *sfile, void *v)
{
	UINT32 i;

	vos_sem_wait(ctl_sen_sem_proc);
	g_seq_file = sfile;

	for (i = 0; i < CTL_SEN_ID_MAX; i++) {
		if (ctl_sen_get_cur_state(i) == CTL_SEN_STATE_NONE) {
			continue;
		}
		sen_dbg_get_obj()->dump_info(ctl_sen_seq_printf, i);
		sen_dbg_get_obj()->dump_ext_info(ctl_sen_seq_printf, i);
		sen_dbg_get_obj()->dump_drv_info(ctl_sen_seq_printf, i);
		sen_dbg_get_obj()->dump_ctl_info(ctl_sen_seq_printf, i);
	}
	sen_dbg_get_obj()->dump_map_tbl_info(ctl_sen_seq_printf);
	sen_dbg_get_obj()->dump_proc_time(ctl_sen_seq_printf);

	vos_sem_sig(ctl_sen_sem_proc);

	return 0;
}

static int nvt_ctl_sen_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_info_show, NULL);
}

static struct file_operations proc_info_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_ctl_sen_proc_info_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};


//=============================================================================
// proc "signal" file operation functions
//=============================================================================
static int nvt_ctl_sen_proc_signal_show(struct seq_file *sfile, void *v)
{
	UINT32 i, j;
	CTL_SEN_INTE inte_vd[5] = {CTL_SEN_INTE_VD_TO_SIE0, CTL_SEN_INTE_VD_TO_SIE1, CTL_SEN_INTE_NONE, CTL_SEN_INTE_VD_TO_SIE3, CTL_SEN_INTE_VD_TO_SIE4};
	CTL_SEN_INTE inte_fmd[5] = {CTL_SEN_INTE_FMD_TO_SIE0, CTL_SEN_INTE_FMD_TO_SIE1, CTL_SEN_INTE_NONE, CTL_SEN_INTE_FMD_TO_SIE3, CTL_SEN_INTE_FMD_TO_SIE4};

	vos_sem_wait(ctl_sen_sem_proc);
	g_seq_file = sfile;

	for (i = 0; i < CTL_SEN_ID_MAX; i++) {
		if ((ctl_sen_get_cur_state(i) & CTL_SEN_STATE_PWRON) != CTL_SEN_STATE_PWRON) {
			continue;
		}
		if (g_ctl_sen_ctrl_obj[i]->cur_sen_mode == CTL_SEN_MODE_UNKNOWN) {
			continue;
		}
		if (g_ctl_sen_ctrl_obj[i]->cur_chgmode.output_dest == CTL_SEN_OUTPUT_DEST_AUTO) {
			if (i < CTL_SEN_MAX_OUTPUT_SIE_IDX) {
				sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_vd[i]);
				sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_fmd[i]);
			} else {
				sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_vd[0]);
				sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_fmd[0]);
			}
		} else {
			for (j = 0; j < CTL_SEN_MAX_OUTPUT_SIE_IDX; j++) {
				if (g_ctl_sen_ctrl_obj[i]->cur_chgmode.output_dest & (1 << j)) {
					sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_vd[j]);
					sen_dbg_get_obj()->wait_inte(ctl_sen_seq_printf, i, inte_fmd[j]);
				}
			}
		}
	}
	vos_sem_sig(ctl_sen_sem_proc);

	return 0;
}

static int nvt_ctl_sen_proc_signal_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_signal_show, NULL);
}

static struct file_operations proc_signal_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_ctl_sen_proc_signal_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};

//=============================================================================
// proc init
//=============================================================================
int nvt_ctl_sen_proc_init(PCTL_SEN_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("ctl_sen", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;

	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_help;
	}
	pdrv_info->pproc_help_entry = pentry;

	pentry = proc_create("info", S_IRUGO | S_IXUGO, pmodule_root, &proc_info_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc info!\n");
		ret = -EINVAL;
		goto remove_info;
	}
	pdrv_info->pproc_info_entry = pentry;

	pentry = proc_create("signal", S_IRUGO | S_IXUGO, pmodule_root, &proc_signal_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc signal!\n");
		ret = -EINVAL;
		goto remove_signal;
	}
	pdrv_info->pproc_signal_entry = pentry;

	pdrv_info_data = pdrv_info;

	return ret;


remove_signal:
	proc_remove(pdrv_info->pproc_signal_entry);
remove_info:
	proc_remove(pdrv_info->pproc_info_entry);
remove_help:
	proc_remove(pdrv_info->pproc_help_entry);
remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);
remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_ctl_sen_proc_remove(PCTL_SEN_DRV_INFO pdrv_info)
{
	if (pdrv_info_data) {
		proc_remove(pdrv_info->pproc_signal_entry);
		proc_remove(pdrv_info->pproc_info_entry);
		proc_remove(pdrv_info->pproc_help_entry);
		proc_remove(pdrv_info->pproc_cmd_entry);
		proc_remove(pdrv_info->pproc_module_root);
	}
	return 0;
}
