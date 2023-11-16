#include "main.h"
#include <asclin.h>
#include "my_stdio.h"
#include "Bluetooth.h"

IfxCpu_syncEvent g_cpuSyncEvent = 0;
int parkingCount = 0;

void goStraight(int duty, int dir);
void turnLeft(int duty, int dir);
void turnRight(int duty, int dir);
int checkBack();
int checkFront();
int checkParking();

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
    Init_Ultrasonics();
    Init_Buzzer();
    Init_Bluetooth();

    /* Start Code */
    stopChA();
    stopChB();

    unsigned char ch = ' ';
    unsigned char prev_ch = ' ';
    int duty = 35;
    int frontObject = 0;
    int backObject = 0;
    setBeepCycle(0);

    while (1)
    {
        char ch = getBluetoothByte_nonBlocked();
        if (ch >= 0)
        {
            setBluetoothByte_Blocked(ch);
            _out_uart3(ch);
        }
        // 후진을 할 상황에만 후방 초음파 체크
        if (prev_ch == 's' || prev_ch == 'b' || prev_ch == 'z' || prev_ch == 'c' || prev_ch == 'p')
        {
            backObject = checkBack();
        }

        if (prev_ch == 'p')
        {
            if(checkParking() == 1){
                prev_ch = 's'; // 후진 이라는 뜻, 인식되면 멈추려고
                turnLeft(duty - 5, 0);
                delay_ms(1800);         // 90도 회전
                goStraight(duty - 15, 0);

                parkingCount = 0;
            }
        }

        // 전진을 할 상황에만 전방 센서 체크
        if (prev_ch == 'w' || prev_ch == 'a' || prev_ch == 'd')
        {
            frontObject = checkFront();
        }

        if(ch < 0){
            continue;
        }

        // Car Control
        if (ch == 'w' || ch == 'W') // up button
        {
            setBeepCycle(0);

            if(frontObject == 0){
                goStraight(duty, 1);
            }

            ch = ' ';
            prev_ch = 'w';
        }
        else if (ch == 'd' || ch == 'D') // right button
        {
            setBeepCycle(0);

            if(frontObject == 0){
                turnRight(duty, 1);
            }

            ch = ' ';
            prev_ch = 'd';
        }
        else if (ch == 'a' || ch == 'A') // left button
        {
            setBeepCycle(0);

            if(frontObject == 0){
                turnLeft(duty, 1);
            }

            ch = ' ';
            prev_ch = 'a';
        }
        else if (ch == 's' || ch == 'S') // back button
        {
            if(backObject == 0){
                goStraight(duty, 0);
            }

            ch = ' ';
            prev_ch = 's';
        }
        else if (ch == 'b' || ch == 'B') // break button
        {
            stopChA();
            stopChB();

            ch = ' ';
            prev_ch = 'b';
        }
        else if (ch == 'c' || ch == 'C') // back right button
        {
            if(backObject == 0){
                turnRight(duty, 0);
            }

            ch = ' ';
            prev_ch = 'c';
        }
        else if (ch == 'z' || ch == 'Z') // back left button
        {
            if(backObject == 0){
                turnLeft(duty, 0);
            }

            ch = ' ';
            prev_ch = 'z';
        }
        else if (ch == 'p' || ch == 'P')
        {
            goStraight(30, 1);

            ch = ' ';
            prev_ch = 'p';
        }


        backObject = 0;
        frontObject = 0;
    }
    return 0;

}

void goStraight(int duty, int dir){
    movChA_PWM(50, dir);
    movChB_PWM(50, dir);
    for (int i = 0; i < 500000; i++)
    {
    }
    movChA_PWM(duty, dir);
    movChB_PWM(duty, dir);
}

void turnRight(int duty, int dir){
    movChA_PWM(duty + 20, dir);
    movChB_PWM(0, dir);
}

void turnLeft(int duty, int dir){
    movChA_PWM(0, dir);
    movChB_PWM(duty + 20, dir);
}

int checkBack(){
    int result = 0;
    float back_distance = ReadRearUltrasonic_Filt();
    bl_printf("back_distance : %f\n", back_distance);
    delay_ms(10);

    if (back_distance <= 25 && back_distance > 18)
    {
        setBeepCycle(260);

    }
    else if (back_distance <= 18 && back_distance > 12)
    {
        setBeepCycle(50);

    }
    else if (back_distance <= 12 && back_distance > 5)
    {
        setBeepCycle(10);

    }
    else if (back_distance <= 5 && back_distance >= 0)
    {
        setBeepCycle(2);
        stopChA();
        stopChB();

        result = 1;
    }else{
        setBeepCycle(0);
    }

    return result;
}

int checkFront(){
    int result = 0;
    int front_distance = getTofDistance();
    bl_printf("front_distance : %d\n", front_distance);

    if(front_distance <= 150 && front_distance > 0){
        stopChA();
        stopChB();

        result = 1;
    }

    return result;
}

int checkParking(){
    int result = 0;
    float side_distance = ReadLeftUltrasonic_Filt();
    bl_printf("side_distance : %f\n", side_distance);
    delay_ms(10);

    if(side_distance > 13){
        parkingCount++;
    }else{
        parkingCount = 0;
    }

    if(parkingCount > 10){
        result = 1;
    }

    return result;
}

