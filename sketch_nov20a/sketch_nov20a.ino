/*
 * 10.02.2021
 * ************************************
 * Сергей Новиков. nsv47nsv36@yandex.ru
 * ************************************
 * acc = 200
 * sp = 9-148(162) мм/сек
 * 1600 импульсов на оборот
 * //----------------------------------------------------------------------------------------------------------
 * Скетч выдает импульсы, количество импульсов определяется расстоянием, которое можно задать.
 * Чтобы задать расстояние необходимо нажать клавишу SET, при этом на OLED дисплее выделится
 * первая строка, при еще одном нажатии выделится следующая строка, при следующем нажатии - выделение исчезнет.
 * Когда строка выделена можно крутить ручку энкодера и на дисплее будет отображаться изменение параметра.
 * При нажатии на кнопку "Движение" произойдет перемещение.
 * //----------------------------------------------------------------------------------------------------------
 * скорость нельзя ставить больше, чем значение acc
 * //----------------------------------------------
 * ускорение в попугаях
 * //------------------------------------------------------------------
 * установка отрицательного значения в скорости приводит к зависанию - 
 * сделать переменную беззнаковой
 * //------------------------------------------------------------------
 * 15.02.21
 * при сбрасывании контроллера переменные сбрасываются до значений
 * по умолчанию - хранить в eeprom
 * //------------------------------------------------------------------
 * delay в функции rotation
 * //------------------------------------------------------------------
 * очень много места занимает библиотека для дисплея - использовать 
 * только те функции, которые необходимы
 * //----------------------------------------------------------------
 * 17.02.21
 * нет сигнала direction (подключил к D7, но не запрограммировал)
 * нет концевиков
 * подсветка кнопки вперед\вверх - D9
 * подсветка кнопки назад\вниз - D10
 * подсветка кнопки SET - D11
 * //----------------------------------------------------------------
 * 18.02.21
 * нет возможности поменять шаг винта
 * баг, запрограммировал включение и отключение кнопок подсветки,
 * но при последующем переходе в меню подсветки значение обнуляется
 * //----------------------------------------------------------------
 * 19.03.21
 * не все платы NANO работают с softSerial
 * не работало и не мог понять почему, но
 * когда заменил плату - заработало
 * //-----------------------------------------------------------------
 * проверить RX TX - работает, но RX-RX TX-TX или крест-накрест
 * //-----------------------------------------------------------------
 * !Баг. При отключении питания питание лазера фактически сбрасывается, 
 * но на дисплее кнопка остается включенной
 * //-----------------------------------------------------------------
 * 18.09.21
 * избавился от delay при формировании импульсов для двигателя.
 * Пытаюсь прикрутить VL53LOX, но иногда он вешает всю систему
 * //-----------------------------------------------------------------
 * 18.09.21
 * добавить установку состояний новых кнопок на дисплее, например,
 * 0.1, 0.5, 1 и 5
 * //-----------------------------------------------------------------
 * 18.09.21
 * рассмотреть функцию controlFromTheDisplay() в блоке pow_on. Там есть комментарий.
 * 372 строка.
 * //-----------------------------------------------------------------
 * 261121
 * дописать функцию, которая будет гасить квадрат на дисплее при достижении позции
 * //-----------------------------------------------------------------
 * исправить в дисплее program_false
 * //-----------------------------------------------------------------
 * неудобно меннять скорость одной глобальной переменной
 * //-----------------------------------------------------------------
 * реализовать функцию визуализации области чистки!
 * //-----------------------------------------------------------------
 * 271121
 * у меня возможны проблемы с использованием git. Последние изменения есть на зеленой флешке
 * //-----------------------------------------------------------------
 */

#include <SoftwareSerial.h>

//bool state_LED_BUILTIN = LOW;
const byte port_stepOut_X = 6;
const byte port_stepOut_Y = 2;
const byte port_stepOut_Z = 13;
const byte port_direction_X = 7;
const byte port_direction_Y = 14;
const byte port_direction_Z = 15;
const uint8_t pinRX = 5;
const uint8_t pinTX = 4;
const uint8_t port_power = 8;
const uint8_t port_power_laser = 9;
const uint8_t port_light = 10;
const uint8_t port_EN_ROT_DEV = 11;
const uint8_t port_limit_up = 12;
const uint8_t port_limit_down = 3;
const uint8_t port_laser_issue_enable = 16;

