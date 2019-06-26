/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*
    Title: Memories implementation
*/

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <board.h>
#include <pio/pio.h>
#include "board_memories.h"

/*
    Macros:
        READ - Reads a register value. Useful to add trace information to read
               accesses.
        WRITE - Writes data in a register. Useful to add trace information to
                write accesses.
*/
#define READ(peripheral, register)          (peripheral->register)
#define WRITE(peripheral, register, value)  (peripheral->register = value)

//------------------------------------------------------------------------------
//         Local types
//------------------------------------------------------------------------------
typedef struct SDdramConfig
{
	unsigned int ddramc_mdr;
	unsigned int ddramc_cr;
	unsigned int ddramc_rtr;
	unsigned int ddramc_t0pr;
	unsigned int ddramc_t1pr;
	unsigned int ddramc_t2pr;
	unsigned int ddramc_lpr;
} SDdramConfig, *PSDdramConfig;

//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Changes the mapping of the chip so that the remap area mirrors the
///   internal ROM or the EBI CS0.
//------------------------------------------------------------------------------
void BOARD_RemapRom()
{
    WRITE(AT91C_BASE_MATRIX, MATRIX_MRCR, 0);
}

//------------------------------------------------------------------------------
/// Changes the mapping of the chip so that the remap area mirrors the
///   internal RAM.
//------------------------------------------------------------------------------
void BOARD_RemapRam()
{
    WRITE(AT91C_BASE_MATRIX,
          MATRIX_MRCR,
          (AT91C_MATRIX_RCA926I | AT91C_MATRIX_RCA926D));
}

//------------------------------------------------------------------------------
/// Initialize Vdd EBI drive
//------------------------------------------------------------------------------
void BOARD_ConfigureVddMemSel(unsigned char VddMemSel)
{

    if (VddMemSel == VDDMEMSEL_3V3) {
        AT91C_BASE_CCFG->CCFG_EBICSA |= (1 << 16);
        AT91C_BASE_CCFG->CCFG_EBICSA &= ~(1 << 17);
    }
    else {
        AT91C_BASE_CCFG->CCFG_EBICSA &= ~(1 << 16);
        AT91C_BASE_CCFG->CCFG_EBICSA &= ~(1 << 17);
    }
}

static	SDdramConfig ddram_config = {
	.ddramc_mdr  = (AT91C_DDRC2_DBW_16_BITS | AT91C_DDRC2_MD_LP_DDR_SDRAM),

	.ddramc_cr   = (AT91C_DDRC2_NC_DDR10_SDR9  | // 10 column bits (1K)
			AT91C_DDRC2_NR_14          |		 // 14 row bits    (8K)
			AT91C_DDRC2_CAS_3          |     	 // CAS Latency 3
			AT91C_DDRC2_DLL_RESET_DISABLED),	 // DLL not reset

	.ddramc_rtr  = 	0x24B,

	/* Timing values conform to JEDEC Standard No. 209B for LPDDR200 (we have 100MHz SysClk DDR (tcyc=10ns)) */
	.ddramc_t0pr = (AT91C_DDRC2_TRAS_5 | // 5 * 10ns = 50ns
			AT91C_DDRC2_TRCD_3  |     	 // 3 * 10ns = 30ns
			AT91C_DDRC2_TWR_2   |		 // 2 * 10ns = 20ns (JEDEC: 15ns, but is not a multiple of 10ns)
			AT91C_DDRC2_TRC_8   |		 // 8 * 10ns = 80ns (TRAS+TRP)
			AT91C_DDRC2_TRP_3   |		 // 3 * 10ns = 30ns
			AT91C_DDRC2_TRRD_2  |		 // 2 * 10ns = 20ns (JEDEC: 15ns, but is not a multiple of 10ns)
			AT91C_DDRC2_TWTR_1  |		 // 1 clock cycle
			AT91C_DDRC2_TMRD_2),		 // 2 clock cycles

	.ddramc_t1pr = (AT91C_DDRC2_TXP_3 | // 3 * 10ns = 30ns    (JEDEC: 25ns, but is not a multiple of 10ns)
			AT91C_DDRC2_TXSNR_20 |		// 20 * 10 ns = 200ns (according to the AT91SAM9G45 datasheet this corresponds to tXSR for LPDDR)
			AT91C_DDRC2_TXSRD_20 |		// 20 * 10 ns = 200ns (according to the AT91SAM9G45 datasheet ths field is unique for DDR2)
			AT91C_DDRC2_TRFC_14),	    // 14 * 10ns = 140ns  (must be 140 ns for 1Gb LPDDR)

	.ddramc_t2pr = (AT91C_DDRC2_TRTP_2 | //  2 * 10ns = 20ns
			AT91C_DDRC2_TRPA_0   |		 //  0 * 10ns = 0ns
			AT91C_DDRC2_TXARDS_0 |		 //  0 clock cycles
			AT91C_DDRC2_TXARD_0),		 //  0 clock cycles

	.ddramc_lpr = (AT91C_DDRC2_TIMEOUT_64  | // Wait 64 clocks before LP Mode
			AT91C_DDRC2_CLK_FR_SELFREFRESH | // Frozen Clock
			AT91C_DDRC2_LPCB_SELFREFRESH),   // Self Refresh
};

