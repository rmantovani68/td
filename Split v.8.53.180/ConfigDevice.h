//----------------------------------------------------------------------------------
//	Progect name:	DefinePeriferiche.h
//	Device:			PIC24FJ64A002 @ 20Mhz
//	Autor:			TDM Team
//	Date:			23/01/2011
//	Description:	Header con le impostazioni del dispositivo
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif

//#ifndef _CONFIG_DEVICE_
	//	#define _CONFIG_DEVICE_
	//----------------------------------------------------------------------------------
	//	Config word
	//----------------------------------------------------------------------------------
	/*
	**
	** _CONFIG1( OPT1_ON & OPT2_OFF & OPT3_PLL )
	**
	**   Watchdog Timer Postscaler:
	**     WDTPS_PS1            1:1
	**     WDTPS_PS2            1:2
	**     WDTPS_PS4            1:4
	**     WDTPS_PS8            1:8
	**     WDTPS_PS16           1:16
	**     WDTPS_PS32           1:32
	**     WDTPS_PS64           1:64
	**     WDTPS_PS128          1:128
	**     WDTPS_PS256          1:256
	**     WDTPS_PS512          1:512
	**     WDTPS_PS1024         1:1,024
	**     WDTPS_PS2048         1:2,048
	**     WDTPS_PS4096         1:4,096
	**     WDTPS_PS8192         1:8,192
	**     WDTPS_PS16384        1:16,384
	**     WDTPS_PS32768        1:32,768
	**
	**   WDT Prescaler:
	**     FWPSA_PR32           Prescaler ratio of 1:32
	**     FWPSA_PR128          Prescaler ratio of 1:128
	**
	**   Watchdog Timer Window:
	**     WINDIS_ON            Windowed Watchdog Timer enabled; FWDTEN must be 1
	**     WINDIS_OFF           Standard Watchdog Timer enabled,(Windowed-mode is disabled)
	**
	**   Watchdog Timer Enable:
	**     FWDTEN_OFF           Watchdog Timer is disabled
	**     FWDTEN_ON            Watchdog Timer is enabled
	**
	**   Comm Channel Select:
	**     ICS_PGx3             Emulator functions are shared with PGEC3/PGED3
	**     ICS_PGx2             Emulator functions are shared with PGEC2/PGED2
	**     ICS_PGx1             Emulator functions are shared with PGEC1/PGED1
	**
	**   Set Clip On Emulation Mode:
	**     COE_ON               Enabled
	**     COE_OFF              Disabled
	**
	**   Background Debug:
	**     BKBUG_ON             Device resets into Debug mode
	**     BKBUG_OFF            Device resets into Operational mode
	**
	**   General Code Segment Write Protect:
	**     GWRP_ON              Writes to program memory are disabled
	**     GWRP_OFF             Writes to program memory are allowed
	**
	**   General Code Segment Code Protect:
	**     GCP_ON               Code protection is enabled for the entire program memory space
	**     GCP_OFF              Code protection is disabled
	**
	**   JTAG Port Enable:
	**     JTAGEN_OFF           JTAG port is disabled
	**     JTAGEN_ON            JTAG port is enabled
	**
	**********************************************************************************************
	**
	** _CONFIG2( OPT1_ON & OPT2_OFF & OPT3_PLL )
	**
	**   Primary Oscillator Select:
	**     POSCMOD_EC           EC oscillator mode selected
	**     POSCMOD_XT           XT oscillator mode selected
	**     POSCMOD_HS           HS oscillator mode selected
	**     POSCMOD_NONE         Primary oscillator disabled
	**
	**   Internal USB 3.3V Regulator Disable bit:
	**     DISUVREG_ON          Regulator is enabled
	**     DISUVREG_OFF         Regulator is disabled
	**
	**   IOLOCK One-Way Set Enable bit:
	**     IOL1WAY_OFF          Unlimited Writes To RP Registers
	**     IOL1WAY_ON           Write RP Registers Once
	**
	**   Primary Oscillator Output Function:
	**     OSCIOFNC_ON          OSCO functions as port I/O (RC15)
	**     OSCIOFNC_OFF         OSCO functions as CLKO (FOSC/2)
	**
	**   Clock Switching and Monitor:
	**     FCKSM_CSECME         Both Clock switching and Fail-safe Clock Monitor are enabled
	**     FCKSM_CSECMD         Clock switching is enabled, Fail-safe Clock Monitor is disabled
	**     FCKSM_CSDCMD         Both Clock Switching and Fail-safe Clock Monitor are disabled
	**
	**   Oscillator Select:
	**     FNOSC_FRC            Fast RC Oscillator (FRC)
	**     FNOSC_FRCPLL         Fast RC oscillator with Postscaler and PLL module (FRCPLL)
	**     FNOSC_PRI            Primary oscillator (XT, HS, EC)
	**     FNOSC_PRIPLL         Primary oscillator (XT, HS, EC) with PLL module (XTPLL,HSPLL, ECPLL)
	**     FNOSC_SOSC           Secondary oscillator (SOSC)
	**     FNOSC_LPRC           Low-Power RC oscillator (LPRC)
	**     FNOSC_FRCDIV         Fast RC oscillator with Postscaler (FRCDIV)
	**
	**   96MHz PLL Disable:
	**     PLL_96MHZ_ON         Enabled
	**
	**   USB 96 MHz PLL Prescaler Select bits:
	**     PLLDIV_NODIV         Oscillator input used directly (4MHz input)
	**     PLLDIV_DIV2          Oscillator input divided by 2 (8MHz input)
	**     PLLDIV_DIV3          Oscillator input divided by 3 (12MHz input)
	**     PLLDIV_DIV4          Oscillator input divided by 4 (16MHz input)
	**     PLLDIV_DIV5          Oscillator input divided by 5 (20MHz input)
	**     PLLDIV_DIV6          Oscillator input divided by 6 (24MHz input)
	**     PLLDIV_DIV10         Oscillator input divided by 10 (40MHz input)
	**     PLLDIV_DIV12         Oscillator input divided by 12 (48MHz input)
	**
	**   Internal External Switch Over Mode:
	**     IESO_OFF             IESO mode (Two-speed start-up)disabled
	**     IESO_ON              IESO mode (Two-speed start-up) enabled
	**
	**********************************************************************************************
	**
	** _CONFIG3( OPT1_ON & OPT2_OFF & OPT3_PLL )
	**
	**   Write Protection Flash Page Segment Boundary:
	**     WPFP_WPFP0           Page 0 (0x00)
	**     WPFP_WPFP1           Page 1 (0x400)
	**     WPFP_WPFP2           Page 2 (0x800)
	**     WPFP_WPFP3           Page 3 (0xC00)
	**     WPFP_WPFP4           Page 4 (0x1000)
	**     WPFP_WPFP5           Page 5 (0x1400)
	**     WPFP_WPFP6           Page 6 (0x1800)
	**     WPFP_WPFP7           Page 7 (0x1C00)
	**     WPFP_WPFP8           Page 8 (0x2000)
	**     WPFP_WPFP9           Page 9 (0x2400)
	**     WPFP_WPFP10          Page 10 (0x2800)
	**     WPFP_WPFP11          Page 11 (0x2C00)
	**     WPFP_WPFP12          Page 12 (0x3000)
	**     WPFP_WPFP13          Page 13 (0x3400)
	**     WPFP_WPFP14          Page 14 (0x3800)
	**     WPFP_WPFP15          Page 15 (0x3C00)
	**     WPFP_WPFP16          Page 16 (0x4000)
	**     WPFP_WPFP17          Page 17 (0x4400)
	**     WPFP_WPFP18          Page 18 (0x4800)
	**     WPFP_WPFP19          Page 19 (0x4C00)
	**     WPFP_WPFP20          Page 20 (0x5000)
	**     WPFP_WPFP21          Page 21 (0x5400)
	**     WPFP_WPFP22          Page 22 (0x5800)
	**     WPFP_WPFP23          Page 23 (0x5C00)
	**     WPFP_WPFP24          Page 24 (0x6000)
	**     WPFP_WPFP25          Page 25 (0x6400)
	**     WPFP_WPFP26          Page 26 (0x6800)
	**     WPFP_WPFP27          Page 27 (0x6C00)
	**     WPFP_WPFP28          Page 28 (0x7000)
	**     WPFP_WPFP29          Page 29 (0x7400)
	**     WPFP_WPFP30          Page 30 (0x7800)
	**     WPFP_WPFP31          Page 31 (0x7C00)
	**     WPFP_WPFP32          Page 32 (0x8000)
	**     WPFP_WPFP33          Page 33 (0x8400)
	**     WPFP_WPFP34          Page 34 (0x8800)
	**     WPFP_WPFP35          Page 35 (0x8C00)
	**     WPFP_WPFP36          Page 36 (0x9000)
	**     WPFP_WPFP37          Page 37 (0x9400)
	**     WPFP_WPFP38          Page 38 (0x9800)
	**     WPFP_WPFP39          Page 39 (0x9C00)
	**     WPFP_WPFP40          Page 40 (0xA000)
	**     WPFP_WPFP41          Page 41 (0xA400)
	**     WPFP_WPFP42          Page 42 (0xA800)
	**     WPFP_WPFP43          Page 43 (0xAC00)
	**     WPFP_WPFP44          Page 44 (0xB000)
	**     WPFP_WPFP45          Page 45 (0xB400)
	**     WPFP_WPFP46          Page 46 (0xB800)
	**     WPFP_WPFP47          Page 47 (0xBC00)
	**     WPFP_WPFP48          Page 48 (0xC000)
	**     WPFP_WPFP49          Page 49 (0xC400)
	**     WPFP_WPFP50          Page 50 (0xC800)
	**     WPFP_WPFP51          Page 51 (0xCC00)
	**     WPFP_WPFP52          Page 52 (0xD000)
	**     WPFP_WPFP53          Page 53 (0xD400)
	**     WPFP_WPFP54          Page 54 (0xD800)
	**     WPFP_WPFP55          Page 55 (0xDC00)
	**     WPFP_WPFP56          Page 56 (0xE000)
	**     WPFP_WPFP57          Page 57 (0xE400)
	**     WPFP_WPFP58          Page 58 (0xE800)
	**     WPFP_WPFP59          Page 59 (0xEC00)
	**     WPFP_WPFP60          Page 60 (0xF000)
	**     WPFP_WPFP61          Page 61 (0xF400)
	**     WPFP_WPFP62          Page 62 (0xF800)
	**     WPFP_WPFP63          Page 63 (0xFC00)
	**     WPFP_WPFP64          Page 64 (0x10000)
	**     WPFP_WPFP65          Page 65 (0x10400)
	**     WPFP_WPFP66          Page 66 (0x10800)
	**     WPFP_WPFP67          Page 67 (0x10C00)
	**     WPFP_WPFP68          Page 68 (0x11000)
	**     WPFP_WPFP69          Page 69 (0x11400)
	**     WPFP_WPFP70          Page 70 (0x11800)
	**     WPFP_WPFP71          Page 71 (0x11C00)
	**     WPFP_WPFP72          Page 72 (0x12000)
	**     WPFP_WPFP73          Page 73 (0x12400)
	**     WPFP_WPFP74          Page 74 (0x12800)
	**     WPFP_WPFP75          Page 75 (0x12C00)
	**     WPFP_WPFP76          Page 76 (0x13000)
	**     WPFP_WPFP77          Page 77 (0x13400)
	**     WPFP_WPFP78          Page 78 (0x13800)
	**     WPFP_WPFP79          Page 79 (0x13C00)
	**     WPFP_WPFP80          Page 80 (0x14000)
	**     WPFP_WPFP81          Page 81 (0x14400)
	**     WPFP_WPFP82          Page 82 (0x14800)
	**     WPFP_WPFP83          Page 83 (0x14C00)
	**     WPFP_WPFP84          Page 84 (0x15000)
	**     WPFP_WPFP85          Page 85 (0x15400)
	**     WPFP_WPFP86          Page 86 (0x15800)
	**     WPFP_WPFP87          Page 87 (0x15C00)
	**     WPFP_WPFP88          Page 88 (0x16000)
	**     WPFP_WPFP89          Page 89 (0x16400)
	**     WPFP_WPFP90          Page 90 (0x16800)
	**     WPFP_WPFP91          Page 91 (0x16C00)
	**     WPFP_WPFP92          Page 92 (0x17000)
	**     WPFP_WPFP93          Page 93 (0x17400)
	**     WPFP_WPFP94          Page 94 (0x17800)
	**     WPFP_WPFP95          Page 95 (0x17C00)
	**     WPFP_WPFP96          Page 96 (0x18000)
	**     WPFP_WPFP97          Page 97 (0x18400)
	**     WPFP_WPFP98          Page 98 (0x18800)
	**     WPFP_WPFP99          Page 99 (0x18C00)
	**     WPFP_WPFP100         Page 100 (0x19000)
	**     WPFP_WPFP101         Page 101 (0x19400)
	**     WPFP_WPFP102         Page 102 (0x19800)
	**     WPFP_WPFP103         Page 103 (0x19C00)
	**     WPFP_WPFP104         Page 104 (0x1A000)
	**     WPFP_WPFP105         Page 105 (0x1A400)
	**     WPFP_WPFP106         Page 106 (0x1A800)
	**     WPFP_WPFP107         Page 107 (0x1AC00)
	**     WPFP_WPFP108         Page 108 (0x1B000)
	**     WPFP_WPFP109         Page 109 (0x1B400)
	**     WPFP_WPFP110         Page 110 (0x1B800)
	**     WPFP_WPFP111         Page 111 (0x1BC00)
	**     WPFP_WPFP112         Page 112 (0x1C000)
	**     WPFP_WPFP113         Page 113 (0x1C400)
	**     WPFP_WPFP114         Page 114 (0x1C800)
	**     WPFP_WPFP115         Page 115 (0x1CC00)
	**     WPFP_WPFP116         Page 116 (0x1D000)
	**     WPFP_WPFP117         Page 117 (0x1D400)
	**     WPFP_WPFP118         Page 118 (0x1D800)
	**     WPFP_WPFP119         Page 119 (0x1DC00)
	**     WPFP_WPFP120         Page 120 (0x1E000)
	**     WPFP_WPFP121         Page 121 (0x1E400)
	**     WPFP_WPFP122         Page 122 (0x1E800)
	**     WPFP_WPFP123         Page 123 (0x1EC00)
	**     WPFP_WPFP124         Page 124 (0x1F000)
	**     WPFP_WPFP125         Page 125 (0x1F400)
	**     WPFP_WPFP126         Page 126 (0x1F800)
	**     WPFP_WPFP127         Page 127 (0x1FC00)
	**     WPFP_WPFP128         Page 128 (0x20000)
	**     WPFP_WPFP129         Page 129 (0x20400)
	**     WPFP_WPFP130         Page 130 (0x20800)
	**     WPFP_WPFP131         Page 131 (0x20C00)
	**     WPFP_WPFP132         Page 132 (0x21000)
	**     WPFP_WPFP133         Page 133 (0x21400)
	**     WPFP_WPFP134         Page 134 (0x21800)
	**     WPFP_WPFP135         Page 135 (0x21C00)
	**     WPFP_WPFP136         Page 136 (0x22000)
	**     WPFP_WPFP137         Page 137 (0x22400)
	**     WPFP_WPFP138         Page 138 (0x22800)
	**     WPFP_WPFP139         Page 139 (0x22C00)
	**     WPFP_WPFP140         Page 140 (0x23000)
	**     WPFP_WPFP141         Page 141 (0x23400)
	**     WPFP_WPFP142         Page 142 (0x23800)
	**     WPFP_WPFP143         Page 143 (0x23C00)
	**     WPFP_WPFP144         Page 144 (0x24000)
	**     WPFP_WPFP145         Page 145 (0x24400)
	**     WPFP_WPFP146         Page 146 (0x24800)
	**     WPFP_WPFP147         Page 147 (0x24C00)
	**     WPFP_WPFP148         Page 148 (0x25000)
	**     WPFP_WPFP149         Page 149 (0x25400)
	**     WPFP_WPFP150         Page 150 (0x25800)
	**     WPFP_WPFP151         Page 151 (0x25C00)
	**     WPFP_WPFP152         Page 152 (0x26000)
	**     WPFP_WPFP153         Page 153 (0x26400)
	**     WPFP_WPFP154         Page 154 (0x26800)
	**     WPFP_WPFP155         Page 155 (0x26C00)
	**     WPFP_WPFP156         Page 156 (0x27000)
	**     WPFP_WPFP157         Page 157 (0x27400)
	**     WPFP_WPFP158         Page 158 (0x27800)
	**     WPFP_WPFP159         Page 159 (0x27C00)
	**     WPFP_WPFP160         Page 160 (0x28000)
	**     WPFP_WPFP161         Page 161 (0x28400)
	**     WPFP_WPFP162         Page 162 (0x28800)
	**     WPFP_WPFP163         Page 163 (0x28C00)
	**     WPFP_WPFP164         Page 164 (0x29000)
	**     WPFP_WPFP165         Page 165 (0x29400)
	**     WPFP_WPFP166         Page 166 (0x29800)
	**     WPFP_WPFP167         Page 167 (0x29C00)
	**     WPFP_WPFP168         Page 168 (0x2A000)
	**     WPFP_WPFP169         Page 169 (0x2A400)
	**     WPFP_WPFP170         Page 170 (0x2A800)
	**     WPFP_WPFP511         Highest Page (same as page 170)
	**
	**   Segment Write Protection Disable bit:
	**     WPDIS_WPEN           Segmented code protection enabled
	**     WPDIS_WPDIS          Segmented code protection disabled
	**
	**   Configuration Word Code Page Protection Select bit:
	**     WPCFG_WPCFGEN        Last page and Flash configuration words are code-protected
	**     WPCFG_WPCFGDIS       Last page(at the top of program memory) and Flash configuration words are not protected
	**
	**   Segment Write Protection End Page Select bit:
	**     WPEND_WPSTARTMEM     Write Protect from page 0 to WPFP
	**     WPEND_WPENDMEM       Write Protect from WPFP to the last page of memory
	**
*/

