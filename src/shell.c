#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"
#include "FreeRTOS.h"
#include "task.h"
#include "host.h"
typedef struct {
    const char *name;
    cmdfunc *fptr;
    const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
void new_command(int, char **);
void _command(int, char **);
int stoi(char *);
#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
    MKCL(ls, "List directory"),
    MKCL(man, "Show the manual of the command"),
    MKCL(cat, "Concatenate files and print on the stdout"),
    MKCL(ps, "Report a snapshot of the current processes"),
    MKCL(host, "Run command on host"),
    MKCL(mmtest, "heap memory allocation test"),
    MKCL(help, "help"),
    MKCL(test, "test fibonacci function"),
    MKCL(new, "test creating a new task ,and show under \"ps\" command "),
    MKCL(, ""),
};

int stoi(char *str)//change char into int
{
    int i;
    int result = 0;
    int count = strlen(str);
    for (i = 0; i < count; ++i)
    {
	result = result*10+(str[i]-'0');
    }
    return result;
}

int parse_command(char *str, char *argv[]){
    int b_quote=0, b_dbquote=0;
    int i;
    int count=0, p=0;
    for(i=0; str[i]; ++i){
	if(str[i]=='\'')
	    ++b_quote;
	if(str[i]=='"')
	    ++b_dbquote;
	if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
	    str[i]='\0';
	    argv[count++]=&str[p];
	    p=i+1;
	}
    }
    /* last one */
    argv[count++]=&str[p];

    return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    if(n == 0){
    }else if(n == 1){
    }else if(n==2){
	fs_opendir(argv[1]);
	fio_printf(1, "\r\n");
    }else{
	fio_printf(1, "Too many argument!\r\n");
	return;
    }
}

int filedump(const char *filename){
    char buf[128];

    int fd=fs_open(filename, 0, O_RDONLY);

    if( fd == -2 || fd == -1)
	return fd;

    fio_printf(1, "\r\n");

    int count;
    while((count=fio_read(fd, buf, sizeof(buf)))>0){
	fio_write(1, buf, count);
    }

    fio_printf(1, "\r");

    fio_close(fd);
    return 1;
}

void ps_command(int n, char *argv[]){
    signed char buf[1024];
    vTaskList(buf);
    fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
    fio_printf(1, "*******************************************\n\r");
    fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
    if(n==1){
	fio_printf(2, "\r\nUsage: cat <filename>\r\n");
	return;
    }

    int dump_status = filedump(argv[1]);
    if(dump_status == -1){
	fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
	fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
    if(n==1){
	fio_printf(2, "\r\nUsage: man <command>\r\n");
	return;
    }

    char buf[128]="/romfs/manual/";
    strcat(buf, argv[1]);

    int dump_status = filedump(buf);
    if(dump_status < 0)
	fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
	for(i = 1; i < n; i++) {
	    memcpy(&command[len], argv[i], strlen(argv[i]));
	    len += (strlen(argv[i]) + 1);
	    command[len - 1] = ' ';
	}
	command[len - 1] = '\0';
	rnt=host_action(SYS_SYSTEM, command);
	fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
	fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
    int i;
    fio_printf(1, "\r\n");
    for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
	fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
    }
}


int fibonacci(int x) {
    if(x<=0) return 0;
    if(x==1) return 1;
    return fibonacci(x-1) + fibonacci(x-2);
}

//Fast inverse square root method
float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;                       // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );               
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
    return y;
}

void calc_prime(int number)
{
    int num = 1/Q_rsqrt(number);
    int count = 0;
    for(int i =2;i <= num;i++)
    {
	if(number%i == 0)
	count++;
    }
    if(count == 0)
	fio_printf(1,"Prime number:%d\r\n",number);
}

void test_command(int n, char *argv[]) {
    int handle;
    int error;
    fio_printf(1, "\r\n");
    char mode = *argv[1];//mode f for calculate fibonacci; mode p for calculate prime numbers
    int int_input = stoi(argv[2]);
    if(mode == 'f')
    {
	if(int_input == -1 || int_input <0)
	    fio_printf(1,"Try again!");
	else
	{
	    for(;int_input>=0;int_input--)
		fio_printf(1, "%s%d%s : %d \r\n","test fibonacci series (",int_input,")",fibonacci(int_input));
	}
    }
    else if(mode == 'p')
    {
	if(int_input == -1 || int_input <= 1)
	    fio_printf(1,"Try again!");
	else
	{
	    for(;int_input>=2;int_input--)
		calc_prime(int_input);
	}
    }
    else
    {
	fio_printf(1,"mode error!");
    }
    *argv[2] = -1;



    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");

    handle = host_action(SYS_OPEN, "output/syslog", 8);
    if(handle == -1) {
	fio_printf(1, "Open file error!\n\r");
	return;
    }

    char *buffer = "Test host_write function which can write data to output/syslog\n";
    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
    if(error != 0) {
	fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
	host_action(SYS_CLOSE, handle);
	return;
    }

    host_action(SYS_CLOSE, handle);
}

xTaskHandle xHandle_new_command;

void new_task(void *pvParameters)
{
    vTaskSuspend(xHandle_new_command);
}

void new_command(int n,char *argv[]){
    fio_printf(1, "\r\nnew command \r\n");
    xTaskCreate(new_task,
	    (signed portCHAR *) "aTASK",
	    128/* stack size */, NULL,
	    tskIDLE_PRIORITY + 2, &xHandle_new_command);
}


void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

cmdfunc *do_command(const char *cmd){

    int i;

    for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
	if(strcmp(cl[i].name, cmd)==0)
	    return cl[i].fptr;
    }
    return NULL;	
}
