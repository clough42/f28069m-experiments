#include "DSP28x_Project.h"

//
// Included Files
//
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "SPI_ControlPanel.h"
#include "StepperDrive.h"

//
// Function Prototypes statements for functions found within this file.
//
__interrupt void cpu_timer0_isr(void);

//
// Main
//
void main(void)
{
    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2806x_SysCtrl.c file.
    //
    InitSysCtrl();

    //
    // Step 2. Initalize GPIO:
    // This example function is found in the F2806x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    // InitGpio();  // Skipped for this example

    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2806x_PieCtrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2806x_DefaultIsr.c.
    // This function is found in F2806x_PieVect.c.
    //
    InitPieVectTable();

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;    // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;      // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. Initialize the Device Peripheral. This function can be
    //         found in F2806x_CpuTimers.c
    //
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

    //
    // Configure CPU-Timer 0 to interrupt every 500 milliseconds:
    // 80MHz CPU Freq, 50 millisecond Period (in uSeconds)
    //
    ConfigCpuTimer(&CpuTimer0, 90, STEPPER_CYCLE_US);

    //
    // To ensure precise timing, use write-only instructions to write to the
    // entire register. Therefore, if any of the configuration bits are changed
    // in ConfigCpuTimer and InitCpuTimers (in F2806x_CpuTimers.h), the
    // below settings must also be updated.
    //

    //
    // Use write-only instruction to set TSS bit = 0
    //
    CpuTimer0Regs.TCR.all = 0x4001;

    //
    // Set up SPI for control panel
    //
    InitControlPanel();

    //
    // Set up GPIO and state machine for stepper drive
    //
    StepperDrive_Init();

    // GPIO 9 as an indicator
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
    EDIS;

    //
    // Step 5. User specific code, enable interrupts:
    //

    //
    // Enable CPU INT1 which is connected to CPU-Timer 0
    //
    IER |= M_INT1;

    //
    // Enable TINT0 in the PIE: Group 1 interrupt 7
    //
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    //
    // Enable global Interrupts and higher priority real-time debug events
    //
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

    //
    // Step 6. IDLE loop. Just sit and loop forever (optional)
    //

    Uint16 data[] = {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7
    };
    Uint16 keys = 0xff;

    for(;;) {
        SendControlPanelData(data, keys);

        keys = ReadKeys();

        //
        // Respond to keypresses
        //
        if( keys & 0x01 ) {
            StepperDrive_SetDesiredPosition(4);
        }
        if( keys & 0x02 ) {
            StepperDrive_SetDesiredPosition(3);
        }
        if( keys & 0x04 ) {
            StepperDrive_SetDesiredPosition(2);
        }
        if( keys & 0x08 ) {
            StepperDrive_SetDesiredPosition(1);
        }
        if( keys & 0x10 ) {
            StepperDrive_SetDesiredPosition(0);
        }
        if( keys & 0x20 ) {
            StepperDrive_SetDesiredPosition(-1);
        }
        if( keys & 0x40 ) {
            StepperDrive_SetDesiredPosition(-2);
        }
        if( keys & 0x80 ) {
            StepperDrive_SetDesiredPosition(-3);
        }

        DELAY_US(10000); // update at 100Hz-ish

    };
}


//
// cpu_timer0_isr -
//
__interrupt void
cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    // flag entrance to ISR for timing
    GpioDataRegs.GPASET.bit.GPIO9 = 1;

    //
    // Service the stepper driver
    //
    StepperDrive_Service_ISR();

    // flag exit from ISR for timing
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// End of File
//

