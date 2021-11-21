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
 */

#include <SoftwareSerial.h>

bool state_LED_BUILTIN = LOW;
const byte port_stepOut_X = 6;
const byte port_stepOut_Y = 2;
const byte port_direction_X = 7;
const byte port_direction_Y = 14;
const uint8_t pinRX = 5;
const uint8_t pinTX = 4;
const uint8_t port_power = 8;
const uint8_t port_las = 9;
const uint8_t port_light = 10;
const uint8_t port_EN_ROT_DEV = 11;
const uint8_t port_limit_up = 12;
const uint8_t port_limit_down = 3;

bool old_state_limit_up = LOW;
bool old_state_limit_down = LOW;
//unsigned long timer_limit_up = 0; // удалить, если все работает, в других моделях тоже
unsigned long timer_impulse = 0;

bool state_power = false; //на 96 стр есть похожая переменная, может можно объединить???

bool state_port_stepOut_X = LOW;
bool state_port_stepOut_Y = LOW;

SoftwareSerial softSerial(pinRX,pinTX); 

int T = 625; // чем меньше, тем выше частота вращения
int mySpeed = 30; //скорость в мм/сек, 1 об/сек = 1600 имп/сек = 0.000625 сек

int acceleration = 300; // чем больше, с тем меньшей скорости начинаем движение
byte screwPitch = 8;
float distance_global = 1;

double theDifferenceIsActual = 0; //переменная для количества миллиметров до фокуса на столе фактическая

int16_t position_X = 0;
int16_t position_Y = 0;

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

bool program = false;
point cleaningTask[25]; 
byte pos_cleaningTask = 0;

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

