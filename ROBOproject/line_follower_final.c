//radanläpikovaa
#if 1
void zmain(void) {

struct sensors_ ref;
struct sensors_ dig;
    
    TickType_t aika;
    TickType_t alkuaika;
    TickType_t startaika;
   
    int rr2=0,ll2=0; //käytetään että robotti muistaa mihin se kääntyi ennen kun meni valkoiselle kokonaan
    int rr1=0,ll1=0; //käytetään loppuviivan tunnistamisessa
    bool online = true; //käytetään mqtt onko viivalla vai ei
    bool musta = false; //kätetään loppuviivan tunnistamisessa
    bool lapi = false; //käytetään että ohjelma lopettaa pyörittämisen loppuviivalla
    
//käynnistetään tarpeelliset laitteet robootissa
motor_start();
reflectance_start();  
IR_Start();
reflectance_set_threshold(10000, 11000, 15000, 15000, 11000, 10000); // set reflector sensor thresholds 
motor_forward(0,0);    

//odotetaan userbuttonin painamista
while(true) {
    if(SW1_Read() == 0) {
        break;
    }
    }
 //menee eteenpäin kunnes on mustalla viivalla 
while(true) {
    reflectance_read(&ref);
    reflectance_digital(&dig); 
    
    //kun robotti saapuu alkuviivalle
    if( dig.l3 == 1 && dig.l2== 1 && dig.l1== 1 && dig.r1== 1 && dig.r2== 1 && dig.r3== 1 ) {      
        motor_forward(0,0);
        alkuaika = xTaskGetTickCount();
        print_mqtt("Zumo042/ready","line");
        break;
    }
    motor_forward(80,0);
    vTaskDelay(1);   
    
    } 

while(true) {
  //odotetaan IR-singaalia
    IR_flush();
    IR_wait();
    startaika = xTaskGetTickCount();
    print_mqtt("Zumo042/start"," %d",startaika-alkuaika);
 motor_forward(255,200); // mennään pois alkuviivalta eteenpäin
 
    for(;;){
  
    reflectance_read(&ref);
    reflectance_digital(&dig); 
 
    //mqtt tarkastaa onko viivalla ilmoitukset
    if (dig.l1 == 1 && dig.r1 == 1) {
    if(!online){
    aika = xTaskGetTickCount();
    print_mqtt("Zumo042/line"," %d",aika-alkuaika); 
    online=true;
    }
    }
    if (dig.l1 == 0 && dig.r1 == 0) {
    if(online) { 
    aika = xTaskGetTickCount();
    print_mqtt("Zumo042/miss"," %d",aika-alkuaika); 
    online=false;
    }
    }
  
    //tarkastetaan ollaanko mustalla viivalla (maali)
    if(dig.l3 == 1 && dig.l2== 1 && dig.l1== 1 && dig.r1== 1 && dig.r2== 1 && dig.r3== 1) {
              
    //tarkastaa että ei ole ensimmäinen maaliviiva
        if(musta && lapi==false && !(rr1==1 && rr2==1 && ll1==1 && ll2==1)) {
        motor_forward(0,0);
        aika = xTaskGetTickCount(); 
        lapi=true;
        print_mqtt("Zumo042/stop"," %d",aika-alkuaika);
        print_mqtt("Zumo042/time"," %d",aika-startaika); 
        motor_stop();
        IR_flush();
        IR_wait();
        
        break;
        } else { //kertoo kun on ekalla maaliviivalla
        musta=true;
        rr1=1,rr2=1,ll1=1,ll2=1;  
            motor_forward(255,75);
        }
    
    }
    //katsoo onko kokonaan valkoisella
    if(dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0) {
        //jos viimeksi oltiin 2. oikealla olevalla sensorilla
        if(rr2==1) {
         motor_turn_right(255,255,0);
            vTaskDelay(1);

        } 
        //jos viimeksi oltiin 2. vasemmalla olevalla sensorilla
        else if(ll2==1) {
         motor_turn_left(255,255,0);
            vTaskDelay(1);      

        }
    }
       //jos ollaan viivalla mennään vain eteenpäin
if(dig.l1 == 1 || dig.r1 == 1)    {
  motor_forward(255,0);
  vTaskDelay(5);
    rr2=0,ll2=0;
    rr1=1,ll1=1;
} 
//kun 2. vasen sensorilla kääntyy oikella katsoen kuinka paljon se on viivalla ref. datan avulla
else if(dig.l2==1) {
     if(ref.l2>22000) {
        ll2=1;
        rr2=0;
    motor_turn(0,255,0);
        vTaskDelay(1);
    } else if (ref.l2>18000) {
        ll2=1;
        rr2=0,rr1=0,ll1=0;
    motor_turn(100,255,0);
        vTaskDelay(1);    
    } else if (ref.l2>14000) {
        ll2=1;
        rr2=0,rr1=0,ll1=0;
    motor_turn(150,255,0);
        vTaskDelay(1);
    } else {    
        ll2=1;
        rr2=0,rr1=0,ll1=0;
    motor_turn(200,255,0);
        vTaskDelay(1);;
    
    }
}
//kun 2. oikea sensori on mustalla se kääntyy oikella katsoen kuinka paljon se on viivalla ref. datan avulla
else if(dig.r2==1) {
    if(ref.r2>22000) {
        rr2=1;
        ll2=0,rr1=0,ll1=0;
    motor_turn(255,0,0);
    vTaskDelay(1);
    } else if (ref.r2>18000) {
    rr2=1;
        ll2=0,rr1=0,ll1=0;
    motor_turn(255,100,0);
        vTaskDelay(1);
    } else if (ref.r2>14000) {
    rr2=1;
        ll2=0,rr1=0,ll1=0;
    motor_turn(255,150,0);
        vTaskDelay(1);
    } else {
        rr2=1;
        ll2=0,rr1=0,ll1=0;
    motor_turn(255,200,0);
        vTaskDelay(1);
    
    }
}
}
}

motor_stop();
for(;;){}
}

#endif
