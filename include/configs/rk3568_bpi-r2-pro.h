#ifndef _RK3568_R2PRO_H
#define _RK3568_R2PRO_H

#define	USEBOOTMENU

#ifdef USEBOOTMENU
#define RUNBOOTMENU \
	"bootdelay=0\0" \
	"bootcmd=setenv bootdelay 2; run reloadmenu;\0"
#else
#define RUNBOOTMENU ""
#endif

#define BOOTENV_R2P \
	"board=bpi-r2pro\0" \
	"device=mmc\0" \
	"partition_sd=1:2\0" \
	"partition_emmc=0:2\0" \
	"bootenv=uEnv.txt\0" \
	"checkenv=test -e ${device} ${partition} ${bootenv}\0" \
	"importenv=env import -t ${scriptaddr} ${filesize}\0" \
	"loadbootenv=if fatload ${device} ${partition} ${scriptaddr} ${bootenv};then run importenv;else echo \"fatload (${bootenv}) failed\";fi\0" \
	"checksd=fatinfo ${device} ${partition_sd}\0" \
	"selectmmc=if run checksd; then echo Boot from SD ; setenv partition ${partition_sd};else echo Boot from eMMC; setenv partition ${partition_emmc} ; fi;\0" \
	"reloadmenu=run selectmmc;if run checkenv; then run loadbootenv; else echo file not found; fi;bootmenu;\0" \
	RUNBOOTMENU \
	"bootmenu_0=1. Run extlinux.=run distro_bootcmd\0" \
	"bootmenu_default=0\0"

#endif