//bool old_state_limit_up = LOW; //не используются
//bool old_state_limit_down = LOW; //не используются
//unsigned long timer_limit_up = 0; // удалить, если все работает, в других моделях тоже
unsigned long timer_impulse = 0;

bool state_power = false; //на 96 стр есть похожая переменная, может можно объединить???

bool state_port_stepOut_X = LOW;
bool state_port_stepOut_Y = LOW;
bool state_port_stepOut_Z = LOW;

SoftwareSerial softSerial(pinRX,pinTX); 

int T = 625; // чем меньше, тем выше частота вращения
int mySpeed = 300; //скорость в мм/сек, 1 об/сек = 1600 имп/сек = 0.000625 сек

int acceleration = 100; // чем больше, с тем меньшей скорости начинаем движение
const byte screwPitch = 10; // Убрать const при настройке для оператора
float distance_global = 1;

double theDifferenceIsActual = 0; //переменная для количества миллиметров до фокуса на столе фактическая

int16_t position_X = 0;
int16_t position_Y = 0;
int16_t position_Z = 0;

//--------------------------------------------------------------------------------
bool state_pow_on = false; //чтобы отследить первую из двух команд от дисплея. 
//Глобальная, потому что заходим в функцию два раза из-за особенностей softSerial
//на 81 стр есть похожая переменная, может можно объединить???
//скорее всего нет, потому что эта переменная для того чтобы нельзя было включить 
//лазер без общего питания???
//--------------------------------------------------------------------------------

char axis_global = char(0);

struct point {
	int coordinate_X_struct;
	int coordinate_Y_struct;
};

bool program = false; // переменная для режима работы (ручное управление либо движение по заданным квадратам)
point cleaningTask[100]; // массив для хранения заданных квадратов
byte pos_cleaningTask = 0; // переменная для движения по массиву

const int16_t coordinate_X_arr[] PROGMEM = {25, 75, 125, 175, 225, 275, 325, 375, 425, 475};
const int16_t coordinate_Y_arr[] PROGMEM = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};

/*
void impulse(int& T, long& pulses){
  pulses*=2;
  while(pulses){
    bool state_port_limit_up = digitalRead(port_limit_up);
    bool state_port_limit_down = digitalRead(port_limit_down);
    if(micros() - timer_impulse >= T){
      state_port_stepOut = !state_port_stepOut;
      digitalWrite(port_stepOut, state_port_stepOut);
      timer_impulse = micros();
      pulses--;
    }
    if(state_port_limit_up || state_port_limit_down){
      pulses = 0; // модернизировать таким образом, чтобы при сбрасывании задания (задаем количество импульсов на перемещение) это учитывалось в опредеении местоположения (для функции выезда в положение 
                  //фокуса на рабочий стол)
    }
    
  }
}
*/

void impulse(int& T, long& pulses, bool& state_port_stepOut, uint8_t& port_stepOut){
  pulses*=2;
  while(pulses){
    //bool state_port_limit_up = digitalRead(port_limit_up);
    //bool state_port_limit_down = digitalRead(port_limit_down);
	bool state_port_limit_up = false;
    bool state_port_limit_down = false;
    if(micros() - timer_impulse >= T){
      state_port_stepOut = !state_port_stepOut;
      digitalWrite(port_stepOut, state_port_stepOut);
      timer_impulse = micros();
      pulses--;
    }
    if(state_port_limit_up || state_port_limit_down){
      Serial.println(F("pulses 0!"));
	  pulses = 0; // модернизировать таким образом, чтобы при сбрасывании задания (задаем количество импульсов на перемещение) это учитывалось в опредеении местоположения (для функции выезда в положение 
                  //фокуса на рабочий стол)
    }
    
  }
}
/*
void rotation(long pulses, int T){
    bool state_port_limit_up = digitalRead(port_limit_up);
    bool state_port_limit_down = digitalRead(port_limit_down);
    bool state_port_direction = digitalRead(port_direction);
    if(state_port_limit_up){
      if(!state_port_direction){
        if(millis()%500<=5){
			delay(5); 
			Serial.println("limit UP!");
        }
      }else{
        impulse(T, pulses);
        //state_port_limit_up = digitalRead(port_limit_up); //удалить если все работает
      }
    }else 
      if(state_port_limit_down){
        if(state_port_direction){
          if(millis()%500<=5){
            delay(5); 
            Serial.println("limit DOWN!");
          }
        }else{
          impulse(T, pulses);
        }
      }
    else{
      impulse(T, pulses);
      state_port_limit_up = digitalRead(port_limit_up); //удалить если все работает
    }
    
    //if(state_port_limit_down){
      //if(state_port_direction){
        //if(millis()%500<=5){delay(5); 
          //Serial.println("limit DOWN!");
        //}
      //}else{
        //impulse(T, pulses);
      //}
    //}//else{
      //impulse(T, pulses);
    //}
}
*/

