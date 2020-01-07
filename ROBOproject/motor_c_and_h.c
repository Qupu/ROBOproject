/* This code was added to Motor.h */
void motor_turn_left(uint8 l_speed, uint8 r_speed, uint32 delay);

void motor_turn_right(uint8 l_speed, uint8 r_speed, uint32 delay);



/* This code was added to Motor.c */
void motor_turn_right(uint8 l_speed, uint8 r_speed, uint32 delay)
{    
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(1);     // set RightMotor backward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);
}

void motor_turn_left(uint8 l_speed, uint8 r_speed, uint32 delay)
{
    MotorDirLeft_Write(1);      // set LeftMotor backward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    PWM_WriteCompare1(l_speed); 
    PWM_WriteCompare2(r_speed); 
    vTaskDelay(delay);    
}