#if (GB210==1 || DA210==1)


/* CONFIG1 */
#pragma config WDTPS = PS32768              // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128                // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config ALTVREF = ALTVREDIS          // Alternate VREF location Enable (VREF is on a default pin (VREF+ on RA9 and VREF- on RA10))
#pragma config WINDIS = OFF                 // Windowed WDT (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF                 // Watchdog Timer (Watchdog Timer is disabled)
#pragma config ICS = PGx2                   // Emulator Pin Placement Select bits (Emulator functions are shared with PGEC2/PGED2)
#pragma config GWRP = OFF                   // General Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = ON                     // General Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF                 // JTAG Port Enable (JTAG port is disabled)

//	_CONFIG1(	WDTPS_PS32768 &     /* Watchdog Timer Postscaler  */
//				FWPSA_PR128 &       /* WDT Prescaler */
//                ALTVREF_ALTVREDIS & /* Alternate VREF location Enable - <<<< GB210 Only >>>> */
//				WINDIS_OFF &        /* Windowed WDT - Watchdog Timer Window */
//				FWDTEN_OFF &        /* Watchdog Timer Enable */
//				ICS_PGx2 &          /* Emulator Pin Placement Select bits */
//				/*COE_OFF &         *//* Set Clip On Emulation Mode - ---- GB110 Only ---- */
//				/*BKBUG_OFF &       *//* Background Debug - ---- GB110 Only ---- */
//				GWRP_OFF &          /* General Code Segment Write Protect */
//				/*GCP_OFF & 		*//* Code protection is disabled */
//				GCP_ON &            /* Code protection is enabled for the entire program memory space */
//				JTAGEN_OFF);        /* JTAG port is disabled */


// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator is disabled)
#pragma config IOL1WAY = OFF            // IOLOCK One-Way Set Enable (The IOLOCK bit can be set and cleared as needed, provided the unlock sequence has been completed)
#pragma config OSCIOFNC = ON            // OSCO Pin Configuration (OSCO/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSDCMD           // Clock Switching and Fail-Safe Clock Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRCPLL           // Initial Oscillator Select (Fast RC Oscillator with Postscaler and PLL module (FRCPLL))
#pragma config PLL96MHZ = ON            // 96MHz PLL Startup Select (96 MHz PLL is enabled automatically on start-up)
#pragma config PLLDIV = NODIV           // 96 MHz PLL Prescaler Select (Oscillator input is used directly (4 MHz input))
#pragma config IESO = OFF               // Internal External Switchover (IESO mode (Two-Speed Start-up) is disabled)

//	_CONFIG2(	POSCMOD_NONE &      /* Primary Oscillator Select */
//				/*DISUVREG_OFF &    *//* Internal USB 3.3V Regulator Disable bit - ---- GB110 Only ---- */
//				IOL1WAY_OFF &       /* IOLOCK One-Way Set Enable */
//				OSCIOFNC_ON &       /* OSCO Pin Configuration */
//				FCKSM_CSDCMD &      /* Clock Switching and Fail-Safe Clock Monitor */
//				FNOSC_FRCPLL &      /* Initial Oscillator Select */
//				/*PLL_96MHZ_ON &    *//* 96MHz PLL Startup Select - ---- GB110 Only ---- */
//                /*PLL96MHZ_ON &     *//* 96MHz PLL Startup Select - <<<< GB210 Only >>>> */
//                //*PLL96MHZ_OFF &      *//* 96MHz PLL Startup Select - <<<< GB210 Only >>>> */
//				PLLDIV_NODIV &      /* 96 MHz PLL Prescaler Select */
//				IESO_OFF);          /* Internal External Switch Over Mode */