void rotation(long pulses, int T, bool& state_port_stepOut, uint8_t& port_stepOut){ // убрать из rotation state_port_direction и port_direction
    bool state_port_limit_up = digitalRead(port_limit_up);
    bool state_port_limit_down = digitalRead(port_limit_down);
    bool state_port_direction;
	if(axis_global==char(88)){
		state_port_direction = digitalRead(port_direction_X);
	}else 
		if(axis_global==char(89)){
			state_port_direction = digitalRead(port_direction_Y);
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
	distance_global = distance_local_action;
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
			rotation(numberOfPulses-pulsesOnTheAcceleration*2L, T, state_port_stepOut, port_stepOut);
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

void departure_to_the_square(int16_t& coordinate_X, int16_t& coordinate_Y){
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

void controlUart(){                          // Эта функция позволяет управлять системой из монитора порта
  if (Serial.available()) {         // есть что на вход
	int16_t coordinate_X;
	int16_t coordinate_Y;
	
    String cmd;
    cmd = Serial.readString();
    if (cmd.equals("d")) {           
      Serial.println("введите количество миллиметров");      
      distance_global = readdata();
      Serial.print("distance: ");
      Serial.println(distance_global);
    }else if (cmd.equals("sp")) {           
      Serial.println("введите скорость");      
      mySpeed = readdata();
      Serial.print("скорость: ");
      Serial.println(mySpeed);
      Serial.print("T: ");
      Serial.println((float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed);
    }else if (cmd.equals("acc")) {           
      Serial.println("введите ускорение");       
      acceleration = readdata();
      Serial.print("acceleration: ");
      Serial.println(acceleration);
    }else if (cmd.equals("start1")) {          
      Serial.println("Поехали!");
      //action(distance, mySpeed, acceleration);
    }else if(cmd.equals("up")){
      Serial.println("Поехали!");
      
	  if(axis_global==char(88)){
		digitalWrite(port_direction_X, LOW);
		action(char(88), distance_global, mySpeed, acceleration);
	  }else
		  if(axis_global==char(89)){
			digitalWrite(port_direction_Y, LOW);
			action(char(89), distance_global, mySpeed, acceleration);
		  }
    }else if(cmd.equals("down")){
		Serial.println("Поехали!");
		if(axis_global==char(88)){
			digitalWrite(port_direction_X, HIGH);
			action(char(88), distance_global, mySpeed, acceleration);
	  }else
		if(axis_global==char(89)){
			digitalWrite(port_direction_Y, HIGH);
			action(char(89), distance_global, mySpeed, acceleration);
		  }
    }else if(cmd.equals("focus")){
      Serial.println("focus!");
      //focusOnTheTable();
      //digitalWrite(port_direction, LOW);
      //action(500, mySpeed, acceleration);
      //digitalWrite(port_direction, HIGH);
      //action(100, mySpeed, acceleration);
    }else if(cmd.equals("sensorInit")){
      Serial.println("debugging information");
    }else if(cmd.equals("sensorRead")){     
      Serial.print("debugging information");
    }else if(cmd.equals("autoFocus")){     
      Serial.println("this feature is in development");
    }else if(cmd.equals("pow_ON")){
      digitalWrite(port_power, HIGH);
      state_power = true;
      //------------------------------------------------------------------------------------------------------------------
      // скорее всего этот блок кода для случая, когда кнопка питания лазера уже активирована, а основного питания еще нет. При этом выход контроллера включит
      // питание лазера. Рассмотреть правильность, может быть не позволять активировать кнопку включения питания лазера без основного питания???
      // Устанавливаем состояние кнопки включения питания лазера:                  
      /*
      softSerial.print((String) "print bt1.val"+char(255)+char(255)+char(255)); // Отправляем команду дисплею: «print bt1.val» заканчивая её тремя байтами 0xFF
      while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt1, отправив 4 байта данных, где 1 байт равен 0x01 или 0x00, а остальные 3 равны 0x00
      digitalWrite(port_las, softSerial.read());       delay(10);               // Устанавливаем на выходе port_las состояние в соответствии с первым принятым байтом ответа дисплея
      while(softSerial.available()){softSerial.read(); delay(10);}
      //------------------------------------------------------------------------------------------------------------------
      //delay(500);          // удалить ели все работает
      */
      
//      if(!state_pow_on){     // при первом включении отправить систему на верхний концевик
//        delay(1000);
//        focusOnTheTable();
//        state_pow_on = true;
//      }
    }
    else 
      if(cmd.equals("las_ON")){
        if(state_power){
          digitalWrite(port_las, HIGH);
        }
      }
    else 
      if(cmd.equals("las_OFF")){
        digitalWrite(port_las, LOW);
      }
    else 
		if(cmd.equals("home")){
			mySpeed = 30;
			int coord_X = 0;
			int coord_Y = 0;
			departure_to_the_square(coord_X, coord_Y);
	}else
		if(cmd.equals("X")){
			axis_global = char(88);
			Serial.println("the x axis selected");
	}else 
		if(cmd.equals("Y")){
			axis_global = char(89);
			Serial.println("the y axis selected");
	}else 
		if(cmd.equals("program")){
			program = true;
			Serial.println("programming mode activated");
	}else 
		if(cmd.equals("program_false")){
			Serial.println("programming mode disable");
			program = false;
			pos_cleaningTask = 0;
			for(byte i = 0; i < 25; ++i){
				cleaningTask[i].coordinate_X_struct = 0;
				cleaningTask[i].coordinate_Y_struct = 0;
			}
	}else
		if(cmd.equals("start")){
			int min_X = 150;
			int max_X = 0;
			
			int min_Y_1 = 150;
			int min_Y_2 = 150;
			int min_Y_3 = 150;
			int min_Y_4 = 150;
			int min_Y_5 = 150;
			
			int max_Y_1 = -1;
			int max_Y_2 = -1;
			int max_Y_3 = -1;
			int max_Y_4 = -1;
			int max_Y_5 = -1;
			
			int coord;
			
			for(byte i = 0; i < pos_cleaningTask; ++i){
				
				if(cleaningTask[i].coordinate_X_struct == 15){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_1){
						min_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_1){
						max_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 45){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_2){
						min_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_2){
						max_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 75){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_3){
						min_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_3){
						max_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 105){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_4){
						min_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_4){
						max_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 135){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_5){
						min_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_5){
						max_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
				}
			}
			
			if(max_Y_1>=0){
				coord = 15;
				mySpeed = 30;
				departure_to_the_square(coord, min_Y_1);
				// включить лазер
				mySpeed = 1;
				departure_to_the_square(coord, max_Y_1);
				// выключить лазер
			}
			
			if(max_Y_2>=0){
				coord = 45;
				mySpeed = 30;
				departure_to_the_square(coord, min_Y_2);
				// включить лазер
				mySpeed = 1;
				departure_to_the_square(coord, max_Y_2);
				// выключить лазер
			}
			
			if(max_Y_3>=0){
				coord = 75;
				mySpeed = 30;
				departure_to_the_square(coord, min_Y_3);
				// включить лазер
				mySpeed = 1;
				departure_to_the_square(coord, max_Y_3);
				// выключить лазер
			}
			
			if(max_Y_4>=0){
				coord = 105;
				mySpeed = 30;
				departure_to_the_square(coord, min_Y_4);
				// включить лазер
				mySpeed = 1;
				departure_to_the_square(coord, max_Y_4);
				// выключить лазер
			}
			
			if(max_Y_5>=0){
				coord = 135;
				mySpeed = 30;
				departure_to_the_square(coord, min_Y_5);
				// включить лазер
				mySpeed = 1;
				departure_to_the_square(coord, max_Y_5);
				// выключить лазер
			}
			
	}else
		if(cmd.equals("a0")){
			coordinate_X = 15;
			coordinate_Y = 0;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("a1")){
			coordinate_X = 15;
			coordinate_Y = 30;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("a2")){
			coordinate_X = 15;
			coordinate_Y = 60;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("a3")){
			coordinate_X = 15;
			coordinate_Y = 90;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("a4")){
			coordinate_X = 15;
			coordinate_Y = 120;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else
		if(cmd.equals("b0")){
			coordinate_X = 45;
			coordinate_Y = 0;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("b1")){
			coordinate_X = 45;
			coordinate_Y = 30;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("b2")){
			coordinate_X = 45;
			coordinate_Y = 60;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else
		if(cmd.equals("b3")){
			coordinate_X = 45;
			coordinate_Y = 90;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("b4")){
			coordinate_X = 45;
			coordinate_Y = 120;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("c0")){
			coordinate_X = 75;
			coordinate_Y = 0;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("c1")){
			coordinate_X = 75;
			coordinate_Y = 30;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else
		if(cmd.equals("c2")){
			coordinate_X = 75;
			coordinate_Y = 60;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("c3")){
			coordinate_X = 75;
			coordinate_Y = 90;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("c4")){
			coordinate_X = 75;
			coordinate_Y = 120;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("d0")){
			coordinate_X = 105;
			coordinate_Y = 0;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("d1")){
			coordinate_X = 105;
			coordinate_Y = 30;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("d2")){
			coordinate_X = 105;
			coordinate_Y = 60;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("d3")){
			coordinate_X = 105;
			coordinate_Y = 90;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("d4")){
			coordinate_X = 105;
			coordinate_Y = 120;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("e0")){
			coordinate_X = 135;
			coordinate_Y = 0;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("e1")){
			coordinate_X = 135;
			coordinate_Y = 30;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("e2")){
			coordinate_X = 135;
			coordinate_Y = 60;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}	
	}else 
		if(cmd.equals("e3")){
			coordinate_X = 135;
			coordinate_Y = 90;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else 
		if(cmd.equals("e4")){
			coordinate_X = 135;
			coordinate_Y = 120;
			if(!program){
				departure_to_the_square(coordinate_X, coordinate_Y);
			}else{
				cleaningTask[pos_cleaningTask].coordinate_X_struct = coordinate_X;
				cleaningTask[pos_cleaningTask++].coordinate_Y_struct = coordinate_Y;
			}
	}else{
      Serial.println("The command is not provided");    // ошибка
    }
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
  Serial.println("Готов");
  Serial.println("---------------------------");
  Serial.print("+ ");
  Serial.print("Количество миллиметров: ");
  Serial.println(distance_global);
  Serial.print("+ ");
  Serial.print("Скорость: ");
  Serial.println(mySpeed);
  Serial.print("+ ");
  Serial.print("Ускорение: ");
  Serial.println(acceleration);
  Serial.print("+ ");
  Serial.print("Скорость мкс: ");
  Serial.println((float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed);
  Serial.println("---------------------------");
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
  digitalWrite(port_las, softSerial.read());         
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
/*
void controlFromTheDisplay(){
  if(softSerial.available()>0){         // Если есть данные принятые от дисплея, то ...
    String str;                         // Объявляем строку для получения этих данных
    while(softSerial.available()){
      str+=char(softSerial.read());     // Читаем принятые от дисплея данные побайтно в строку str
      delay(10);
    }
    Serial.println(str);                
    for(int i=0; i<str.length(); i++){ // Проходимся по каждому символу строки str
      //----------------------------------------------
      if(memcmp(&str[i],"movingUp" , 8)==0){ // Если в строке str начиная с символа i находится текст "movingUp",  значит кнопка дисплея была включена
        i+=7; 
        digitalWrite(port_direction, LOW);
        action(distance, mySpeed, acceleration);
        theDifferenceIsActual += distance;
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"movingDown", 10)==0){
        i+=9; 
        digitalWrite(port_direction, HIGH);
        state_LED_BUILTIN = !state_LED_BUILTIN;
        digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
        action(distance, mySpeed, acceleration);
        theDifferenceIsActual -= distance;
      }else              
      //----------------------------------------------
      if(memcmp(&str[i],"light_ON", 8)==0){
        i+=7; 
        digitalWrite(port_light, HIGH);
        state_LED_BUILTIN = true;
        digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&str[i],"light_OFF", 9)==0){
        i+=8; 
        digitalWrite(port_light, LOW);
        state_LED_BUILTIN = false;
        digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&str[i],"pow_OFF", 7)==0){
        i+=6; 
        digitalWrite(port_power, LOW);
        state_power = false;
        digitalWrite(port_las, LOW);
        state_LED_BUILTIN = false;
        digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&str[i],"pow_ON", 6)==0){
        i+=5; 
        digitalWrite(port_power, HIGH);
        state_power = true;
        //------------------------------------------------------------------------------------------------------------------
        // скорее всего этот блок кода для случая, когда кнопка питания лазера уже активирована, а основного питания еще нет. При этом выход контроллера включит
        // питание лазера. Рассмотреть правильность, может быть не позволять активировать кнопку включения питания лазера без основного питания???
        // Устанавливаем состояние кнопки включения питания лазера:                  
        softSerial.print((String) "print bt1.val"+char(255)+char(255)+char(255)); // Отправляем команду дисплею: «print bt1.val» заканчивая её тремя байтами 0xFF
        while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt1, отправив 4 байта данных, где 1 байт равен 0x01 или 0x00, а остальные 3 равны 0x00
        digitalWrite(port_las, softSerial.read());       delay(10);               // Устанавливаем на выходе port_las состояние в соответствии с первым принятым байтом ответа дисплея
        while(softSerial.available()){softSerial.read(); delay(10);}
        //------------------------------------------------------------------------------------------------------------------
        //delay(500);          // удалить ели все работает
        if(!state_pow_on){     // при первом включении отправить систему на верхний концевик
          focusOnTheTable();
          state_pow_on = true;
        }
      }else 
      //----------------------------------------------
      if(memcmp(&str[i],"las_OFF", 7)==0){
        i+=6; 
        digitalWrite(port_las, LOW);
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"las_ON", 6)==0){
        i+=5;
        if(state_power){
          digitalWrite(port_las, HIGH);
        }
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"EN_ROT_DEV_ON", 13)==0){
        i+=12;
        digitalWrite(port_EN_ROT_DEV, HIGH);
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"EN_ROT_DEV_OFF", 14)==0){
        i+=13;
        digitalWrite(port_EN_ROT_DEV, LOW);
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"set_dist_0.1", 12)==0){
        i+=11;
        distance = 0.1;
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"set_dist_0.5", 12)==0){
        i+=11;
        distance = 0.5;
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"set_dist_1", 10)==0){
        i+=9;
        distance = 1;
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"set_dist_5", 10)==0){
        i+=9;
        distance = 5;
      }else
      //----------------------------------------------
      if(memcmp(&str[i],"focus", 5)==0){
        i+=4;
        Serial.println("focus on the table!");
        focusOnTheTable();
      }
    }
  }
}
*/

void setup() {
 
  Serial.begin(9600);
  
  softSerial.begin(9600);

  pinMode(port_stepOut_X, OUTPUT);  
  pinMode(port_stepOut_Y, OUTPUT);  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(port_direction_X, OUTPUT);
  pinMode(port_direction_Y, OUTPUT);
  pinMode(port_power, OUTPUT);
  pinMode(port_las, OUTPUT);
  pinMode(port_light, OUTPUT);
  pinMode(port_EN_ROT_DEV, OUTPUT);
  pinMode(port_limit_up, INPUT_PULLUP);
  pinMode(port_limit_down, INPUT_PULLUP);
  
  terminal();
  //-----------------------------------------------------------------------
  //settingTheDisplayButtonStates();
  //-----------------------------------------------------------------------
  Serial.println("Ready!");
}

void loop() {

  //controlFromTheDisplay();
  
  controlUart();
  
}
