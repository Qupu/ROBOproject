/*
Sumo wrestling 12.3 -- final 
*/
#if 1
bool scanForEdges();
int ultraScan();
void turnFromEdge();
void dodge(int direction);

void zmain(void) {
    struct sensors_ ref;
    struct sensors_ dig;
    
    TickType_t startTime = 0;
    TickType_t stopTime = 0;
    
    motor_start();
    reflectance_start();
    Ultra_Start();
    IR_Start();
    LSM303D_Start();
    reflectance_set_threshold(11000,11000,11000,11000,11000,11000);
    motor_forward(0,0);
    struct accData_ data;
    
    // Waits for input from the user button
    for (;;) {
        if (SW1_Read() == 0) {
            break;
        }
        vTaskDelay(10);
    }
    
    // Zumo robot moves forward until the reflectance sensors notice the black line
    // Then Zumo sends ready notification
    while(true) {
		reflectance_read(&ref);
		reflectance_digital(&dig); 
		
		if( dig.l3 == 1 && dig.l2== 1 && dig.l1== 1 
		&& dig.r1== 1 && dig.r2== 1 && dig.r3== 1 ) {      
			motor_forward(0,0);
            print_mqtt("Zumo042/ready", "zumo");
			break;
		}
		motor_forward(80,0);
		vTaskDelay(1);   		
	}
    
    // Zumo waits for remote IR command to start the competition
    // It sends start notification and moves inside the ring
    IR_flush();
    IR_wait();
    startTime = xTaskGetTickCount();
    print_mqtt("Zumo042/start", "%d", startTime);
    
    motor_forward(200,1000);
    
    // Code for Zumo when it's inside the ring
    // It is constantly checking reflectance before taking any other actions
    // In case sensors notice black from left (l2) or right (r2) it moves backwards and turns accordingly
    // Or if three sensors notice black it turns away from the edge (turnFromEdge())
    // Hit detection using accelerometer and reaction (dodge) depending from the direction of the impact
    for(;;) {
        if (SW1_Read() == 0) {
            vTaskDelay(50);
            if (SW1_Read() ==0) {
                break;
            }    
        }
        reflectance_digital(&dig);
        
        if (dig.l2 == 1) {
            motor_forward(0,0);
            motor_backward(255,100);
            motor_forward(0,0);
            motor_turn_right(255,255,200);
            motor_forward(150,100);
        }        
        else if (dig.r2== 1) {
            motor_forward(0,0);
            motor_backward(255,100);
            motor_forward(0,0);
            motor_turn_left(255,255,200);
            motor_forward(150,100);
        }
        else if (scanForEdges()) {
            turnFromEdge();
        }
		else {
			motor_forward(250,10);
        }
        // When Zumo is hit with large enough force it calculates angle of hit
        // Depending on if the hit is coming from positive or negative side of either x- or y-axis
        // it calculates the hit angle using ratio and angle from which quadrant of circle the hit came from
        LSM303D_Read_Acc(&data);
        int x2 = data.accX;
        int y2 = data.accY;
        double ratio = 0;
        int hitAngle = 0;
        if (abs(x2) > 15000 || abs(y2) > 15000) {
            ratio = (1.0) * abs(x2) / abs(y2);
            int angle = round(atan(ratio*180/(2*pi)));
            if (x2 > 0 && y2 > 0) {
                hitAngle = ratio + 270;    
            } 
            else if (x2 > 0 && y2 < 0) {
                hitAngle = ratio;
            }
            else if (x2 < 0 && y2 < 0) {
                hitAngle = ratio + 90;    
            }
            else if (y2 > 0 && x2 < 0) {
                hitAngle = ratio + 180;
            }
            print_mqtt("Zumo042/hit", "%d %d", xTaskGetTickCount(), hitAngle);       
            
            if (hitAngle > 45 && hitAngle < 155) {
                dodge(1);    
            } else if (hitAngle > 205 && hitAngle < 315) {
                dodge(0);
            }
            hitAngle = 0;
        }
    }
    motor_stop();
    stopTime = xTaskGetTickCount();
    print_mqtt("Zumo042/stop", "%d", stopTime);
    print_mqtt("Zumo042/time", "%d", (stopTime - startTime));
}

// Returns true if at least three sensors are seeing black (the edge of the ring).
bool scanForEdges() {
    struct sensors_ ref;
    struct sensors_ dig;
    reflectance_read(&ref);
    reflectance_digital(&dig);
        
    if ((dig.l3 + dig.l2 + dig.l1 + dig.r3 + dig.r2 + dig.r1) > 2) {
        return true;
    }
    return false;
}

// Zumo moves away from the edge and randomly selects direction to continue moving
void turnFromEdge() {
    motor_forward(0,0);
    motor_backward(255,100);
    motor_forward(0,0);
    
    int rnd = rand() % 2;
    if (rnd == 0) {
        motor_turn_left(255,255,300);
        motor_forward(0,0);
        motor_forward(200,10);
    }          
    else if (rnd != 0) {
        motor_turn_right(255,255,300);
        motor_forward(0,0);
        motor_forward(200,10);
    }
}

// Tries to dodge incoming hits from other robots by turning
// The turn direction depends on if the impact is coming from left side (0) or from right (1)
void dodge(int direction) {
    motor_forward(0,0);
    if (direction == 0) {
        motor_turn_left(255,255,800);
    }
    else {
        motor_turn_right(255,255,800);
    }
    motor_forward(0,0);
    motor_forward(200,10);
}

#endif