// CONFIG3
#pragma config WPFP = WPFP255           // Write Protection Flash Page Segment Boundary (Highest Page (same as page 170))
#pragma config SOSCSEL = SOSC           // Secondary Oscillator Power Mode Select (Secondary oscillator is in Default (high drive strength) Oscillator mode)
#pragma config WUTSEL = LEG             // Voltage Regulator Wake-up Time Select (Default regulator start-up time is used)
#pragma config ALTPMP = ALPMPDIS        // Alternate PMP Pin Mapping (EPMP pins are in default location mode)
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable (Segmented code protection is disabled)
#pragma config WPCFG = WPCFGDIS         // Write Protect Configuration Page Select (Last page (at the top of program memory) and Flash Configuration Words are not write-protected)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select (Protected code segment upper boundary is at the last page of program memory; the lower boundary is the code page specified by WPFP)


//	_CONFIG3(	WPFP_WPFP255 &      /* Write Protection Flash Page Segment Boundary - <<<< GB210 Only >>>> */
//                /*WPFP_WPFP511 &    *//* Write Protection Flash Page Segment Boundary - ---- GB110 Only ---- */
//				SOSCSEL_SOSC &      /* Secondary Oscillator Power Mode Select - <<<< GB210 Only >>>> */
//                WUTSEL_LEG &        /* Voltage Regulator Wake-up Time Select - <<<< GB210 Only >>>> */
//                ALTPMP_ALPMPDIS &   /* Alternate PMP Pin Mapping - <<<< GB210 Only >>>> */
//                WPDIS_WPDIS &		/* Segment Write Protection Disable */
//				WPCFG_WPCFGDIS &	/* Write Protect Configuration Page Select */
//				WPEND_WPENDMEM);    /* Segment Write Protection End Page Select */