void printPosition(){
	Serial.print("текущая позиция X:");
	Serial.println(position_X);
	Serial.print("текущая позиция Y:");
	Serial.println(position_Y);
}

void position(){
	bool state_port_direction;
	if(axis_global==char(88)){
		state_port_direction = digitalRead(port_direction_X);
		
		if(state_port_direction){
			position_X += distance_global;
			printPosition();
		}else{
			position_X -= distance_global;
			printPosition();
		}
	}else 
		if(axis_global==char(89)){
			state_port_direction = digitalRead(port_direction_Y);
			
			if(state_port_direction){
				position_Y += distance_global;
				printPosition();
			}else{
				position_Y -= distance_global;
				printPosition();
			}
		}
}

void rotation(long pulses, int T, bool& state_port_stepOut, uint8_t& port_stepOut){ 
    //bool state_port_limit_up = digitalRead(port_limit_up);
    //bool state_port_limit_down = digitalRead(port_limit_down);
	bool state_port_limit_up = false;
    bool state_port_limit_down = false;
    bool state_port_direction;
	if(axis_global==char(88)){
		state_port_direction = digitalRead(port_direction_X);
	}else 
		if(axis_global==char(89)){
			state_port_direction = digitalRead(port_direction_Y);
		}else
			if(axis_global==char(90)){
				state_port_direction = digitalRead(port_direction_Z);
			}
	
    if(state_port_limit_up){
      if(!state_port_direction){
        if(millis()%500<=5){
			delay(5); 
			Serial.println("limit UP!");
        }
      }else{
        impulse(T, pulses, state_port_stepOut, port_stepOut);
		
        //state_port_limit_up = digitalRead(port_limit_up); //удалить если все работает
      }
    }else 
      if(state_port_limit_down){
        if(state_port_direction){
          if(millis()%500<=5){
            delay(5); 
            Serial.println("limit DOWN!");
          }
        }else{
          impulse(T, pulses, state_port_stepOut, port_stepOut);
		  
        }
      }
    else{
      impulse(T, pulses, state_port_stepOut, port_stepOut);
	  
      //state_port_limit_up = digitalRead(port_limit_up); //удалить если все работает
    }
}

/*
void acceleration_function(int initialFreqiency, int finalFrequency){
  for(int i = initialFreqiency; i > finalFrequency; i--){ 
    rotation(8, i);
  }
}

void bracking_function(int initialFreqiency, int finalFrequency){
  for(int i = initialFreqiency; i < finalFrequency; i++){
    rotation(8, i);
  }
}
*/

void acceleration_function(int initialFreqiency, int finalFrequency, bool& state_port_stepOut, uint8_t& port_stepOut){
  for(int i = initialFreqiency; i > finalFrequency; i--){ 
    rotation(8, i, state_port_stepOut, port_stepOut);
  }
}

