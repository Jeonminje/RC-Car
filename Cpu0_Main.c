#include "main.h"
#include <asclin.h>
#include "my_stdio.h"
#include "Bluetooth.h"

IfxCpu_syncEvent g_cpuSyncEvent = 0;

int core0_main (void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    /* Module Initialize */
    _init_uart1();
    _init_uart3();
    init_gpt2();
    Init_GPIO();
    Init_Mystdio();
    Init_DCMotors();

    /* Start Code */
    stopChA();
    stopChB();

    unsigned char ch;
    volatile int distance;
    int duty = 20;

    while (1)
    {
        distance = getTofDistance();

        if(distance <= 150 && distance > 100){
            stopChA();
            stopChB();
//            my_printf("distance : %d", distance);
            continue;
        }

        // Car Control
        int result;
        result = _poll_uart3(&ch);

        if(result != 1){
            continue;
        }

        if (ch == 'w' || ch == 'W') // up button
        {
            movChA_PWM(40, 1);
            movChB_PWM(40, 1);
            for(int i=0; i<5000000; i++){
            }
            movChA_PWM(duty, 1);
            movChB_PWM(duty, 1);

            ch = ' ';
        }
        else if (ch == 'd' || ch == 'D') // right button
        {
            movChA_PWM(duty + 20, 1);
            movChB_PWM(duty - 10, 1);

            ch = ' ';
        }
        else if (ch == 'a' || ch == 'A') // left button
        {
            movChA_PWM(duty - 10, 1);
            movChB_PWM(duty + 20, 1);

            ch = ' ';
        }
        else if (ch == 's' || ch == 'S') // down button
        {
            movChA_PWM(40, 0);
            movChB_PWM(40, 0);
            for(int i=0; i<5000000; i++){

            }
            movChA_PWM(duty, 0);
            movChB_PWM(duty, 0);

            ch = ' ';
        }
        else if (ch == 'b' || ch == 'B') // down button
        {
            stopChA();
            stopChB();

            ch = ' ';
        }
        else if (ch == 'g' || ch == 'G') // start
        {
            movChA_PWM(40, 1);
            movChB_PWM(40, 1);

            ch = ' ';
        }
        else if (ch == 'c' || ch == 'C') // right button
        {
            movChA_PWM(duty + 20, 0);
            movChB_PWM(duty - 10, 0);

            ch = ' ';
        }
        else if (ch == 'z' || ch == 'Z') // left button
        {
            movChA_PWM(duty - 10, 0);
            movChB_PWM(duty + 20, 0);

            ch = ' ';
        }

    }
    return 0;

}

