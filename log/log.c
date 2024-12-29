/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"


int log_write(char in[]);
char * inode2filename(int inode, char * filename);
struct proc *		pcloser;
void start_log(){
    printl("\n\n\nlogstarting\n");
    logbuffer[0]=0;
    logbufp=0;
    const char filename[] = "log";
    int fd = open_log(filename, O_CREAT);
    close_log(fd);
    logcount=0;
    
    printl("\n\n\nlogstarted\n");
    log_executing=1;
    proc_logging =0;
    log_hardware=0;
    file_logging=1;
    syscall_logging=1;
    log_debug=1;
}

int log_generate(MESSAGE* message){
    // char *name;
    
    if(log_executing!=1){
        return;
    }
    char buffer[40];
    MESSAGE msg=*message;

    switch (msg.FLAGS) {
		case OPEN:
        if(file_logging==0){
            return 1;
        }
            //printl("generating\n");
			strcpy(buffer,"opFILE:OPEN ");
            strcat(buffer,msg.PATHNAME);
			break;

		case CLOSE:
        if(file_logging==0){
            return 1;
        }
            //printl("generating close\n");
			strcpy(buffer,"opFILE:CLOSE ");
            //pcloser=&proc_table[src];
           // inode2filename(pcloser->filp[fd]->fd_inode->i_num,name); 报错
            //printl("%s\n\n\n",name);
            //strcat(buffer,name);
			break;

		default:
             
        if(syscall_logging==0){
            return 1;
        }
            //printl("generate fail:%d\n",msg.FLAGS);//没有清空
            strcpy(buffer,"SYSCALL ");
            strcat(buffer,syscallname[msg.FLAGS]);
            //printl("%s\n",buffer);
			break;
		}
   // printl("generat completee:\n%s\n\n\n",buffer);
    return log_write(buffer);
    return 1;
    
}

int log_write(char in[]){
    if(log_executing==1){
        istrcat(logbuffer,in,&logbufp);
        istrcat(logbuffer,"\n",&logbufp);
        //printl("%s\n",logbuffer);
        if(logbufp>=8000){
            log_write_in_to_file();
        }
    }
    //printl("\ncomplete!!!!!!!!\n");
    log_executing=1;
    //log_debug=1;
    return 1;
}

int log_write_in_to_file(){
    int fd;
    int n;
    fd = open_log("log", O_RDWR);
    assert(fd != -1);
    n = write_log(fd, logbuffer, logbufp);
    assert(n==logbufp);
    /* close */
    close_log(fd);
    //logbuffer[0]=0;
    //logcount+=logbufp;
    //logbufp=0;
    return 1;
}

int log_read(){
    if(logbufp>0){
        log_write_in_to_file();
    }
    //printl("write complete");
    int fd;
	int n;
	const int rd_bytes = logbufp;//logcount
	char bufr[rd_bytes];
	/* open */
	fd = open_log("log", O_RDWR);
	assert(fd != -1);
    
	/* read */
	n = read_log(fd, bufr, rd_bytes);
	//assert(n == rd_bytes);
	bufr[n] = 0;
	printl("%d bytes read: \n%s\n\n\n\n", n, bufr);
    log_msg.PATHNAME=bufr;
    //printl("write complete");
	/* close */
	close_log(fd);
    log_executing=1;
    return 1;
}

int end_log(){
    logbuffer[0]=0;
    logbufp=0;
    logcount=0;
    log_executing=0;
    proc_logging =0;
    log_debug=0;
    log_hardware=0;
    file_logging=0;
    syscall_logging=0;
    printl("log ended\n");
    return 1;
}

#define DISKLOG_RD_SECT(dev,sect_nr) rw_sector(DEV_READ, \
					       dev,			\
					       (sect_nr) * SECTOR_SIZE,	\
					       SECTOR_SIZE, /* read one sector */ \
					       getpid(),		\
					       logdiskbuf);
char * inode2filename(int inode, char * filename) {
    int i, j;

    struct inode * dir_inode = root_inode;

    int dir_blk0_nr = dir_inode->i_start_sect;
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = 
      dir_inode->i_size / DIR_ENTRY_SIZE;
    int m = 0;
    struct dir_entry * pde;
    for (i = 0; i < nr_dir_blks; i++) {
        DISKLOG_RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        pde = (struct dir_entry *)logdiskbuf;
        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
            if (pde->inode_nr == inode) {
                memcpy(filename, pde->name, MAX_FILENAME_LEN);
                filename[MAX_FILENAME_LEN] = 0;

                if (filename[0] == '.')
                    filename[0] = '/';

                return filename;
            }
            if (++m > nr_dir_entries)
                break;
        }
        if (m > nr_dir_entries)
            break;
    }

    return 0;
}