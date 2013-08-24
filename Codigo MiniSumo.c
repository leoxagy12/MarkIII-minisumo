//=================== AREA DE CONSTANTES Y VARIABLES =============================

//---------- GENERALES---------------
#define NO 0
#define YES 1
#define SI 1

unsigned int tiempo=0; /*Para contar algunos tiempos que tengo que durar
                       haciendo alguna accion especifica. Cada incremento*/

//---------------- SERVOMOTORES -------------------

#define SERVO_R 1     //Puerto B bit 1
#define SERVO_L 2     //Puerto B bit 2

//Valores para setear la direccion de los motores
#define BACKWARD 0
#define STOP 1
#define FORWARD 2


//Con estas variables controlo el moviemiento de los motores
unsigned int l_motor_dir=STOP;
unsigned int r_motor_dir=STOP;

//Para llevar el conteo de las interrupciones para generar el PWM en los tiempos correctos
unsigned int conteo = 0;
unsigned int retardo = 0;


//-------------- SENSORES DE PROXIMIDAD --------------
#define EYE_R 2       //Entrada analoga 2
#define EYE_L 3       //Entrada analoga 3
#define BOTH 1        //Este valor significa que lo veo con los dos rangers
#define EYE_RANGE 0x95 /*para controlar el rango de vision de los rangers
                       mayor que este valor significa que lo estoy viendo*/

unsigned int ojos[2] = {0,0}; //Aqui van los valores de los valores de los ojos frescos
unsigned int memoria_ojos[2] = {0,0}; //Aqui un registro de las ultimas veces visto para ayudar en la persecucion del


//------------- SENSORES DE PISO --------------
#define LINE_R 5      // Entrada analoga 5
#define LINE_C 6      // Entrada analoga 6
#define LINE_L 7      // Entrada analoga 5

//Estas variables son para guardar la lectura de los sensores de piso.
unsigned int sensores_de_piso[3] = {NO,NO,NO};

unsigned int average=0; /*Aqui guardo el promedio de la lectura inicial de los
                        tres sensores de piso*/


//====================================================================


//=================== SUB-RUTINAS =============================

//Inicializacion del Micro
void on_init(){

     trisa = 0x3F;      //Puerto A como entrada
     porta = 0x00;
     trisb = 0xE1;      //Puerto B como entrada exceptuando PB1 y PB2
     portb = 0x00;
     trisc = 0xFF;      //Puerto C como entrada
     portc = 0x99;
     trisd = 0xFF;      //Puerto D como entrada
     portd = 0x00;
     trise = 0x07;      //Los disponibles del puerto E como entrada
     porte = 0x00;
     
     adcon0 = 0x41;     //Habilita el AD, 8 canales, 10 bits
     adcon1 = 0x00;     //+VERF=VDD  -VREF=VSS
     
     option_reg = 0xD0; //Reloj interno como reloj del Timer0, preescaler asignado con una escala de 1:2

     INTCON = 0xA0; //Habilito la interrupcion del Timer0 y borro el flag de la interrupion del Timer0

     conteo=0; //Reinicio el conteo del tiempo para producir los pulsos del PWM

}

//Interrupcion cada 0.1 mS
void interrupt()
{
     INTCON = 0xA0;  //Borro el flag de la interrupcion del Timer0, para que se vuelva a producir en el siguiente desborde.
     
     conteo++; //Incremento el conteo del tiempo en 0.1 mS
     

     //Reviso el conteo y termino el pulso para los motores en el tiempo correcto segun sea la direccion configurada
     switch(conteo){
         //1 mS
         case 10:

             if(l_motor_dir == BACKWARD) portb.f2=0;
	           if(r_motor_dir == FORWARD) portb.f1=0;

             break;
	     
	       //1.5 mS (Parado)
	       case 15:

             if(l_motor_dir == STOP) portb.f2=0;
	           if(r_motor_dir == STOP) portb.f1=0;

             break;

         //2 mS
         case 20:

             if(l_motor_dir == FORWARD) portb.f2=0;
	           if(r_motor_dir == BACKWARD)	portb.f1=0;

 	           break;

         //20 mS (Este es el periodo de la señal, asi que inicio el otro pulso poniendo las salidas en algo)
         case 200:

             if(l_motor_dir != STOP); portb.f2=1;
             if(r_motor_dir != STOP); portb.f1=1;

             conteo = 0; //como ya se termino un periodo de la onda, reinicio el conteo
             
	           tiempo++; //incremento la variable de tiempo (cada incremento vale 20 mS)

             break;
      }
}

//Rutinas para ocnfigurar la direccion de los motores segun el movimiento que se desea
void atras(){
     l_motor_dir = FORWARD;
     r_motor_dir = FORWARD;
}