//#endif
#else
	_CONFIG1(	WDTPS_PS32768 &     /* Watchdog Timer Postscaler  */
				FWPSA_PR128 &       /* WDT Prescaler */
                /*ALTVREF_ALTVREDIS & *//* Alternate VREF location Enable - <<<< GB210 Only >>>> */
				WINDIS_OFF &        /* Windowed WDT - Watchdog Timer Window */
				FWDTEN_OFF &        /* Watchdog Timer Enable */
				ICS_PGx2 &          /* Emulator Pin Placement Select bits */
				/*COE_OFF &         /* Set Clip On Emulation Mode - ---- GB110 Only ---- */
				BKBUG_OFF &       /* Background Debug - ---- GB110 Only ---- */
				GWRP_OFF &          /* General Code Segment Write Protect */
				/*GCP_OFF & 		*//* Code protection is disabled */
				GCP_ON &            /* Code protection is enabled for the entire program memory space */
				JTAGEN_OFF);        /* JTAG port is disabled */



	_CONFIG2(	POSCMOD_NONE &      /* Primary Oscillator Select */
				DISUVREG_OFF &    /* Internal USB 3.3V Regulator Disable bit - ---- GB110 Only ---- */
				IOL1WAY_OFF &       /* IOLOCK One-Way Set Enable */
				OSCIOFNC_ON &       /* OSCO Pin Configuration */
				FCKSM_CSDCMD &      /* Clock Switching and Fail-Safe Clock Monitor */
				FNOSC_FRCPLL &      /* Initial Oscillator Select */
				/*PLL_96MHZ_ON &    *//* 96MHz PLL Startup Select - ---- GB110 Only ---- */
                /*PLL96MHZ_ON &     *//* 96MHz PLL Startup Select - <<<< GB210 Only >>>> */
                /*PLL96MHZ_OFF &      *//* 96MHz PLL Startup Select - <<<< GB210 Only >>>> */
				PLLDIV_NODIV &      /* 96 MHz PLL Prescaler Select */
				IESO_OFF);          /* Internal External Switch Over Mode */



	_CONFIG3(	/*WPFP_WPFP255 &      *//* Write Protection Flash Page Segment Boundary - <<<< GB210 Only >>>> */
                WPFP_WPFP511 &    /* Write Protection Flash Page Segment Boundary - ---- GB110 Only ---- */
				/*SOSCSEL_SOSC &      *//* Secondary Oscillator Power Mode Select - <<<< GB210 Only >>>> */
                /*WUTSEL_LEG &        *//* Voltage Regulator Wake-up Time Select - <<<< GB210 Only >>>> */
                /*ALTPMP_ALPMPDIS &   *//* Alternate PMP Pin Mapping - <<<< GB210 Only >>>> */
                WPDIS_WPDIS &		/* Segment Write Protection Disable */
				WPCFG_WPCFGDIS &	/* Write Protect Configuration Page Select */
				WPEND_WPENDMEM);    /* Segment Write Protection End Page Select */
#endif    
