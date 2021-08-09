#define soc_cv_av

#define DEBUG

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"
#include "hps_0.h"


#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 ) //64 MB with 32 bit adress space this is 256 MB
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )


//setting for the HPS2FPGA AXI Bridge
#define ALT_AXI_FPGASLVS_OFST (0xC0000000) // axi_master
#define HW_FPGA_AXI_SPAN (0x40000000) // Bridge span 1GB
#define HW_FPGA_AXI_MASK ( HW_FPGA_AXI_SPAN - 1 )

void RotateMotorCommand(char increase_decrease, char rotation_sens, uint8_t numbers_of_pulses)
{
	/* send 'I' to increase, 'D' to decrease, 'C' to clockwise, 'A' to anticlockwise and numbers of pulses in one sens 
	 *	increase_decrease can be 'I' to increase or 'D' to decrease ;
	 *	rotation_sens can be 'C' to clockwise or 'A' to anticlockwise ;
	 *	numbers_of_pulses is the number of pulses in one sens
	*/
	if(rotation_sens == 'A')
	{
		*(uint32_t *)h2p_lw_SENS_H_addr = 0;
	}
	else if(rotation_sens == 'C')
	{
		*(uint32_t *)h2p_lw_SENS_H_addr = 1;
	}

	for(uint8_t i = 0; i<numbers_of_pulses; i++)
	{	
		if(increase_decrease == 'I')
		{
			*(uint32_t *)h2p_lw_DUTY_INCREASE_addr = 1;
			*(uint32_t *)h2p_lw_DUTY_DECREASE_addr = 0;
		}
		else if(increase_decrease == 'D')
		{
			*(uint32_t *)h2p_lw_DUTY_INCREASE_addr = 0;
			*(uint32_t *)h2p_lw_DUTY_DECREASE_addr = 1;
		}
		usleep(100); //wait 100 micro seconds

		*(uint32_t *)h2p_lw_DUTY_INCREASE_addr = 0;
		*(uint32_t *)h2p_lw_DUTY_DECREASE_addr = 0;
		usleep(100);
	}
}


int main() {

	//pointer to the different address spaces

	void *virtual_base;
	void *axi_virtual_base;
	int fd;


	void *h2p_lw_DUTY_INCREASE_addr;
	void *h2p_lw_DUTY_DECREASE_addr;
	void *h2p_lw_SENS_H_addr;
	//void *h2p_lw_myBus_addr;


//	void *h2p_led_addr; //led via AXI master
	



	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span

	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	//lightweight HPS-to-FPGA bridge
	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	//HPS-to-FPGA bridge
	axi_virtual_base = mmap( NULL, HW_FPGA_AXI_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd,ALT_AXI_FPGASLVS_OFST );

	if( axi_virtual_base == MAP_FAILED ) {
		printf( "ERROR: axi mmap() failed...\n" );
		close( fd );
		return( 1 );
	}


//-----------------------------------------------------------
	//configure the LEDs of the Golden Reference design
	printf( "\n\n\n-----------Set the LEDs on-------------\n\n" );

	//LED connected to the HPS-to-FPGA bridge
	//h2p_led_addr=axi_virtual_base + ( ( unsigned long  )( 0x0 + PIO_LED_BASE ) & ( unsigned long)( HW_FPGA_AXI_MASK ) );

	//*(uint32_t *)h2p_led_addr = 0b10111100;

//-----------------------------------------------------------
	//Adder test: Two registers are connected to a adder and place the result in the third register
	printf( "\n\n\n-----------Add two numbers in the FPGA-------------\n\n" );

	//the address of the two input (reg1 and reg2) registers and the output register (reg3)
	h2p_lw_DUTY_INCREASE_addr=virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + DUTY_INCREASE_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	h2p_lw_DUTY_DECREASE_addr=virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + DUTY_DECREASE_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	h2p_lw_SENS_H_addr=virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SENS_H_BASE ) & ( unsigned long)( HW_REGS_MASK ) );


	//write into register to test the PWM
	printf("Duty increase press running. The motor turns clockwise and the speed increases. \n");
	RotateMotorCommand('I', 'C',10);
	usleep(200); //wait 200 micro seconds
	printf("Duty decrease press running. The motor turns clockwise and the speed decreases. \n");
	RotateMotorCommand('D', 'C',10);
	usleep(1000); //wait 1 milli seconds
	printf("Duty increase press running. The motor turns anticlockwise and the speed increases. \n");
	RotateMotorCommand('I', 'A',10);
	usleep(200); //wait 200 micro seconds
	printf("Duty increase press running. The motor turns anticlockwise and the speed decreases. \n");
	RotateMotorCommand('D', 'A',10);


	//read result of the adder from register 3
//	printf( "Adder result:%d + %d = %d\n", *((uint32_t *)h2p_lw_reg1_addr), *((uint32_t *)h2p_lw_reg2_addr), *((uint32_t *)h2p_lw_reg3_addr) );




	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	if( munmap( axi_virtual_base, HW_FPGA_AXI_SPAN ) != 0 ) {
		printf( "ERROR: axi munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}