void adelante(){
     l_motor_dir = BACKWARD;
     r_motor_dir = BACKWARD;
}

void parado(){
     l_motor_dir = STOP;
     r_motor_dir = STOP;
}

void giro_izquierda(){
     l_motor_dir = BACKWARD;
     r_motor_dir = FORWARD;
}

void giro_derecha(){
     l_motor_dir = FORWARD;
     r_motor_dir = BACKWARD;
}

void doblar_izquierda(){
     l_motor_dir = BACKWARD;
     r_motor_dir = STOP;
}

void doblar_derecha(){
     l_motor_dir = STOP;
     r_motor_dir = BACKWARD;
}

//Aqui reviso el piso y guardo los resultados en las variables

//Aqui reviso el piso y guardo los resultados en las variables
void tantear_piso(){

     if(adc_read(LINE_R)<(average*0.70))
         sensores_de_piso[2] = SI;
     else
         sensores_de_piso[2] = NO;
         
     if(adc_read(LINE_C)<(average*0.70))
         sensores_de_piso[1] = SI;
     else
         sensores_de_piso[1] = NO;
         
     if(adc_read(LINE_L)<(average*0.70))
         sensores_de_piso[0] = SI;
     else
         sensores_de_piso[0] = NO;
}



//Aqui reviso los sensores de proximidad y aplico los valores a las variables que me describen el estado de la vision

//Aqui reviso los sensores de proximidad y aplico los valores a las variables que me describen el estado de la vision
void mirar(){

     int cd = 0;
     int ci = 0;
     int i = 0;

     if(ojos[0] || ojos[1] )  {
                memoria_ojos[0] =  ojos[0];
                memoria_ojos[1] =  ojos[1];

     }

     
     for(i = 0; i < 60 ; i++){
         if(adc_read(EYE_R)>0x95) cd++;
         if(adc_read(EYE_L)>0xB0) ci++;
     }
     
     if(cd == 60) ojos[1] = YES; else ojos[1] = NO;
     if(ci == 60) ojos[0] = YES; else ojos[0] = NO;
     
     //if(adc_read(EYE_R)>EYE_RANGE) ojos[1] = YES; else ojos[1] = NO;
     //if(adc_read(EYE_L)>EYE_RANGE) ojos[0] = YES; else ojos[0] = NO;
}

void sensar(){

     tantear_piso(); // Con esta funcion especificamente senso los ensores de infrarojos del frente, los que ven "el piso" ...
     mirar();   // Con esta funcion especificamente senso los sensores de rango, "los ojos" ...

     return;
}

void mover(){

     if( sensores_de_piso[0] || sensores_de_piso[1] || sensores_de_piso[2] )  {
  		if( (sensores_de_piso[0] + sensores_de_piso[2]) == 2 || (sensores_de_piso[0] + sensores_de_piso[1] + sensores_de_piso[2]) == 3 ){
  		  //Para atras
  		  atras();
  		  memoria_ojos[0] = 0;
        memoria_ojos[1] = 1;
  		  return;
  		}
  		if( sensores_de_piso[0]  || sensores_de_piso[2]  ) {
          if(sensores_de_piso[0]){
              doblar_derecha();
              memoria_ojos[0] = 0;
              memoria_ojos[1] = 1;
              return;
          }
          else if (sensores_de_piso[2]) {
              doblar_izquierda();
              memoria_ojos[0] = 1;
              memoria_ojos[1] = 0;
              return;
          }
  		}
     }

     if ( ojos[0] || ojos[1] )  {
     
          if ( ojos[0] && ojos[1] ) {
             adelante();
             return;
          }
          else if( ojos[1] ) {
    				 doblar_derecha();
    				 return;
    			}
    			else if( ojos[0] ) {
    				 doblar_izquierda();
    				 return;
    			}

       }
       else {
                 if ( memoria_ojos[0] || memoria_ojos[1] )  {

                    if ( memoria_ojos[0] && memoria_ojos[1] ) {
                       adelante();
                       return;
                    }

              			else if( memoria_ojos[1] ) {
              				 doblar_derecha();
              				 return;
              			}
              			else if( memoria_ojos[0] ) {
              				 doblar_izquierda();
              				 return;
              			}
                 }
                 else {
                      giro_derecha();
                      return;
                 }
       }
}


//Rutina principal
void main() {

       //Inicializo el micro
       on_init();
       average=((adc_read(LINE_R)+adc_read(LINE_L)+adc_read(LINE_C))*2)/9;

       while(1){

			// Senso el mundo, guardo datos si voy a guardar.
  			sensar();

  			mover();

       }
}
