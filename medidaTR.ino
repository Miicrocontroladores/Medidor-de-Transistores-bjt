///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                                Medidor de transistores                                        //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//  Autores: Andrés Camilo Patiño Ariza 20211005105                                              //
//           Alejandro Santamaria Torres 20211005097                                             //
//           Juan Camilo Cárdenas Murcia 20211005026                                             //
//                                                                                               //
//  Componentes: 1 LCD 16x2                                                                      //
//               1 Potenciometro de 10k                                                          //
//               1 pulsador                                                                      //
//               1 resistencia de 1M                                                             //
//               2 resistencia de 4.7k                                                           //
//               2 resistencia de 2k                                                             //
//               20 jumpers                                                                      //
//               transistore de prueba                                                           //
//               1 condensador 100nF                                                             //
//                                                                                               //
//   Caracteristicas: Reconocimiento de pines (EBC/CBE)                                          //
//                    Reconocimiento de juntura (NPN/PNP)                                        //
//                    Medicion de hfe/Beta                                                       //
//                    Medicion de voltaje early (Vaf)                                            //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                                Definiciones globales                                          //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <LiquidCrystal.h>

#define rs 8
#define e 9
#define d4 10
#define d5 11
#define d6 12
#define d7 13

LiquidCrystal lcd(rs,e,d4,d5,d6,d7);

#define pin1 A3
#define pin2 A4
#define pin3 A5//izq = pin1     medio =  pin2         der = pin3

#define Vpin1 A0
#define Vpin2 A1
#define Vpin3 A2//izq = Vpin1     medio =  Vpin2         der = Vpin3

#define Rbaja1 4
#define Rbaja3 3//izq = Rbaja1       der = Rbaja3

#define Ralta1 7
#define Ralta2 6
#define Ralta3 5//izq = Ralta1     medio =  2         der = Ralta3

#define BTN 2

/////////////////////////////////////////////////////////////////
//                                                             //
//            Declaracion de variables                         //
//                                                             //
/////////////////////////////////////////////////////////////////

float volt1,volt2,volt3,diferencia,diferencia1,diferencia2;
float corriente1,corriente2,vrb,vre;
float hfe;
float x1,x2,y1,y2,m,b;
float vaf1;
float umbral = 0.5;


int Vd1 = 0;
int Vd2 = 0;
float val0Corrected = 0;
float val0Corrected1 = 0;

int cont=0;

/////////////////////////////////////////////////////////////////
//                                                             //
//            ISR (Interrupt Service Routine)                  //   
//                                                             //
/////////////////////////////////////////////////////////////////

void interrupcion(){
      if(cont==0){cont++;}
      delay(120);
}

/////////////////////////////////////////////////////////////////
//                                                             //
//                    SETUP                                    //
//                                                             //
/////////////////////////////////////////////////////////////////

void setup() {

  Serial.begin(9600);
  lcd.begin(16,2);
  attachInterrupt(digitalPinToInterrupt(2),interrupcion,FALLING); //Activa la interrupcion del pin 2
                                                                  //Indica que la interrupcion es en flanco de bajada (por pull-up)
                                                                  //Identifica la funcion "interrupion" como el vector de interrupcion
  
  pinMode(BTN,INPUT_PULLUP);
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  pinMode(pin3,OUTPUT);
  
  pinMode(Rbaja1,INPUT);
  pinMode(Rbaja3,INPUT);

  pinMode(Ralta1,INPUT);
  pinMode(Ralta2,INPUT);
  pinMode(Ralta3,INPUT);

  //inicializa todos los pines en cero 
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,LOW);

  cont=0;

}


/////////////////////////////////////////////////////////////////
//                                                             //
//                      Loop principal                         //
//                                                             //
/////////////////////////////////////////////////////////////////