//------------------------------------------------------------------------------
/// Configure DDR
//------------------------------------------------------------------------------
void BOARD_ConfigureDdram(unsigned int ebiAddr, unsigned char ddrModel, unsigned char busWidth)
{    
    AT91PS_HDDRSDRC2 pDdrc = (AT91PS_HDDRSDRC2)ebiAddr;
    volatile unsigned int *pDdr = (unsigned int *) AT91C_DDR2;
    unsigned short ddrc_dbw = 0;
	unsigned int i = 0;
	unsigned int cr = 0;

    switch (busWidth) {
        case 32:
            ddrc_dbw = AT91C_DDRC2_DBW_32_BITS;
            break;
        case 16:
        default:
            ddrc_dbw = AT91C_DDRC2_DBW_16_BITS;
            break;
    }
    
    if (ebiAddr != (unsigned int)AT91C_BASE_DDR2C) {
        pDdr = (unsigned int *) AT91C_EBI_SDRAM;
        AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_CS1A_SDRAMC;
    } else {
        AT91C_BASE_CCFG->CCFG_EBICSA &= (~AT91C_EBI_CS1A_SDRAMC);
    }
    // Enable DDR2 clock x2 in PMC
    WRITE(AT91C_BASE_PMC, PMC_SCER, AT91C_PMC_DDR);

    // Disable anticipated read (needed?)
    //WRITE(pDdrc, HDDRSDRC2_HS, (READ(pDdrc, HDDRSDRC2_HS) | AT91C_DDRC2_NO_ANT));
	
	// Step 1: Program the memory device type.
	// Configure the DDR controller.
	if(DDR2 == ddrModel)
		WRITE(pDdrc, HDDRSDRC2_MDR, ddrc_dbw | AT91C_DDRC2_MD_DDR2_SDRAM);
	else
		WRITE(pDdrc, HDDRSDRC2_MDR, ddrc_dbw | AT91C_DDRC2_MD_LP_DDR_SDRAM);
		
	// Step2: Program the DDR Controller
	WRITE(pDdrc, HDDRSDRC2_CR, ddram_config.ddramc_cr); 
	WRITE(pDdrc, HDDRSDRC2_T0PR, ddram_config.ddramc_t0pr);
	WRITE(pDdrc, HDDRSDRC2_T1PR, ddram_config.ddramc_t1pr);
	WRITE(pDdrc, HDDRSDRC2_T2PR, ddram_config.ddramc_t2pr);

	// Step 3: (LP-DDR): Program Low Power Register
	if(LPDDR == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_LPR, ddram_config.ddramc_lpr);
	}

	// Step 3 (DDR2) / Step 4 (LP-DDR): NOP command -> allow to enable clk.
	WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NOP_CMD);
	*pDdr = 0;

	// ... must wait 200 us (6 core cycles per iteration, core is at 400MHz: min 13334 loops).
	for (i = 0; i < 13340; i++) {
		asm("    nop");
	}

	// Step 4 (DDR2):  An NOP command is issued to the DDR2-SDRAM.
	// NOP command -> allow to enable cke.
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NOP_CMD);
		*pDdr = 0;

		// wait 400 ns min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 5: Set All Bank Precharge.
	WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD);
	*pDdr = 0;

	// wait 400 ns min
	for (i = 0; i < 100; i++) {
		asm("    nop");
	}

	// Step 6 (LP_DDR): Two auto-refresh (CBR) cycles are provided. Program the
	//                  auto refresh command (CBR) into the Mode Register.
	if(LPDDR == ddrModel) {
		// Set 1st CBR.
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
		*pDdr = 0;

		// wait 10 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}

		// Set 2nd CBR.
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
		*pDdr = 0;

		// wait 10 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

    // Step 6 (DDR2) / Step 7 (LPDDR): Set EMR operation (EMRS2).
	WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
	*((unsigned int *)((unsigned char *)pDdr + 0x4000000)) = 0; // Ugly, depends on the SDRAM layout

	// wait 2 cycles min
	for (i = 0; i < 100; i++) {
		asm("    nop");
	}

	// Step 7 (DDR2): Set EMR operation (EMRS3).
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
		*((unsigned int *)((unsigned char *)pDdr + 0x6000000)) = 0; // Ugly, depends on the SDRAM layout

		// wait 2 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 8 (LP-DDR): A mode register set (MRS) cycle is issued to programm the
	//                  parameters of the of the low-power DDR1-SDRAM.
	if(LPDDR == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD);
		*pDdr = 0;

		// wait 2 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 8 (DDR2): Set EMR operation (EMRS1).
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
		*((unsigned int *)((unsigned char *)pDdr + 0x2000000)) = 0;

		// wait 200 cycles min
		for (i = 0; i < 10000; i++) {
			asm("    nop");
		}
	}

	// Step 9 (LP-DDR): A mode normal command is provided. Program the normal mode into mode register.
	if(LPDDR == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NORMAL_CMD);
		*pDdr = 0;
	}

	// Step 9 (DDR2): enable DLL reset.
	if(DDR2 == ddrModel) {
		cr = READ(pDdrc, HDDRSDRC2_CR);
		WRITE(pDdrc, HDDRSDRC2_CR, cr | AT91C_DDRC2_DLL_RESET_ENABLED);
	}

	// Step 10 (LP-DDR): Perform a write access to any low-power DDR-SDRAM address.
	if(LPDDR == ddrModel) {
		*pDdr = 0;
	}

	// Step 10 (DDR2): reset DLL
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
		*pDdr = 0;

		// wait 2 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 11 (DDR2): Set All Bank Precharge
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD);
		*pDdr = 0;

		// wait 400 ns min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 12 (DDR2): Two auto-refresh (CBR) cycles are provided. Program the auto refresh command (CBR)
	//                 into the Mode Register.
	if(DDR2 == ddrModel) {
		// Set 1st CBR.
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
		*pDdr = 0;

		// wait 10 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}

		// Set 2nd CBR.
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
		*pDdr = 0;

		// wait 10 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 13 (DDR2): Program DLL field into the Configuration Register to low(Disable DLL reset).
	if(DDR2 == ddrModel) {
		cr = READ(pDdrc, HDDRSDRC2_CR);
		WRITE(pDdrc, HDDRSDRC2_CR, cr & (~AT91C_DDRC2_DLL_RESET_ENABLED));
	}

	// Step 14 (DDR2): A Mode Register set (MRS) cycle is issued to program the parameters of 
	//                 the DDR2-SDRAM devices.
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD);
		*pDdr = 0;
	}

	// Step 15 (DDR2): Program OCD field into the Configuration Register to high (OCD calibration default).
	if(DDR2 == ddrModel) {
		cr = READ(pDdrc, HDDRSDRC2_CR);
		WRITE(pDdrc, HDDRSDRC2_CR, cr | AT91C_DDRC2_OCD_DEFAULT);
	}

	// Step 16 (DDR2): An Extended Mode Register set (EMRS1) cycle is issued to OCD default value.
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
		*((unsigned int *)((unsigned char *)pDdr + 0x2000000)) = 0;
		// wait 2 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 17 (DDR2): Program OCD field into the Configuration Register to low (OCD calibration mode exit).
	if(DDR2 == ddrModel) {
		cr = READ(pDdrc, HDDRSDRC2_CR);
		WRITE(pDdrc, HDDRSDRC2_CR, cr & (~AT91C_DDRC2_OCD_EXIT));
	}

	// Step 18 (DDR2): An Extended Mode Register set (EMRS1) cycle is issued to enable OCD exit.
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
		*((unsigned int *)((unsigned char *)pDdr + 0x6000000)) = 0;

		// wait 2 cycles min
		for (i = 0; i < 100; i++) {
			asm("    nop");
		}
	}

	// Step 19,20 (DDR2): A mode Normal command is provided. Program the Normal mode into Mode Register.
	if(DDR2 == ddrModel) {
		WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NORMAL_CMD);
		*pDdr = 0;
	}

	// Step 21 (DDR2) / Step 11 (LP-DDR): Write the refresh rate into the count field in the Refresh Timer
	// register. The DDR2-SDRAM device requires a refresh every 15.625 ¦Is or 7.81 ¦Ìs. With a 100MHz
	// frequency, the refresh timer count register must to be set with
	// (15.625 /100 MHz) = 1562 i.e. 0x061A or (7.81 /100MHz) = 781 i.e. 0x030d.

	// Set Refresh timer.
	WRITE(pDdrc, HDDRSDRC2_RTR, ddram_config.ddramc_rtr);

	// OK now we are ready to work on the DDRSDR.

	// wait for end of calibration
	for (i = 0; i < 500; i++) {
		asm("    nop");
	}
}