void bracking_function(int initialFreqiency, int finalFrequency, bool& state_port_stepOut, uint8_t& port_stepOut){
  
  for(int i = initialFreqiency; i < finalFrequency; i++){
    rotation(8, i, state_port_stepOut, port_stepOut);
  }
}
/*
void action(float distance, int mySpeed, int acceleration){
  int T = (float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed/2;
  long numberOfPulses = distance*1600L/screwPitch;
  long pulsesOnTheAcceleration = (acceleration - T)*8L; //разбил оборот на 200 частей
  if(pulsesOnTheAcceleration*2L>=numberOfPulses){  
  acceleration_function(acceleration, acceleration-numberOfPulses/2/8);
  bracking_function(acceleration-numberOfPulses/2/8, acceleration);
  }else{
    acceleration_function(acceleration, T);
    rotation(numberOfPulses-pulsesOnTheAcceleration*2L, T);
    bracking_function(T, acceleration);
  }
}
*/
void action(char axis_local_action, float distance_local_action, int mySpeed, int acceleration){
	distance_global = distance_local_action; // вылезла ошибка. Для Z axis пришлось ввести доп локальную переменную, чтобы увеличить dist в 2 раза. Но произошел лавинообразный эффект
	axis_global = axis_local_action;
	position(); // нет учета попадания на концевик!!!!
	uint8_t port_direction = 0;
	bool state_port_stepOut = LOW;
	uint8_t port_stepOut = 0;
	if(axis_local_action==char(88)){
		state_port_stepOut = state_port_stepOut_X;
		port_stepOut = port_stepOut_X;
	}else
		if(axis_local_action==char(89)){
			state_port_stepOut = state_port_stepOut_Y;
			port_stepOut = port_stepOut_Y;
		}else 
			if(axis_local_action==char(90)){
				state_port_stepOut = state_port_stepOut_Z;
				//distance_global*=2; // На Z оси другой шаг винта. Лавинообразный эффект.
				distance_local_action*=2;
				port_stepOut = port_stepOut_Z;
		}else{
			Serial.println("The axis is not connected");
		}
	int T = (float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed/2;
	long numberOfPulses = distance_local_action*1600L/screwPitch;
	long pulsesOnTheAcceleration = (acceleration - T)*8L; //разбил оборот на 200 частей
	if(pulsesOnTheAcceleration*2L>=numberOfPulses){  
		acceleration_function(acceleration, acceleration-numberOfPulses/2/8, state_port_stepOut, port_stepOut);
		bracking_function(acceleration-numberOfPulses/2/8, acceleration, state_port_stepOut, port_stepOut);
	}else{
		if(pulsesOnTheAcceleration>=0){                                                               // ускорение задается в попугаях. На мылых скоростях количество импульсов
			acceleration_function(acceleration, T, state_port_stepOut, port_stepOut);                 // может принимать отрицательное значение.
			long value = numberOfPulses-pulsesOnTheAcceleration*2L;
			rotation(value, T, state_port_stepOut, port_stepOut);
			bracking_function(T, acceleration, state_port_stepOut, port_stepOut);
		}else{
			rotation(numberOfPulses, T, state_port_stepOut, port_stepOut);
		}
  }
}

int readdata(){                              //Эта функция возвращает значение переменной в int, введенной в Serialmonitor  
  byte getByte;
  int outByte=0;
  do{
    while(Serial.available()!=0){ 
      getByte=Serial.read()-48;     // Вычитаем из принятого символа 48 для преобразования из ASCII в int
      outByte=(outByte*10)+getByte; //Сдвигаем outByte на 1 разряд влево, и прибавляем getByte 
      delay(500);                   // Чуть ждем для получения следующего байта из буфера (Чем больше скорость COM, тем меньше ставим задержку. 500 для скорости 9600, и приема 5-значных чисел)
    }
    }while (outByte==0);            //Зацикливаем функцию для получения всего числа
  Serial.println("Ok");
  Serial.flush();                   //Вычищаем буфер (не обязательно)
  return outByte;
}

void departure_to_the_square(const int16_t& coordinate_X, const int16_t& coordinate_Y){
	if(position_X - coordinate_X < 0){
		digitalWrite(port_direction_X, HIGH);
	}else{
		digitalWrite(port_direction_X, LOW);
	}
	action(char(88), abs(position_X - coordinate_X), mySpeed, acceleration);
		
	if(position_Y - coordinate_Y < 0){
		digitalWrite(port_direction_Y, HIGH);
	}else{
		digitalWrite(port_direction_Y, LOW);
	}
	action(char(89), abs(position_Y-coordinate_Y), mySpeed, acceleration);
}

void check_out_or_record(const int16_t& coordinate_X, const int16_t& coordinate_Y){
	if(!program){
		departure_to_the_square(coordinate_X, coordinate_Y);
	}else{
		cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
		cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
	}
}


/*
void movingToZero(double count){             // Эта функция перемещает систему в положение нуля
  if (count>0){
    digitalWrite(port_direction, HIGH);
  }else{
    digitalWrite(port_direction, LOW);
  }
  action(abs(count), mySpeed, acceleration);
}
*/
/*
void focusOnTheTable(){                      // Эта функция при первом включении уводит систему на верхний концевик, а потом перемещает систему в положение нуля и обнуляет положение
   Serial.println("focus on the table!");   
   if(!state_pow_on){
     digitalWrite(port_direction, LOW);
     action(500, mySpeed, acceleration);
     digitalWrite(port_direction, HIGH);
     action(142, mySpeed, acceleration);
     theDifferenceIsActual = 0;
   }else{
      movingToZero(theDifferenceIsActual);
      theDifferenceIsActual = 0;
   }
}
*/
void terminal(){                             // Эта функция выводит в монитор порта информацию о настройках системы
  Serial.println(F("Готов"));
  Serial.println(F("---------------------------"));
  Serial.print(F("+ "));
  Serial.print(F("Количество миллиметров: "));
  Serial.println(distance_global);
  Serial.print(F("+ "));
  Serial.print(F("Скорость: "));
  Serial.println(mySpeed);
  Serial.print(F("+ "));
  Serial.print(F("Ускорение: "));
  Serial.println(acceleration);
  Serial.print(F("+ "));
  Serial.print(F("Скорость мкс: "));
  int value_tmp = (float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed;
  Serial.println(value_tmp);
  Serial.println(F("---------------------------"));
}
/*
void settingTheDisplayButtonStates(){        // Эта функция устанавливает состояние выходов в соответствие с состоянием кнопок на дисплее
  //  Устанавливаем состояние кнопки bt0:  
  softSerial.print((String) "print bt0.val"+char(255)+char(255)+char(255)); // Отправляем команду дисплею: «print bt0.val» заканчивая её тремя байтами 0xFF
  while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt0, отправив 4 байта данных, 
                                                                            // где первый байт равен 0x01 или 0x00, а остальные 3 равны 0x00
  digitalWrite(port_power, softSerial.read());                              // Устанавливаем на выходе port_power состояние в соответствии с первым принятым байтом ответа дисплея
  delay(10);
  while(softSerial.available()){
    softSerial.read();
    delay(10);
  }
  //-----------------------------------------------------------------------
  softSerial.print((String) "print bt1.val"+char(255)+char(255)+char(255));
  while(!softSerial.available()){}
  digitalWrite(port_power_laser, softSerial.read());         
  state_LED_BUILTIN = !state_LED_BUILTIN;
  digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
  delay(10);
  while(softSerial.available()){
    softSerial.read();
    delay(10);
  }
  //-----------------------------------------------------------------------
  softSerial.print((String) "print bt2.val"+char(255)+char(255)+char(255));
  while(!softSerial.available()){}
  digitalWrite(port_light, softSerial.read());         
  state_LED_BUILTIN = !state_LED_BUILTIN;
  digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
  delay(10);
  while(softSerial.available()){
    softSerial.read();
    delay(10);
  }
}
*/




void setup() {
 
  Serial.begin(9600);
  
  softSerial.begin(9600);

  pinMode(port_stepOut_X, OUTPUT);  
  pinMode(port_stepOut_Y, OUTPUT);  
  pinMode(port_stepOut_Z, OUTPUT);
  pinMode(port_direction_X, OUTPUT);
  pinMode(port_direction_Y, OUTPUT);
  pinMode(port_direction_Z, OUTPUT);
  pinMode(port_power, OUTPUT);
  pinMode(port_power_laser, OUTPUT);
  pinMode(port_light, OUTPUT);
  pinMode(port_EN_ROT_DEV, OUTPUT);
  pinMode(port_limit_up, INPUT_PULLUP);
  pinMode(port_limit_down, INPUT_PULLUP);
  pinMode(port_laser_issue_enable, OUTPUT);
  
  terminal();
  //-----------------------------------------------------------------------
  //settingTheDisplayButtonStates();
  //-----------------------------------------------------------------------
  Serial.println("Ready!");
}

void loop() {

  controlFromTheDisplay();
  
  controlUart();
  
  if(!program){
	pos_cleaningTask = 0;
	mySpeed = 300;
		for(byte i = 0; i < 100; ++i){
			cleaningTask[i].coordinate_X_struct = -1;
			cleaningTask[i].coordinate_Y_struct = -1;
		}
  }
}