void loop() {

  cont=0;
  while(cont==0){
    lcdDefault();
  }
  
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,LOW);
  delay(50);
  volt2 = readV(Vpin2); //Mide el voltaje en base

  digitalWrite(pin2,LOW);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt3 = readV(Vpin3); //Mide una de las dos patas extremas

  //Si no hay BJT 
  diferencia = abs(volt3-volt2) ;

  if(diferencia < umbral)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Transistor");
    lcd.setCursor(0,1);
    lcd.print("desconectado");
  }

  //Si el BJT es NPN
  else if(volt3>volt2)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NPN");

    {
  digitalWrite(pin1,HIGH);
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,LOW);
  delay(50);
  volt2 = readV(Vpin2); //mide el voltaje en la base
  volt3 = readV(Vpin3); //mide el voltaje en una de las patas exteriores
  diferencia1 = abs(volt3-volt2);  //diferencia de voltaje (voltaje del diodo)
  Serial.print("VD2 :");
  Serial.print(diferencia1,4);
  delay(10);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt1 = readV(Vpin1); //mide el voltaje en la base
  volt2 = readV(Vpin2); //mide el voltaje en una de las patas exteriores
  diferencia2 = abs(volt2-volt1);//diferencia de voltaje (voltaje del diodo)
  Serial.print("  VD1 :");
  Serial.print(diferencia2,4);

  //En caso de que NPN CBE
  if(diferencia1>diferencia2)
  {
    Serial.print(" NPN  CBE");
    lcd.setCursor(5,0);
    lcd.print("CBE");

    pinMode(pin1,INPUT);
    pinMode(pin2,INPUT);
    pinMode(pin3,OUTPUT);
  
    pinMode(Rbaja1,OUTPUT);
    pinMode(Rbaja3,INPUT);

    pinMode(Ralta1,INPUT);
    pinMode(Ralta2,OUTPUT);
    pinMode(Ralta3,INPUT);

    digitalWrite(pin3,LOW);
    digitalWrite(Rbaja1,HIGH);
    digitalWrite(Ralta2,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin1);

    vrb=((readVcc()/1000.0)-volt1);
    vre=((readVcc()/1000.0)-volt2);

    corriente1 = vrb;
    Serial.print(" IB = "); 
    Serial.print(corriente1);
    Serial.print("uA");

    corriente2 = vre/4.7;
    Serial.print(" IC = "); 
    Serial.print(corriente2);
    Serial.print("mA");

    hfe=(corriente2*1000)/corriente1;
    Serial.print(" B = ");
    Serial.print(hfe);
    
    lcd.setCursor(0,1);
    lcd.print("B=");
    lcd.setCursor(3,1);
    lcd.print(hfe);
    delay(5000);
    
    //VAF NPN CBE
    
    y1=corriente2;
    x1=volt2;

    pinMode(Rbaja1,INPUT);
    pinMode(Ralta1,OUTPUT);
    digitalWrite(Ralta1,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin1);
    vre=((readVcc()/1000.0)-volt2);

    corriente2 = vre/2;
    y2= corriente2;
    x2= volt2;

    m = (y2-y1)/(x2-x1);
    b = y1-(x1*m);
    vaf1 = (-b)/(m);
    Serial.print(" Vaf = ");
    reset();
    Serial.println(vaf1);
    
    lcd.setCursor(0,1);
    lcd.print("vaf= ");
    lcd.setCursor(5,1);
    lcd.print(vaf1);
    delay(5000);

  }
  
   //En caso de que NPN EBC
  else if(diferencia1<=diferencia2)
  {
    Serial.print(" NPN  EBC");
    lcd.setCursor(5,0);
    lcd.print("EBC");

    pinMode(pin1,OUTPUT);
    pinMode(pin2,INPUT);
    pinMode(pin3,INPUT);
  
    pinMode(Rbaja1,INPUT);
    pinMode(Rbaja3,OUTPUT);

    pinMode(Ralta1,INPUT);
    pinMode(Ralta2,OUTPUT);
    pinMode(Ralta3,INPUT);

    digitalWrite(pin1,LOW);
    digitalWrite(Rbaja3,HIGH);
    digitalWrite(Ralta2,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin3);

    vrb=((readVcc()/1000.0)-volt1);
    vre=((readVcc()/1000.0)-volt2);

    corriente1 = vrb;
    Serial.print(" IB = "); 
    Serial.print(corriente1);
    Serial.print("uA");

    corriente2 = vre/4.7;
    Serial.print(" IC = "); 
    Serial.print(corriente2);
    Serial.print("mA");

    hfe=(corriente2*1000)/corriente1;
    Serial.print(" B = ");
    Serial.print(hfe);
    
    lcd.setCursor(0,1);
    lcd.print("B=");
    lcd.setCursor(3,1);
    lcd.print(hfe);
    delay(5000);

    //VAF NPN EBC
    
    y1=corriente2;
    x1=volt2;

    pinMode(Rbaja3,INPUT);
    pinMode(Ralta3,OUTPUT);
    digitalWrite(Ralta3,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin3);
    vre=((readVcc()/1000.0)-volt2);

    corriente2 = vre/2;
    y2= corriente2;
    x2= volt2;

    m = (y2-y1)/(x2-x1);
    b = y1-(x1*m);
    vaf1 = (-b)/(m);

    Serial.print(" Vaf = ");
    reset();
    Serial.println(vaf1);

    lcd.setCursor(0,1);
    lcd.print("vaf= ");
    lcd.setCursor(5,1);
    lcd.print(vaf1);
    delay(5000);

  }

      
    }
  }


  
  //si el BJT es PNP
  else if(volt3<volt2) 
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("PNP");

      {
  digitalWrite(pin1,HIGH);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,LOW);
  delay(50);
  volt2 = readV(Vpin2); //mide voltaje en la base
  volt1 = readV(Vpin1); //mide el voltaje en un de las patas exteriores
  diferencia1 = abs(volt1-volt2);  //diferencia de voltaje entre las terminales (voltaje diodo)
  Serial.print("VD1 :");
  Serial.print(diferencia1,4);
  delay(10);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt3 = readV(Vpin3); //mide voltaje en la base
  volt2 = readV(Vpin2); //mide el voltaje en un de las patas exteriores
  diferencia2 = abs(volt2-volt3); //diferencia de voltaje entre las terminales (voltaje diodo)
  Serial.print("  VD2 :");
  Serial.print(diferencia2,4);

