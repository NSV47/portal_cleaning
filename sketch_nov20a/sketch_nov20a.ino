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
 * position_Z имеет целочисленный тип, а фокусировка будет с точностью 0,5 мм
 * //-----------------------------------------------------------------
 * проверить a1, a2, b1, b2. Не доезжаю по Y одного квадрата из-за координат, необходимо к заданию добавлять по Y один квадрат
 * //-----------------------------------------------------------------
 * 29.11.21
 * Визуализация срабатывает каждый раз при срабатывании условия arr_of_max_Y[0]>-1, а должна работать только один раз
 * //--------------------------------------------------------------------------------------------------------------------------
 */

#include <SoftwareSerial.h>

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

unsigned long timer_impulse = 0;

bool state_power = false; //на 96 стр есть похожая переменная, может можно объединить???

bool state_port_stepOut_X = LOW;
bool state_port_stepOut_Y = LOW;
bool state_port_stepOut_Z = LOW;

SoftwareSerial softSerial(pinRX,pinTX); 

uint16_t T = 625; // чем меньше, тем выше частота вращения
uint16_t movementSpeed = 30; //скорость в мм/сек, 1 об/сек = 1600 имп/сек = 0.000625 сек
uint16_t idleSpeed = 30;
uint16_t cleaningSpeed = 10;

uint16_t acceleration = 300; // чем больше, с тем меньшей скорости начинаем движение
const byte screwPitch = 8; // Убрать const при настройке для оператора
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
	int16_t coordinate_X_struct;
	int16_t coordinate_Y_struct;
};

bool program = false; // переменная для режима работы (ручное управление либо движение по заданным квадратам)
point cleaningTask[100]; // массив для хранения заданных квадратов
byte pos_cleaningTask = 0; // переменная для движения по массиву

const int16_t coordinate_X_arr[] PROGMEM = {25, 75, 125, 175, 225, 275, 325, 375, 425, 475};
const int16_t coordinate_Y_arr[] PROGMEM = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};

bool flag_visualization_is_done = false; // Визуализация срабатывает каждый раз при срабатывании условия arr_of_max_Y[0]>-1, а должна работать только один раз

bool state_port_laser_issue_enable = false;