#if 0
//------------------------------------------------------------------------------
/// Configure DDR on EBI bank 1
//------------------------------------------------------------------------------
void BOARD_ConfigureDdramCp1(unsigned char busWidth)
{    
    AT91PS_HDDRSDRC2 pDdrc = AT91C_BASE_DDR2CP1;
    volatile unsigned int *pDdr = (unsigned int *) AT91C_EBI_SDRAM;
    int i;
    volatile unsigned int cr = 0;
    unsigned short ddrc_dbw = 0;

    // Configure EBI
    AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_CS1A_SDRAMC;
    
    switch (busWidth) {
        case 16:
        default:
            ddrc_dbw = AT91C_DDRC2_DBW_16_BITS;
            break;

        case 32:
            ddrc_dbw = AT91C_DDRC2_DBW_32_BITS;
            break;

    }

    // Enable DDR2 clock x2 in PMC
    WRITE(AT91C_BASE_PMC, PMC_SCER, AT91C_PMC_DDR);

    // Disable anticipated read
    WRITE(pDdrc, HDDRSDRC2_HS, (READ(pDdrc, HDDRSDRC2_HS) | AT91C_DDRC2_NO_ANT));

  
    //Configure DDR_MICRON_MT47H64M8.
    // Step 1: Program the memory device type
    WRITE(pDdrc, HDDRSDRC2_MDR, ddrc_dbw   |
                                AT91C_DDRC2_MD_DDR2_SDRAM);     // DDR2
    
    // Step 2:                            
    // 1. Program the features of DDR2-SDRAM device into the Configuration Register.                    
    // 2. Program the features of DDR2-SDRAM device into the Timing Register HDDRSDRC2_T0PR.            
    // 3. Program the features of DDR2-SDRAM device into the Timing Register HDDRSDRC2_T1PR.            
    // 4. Program the features of DDR2-SDRAM device into the Timing Register HDDRSDRC2_T2PR.            

    WRITE(pDdrc, HDDRSDRC2_CR, AT91C_DDRC2_NC_DDR10_SDR9  |     // 10 column bits (1K)
                               AT91C_DDRC2_NR_14          |     // 14 row bits    (8K)
                               AT91C_DDRC2_CAS_3          |     // CAS Latency 3
                               AT91C_DDRC2_DLL_RESET_DISABLED
                               ); // DLL not reset

    // assume timings for 7.5ns min clock period
    WRITE(pDdrc, HDDRSDRC2_T0PR, AT91C_DDRC2_TRAS_6       |     //  6 * 7.5 = 45 ns
                                 AT91C_DDRC2_TRCD_2       |     //  2 * 7.5 = 15 ns
                                 AT91C_DDRC2_TWR_2        |     //  2 * 7.5 = 15 ns
                                 AT91C_DDRC2_TRC_8        |     //  8 * 7.5 = 75 ns
                                 AT91C_DDRC2_TRP_2        |     //  2 * 7.5 = 15 ns
                                 AT91C_DDRC2_TRRD_1       |     //  1 * 7.5 = 7.5 ns
                                 AT91C_DDRC2_TWTR_1       |     //  1 clock cycle
                                 AT91C_DDRC2_TMRD_2);           //  2 clock cycles

    WRITE(pDdrc, HDDRSDRC2_T1PR, AT91C_DDRC2_TXP_2  |           //  2 * 7.5 = 15 ns
                                 200 << 16          |           // 200 clock cycles, TXSRD: Exit self refresh delay to Read command
                                 16 << 8            |           // 16 * 7.5 = 120 ns TXSNR: Exit self refresh delay to non read command
                                 AT91C_DDRC2_TRFC_14 << 0);     // 14 * 7.5 = 105 ns (must be 105 ns for 512M DDR)

    WRITE(pDdrc, HDDRSDRC2_T2PR, AT91C_DDRC2_TRTP_1   |         //  1 * 7.5 = 7.5 ns
                                 AT91C_DDRC2_TRPA_0   |         
                                 AT91C_DDRC2_TXARDS_7 |         //  7 clock cycles
                                 AT91C_DDRC2_TXARD_2);          //  2 clock cycles

    // Step 3: An NOP command is issued to the DDR2-SDRAM to enable clock.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NOP_CMD);
    *pDdr = 0;

    // Initialization Step 3 (must wait 200 us) (6 core cycles per iteration, core is at 396MHz: min 13200 loops)
    for (i = 0; i < 13300; i++) {
        asm("    nop");
    }
    // Step 4:  An NOP command is issued to the DDR2-SDRAM 
    // NOP command -> allow to enable cke
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NOP_CMD);
    *pDdr = 0;
    
    // wait 400 ns min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }

    // Step 5: An all banks precharge command is issued to the DDR2-SDRAM.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD);
    *pDdr = 0;
    
    // wait 400 ns min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }

    // Step 6: An Extended Mode Register set (EMRS2) cycle is  issued to chose between commercialor high  temperature operations.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
    *((unsigned int *)((unsigned char *)pDdr + 0x4000000)) = 0;
    
    // wait 2 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    
    // Step 7: An Extended Mode Register set (EMRS3) cycle is issued to set all registers to 0.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
    *((unsigned int *)((unsigned char *)pDdr + 0x6000000)) = 0;

    // wait 2 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
     
    // Step 8:  An Extended Mode Register set (EMRS1) cycle is issued to enable DLL.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
    *((unsigned int *)((unsigned char *)pDdr + 0x2000000)) = 0;

    // wait 200 cycles min
    for (i = 0; i < 10000; i++) {
        asm("    nop");
    }
    
    // Step 9:  Program DLL field into the Configuration Register.
    cr = READ(pDdrc, HDDRSDRC2_CR);
    WRITE(pDdrc, HDDRSDRC2_CR, cr | AT91C_DDRC2_DLL_RESET_ENABLED);
    
    // Step 10: A Mode Register set (MRS) cycle is issued to reset DLL.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD);
    *(pDdr) = 0;

    // wait 2 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    
    // Step 11: An all banks precharge command is issued to the DDR2-SDRAM.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD);
    *(pDdr) = 0;

    // wait 400 ns min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }

    // Step 12: Two auto-refresh (CBR) cycles are provided. Program the auto refresh command (CBR) into the Mode Register.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
    *(pDdr) = 0;

    // wait 10 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    // Set 2nd CBR
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD);
    *(pDdr) = 0;

    // wait 10 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    
    // Step 13: Program DLL field into the Configuration Register to low(Disable DLL reset).
    cr = READ(pDdrc, HDDRSDRC2_CR);
    WRITE(pDdrc, HDDRSDRC2_CR, cr & (~AT91C_DDRC2_DLL_RESET_ENABLED));
    
    // Step 14: A Mode Register set (MRS) cycle is issued to program the parameters of the DDR2-SDRAM devices.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD);
    *(pDdr) = 0;

    // Step 15: Program OCD field into the Configuration Register to high (OCD calibration default).
    cr = READ(pDdrc, HDDRSDRC2_CR);
    WRITE(pDdrc, HDDRSDRC2_CR, cr | AT91C_DDRC2_OCD_DEFAULT);
    
    // Step 16: An Extended Mode Register set (EMRS1) cycle is issued to OCD default value.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
    *((unsigned int *)((unsigned char *)pDdr + 0x2000000)) = 0;

    // wait 2 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    
    // Step 17: Program OCD field into the Configuration Register to low (OCD calibration mode exit).
    cr = READ(pDdrc, HDDRSDRC2_CR);
    WRITE(pDdrc, HDDRSDRC2_CR, cr & (~AT91C_DDRC2_OCD_EXIT));
    
    // Step 18: An Extended Mode Register set (EMRS1) cycle is issued to enable OCD exit.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD);
    *((unsigned int *)((unsigned char *)pDdr + 0x6000000)) = 0;

    // wait 2 cycles min
    for (i = 0; i < 100; i++) {
        asm("    nop");
    }
    
    // Step 19,20: A mode Normal command is provided. Program the Normal mode into Mode Register.
    WRITE(pDdrc, HDDRSDRC2_MR, AT91C_DDRC2_MODE_NORMAL_CMD);
    *(pDdr) = 0;

    // Step 21: Write the refresh rate into the count field in the Refresh Timer register. The DDR2-SDRAM device requires a
    // refresh every 15.625 s or 7.81 s. With a 100MHz frequency, the refresh timer count register must to be set with
    // (15.625 /100 MHz) = 1562 i.e. 0x061A or (7.81 /100MHz) = 781 i.e. 0x030d.
  
    // Set Refresh timer
    WRITE(pDdrc, HDDRSDRC2_RTR, 0x00000411);

    // OK now we are ready to work on the DDRSDR

    // wait for end of calibration
    for (i = 0; i < 500; i++) {
        asm("    nop");
    }
}
#endif

