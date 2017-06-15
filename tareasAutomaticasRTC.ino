// LIBRERIAS
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>                                  // Librería Pantalla LCD
#include <Wire.h>                                               // For some strange reasons, Wire.h must be included here
#include <DS1307new.h>                                          // Libreria del Reloj

// Variables
LiquidCrystal_I2C lcd(0x27, 16, 2);                             // Pantalla LCD

uint16_t startAddr = 0x0000;                                    // Start address to store in the NV-RAM
uint16_t lastAddr;                                              // new address for storing in NV-RAM
uint16_t TimeIsSet = 0xaa55;                                    // Helper that time must not set again

//Estructura que guarda toda la información de las Tareas
struct Tarea {                                                  // Tarea
  byte h1 = 0;                                                  // Hora de inicio
  byte m1 = 0;                                                  // Minuto de inicio
  byte h2 = 0;                                                  // Hora de finalizacion
  byte m2 = 0;                                                  // Minuto de finalizacion
  byte a = 0;                                                   // Tipo de acción a realizar
  byte p = 0;                                                   // Pin a usar
  byte d = 0;                                                   // Dias que se ejecuta
  byte l = 0;                                                   // Dato adicional
};

// Mis variables
boolean act = false;                                            // Actualizar fecha y hora
int an = 2010, me = 9, dia = 16;                                // Año, Mes y Día
int ho = 12, minu = 00, se = 00;                                // Hora, Minuto y Segundo
byte hi = 00, mi = 00, hf = 1, mf = 1;                          // HoraInicial, MinutoInicial, HoraFinal y MinutoFinal
byte ac = 1, pi =  10, di = 103, la = 0;                        // Acción, Pin, Días y DatoAdicional

int totalTareas = (int)EEPROM.read(0);                          // Total de tareas guardadas en la EEPROM
Tarea Tareas[20];                                               // Declaramos las 20 tareas posibles
int estadosPines[4] = {20, 20 , 20, 20};                        // Pines tareas, en cada posición sub[i] del arreglo irá el número de la tarea que está usando el respectivo pin i
int diaSemana = 7;                                              // Número cualquiera que sea diferente de cero <- Por qué diferente de cero?
int Hora = 0, Minuto = 0;                                       // Hora y minuto del RELOJ
int  hs = 0;                                                    // Variable para almacenar Humedad del suelo

boolean tareaActiva = false;                                    // si la tarea es activa o inactiva

//-------------------------------------------------------------> Setup
//-->
void setup()
{
  lcd.begin();                                                  // Iniciamos la pantalla LCD
  lcd.backlight();                                              // Supongo que la luz de fondo de la LCD
  for (int i = 1; i <= totalTareas; i++) {
    Tarea b;
    EEPROM.get((8 * i) - 7, b);
    Tareas[i - 1] = b;
  }

  Serial.begin(57600);
  pinMode(2, INPUT);                                            // Test of the SQW pin, D2 = INPUT
  digitalWrite(2, HIGH);                                        // Test of the SQW pin, D2 = Pullup on

  pinMode(7, OUTPUT);                                           // Inicializamos los 4 pines que controlan los relé
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  RTC.setRAM(0, (uint8_t *)&startAddr, sizeof(uint16_t));       // Store startAddr in NV-RAM address 0x08


  if (act == true) {                                            // Si queremos actualizar Fecha y Hora
    TimeIsSet = 0xffff;
    RTC.setRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
  }


  RTC.getRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));      // Actualiza la Fecha y Hora
  if (TimeIsSet != 0xaa55)
  {
    RTC.stopClock();

    RTC.fillByYMD(an, me, dia);
    RTC.fillByHMS(ho, minu, se);

    RTC.setTime();
    TimeIsSet = 0xaa55;
    RTC.setRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
    RTC.startClock();
  }
  else
  {
    RTC.getTime();
  }

  RTC.ctrl = 0x00;                                              // 0x00=disable SQW pin, 0x10=1Hz,
  // 0x11=4096Hz, 0x12=8192Hz, 0x13=32768Hz
  RTC.setCTRL();

  uint8_t MESZ;

  for (int i = 0; i < totalTareas; i++) {                       //¿Pa' que weas es esto?

  }
}
//<-
//<------------------------------------------------------------- Final setup