/*
uint8_t cancel(){
	
	uint8_t value = 0;
	
	if(softSerial.available()>0){         // Если есть данные принятые от дисплея, то ...
		char cmd[15];
		uint8_t pos = 0;
		while(softSerial.available()){
			cmd[pos++] = char(softSerial.read());
			delay(10);
		}
		cmd[pos] = char(0);
		Serial.println(cmd);                
		for(int i=0; i<pos; i++){
			if(memcmp(&cmd[i],"cancel" , 6)==0){ // Если в строке str начиная с символа i находится текст "movingUp",  значит кнопка дисплея была включена
				i+=5; 
				
			}
		}
	}
	
	return value;
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

void printPosition(){
	Serial.print(F("текущая позиция X:"));
	Serial.println(position_X);
	Serial.print(F("текущая позиция Y:"));
	Serial.println(position_Y);
	Serial.println(F("--------------------"));
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
			Serial.println(F("limit UP!"));
        }
      }else{
        impulse(T, pulses, state_port_stepOut, port_stepOut);
      }
    }else 
      if(state_port_limit_down){
        if(state_port_direction){
          if(millis()%500<=5){
            delay(5); 
            Serial.println(F("limit DOWN!"));
          }
        }else{
          impulse(T, pulses, state_port_stepOut, port_stepOut);
        }
      }
    else{
      impulse(T, pulses, state_port_stepOut, port_stepOut);
	  
    }
}

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

void action(char axis_local_action, float distance_local_action, int movementSpeed, int acceleration){
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
			Serial.println(F("The axis is not connected"));
		}
	int T = (float(1.0f/(1600L/screwPitch))*1000000L)/movementSpeed/2;
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

/* ------------------------------------------------------------------------------------
 * Система постоянно запоминает свои координаты. При получении нового задания из текущих
 * координат вычитается значение заданной координаты и это является количеством миллиметров,
 * которые необходио пройти. 
 * ------------------------------------------------------------------------------------
*/ 
void departure_to_the_square(const int16_t& coordinate_X, const int16_t& coordinate_Y){
	
	if(position_X - coordinate_X < 0){
		digitalWrite(port_direction_X, HIGH);
	}else{
		digitalWrite(port_direction_X, LOW);
	}
	action(char(88), abs(position_X - coordinate_X), movementSpeed, acceleration);
		
	if(position_Y - coordinate_Y < 0){
		digitalWrite(port_direction_Y, HIGH);
	}else{
		digitalWrite(port_direction_Y, LOW);
	}
	action(char(89), abs(position_Y-coordinate_Y), movementSpeed, acceleration);
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
  action(abs(count), movementSpeed, acceleration);
}
*/
/*
void focusOnTheTable(){                      // Эта функция при первом включении уводит систему на верхний концевик, а потом перемещает систему в положение нуля и обнуляет положение
   Serial.println("focus on the table!");   
   if(!state_pow_on){
     digitalWrite(port_direction, LOW);
     action(500, movementSpeed, acceleration);
     digitalWrite(port_direction, HIGH);
     action(142, movementSpeed, acceleration);
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
  Serial.println(movementSpeed);
  Serial.print(F("+ "));
  Serial.print(F("Ускорение: "));
  Serial.println(acceleration);
  Serial.print(F("+ "));
  Serial.print(F("Скорость мкс: "));
  int value_tmp = (float(1.0f/(1600L/screwPitch))*1000000L)/movementSpeed;
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

uint16_t calculation_of_the_working_rectangle(int16_t* arr_of_min_Y, int16_t* arr_of_max_Y){
	  
	  uint16_t max_x = 0;
	  
	  for(byte i = 0; i < pos_cleaningTask; ++i){
		
		if(cleaningTask[i].coordinate_X_struct == 25){
			
			max_x = 25; // переменная для визуализации рабочего прямоугольника
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[0]){
				arr_of_min_Y[0] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[0]){
				arr_of_max_Y[0] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 75){
			
			max_x = 75;
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[1]){
				arr_of_min_Y[1] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[1]){
				arr_of_max_Y[1] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 125){
			
			max_x = 125;
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[2]){
				arr_of_min_Y[2] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[2]){
				arr_of_max_Y[2] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 175){
			
			max_x = 175;
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[3]){
				arr_of_min_Y[3] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[3]){
				arr_of_max_Y[3] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 225){
		  
			max_x = 225;
		  
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[4]){
				arr_of_min_Y[4] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[4]){
				arr_of_max_Y[4] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 275){
		  
			max_x = 275;
		  
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[5]){
				arr_of_min_Y[5] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[5]){
				arr_of_max_Y[5] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 325){
		  
			max_x = 325;
				  
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[6]){
				arr_of_min_Y[6] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[6]){
				arr_of_max_Y[6] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 375){
			
			max_x = 375;
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[7]){
				arr_of_min_Y[7] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[7]){
				arr_of_max_Y[7] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 425){
			
			max_x = 425;
			
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[8]){
				arr_of_min_Y[8] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[8]){
				arr_of_max_Y[8] = cleaningTask[i].coordinate_Y_struct;
			}
		}
		
		if(cleaningTask[i].coordinate_X_struct == 475){
		  
			max_x = 475;
		  
			if(cleaningTask[i].coordinate_Y_struct < arr_of_min_Y[9]){
				arr_of_min_Y[9] = cleaningTask[i].coordinate_Y_struct;
			}
			if(cleaningTask[i].coordinate_Y_struct > arr_of_max_Y[9]){
				arr_of_max_Y[9] = cleaningTask[i].coordinate_Y_struct;
			}
		}
	  }
	return max_x;
}

void visualization_function(int16_t* arr_of_min_Y, int16_t* arr_of_max_Y, uint8_t& pos, uint16_t& max_X){
	
	flag_visualization_is_done = false;
	
	const uint8_t offset_X = 25; // Нужна потому что, система ориентирона на центр квадрата. Константа (ее можно записать в flash) для визуализации, чтобы доехать недостающие 25 мм от квадрата
	const uint8_t offset_Y = 50; // для того, чтобы доезжать последний квадрат. Константа (ее можно записать в flash) для визуализации, чтобы доехать недостающие 25 мм от квадрата
	movementSpeed = idleSpeed;
	
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos]), arr_of_min_Y[pos]); // выезд в заданную точку
	departure_to_the_square(max_X+offset_X, arr_of_min_Y[pos]); // по X до максимума
	departure_to_the_square(max_X+offset_X, arr_of_max_Y[pos]+offset_Y); // по Y до максимума
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos])-offset_X, arr_of_max_Y[pos]+offset_Y);
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos])-offset_X, arr_of_min_Y[pos]);
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos]), arr_of_min_Y[pos]); // выезд в заданную точку
	
	softSerial.print(F("click bt3,1"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	softSerial.print(F("click bt3,0"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	
}

void cleaning_process(int16_t* arr_of_min_Y, int16_t* arr_of_max_Y, uint8_t& pos){
	movementSpeed = idleSpeed;
	const uint8_t offset_Y = 50; // для того, чтобы доезжать последний квадрат. Константа (ее можно записать в flash) для визуализации, чтобы доехать недостающие 25 мм от квадрата
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos]), arr_of_min_Y[pos]);
	// включить лазер
	digitalWrite(port_laser_issue_enable, HIGH);
	softSerial.print(F("click bt2,1"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	softSerial.print(F("click bt2,0"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	movementSpeed = cleaningSpeed;
	departure_to_the_square(pgm_read_word(&coordinate_X_arr[pos]), arr_of_max_Y[pos]+offset_Y);
	// выключить лазер
	digitalWrite(port_laser_issue_enable, LOW);
	softSerial.print(F("click bt2,1"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	softSerial.print(F("click bt2,0"));
	softSerial.print(char(255)+char(255)+char(255)); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
	movementSpeed = idleSpeed;
}

/* 291121
 * Визуализация срабатывает каждый раз при срабатывании условия arr_of_max_Y[0]>-1, а должна работать только один раз
 * Решено с помощью flag_visualization_is_done
*/
void trajectory_movement(int16_t* arr_of_min_Y, int16_t* arr_of_max_Y, bool flag_cleaning=true, uint16_t max_X=0){
	uint8_t pos;
	if(arr_of_max_Y[0]>-1){
		pos = 0;

		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
		
	}

	if(arr_of_max_Y[1]>-1){
		pos = 1;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[2]>-1){
		pos = 2;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[3]>-1){
		pos = 3;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[4]>-1){
		pos = 4;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[5]>-1){
		pos = 5;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[6]>-1){
		pos = 6;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[7]>-1){
		pos = 7;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[8]>-1){
		pos = 8;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}

	if(arr_of_max_Y[9]>-1){
		pos = 9;
		
		if(!flag_cleaning && flag_visualization_is_done){
			visualization_function(arr_of_min_Y, arr_of_max_Y, pos, max_X);
		}
		if(flag_cleaning){
			cleaning_process(arr_of_min_Y, arr_of_max_Y, pos);
		}
	}
	//-----------------------------------------------------------------------
	if(flag_cleaning){
		softSerial.print(F("click bt1,1")); // Отправляем команду дисплею, заканчивая её тремя байтами 0xFF
		softSerial.print(char(255)+char(255)+char(255));
		softSerial.print(F("click bt1,0")); 
		softSerial.print(char(255)+char(255)+char(255));
	}
	
	//while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt0, отправив 4 байта данных, 
                                                                            // где первый байт равен 0x01 или 0x00, а остальные 3 равны 0x00
	//digitalWrite(port_power, softSerial.read());                              // Устанавливаем на выходе port_power состояние в соответствии с первым принятым байтом ответа дисплея
	//delay(10);
	//while(softSerial.available()){
	//	softSerial.read();
	//	delay(10);
	//}
	//-----------------------------------------------------------------------
}

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
  Serial.println(F("Ready!"));
}

void loop() {

  controlFromTheDisplay();
  
  controlUart();
  
  if(!program){
	pos_cleaningTask = 0;
	movementSpeed = idleSpeed;
		for(byte i = 0; i < 100; ++i){
			cleaningTask[i].coordinate_X_struct = -1;
			cleaningTask[i].coordinate_Y_struct = -1;
		}
  }
}