//------------------------------------------------------------------------------
/// Initialize and configure the SDRAM
//------------------------------------------------------------------------------
void BOARD_ConfigureSdram(unsigned char busWidth)
{

    // TO BE IMPLEMENTED IN CASE SDRAM IS MOUNTED.

}

//------------------------------------------------------------------------------
/// Configures the EBI for NandFlash access. Pins must be configured after or
/// before calling this function.
//------------------------------------------------------------------------------
void BOARD_ConfigureNandFlash(unsigned char busWidth)
{
    // Configure EBI
    AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_CS3A_SM;

    // Configure SMC
    AT91C_BASE_SMC->SMC_SETUP3 = 0x03030303;
    AT91C_BASE_SMC->SMC_PULSE3 = 0x04040404;
    AT91C_BASE_SMC->SMC_CYCLE3 = 0x00080008;
    AT91C_BASE_SMC->SMC_CTRL3  = 0x00160003;

    if (busWidth == 8) {

        AT91C_BASE_SMC->SMC_CTRL3 |= AT91C_SMC_DBW_WIDTH_EIGTH_BITS;
    }
    else if (busWidth == 16) {
 
        AT91C_BASE_SMC->SMC_CTRL3 |= AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS;
    }
}

//------------------------------------------------------------------------------
/// Configures the EBI for NandFlash access at 48MHz. Pins must be configured
/// after or before calling this function.
//------------------------------------------------------------------------------
void BOARD_ConfigureNandFlash48MHz(unsigned char busWidth)
{
    // Configure EBI
    AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_CS3A_SM;

    // Configure SMC
    AT91C_BASE_SMC->SMC_SETUP3 = 0x00010001;
    AT91C_BASE_SMC->SMC_PULSE3 = 0x04030302;
    AT91C_BASE_SMC->SMC_CYCLE3 = 0x00070004;
    AT91C_BASE_SMC->SMC_CTRL3  = (AT91C_SMC_READMODE
                                 | AT91C_SMC_WRITEMODE
                                 | AT91C_SMC_NWAITM_NWAIT_DISABLE
                                 | ((0x1 << 16) & AT91C_SMC_TDF));
    
    if (busWidth == 8) {

        AT91C_BASE_SMC->SMC_CTRL3 |= AT91C_SMC_DBW_WIDTH_EIGTH_BITS;
    }
    else if (busWidth == 16) {
 
        AT91C_BASE_SMC->SMC_CTRL3 |= AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS;
    }
}

