#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "RpiDma.h"
#include "RpiGpio.h"
char InitDma(void *FunctionTerminate)
{
	DMA_CHANNEL=4;
	if (system("rm -f linuxversion.txt") != 0) {
		fprintf(stderr, "rm failed\n");
	}
	if (system("uname -r >> linuxversion.txt") != 0) {
		fprintf(stderr, "uname failed\n");
	}
	char *line = NULL;
	size_t size;
//	int fLinux=open("Flinuxversion.txt", "r');
	FILE * flinux=fopen("linuxversion.txt", "r");
	if (getline(&line, &size, flinux) == -1) 
	{
		printf("Could no get Linux version\n");
	}
	else
	{
		if(line[0]=='3')
		{
			 printf("Wheezy\n");
			 DMA_CHANNEL=DMA_CHANNEL_WHEEZY;
		}
		
		if(line[0]=='4')
		{
			 printf("Jessie\n");
			DMA_CHANNEL=DMA_CHANNEL_JESSIE;
		}

	}
	//printf("Init DMA\n");
	
	// Catch all signals possible - it is vital we kill the DMA engine
	// on process exit!
	int i;
	for (i = 0; i < 64; i++) {
		struct sigaction sa;

		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = FunctionTerminate;//terminate;
		sigaction(i, &sa, NULL);
	}

	//NUM_SAMPLES = NUM_SAMPLES_MAX;

	/* Use the mailbox interface to the VC to ask for physical memory */
	/*
	unlink(MBFILE);
	if (mknod(MBFILE, S_IFCHR|0600, makedev(100, 0)) < 0)
	{
		printf("Failed to create mailbox device\n");
		return 0;
	}*/
	mbox.handle = mbox_open();
	if (mbox.handle < 0)
	{
		printf("Failed to open mailbox\n");
		return(0);
	}
	mbox.mem_ref = mem_alloc(mbox.handle, NUM_PAGES* PAGE_SIZE, PAGE_SIZE, mem_flag);
	/* TODO: How do we know that succeeded? */
	//printf("mem_ref %x\n", mbox.mem_ref);
	mbox.bus_addr = mem_lock(mbox.handle, mbox.mem_ref);
	//printf("bus_addr = %x\n", mbox.bus_addr);
	mbox.virt_addr = mapmem(BUS_TO_PHYS(mbox.bus_addr), NUM_PAGES* PAGE_SIZE);
	//printf("virt_addr %p\n", mbox.virt_addr);
	virtbase = (uint8_t *)((uint32_t *)mbox.virt_addr);
	//printf("virtbase %p\n", virtbase);
	return(1);
	
}

uint32_t
mem_virt_to_phys(volatile void *virt)
{	
	
	//MBOX METHOD
	uint32_t offset = (uint8_t *)virt - mbox.virt_addr;
	return mbox.bus_addr + offset;

}

uint32_t
mem_phys_to_virt(volatile uint32_t phys)
{
	
	//MBOX METHOD
	uint32_t offset=phys-mbox.bus_addr;
	uint32_t result=(uint32_t)((uint8_t *)mbox.virt_addr+offset);
	//printf("MemtoVirt:Offset=%lx phys=%lx -> %lx\n",offset,phys,result);
	return result;
}
