FreeRTOS folder is git clone from official repo.  
Source folder and FreeRTOSConfig.h and other files is used for setting up our Stellaris. Copy these tp your projects if needed

Changes done in original FreeRTOSConfig.h to match with Stellaris
/******************************************************************************/
/* Hardware description related definitions. **********************************/
/******************************************************************************/

/* Update to 80 MHz to match main.c SysCtlClockSet() configuration */
#define configCPU_CLOCK_HZ    ( ( unsigned long ) 80000000 )

/* Set standard tick rate to 1000 Hz (1 ms tick) */
#define configTICK_RATE_HZ    1000


/******************************************************************************/
/* Interrupt nesting behaviour configuration (TM4C123 / Cortex-M4). ***********/
/******************************************************************************/

/* TM4C123 uses 3 priority bits (8 levels: 0-7, left-aligned in NVIC registers).
 * Priority 7 (lowest) = ( 7 << 5 ) = 0xE0 (224).
 * Kernel interrupts (SysTick & PendSV) MUST run at the lowest priority level. */
#define configKERNEL_INTERRUPT_PRIORITY         ( 7 << 5 )

/* Priority 5 = ( 5 << 5 ) = 0xA0. Interrupts at or above priority 5 
 * are never masked by FreeRTOS critical sections. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << 5 )
#define configMAX_API_CALL_INTERRUPT_PRIORITY   ( 5 << 5 )


/******************************************************************************/
/* Map FreeRTOS port exception handlers to standard vector names. *************/
/******************************************************************************/

#define vPortSVCHandler     vPortSVCHandler
#define xPortPendSVHandler  xPortPendSVHandler
#define xPortSysTickHandler xPortSysTickHandler