//------------------------------------------------------------------------------
/// Configures the EBI for NorFlash access
/// \Param busWidth Bus width 
//------------------------------------------------------------------------------
void BOARD_ConfigureNorFlash(unsigned char busWidth)
{
    // Configure SMC

    AT91C_BASE_SMC->SMC_SETUP0 = 0x00000002;
    AT91C_BASE_SMC->SMC_PULSE0 = 0x0A0A0A06;
    AT91C_BASE_SMC->SMC_CYCLE0 = 0x000A000A;
    AT91C_BASE_SMC->SMC_CTRL0  = (AT91C_SMC_READMODE
                                  | AT91C_SMC_WRITEMODE
                                  | AT91C_SMC_NWAITM_NWAIT_DISABLE
                                  | ((0x1 << 16) & AT91C_SMC_TDF));

    if (busWidth == 8) {
        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_EIGTH_BITS;
    }
    else if (busWidth == 16) {
        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS;
    }
    else if (busWidth == 32) {
        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_THIRTY_TWO_BITS;
    }
}

//------------------------------------------------------------------------------
/// Configures the EBI for NorFlash access at 48MHz.
/// \Param busWidth Bus width 
//------------------------------------------------------------------------------
void BOARD_ConfigureNorFlash48MHz(unsigned char busWidth)
{
    // Configure SMC
    AT91C_BASE_SMC->SMC_SETUP0 = 0x00000001;
    AT91C_BASE_SMC->SMC_PULSE0 = 0x07070703;
    AT91C_BASE_SMC->SMC_CYCLE0 = 0x00070007;
    AT91C_BASE_SMC->SMC_CTRL0  = (AT91C_SMC_READMODE
                                  | AT91C_SMC_WRITEMODE
                                  | AT91C_SMC_NWAITM_NWAIT_DISABLE
                                  | ((0x1 << 16) & AT91C_SMC_TDF));
                           
    if (busWidth == 8) {

        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_EIGTH_BITS;
    }
    else if (busWidth == 16) {
 
        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS;
    }
    else if (busWidth == 32) {
 
        AT91C_BASE_SMC->SMC_CTRL0 |= AT91C_SMC_DBW_WIDTH_THIRTY_TWO_BITS;
    }
}
