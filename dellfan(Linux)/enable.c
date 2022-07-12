#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

#define DISABLE_BIOS_METHOD1 0x30a3
#define ENABLE_BIOS_METHOD1  0x31a3
#define DISABLE_BIOS_METHOD2 0x34a3
#define ENABLE_BIOS_METHOD2  0x35a3
#define ENABLE_FN            0x32a3
#define SET_FAN              0x01a3
#define GET_FAN              0x00a3
#define FN_STATUS            0x0025

int init_ioperm(void)
{
    if (ioperm(0xb2, 4, 1))
        perror("ioperm:");
    if (ioperm(0x84, 4, 1))
        perror("ioperm:");
}

struct smm_regs {
    unsigned int eax;
    unsigned int ebx __attribute__ ((packed));
    unsigned int ecx __attribute__ ((packed));
    unsigned int edx __attribute__ ((packed));
    unsigned int esi __attribute__ ((packed));
    unsigned int edi __attribute__ ((packed));
};

static int i8k_smm(struct smm_regs *regs)
{
    int rc;
    int eax = regs->eax;


    asm volatile("pushq %%rax\n\t"
            "movl 0(%%rax),%%edx\n\t"
            "pushq %%rdx\n\t"
            "movl 4(%%rax),%%ebx\n\t"
            "movl 8(%%rax),%%ecx\n\t"
            "movl 12(%%rax),%%edx\n\t"
            "movl 16(%%rax),%%esi\n\t"
            "movl 20(%%rax),%%edi\n\t"
            "popq %%rax\n\t"
            "out %%al,$0xb2\n\t"
            "out %%al,$0x84\n\t"
            "xchgq %%rax,(%%rsp)\n\t"
            "movl %%ebx,4(%%rax)\n\t"
            "movl %%ecx,8(%%rax)\n\t"
            "movl %%edx,12(%%rax)\n\t"
            "movl %%esi,16(%%rax)\n\t"
            "movl %%edi,20(%%rax)\n\t"
            "popq %%rdx\n\t"
            "movl %%edx,0(%%rax)\n\t"
            "pushfq\n\t"
            "popq %%rax\n\t"
            "andl $1,%%eax\n"
            :"=a"(rc)
            :    "a"(regs)
            :    "%ebx", "%ecx", "%edx", "%esi", "%edi");

    if (rc != 0 || (regs->eax & 0xffff) == 0xffff || regs->eax == eax)
            return -1;

    return 0;
}


int send(unsigned int cmd, unsigned int arg) {

    struct smm_regs regs = { .eax = cmd, };

    regs.ebx = arg;

    i8k_smm(&regs);
    return regs.eax ;

}

int main(int argc, char **argv) {
    if (geteuid() != 0) {
        perror("need root privileges\n");
        exit(EXIT_FAILURE);
    }
    init_ioperm();
    if(send(GET_FAN,0)==2)
    {
    	return EXIT_SUCCESS;
    }
    send(DISABLE_BIOS_METHOD1,0);
    send(SET_FAN,0x0200);
    send(SET_FAN,0x0200);
	return ((send(GET_FAN,0)) == 2) ? EXIT_FAILURE : EXIT_SUCCESS;
}