//En caso de PNP EBC
  if(diferencia1>diferencia2)
  {
    Serial.print(" PNP  EBC");
    lcd.setCursor(5,0);
    lcd.print("EBC");

    pinMode(pin1,INPUT);
    pinMode(pin2,INPUT);
    pinMode(pin3,OUTPUT);
  
    pinMode(Rbaja1,OUTPUT);
    pinMode(Rbaja3,INPUT);

    pinMode(Ralta1,INPUT);
    pinMode(Ralta2,OUTPUT);
    pinMode(Ralta3,INPUT);

    digitalWrite(pin3,LOW);
    digitalWrite(Rbaja1,HIGH);
    digitalWrite(Ralta2,LOW);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin1);

    vrb=volt1;
    vre=((readVcc()/1000.0)-volt2);

    corriente1 = vrb;
    Serial.print(" IB = "); 
    Serial.print(corriente1);
    Serial.print("uA");

    corriente2 = vre/4.7;
    Serial.print(" IC = "); 
    Serial.print(corriente2);
    Serial.print("mA");

    hfe=(corriente2*1000)/corriente1;
    Serial.print(" B = ");
    Serial.print(hfe);
    
    lcd.setCursor(0,1);
    lcd.print("B=");
    lcd.setCursor(3,1);
    lcd.print(hfe);
    delay(5000);

    //VAF PNP EBC
    
    y1=corriente2;
    x1=volt2;

    pinMode(Rbaja1,INPUT);
    pinMode(Ralta1,OUTPUT);
    digitalWrite(Ralta1,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin3);
    vre=((readVcc()/1000.0)-volt2);

    corriente2 = vre/2;
    y2= corriente2;
    x2= volt2;

    m = (y2-y1)/(x2-x1);
    b = y1-(x1*m);
    vaf1 = (b)/(m); // b positivo pues la pendiente se sabe es negativa (ley de signos)

    Serial.print(" Vaf = ");
    reset();
    Serial.println(vaf1);
    
    lcd.setCursor(0,1);
    lcd.print("vaf= ");
    lcd.setCursor(5,1);
    lcd.print(vaf1);
    delay(5000);
  }

   //En caso de PNP CBE
  else if(diferencia1<=diferencia2)
  {
    Serial.print(" PNP  CBE");
    lcd.setCursor(5,0);
    lcd.print("CBE");

     pinMode(pin1,OUTPUT);
    pinMode(pin2,INPUT);
    pinMode(pin3,INPUT);
  
    pinMode(Rbaja1,INPUT);
    pinMode(Rbaja3,OUTPUT);

    pinMode(Ralta1,INPUT);
    pinMode(Ralta2,OUTPUT);
    pinMode(Ralta3,INPUT);

    digitalWrite(pin1,LOW);
    digitalWrite(Rbaja3,HIGH);
    digitalWrite(Ralta2,LOW);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin3);

    vrb=volt1;
    vre=(4.76-volt2);

    corriente1 = vrb;
    Serial.print(" IB = "); 
    Serial.print(corriente1);
    Serial.print("uA");

    corriente2 = vre/4.7;
    Serial.print(" IC = "); 
    Serial.print(corriente2);
    Serial.print("mA");

    hfe=(corriente2*1000)/corriente1;
    Serial.print(" B = ");
    Serial.print(hfe);
    
    lcd.setCursor(0,1);
    lcd.print("B=");
    lcd.setCursor(3,1);
    lcd.print(hfe);
    delay(5000);

    //VAF PNP CBE
    
    y1=corriente2;
    x1=volt2;

    pinMode(Rbaja3,INPUT);
    pinMode(Ralta3,OUTPUT);
    digitalWrite(Ralta3,HIGH);

    volt1 = readV(Vpin2);
    volt2 = readV(Vpin3);
    vre=((readVcc()/1000.0)-volt2);

    corriente2 = vre/2;
    y2= corriente2;
    x2= volt2;

    m = (y2+y1)/(x2-x1);
    b = y1-(x1*m);
    vaf1 = (b)/(m); //

    Serial.print(" Vaf = ");
    reset();
    Serial.println(vaf1);
    lcd.setCursor(0,1);
    lcd.print("vaf= ");
    lcd.setCursor(5,1);
    lcd.print(vaf1);
    delay(5000);
  }

      }
    
    
  }
  //En caso de que ninguna de las condiciones se cumple
  else
  {
    lcd.setCursor(0,0);
    lcd.print("Transistor");
    lcd.setCursor(0,1);
    lcd.print("desconectado");
  }
  //reset de los pines y de la interrupcion
  reset();
  cont--;

}


