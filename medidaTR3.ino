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
#define pin3 A5//left leg = pin1     middle leg =  pin2         right leg =pin3

#define Vpin1 A0
#define Vpin2 A1
#define Vpin3 A2

float volt1,volt2,volt3,diferencia,diferencia1,diferencia2; //variable initializations 
float umbral = 0.5;

int Vd1 = 0;
int Vd2 = 0;
float val0Corrected = 0;
float val0Corrected1 = 0;

/*
void interrupcion(){
      cont++;
      if(cont>1){
        cont=0;
      }
}
*/

void setup() {

  Serial.begin(9600);
  lcd.begin(16,2);
  //attachInterrupt(digitalPinToInterrupt(2),interrupcion,FALLING); //Activa la interrupcion del pin 2
                                                                  //Indica que la interrupcion es en flanco de bajada (por pull-up)
                                                                  //Identifica la funcion "interrupion" como el vector de interrupcion
  
  //pinMode(2,INPUT_PULLUP);
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  pinMode(pin3,OUTPUT);
  //at start pins will be at 0 level 
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,LOW);//left leg = pin1     middle leg =  pin2         right leg =pin3

}

void loop() {
  
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,LOW);
  delay(50);
  volt2 = readV(Vpin2); //measuring voltage at node pin2  (middle leg)
  //Serial.print("  volt2 :");
  //Serial.print(volt2);
  //Serial.print("    volt3 :");

  digitalWrite(pin2,LOW);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt3 = readV(Vpin3); //measuring voltage at node pin3 (any of extreme 2 legs)
  //Serial.println(volt3);
  //when there is no BJT 
  diferencia = abs(volt3-volt2) ;

  if(diferencia < umbral)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Transistor");
    lcd.setCursor(0,1);
    lcd.print("desconectado");
  }
  //if BJT is NPN
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
  volt2 = readV(Vpin2); //measuring voltage at node pin2  (middle leg)
  volt3 = readV(Vpin3); //measuring voltage at node pin2  (middle leg)
  diferencia1 = abs(volt3-volt2);  //voltage difference between pin2 and pin3
  Serial.print("VD2 :");
  Serial.print(diferencia1,4);
  delay(10);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt1 = readV(Vpin1); //measuring voltage at node pin1
  volt2 = readV(Vpin2); //measuring voltage at node pin2
  diferencia2 = abs(volt2-volt1);//voltage difference between pin1 and pin2 
  Serial.print("  VD1 :");
  Serial.print(diferencia2,4);
  if(diferencia1>diferencia2)
  {
    Serial.println(" NPN  CBE");
    lcd.setCursor(5,0);
    lcd.print("CBE");
  }else if(diferencia1<=diferencia2)
  {
    Serial.println(" NPN  EBC");
    lcd.setCursor(5,0);
    lcd.print("EBC");
    delay(1000);
  }

      
    }
  }
  //if BJT is PNP
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
  volt2 = readV(Vpin2); //measuring voltage at node pin2  (middle leg)
  volt1 = readV(Vpin1); //measuring voltage at node pin2  (middle leg)
  diferencia1 = abs(volt1-volt2);  //voltage difference between pin2 and pin1
  Serial.print("VD1 :");
  Serial.print(diferencia1,4);
  delay(10);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  digitalWrite(pin3,HIGH);
  delay(50);
  volt3 = readV(Vpin3); //measuring voltage at node pin1  
  volt2 = readV(Vpin2); //measuring voltage at node pin2  
  diferencia2 = abs(volt2-volt3);//voltage difference between pin3 and pin2 
  Serial.print("  VD2 :");
  Serial.print(diferencia2,4);
  if(diferencia1>diferencia2)
  {
    Serial.println(" PNP  EBC");
    lcd.setCursor(5,0);
    lcd.print("EBC");
  }
  else if(diferencia1<=diferencia2)
  {
    Serial.println(" PNP  CBE");
    lcd.setCursor(5,0);
    lcd.print("CBE");
    delay(1000);
  }

  
  }    
    
    
  }
  //If none of above condition is true both leds will turn off
  else
  {
    lcd.setCursor(0,0);
    lcd.print("Transistor");
    lcd.setCursor(0,1);
    lcd.print("desconectado");
  }
}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // Calculate Vcc (in mV); 1126400 = 1.1*1024*1000
  return result;
}

/*float supply = readVcc() / 1000.0;
*/
float readV(float Vpin){
    float V, prom, val0,val0Corrected = 0;
    for(int i = 0; i < 64 ; i++){
    V = analogRead(Vpin);
    prom = V + prom;
  }
  prom = prom/64 ;
  val0 = prom * 4.76 / 1023.0;
  float supply = readVcc() / 1000.0;
  val0Corrected = supply / 4.76 * val0;
  return val0Corrected;
}

float readV_5m(float Vpin){
  delay(5);
  float Volta = readV(Vpin);
}