//-------------------------------------------------------------> Loop
//-->
void loop()
{
  while (Serial.available() > 0) {                              // Si existen datos por leer
    switch (Serial.read()) {
      case 'a': {                                               // Asigna año
          an = Serial.parseInt();
          Serial.println("an");
          break;
        }
      case 'b': {                                               // Asigna mes
          me = Serial.parseInt();
          Serial.println("me");
          break;
        }
      case 'c': {                                               // Asigna día
          dia = Serial.parseInt();
          Serial.println("dia");
          break;
        }
      case 'd': {                                               // Asigna hora
          ho = Serial.parseInt();
          Serial.println("ho");
          break;
        }
      case 'e': {                                               // Asigna minutos
          minu = Serial.parseInt();
          Serial.println("minu");
          break;
        }
      case 'f': {                                               // Asigna segundos
          se = Serial.parseInt();
          Serial.println("se");
          break;
        }
      case 'g': {                                               // Actualiza la fecha y hora
          actualizar();
          Serial.println("Fecha y Hora Actualizada");
          break;
        }
      case 's': {                                               // Asigna hora inicial
          hi = Serial.parseInt();
          Serial.println("hi");
          break;
        }
      case 't': {                                               // Asigna minuto inicial
          mi = Serial.parseInt();
          Serial.println("mi");
          break;
        }
      case 'u': {                                               // Asigna hora final
          hf = Serial.parseInt();
          Serial.println("hf");
          break;
        }
      case 'v': {                                               // Asigna minuto final
          mf = Serial.parseInt();
          Serial.println("mf");
          break;
        }
      case 'w': {                                               // Asigna acción
          ac = Serial.parseInt();
          Serial.println("ac");
          break;
        }
      case 'x': {                                               // Asigna pin de la acción
          pi = Serial.parseInt();
          Serial.println("pi");
          break;
        }
      case 'y': {                                               // Asigna días de la semana a ejecutar
          di = Serial.parseInt();
          Serial.println("di");
          break;
        }
      case 'z': {                                               // Asigna dato adicional
          la = Serial.parseInt();
          Serial.println("la");
          break;
        }
      case 'h': {                                               // Agregar Tarea
          agregarTarea();
          break;
        }
      case 'i': {
          Serial.println(totalTareas);                          // Muestra la cantidad de Tareas
          break;
        }
      case 'j': {
          int x = Serial.parseInt();                            // Muestra la tarea x
          mostrarTarea(x);
          break;
        }
      case 'k': {                                               // Borra todas las tareas
          EEPROM.update(0, 0);
          totalTareas = EEPROM.read(0);
          Serial.println("Tareas borradas");
          //setup();
          break;
        }
      case 'l': {                                               // Muestra la tarea x si esta exita
          int x = Serial.parseInt();
          Serial.print("[");
          Serial.print(x);
          Serial.print("] = ");
          Serial.println(EEPROM.read(x));
          break;
        }
      case 'm': {
          int x = Serial.parseInt();
          digitalWrite(x, HIGH);
          delay(10000);
          digitalWrite(x, LOW);
          break;
        }
      case 'n': {
          byte aux1 = 48;
          byte aux = Serial.read() - aux1;
          Serial.write(aux);
          digitalWrite(7, GetBit(aux, 4));
          Serial.println(GetBit(aux, 4));
          digitalWrite(8, GetBit(aux, 5));
          Serial.println(GetBit(aux, 5));
          delay(2000);
          digitalWrite(9, GetBit(aux, 6));
          Serial.println(GetBit(aux, 6));
          digitalWrite(10, GetBit(aux, 7));
          Serial.println(GetBit(aux, 7));
          break;
        }
      case 'o': {

          break;
        }
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Hora: ");                                          // Mostramos hora y fecha en la pantalla lcd
  lcd.setCursor(6, 0);
  lcd.print(hora());                                            // Obtenemos la hora
  lcd.setCursor(0, 1);
  lcd.print(fecha());                                           // Obtenemos la fecha
  lcd.setCursor(11, 1);
  lcd.print(dias());                                            // Obtenemos el día de la semana

  uint8_t MESZ = RTC.isMEZSummerTime();                         // Toda esta parte no la entiendo

  RTC.getRAM(0, (uint8_t *)&lastAddr, sizeof(uint16_t));
  lastAddr = lastAddr + 1;                                      // we want to use it as addresscounter for example
  RTC.setRAM(0, (uint8_t *)&lastAddr, sizeof(uint16_t));
  RTC.getRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
  if (TimeIsSet == 0xaa55)                                      // check if the clock was set or not
  {

  }
  else
  {

  }                                                             // Hasta aquí es la parte que no entiendo :D
  comprobarTareas();
  delay(1000);                                                  // wait a second motherfucka
  //enviarDatos();                                                // Envia los datos de los sensores por el puerto serial

}
//<-
//<------------------------------------------------------------- Final loop

//-------------------------------------------------------------> Sección de reloj
//-->
String fecha() {
  String aux = "";
  if (RTC.day < 10)                                             // correct date if necessary
  {
    aux = aux + "0" + RTC.day;
  }
  else
  {
    aux = aux + RTC.day;
  }
  aux = aux + "-";
  if (RTC.month < 10)                                           // correct month if necessary
  {
    aux = aux + "0" + RTC.month;
  }
  else
  {
    aux = aux + RTC.month;
  }
  aux = aux + "-" + RTC.year;
  return (aux);
}

String hora() {
  String aux = "";
  RTC.getTime();
  Hora = RTC.hour;
  if (Hora < 10)                                                // correct hour if necessary
  {
    aux = aux + "0" + Hora;
  }
  else
  {
    aux = aux + Hora;
  }
  aux = aux + ":";
  Minuto = RTC.minute;
  if (Minuto < 10)                                              // correct minute if necessary
  {
    aux = aux + "0" + Minuto;
  }
  else
  {
    aux = aux + Minuto;
  }
  aux = aux + ":";
  if (RTC.second < 10)                                          // correct second if necessary
  {
    aux = aux + "0" + RTC.second;
  }
  else
  {
    aux = aux + RTC.second;
  }
  return (aux);
}

String dias() {
  String aux = "";
  diaSemana = RTC.dow;
  switch (diaSemana)                                             // Friendly printout the weekday
  {
    case 1:
      aux = "LUN";
      break;
    case 2:
      aux = "MAR";
      break;
    case 3:
      aux = "MIE";
      break;
    case 4:
      aux = "JUE";
      break;
    case 5:
      aux = "VIE";
      break;
    case 6:
      aux = "SAB";
      break;
    case 0:
      aux = "DOM";
      break;
  }
  return (aux);
}

void actualizar() {
  act = true;
  setup();
  act = false;
  setup();
}
//<-
//<------------------------------------------------------------- Final Sección de reloj

//-------------------------------------------------------------> Sección de sensores
//-->
void enviarDatos() {
  hs = map(analogRead(A6), 0, 1023, 0, 255);
  Serial.println("S" + String(hs));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tem: ");
  lcd.setCursor(5, 0);
  lcd.setCursor(8, 0);
  lcd.print("HRA: ");
  lcd.setCursor(14, 0);
  lcd.setCursor(0, 1);
  lcd.print("  HRSuelo: ");
  lcd.setCursor(11, 1);
  lcd.print(hs);
  delay(1000);
  lcd.clear();
}
//<-
//<------------------------------------------------------------- Final sección de sensores

//-------------------------------------------------------------> Sección tareas
//-->
void agregarTarea() {                                            // Agrega una nueva tarea en la posicion correspondiente
  if (EEPROM.read(0) == 20) {
    Serial.println("Sin espacio para agregar tarea");
    return;
  }
  EEPROM.update(0, EEPROM.read(0) + 1);                          // Aumenta en 1 la cantidad de tareas
  Tarea variable;
  variable.h1 = hi;
  variable.m1 = mi;
  variable.h2 = hf;
  variable.m2 = mf;
  variable.a = ac;
  variable.p = pi;
  variable.d = di;
  variable.l = la;
  EEPROM.put((8 * EEPROM.read(0)) - 7, variable);                // Agrega la tarea al EEPROM
  actualizarTareas();
  Serial.print("Tarea ");
  Serial.print(totalTareas);
  Serial.println(" Agregada");
}

void mostrarTarea(int a) {                                       // Muestra una tarea desde la posición a
  if (a <= 0 || a > 20) {
    Serial.println("Tarea invalida!");
    return;
  }
  if (totalTareas == 0) {
    Serial.println("No hay tareas!");
    return;
  } else {
    Tarea b;
    b = Tareas[a - 1];
    //EEPROM.get((8*a)-7,b);                                     //Así depronto genera un desgaste en la EEPROM mejor lo pasé a un arreglo
    Serial.print((String)"-Tarea: " + a + "-");
    Serial.print((String)" HoIni: " + b.h1);
    Serial.print((String)" MiIni: " + b.m1);
    Serial.print((String)" HoFin: " + b.h2);
    Serial.print((String)" MiFin: " + b.m2);
    Serial.print((String)" Accion: " + b.a);
    Serial.print((String)" Pin: " + b.p);
    Serial.print((String)" Dias: " + b.d);
    Serial.println((String)" Dato: " + b.l);
  }
}

void actualizarTareas() {
  totalTareas = EEPROM.read(0);
  for (int i = 1; i <= totalTareas; i++) {
    Tarea b;
    EEPROM.get((8 * i) - 7, b);
    Tareas[i - 1] = b;
  }
  Serial.println("EEPROM y RAM sincronizadas");
}

bool GetBit( byte N, int pos) {                                  // pos = 7 6 5 4 3 2 1 0 éste me lo robé de aquí http://www.prometec.net/operaciones-bits/
  int b = N >> pos ;                                             // Shift bits
  b = b & 1 ;                                                    // coger solo el ultimo bit
  return b ;
}

void comprobarTareas() {                                         // Aquí empieza "cristo a padecer" como diría mi abuela
  if (totalTareas == 0) {                                        // Si no hay tareas para comprobar
    return;
  }
  tareaActiva = false;
  for (int i = 0; i < totalTareas; i++) {
    if (Tareas[i].h1 <= Hora && Tareas[i].h2 >= Hora && Tareas[i].m1 <= Minuto && Tareas[i].m2 >= Minuto) { // si la hora esta dentro del rango de la tarea programada
      Serial.println("Tarea "+String(i,DEC) +": Activa");
      acciones(i, true);
    } else {                                                                                                 // si la hora no está en el rango entonces la tarea está apagada
      Serial.println("Tarea "+String(i,DEC) +": Inactiva");
      acciones(i, false);
    }
  }
  Serial.println("-------------");
}

void acciones(int a, bool enabled) {
  //Serial.println("- Pin: "+ (String)Tareas[a].p); //// Borrar estas cosas
  switch (Tareas[a].a) {
    case 1: {
        Serial.println("- Control HS"); // Borrar estas cosas
        if (enabled == true) {
          if (hs <= Tareas[a].l) {
            if (estadosPines[Tareas[a].p - 7] == 20) {
              digitalWrite(Tareas[a].p, HIGH);
              estadosPines[Tareas[a].p - 7] = a;
            }
          } else {
            if (estadosPines[Tareas[a].p - 7] == a) {
              digitalWrite(Tareas[a].p, LOW);
              estadosPines[Tareas[a].p] = 20;
            }
          }
        } else {
          if (estadosPines[Tareas[a].p - 7] == a) {
            digitalWrite(Tareas[a].p, LOW);
            Serial.println("Apagado, Humedad: " + (String)hs);
            estadosPines[Tareas[a].p - 7] = 20;
          }
        }
        break;
      }
    case 2: {
        if (enabled == true) {
          if (estadosPines[Tareas[a].p - 7] == 20) {
            digitalWrite(Tareas[a].p, HIGH);
            Serial.println(" -Luz Encendida");
            estadosPines[Tareas[a].p - 7] = a;
          }

        } else {
          if (estadosPines[Tareas[a].p - 7] == a) {
            digitalWrite(Tareas[a].p, LOW);
            Serial.println(" -Luz Apagada");
            estadosPines[Tareas[a].p - 7] = 20;
          }
        }
        break;
      }
    case 3: {
        Serial.println("entre caso 3");
        break;
      }
    case 4: {

        break;
      }
    case 5: {
        Serial.println("Quejesto");
        break;
      }
  }
}

//<-
//<------------------------------------------------------------- Final tareas