/////////////////////////////////////////////////////////////////
//                                                             //
//                          Funciones                          //
//                                                             //
/////////////////////////////////////////////////////////////////

long readVcc() {
  long result;
    // lee 1.1V de referencia comparado con AVcc
  
    // En caso de arduino leonardo/seeduino GPRS
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#else //En caso de arduino uno
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2); // espera a que Vref se estabilice
  ADCSRA |= _BV(ADSC); // convierte
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // calcula Vcc (en mV); 1126400 = 1.1*1024*1000
  return result;
}


float readV(float Vpin){
    float V, prom, val0,val0Corrected = 0;
    for(int i = 0; i < 64 ; i++){
    V = analogRead(Vpin);
    prom = V + prom;
  }
  prom = prom/64 ;
  val0 = prom * 4.76 / 1023.0;            //4.76 es el voltaje medido
  float supply = readVcc() / 1000.0;
  val0Corrected = supply / 4.76 * val0;   //4.76 es el voltaje medido
  return val0Corrected;
}

void reset (){

  if(vaf1>-20){
      if(vaf1>-10){
        if(vaf1>-5){
          vaf1=vaf1*30;
        }else{
          vaf1=vaf1*10;
        }
      }else{
        vaf1=vaf1*4;
      }
  }

  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  pinMode(pin3,OUTPUT);
  
  pinMode(Rbaja1,INPUT);
  pinMode(Rbaja3,INPUT);

  pinMode(Ralta1,INPUT);
  pinMode(Ralta2,INPUT);
  pinMode(Ralta3,INPUT);

  //Inicializa todo en cero
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,LOW);

}

void lcdDefault(){
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Medidor de");
  lcd.setCursor(0,1);
  lcd.print("transistores");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("conecta tu");
  lcd.setCursor(0,1);
  lcd.print("transistor");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Luego presiona");
  lcd.setCursor(0,1);
  lcd.print("el boton");
  delay(2000);

}