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

void controlUart(){                          // Эта функция позволяет управлять системой из монитора порта
  if (Serial.available()) {         // есть что на вход
	//int16_t coordinate_X;
	//int16_t coordinate_Y;
	//String cmd;
    char cmd[15];
    //cmd+=char(softSerial.read());
	uint8_t pos = 0;
	
	while(Serial.available()){
		cmd[pos++] = char(Serial.read());
		delay(10);
	}
	cmd[pos] = char(0);
	Serial.println(cmd);
	/*
	 for(int i=0; i<str.length(); i++){ // Проходимся по каждому символу строки str
      //----------------------------------------------
      if(memcmp(&str[i],"movingUp" , 8)==0){ // Если в строке str начиная с символа i находится текст "movingUp",  значит кнопка дисплея была включена
        i+=7; 
        digitalWrite(port_direction, LOW);
        action(distance, mySpeed, acceleration);
        theDifferenceIsActual += distance;
      }else
      //----------------------------------------------
  */
  for(uint8_t i=0; i<pos; i++){
    if (memcmp(&cmd[i], "dist", 4)==0) {
		i+=3; // что делать в случае принятия одного символа
		Serial.println(F("введите количество миллиметров"));      
		distance_global = readdata();
		Serial.print(F("distance: "));
		Serial.println(distance_global);
    //}else if (cmd.equals("sp")) {
	}else 
		if (memcmp(&cmd[i], "sp", 2)==0) {
			i+=1;
			Serial.println(F("введите скорость"));      
			mySpeed = readdata();
			Serial.print(F("скорость: "));
			Serial.println(mySpeed);
			Serial.print(F("T: "));
			uint16_t T_tmp = (float(1.0f/(1600L/screwPitch))*1000000L)/mySpeed;
			Serial.println(T_tmp);
    //}else if (cmd.equals("acc")) {
	}else 
		if (memcmp(&cmd[i], "acc", 3)==0) {
			i+=2;
			Serial.println(F("введите ускорение"));       
			acceleration = readdata();
			Serial.print(F("acceleration: "));
			Serial.println(acceleration);
    }else 
		//if (cmd.equals("start1")) {
		if (memcmp(&cmd[i], "start1", 6)==0) {
			i+=5;
			Serial.println(F("Команда не используется"));
			//action(distance, mySpeed, acceleration);
    }else 
		//if(cmd.equals("up")){
		if(memcmp(&cmd[i], "up", 2)==0){
			i+=1;
			Serial.println(F("Движение к нулю"));
      
			if(axis_global==char(88)){
				digitalWrite(port_direction_X, LOW);
				action(char(88), distance_global, mySpeed, acceleration);
			}else
			  if(axis_global==char(89)){
				digitalWrite(port_direction_Y, LOW);
				action(char(89), distance_global, mySpeed, acceleration);
			  }else
				  if(axis_global==char(90)){
					digitalWrite(port_direction_Z, LOW);
					
					action(char(90), distance_global, mySpeed, acceleration);
				  }
    }else 
		//if(cmd.equals("down")){
		if(memcmp(&cmd[i], "down", 4)==0){
			i+=3;
			Serial.println(F("Поехали!"));
			if(axis_global==char(88)){
				digitalWrite(port_direction_X, HIGH);
				action(char(88), distance_global, mySpeed, acceleration);
			}else
				if(axis_global==char(89)){
					digitalWrite(port_direction_Y, HIGH);
					action(char(89), distance_global, mySpeed, acceleration);
				  }else
					  if(axis_global==char(90)){
						digitalWrite(port_direction_Z, HIGH);
						//float special_dist_for_Z = distance_global*2; // На Z оси другой шаг винта. Лавинообразный эффект. Комментарий в action
						action(char(90), distance_global, mySpeed, acceleration);
					  }
    }else 
		//if(cmd.equals("focus")){
		if(memcmp(&cmd[i], "focus", 5)==0){
			i+=4;
			Serial.println(F("focus!"));
			//focusOnTheTable();
			//digitalWrite(port_direction, LOW);
			//action(500, mySpeed, acceleration);
			//digitalWrite(port_direction, HIGH);
			//action(100, mySpeed, acceleration);
    }else 
		//if(cmd.equals("sensorInit")){
		if(memcmp(&cmd[i], "sensorInit", 10)==0){
			i+=9;
			Serial.println(F("debugging information"));
    }else 
		//if(cmd.equals("sensorRead")){
		if(memcmp(&cmd[i], "sensorRead", 10)==0){
			i+=9;
			Serial.print(F("debugging information"));
    }else 
		//if(cmd.equals("autoFocus")){
		if(memcmp(&cmd[i], "autoFocus", 9)==0){
			i+=9;
			Serial.println(F("this feature is in development"));
    }else 
		//if(cmd.equals("pow_ON")){
		if(memcmp(&cmd[i], "pow_ON", 6)==0){
			i+=5;
			digitalWrite(port_power, HIGH);
			state_power = true;
      //------------------------------------------------------------------------------------------------------------------
      // скорее всего этот блок кода для случая, когда кнопка питания лазера уже активирована, а основного питания еще нет. При этом выход контроллера включит
      // питание лазера. Рассмотреть правильность, может быть не позволять активировать кнопку включения питания лазера без основного питания???
      // Устанавливаем состояние кнопки включения питания лазера:                  
      /*
      softSerial.print((String) "print bt1.val"+char(255)+char(255)+char(255)); // Отправляем команду дисплею: «print bt1.val» заканчивая её тремя байтами 0xFF
      while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt1, отправив 4 байта данных, где 1 байт равен 0x01 или 0x00, а остальные 3 равны 0x00
      digitalWrite(port_power_laser, softSerial.read());       delay(10);               // Устанавливаем на выходе port_power_laser состояние в соответствии с первым принятым байтом ответа дисплея
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
      //if(cmd.equals("las_ON")){
		if(memcmp(&cmd[i], "las_ON", 6)==0){
			i+=5;
        if(state_power){
          digitalWrite(port_power_laser, HIGH);
        }
      }
    else 
		//if(cmd.equals("las_OFF")){
		if(memcmp(&cmd[i], "las_OFF", 7)==0){
			i+=6;
			digitalWrite(port_power_laser, LOW);
      }
    else 
		//if(cmd.equals("home")){
		if(memcmp(&cmd[i], "home", 4)==0){
			i+=3;
			mySpeed = 30;
			int8_t coord_X_tmp = 0;
			int8_t coord_Y_tmp = 0;
			departure_to_the_square(coord_X_tmp, coord_Y_tmp);
	}else
		//if(cmd.equals("X")){
		if(memcmp(&cmd[i], "X", 1)==0){
			axis_global = char(88);
			Serial.println(F("the x axis selected"));
	}else 
		//if(cmd.equals("Y")){
		if(memcmp(&cmd[i], "Y", 1)==0){
			axis_global = char(89);
			Serial.println(F("the y axis selected"));
	}else 
		//if(cmd.equals("Z")){
		if(memcmp(&cmd[i], "Z", 1)==0){	
			axis_global = char(90);
			Serial.println(F("the z axis selected"));
	}else
		//if(cmd.equals("program")){
		if(memcmp(&cmd[i], "program", 7)==0){
			i+=6;
			program = !program;
			Serial.println(F("programming mode activated"));
	}else 
		//if(cmd.equals("program_false")){
		if(memcmp(&cmd[i], "program_off", 11)==0){
			i+=10;
			Serial.println(F("programming mode disable"));
			program = false;
			pos_cleaningTask = 0;
			for(byte i = 0; i < 100; ++i){
				cleaningTask[i].coordinate_X_struct = -1;
				cleaningTask[i].coordinate_Y_struct = -1;
			}
	}else
		//if(cmd.equals("start")){
		if(memcmp(&cmd[i], "start", 5)==0){
			i+=4;
			
			int min_Y_1 = 550;
			int min_Y_2 = 550;
			int min_Y_3 = 550;
			int min_Y_4 = 550;
			int min_Y_5 = 550;
			int min_Y_6 = 550;
			int min_Y_7 = 550;
			int min_Y_8 = 550;
			int min_Y_9 = 550;
			int min_Y_10 = 550;
			
			int max_Y_1 = -1;
			int max_Y_2 = -1;
			int max_Y_3 = -1;
			int max_Y_4 = -1;
			int max_Y_5 = -1;
			int max_Y_6 = -1;
			int max_Y_7 = -1;
			int max_Y_8 = -1;
			int max_Y_9 = -1;
			int max_Y_10 = -1;
			
			//int coord;
			
			for(byte i = 0; i < pos_cleaningTask; ++i){
				
				if(cleaningTask[i].coordinate_X_struct == 25){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_1){
						min_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_1){
						max_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 75){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_2){
						min_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_2){
						max_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 125){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_3){
						min_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_3){
						max_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 175){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_4){
						min_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_4){
						max_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 225){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_5){
						min_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_5){
						max_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 275){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_6){
						min_Y_6 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_6){
						max_Y_6 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 325){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_7){
						min_Y_7 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_7){
						max_Y_7 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 375){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_8){
						min_Y_8 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_8){
						max_Y_8 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 425){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_9){
						min_Y_9 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_9){
						max_Y_9 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 475){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_10){
						min_Y_10 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_10){
						max_Y_10 = cleaningTask[i].coordinate_Y_struct;
					}
				}
			}
			//const int16_t coordinate_X_arr[] PROGMEM = {25, 75, 125, 175, 225, 275, 325, 375, 425, 475};
			//const int16_t coordinate_Y_arr[] PROGMEM = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};
			if(max_Y_1>-1){
				//coord = 25;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[0]), min_Y_1);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[0]), max_Y_1);
				// выключить лазер
			}
			
			if(max_Y_2>-1){
				//coord = 75;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[1]), min_Y_2);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[1]), max_Y_2);
				// выключить лазер
			}
			
			if(max_Y_3>-1){
				//coord = 125;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[2]), min_Y_3);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[2]), max_Y_3);
				// выключить лазер
			}
			
			if(max_Y_4>-1){
				//coord = 175;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[3]), min_Y_4);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[3]), max_Y_4);
				// выключить лазер
			}
			
			if(max_Y_5>-1){
				//coord = 225;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[4]), min_Y_5);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[4]), max_Y_5);
				// выключить лазер
			}
			
			if(max_Y_6>-1){
				//coord = 275;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[5]), min_Y_6);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[5]), max_Y_6);
				// выключить лазер
			}
			
			if(max_Y_7>-1){
				//coord = 325;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[6]), min_Y_7);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[6]), max_Y_7);
				// выключить лазер
			}
			
			if(max_Y_8>-1){
				//coord = 375;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[7]), min_Y_8);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[7]), max_Y_8);
				// выключить лазер
			}
			
			if(max_Y_9>-1){
				//coord = 425;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[8]), min_Y_9);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[8]), max_Y_9);
				// выключить лазер
			}
			
			if(max_Y_10>-1){
				//coord = 475;
				mySpeed = 30;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[9]), min_Y_10);
				// включить лазер
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[9]), max_Y_10);
				// выключить лазер
			}
			
	}else
		//if(cmd.equals("a0")){
		if(memcmp(&cmd[i], "a0", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[0]));
	}else 
		//if(cmd.equals("a1")){
		if(memcmp(&cmd[i], "a1", 2)==0){
			i+=1;	
			//coordinate_X = 25;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[1]));
	}else 
		//if(cmd.equals("a2")){
		if(memcmp(&cmd[i], "a2", 2)==0){
			i+=1;	
			//coordinate_X = 25;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[2]));
	}else 
		//if(cmd.equals("a3")){
		if(memcmp(&cmd[i], "a3", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[3]));
	}else 
		//if(cmd.equals("a4")){
		if(memcmp(&cmd[i], "a4", 2)==0){
			i+=1;	
			//coordinate_X = 25;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("a5")){
		if(memcmp(&cmd[i], "a5", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("a6")){
		if(memcmp(&cmd[i], "a6", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("a7")){
		if(memcmp(&cmd[i], "a7", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("a8")){
		if(memcmp(&cmd[i], "a8", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("a9")){
		if(memcmp(&cmd[i], "a9", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("b0")){
		if(memcmp(&cmd[i], "b0", 2)==0){
			i+=1;	
			//coordinate_X = 75;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[0]));
	}else 
		//if(cmd.equals("b1")){
		if(memcmp(&cmd[i], "b1", 2)==0){
			i+=1;	
			//coordinate_X = 75;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[1]));
	}else 
		//if(cmd.equals("b2")){
		if(memcmp(&cmd[i], "b2", 2)==0){
			i+=1;	
			//coordinate_X = 75;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("b3")){
		if(memcmp(&cmd[i], "b3", 2)==0){
			i+=1;	
			//coordinate_X = 75;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[3]));
	}else 
		//if(cmd.equals("b4")){
		if(memcmp(&cmd[i], "b4", 2)==0){
			i+=1;
			//coordinate_X = 75;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("b5")){
		if(memcmp(&cmd[i], "b5", 2)==0){
			i+=1;	
			//coordinate_X = 75;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("b6")){
		if(memcmp(&cmd[i], "b6", 2)==0){
			i+=1;
			//coordinate_X = 75;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("b7")){
		if(memcmp(&cmd[i], "b7", 2)==0){
			i+=1;
			//coordinate_X = 75;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("b8")){
		if(memcmp(&cmd[i], "b8", 2)==0){
			i+=1;
			//coordinate_X = 75;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("b9")){
		if(memcmp(&cmd[i], "b9", 2)==0){
			i+=1;
			//coordinate_X = 75;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("c0")){
		if(memcmp(&cmd[i], "c0", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[0]));
	}else 
		//if(cmd.equals("c1")){
		if(memcmp(&cmd[i], "c1", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("c2")){
		if(memcmp(&cmd[i], "c2", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[2]));
	}else 
		//if(cmd.equals("c3")){
		if(memcmp(&cmd[i], "c3", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[3]));
	}else 
		//if(cmd.equals("c4")){
		if(memcmp(&cmd[i], "c4", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("c5")){
		if(memcmp(&cmd[i], "c5", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("c6")){
		if(memcmp(&cmd[i], "c6", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("c7")){
		if(memcmp(&cmd[i], "c7", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("c8")){
		if(memcmp(&cmd[i], "c8", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("c9")){
		if(memcmp(&cmd[i], "c9", 2)==0){
			i+=1;
			//coordinate_X = 125;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("d0")){
		if(memcmp(&cmd[i], "d0", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[0]));
	}else 
		//if(cmd.equals("d1")){
		if(memcmp(&cmd[i], "d1", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[1]));
	}else 
		//if(cmd.equals("d2")){
		if(memcmp(&cmd[i], "d2", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[2]));
	}else 
		//if(cmd.equals("d3")){
		if(memcmp(&cmd[i], "d3", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[3]));
	}else 
		//if(cmd.equals("d4")){
		if(memcmp(&cmd[i], "d4", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("d5")){
		if(memcmp(&cmd[i], "d5", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("d6")){
		if(memcmp(&cmd[i], "d6", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("d7")){
		if(memcmp(&cmd[i], "d7", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("d8")){
		if(memcmp(&cmd[i], "d8", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("d9")){
		if(memcmp(&cmd[i], "d9", 2)==0){
			i+=1;
			//coordinate_X = 175;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("e0")){
		if(memcmp(&cmd[i], "e0", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[0]));
	}else 
		//if(cmd.equals("e1")){
		if(memcmp(&cmd[i], "e1", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[1]));
	}else 
		//if(cmd.equals("e2")){
		if(memcmp(&cmd[i], "e2", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[2]));
	}else 
		//if(cmd.equals("e3")){
		if(memcmp(&cmd[i], "e3", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[3]));
	}else 
		//if(cmd.equals("e4")){
		if(memcmp(&cmd[i], "e4", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("e5")){
		if(memcmp(&cmd[i], "e5", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("e6")){
		if(memcmp(&cmd[i], "e6", 2)==0){
			i+=1;	
			//coordinate_X = 225;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("e7")){
		if(memcmp(&cmd[i], "e7", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("e8")){
		if(memcmp(&cmd[i], "e8", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("e9")){
		if(memcmp(&cmd[i], "e9", 2)==0){
			i+=1;
			//coordinate_X = 225;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("f0")){
		if(memcmp(&cmd[i], "f0", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[0]));
	}else
		//if(cmd.equals("f1")){
		if(memcmp(&cmd[i], "f1", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("f2")){
		if(memcmp(&cmd[i], "f2", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("f3")){
		if(memcmp(&cmd[i], "f3", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[3]));
	}else
		//if(cmd.equals("f4")){
		if(memcmp(&cmd[i], "f4", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("f5")){
		if(memcmp(&cmd[i], "f5", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("f6")){
		if(memcmp(&cmd[i], "f6", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("f7")){
		if(memcmp(&cmd[i], "f7", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("f8")){
		if(memcmp(&cmd[i], "f8", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("f9")){
		if(memcmp(&cmd[i], "f9", 2)==0){
			i+=1;
			//coordinate_X = 275;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("g0")){
		if(memcmp(&cmd[i], "g0", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[0]));
	}else
		//if(cmd.equals("g1")){
		if(memcmp(&cmd[i], "g1", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("g2")){
		if(memcmp(&cmd[i], "g2", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("g3")){
		if(memcmp(&cmd[i], "g3", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[3]));
	}else
		//if(cmd.equals("g4")){
		if(memcmp(&cmd[i], "g4", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("g5")){
		if(memcmp(&cmd[i], "g5", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("g6")){
		if(memcmp(&cmd[i], "g6", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("g7")){
		if(memcmp(&cmd[i], "g7", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("g8")){
		if(memcmp(&cmd[i], "g8", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("g9")){
		if(memcmp(&cmd[i], "g9", 2)==0){
			i+=1;
			//coordinate_X = 325;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("h0")){
		if(memcmp(&cmd[i], "h0", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[0]));
	}else
		//if(cmd.equals("h1")){
		if(memcmp(&cmd[i], "h1", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("h2")){
		if(memcmp(&cmd[i], "h2", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("h3")){
		if(memcmp(&cmd[i], "h3", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[3]));
	}else
		//if(cmd.equals("h4")){
		if(memcmp(&cmd[i], "h4", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("h5")){
		if(memcmp(&cmd[i], "h5", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("h6")){
		if(memcmp(&cmd[i], "h6", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("h7")){
		if(memcmp(&cmd[i], "h7", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("h8")){
		if(memcmp(&cmd[i], "h8", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("h9")){
		if(memcmp(&cmd[i], "h9", 2)==0){
			i+=1;
			//coordinate_X = 375;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("k0")){
		if(memcmp(&cmd[i], "k0", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[0]));
	}else
		//if(cmd.equals("k1")){
		if(memcmp(&cmd[i], "k1", 2)==0){
			i+=1;	
			//coordinate_X = 425;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("k2")){
		if(memcmp(&cmd[i], "k2", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("k3")){
		if(memcmp(&cmd[i], "k3", 2)==0){
			i+=1;	
			//coordinate_X = 425;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[3]));
	}else
		//if(cmd.equals("k4")){
		if(memcmp(&cmd[i], "k4", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("k5")){
		if(memcmp(&cmd[i], "k5", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("k6")){
		if(memcmp(&cmd[i], "k6", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("k7")){
		if(memcmp(&cmd[i], "k7", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("k8")){
		if(memcmp(&cmd[i], "k8", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("k9")){
		if(memcmp(&cmd[i], "k9", 2)==0){
			i+=1;
			//coordinate_X = 425;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[9]));
	}else
		//if(cmd.equals("l0")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l0", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[0]));
	}else
		//if(cmd.equals("l1")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l1", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 50;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[1]));
	}else
		//if(cmd.equals("l2")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l2", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 100;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[2]));
	}else
		//if(cmd.equals("l3")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l3", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 150;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[3]));
	}else
		//if(cmd.equals("l4")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l4", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 200;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[4]));
	}else
		//if(cmd.equals("l5")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l5", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 250;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[5]));
	}else
		//if(cmd.equals("l6")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l6", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 300;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[6]));
	}else
		//if(cmd.equals("l7")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l7", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 350;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[7]));
	}else
		//if(cmd.equals("l8")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l8", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 400;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[8]));
	}else
		//if(cmd.equals("l9")){ // буква l маленькая (L)
		if(memcmp(&cmd[i], "l9", 2)==0){
			i+=1;
			//coordinate_X = 475;
			//coordinate_Y = 450;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[9]));
	
			
	}else
		{
		  Serial.println(F("The command is not provided"));    // ошибка
		}
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

void controlFromTheDisplay(){
  if(softSerial.available()>0){         // Если есть данные принятые от дисплея, то ...
    //String cmd;                         // Объявляем строку для получения этих данных
	char cmd[15];
	uint8_t pos = 0;
	while(softSerial.available()){
		cmd[pos++] = char(softSerial.read());
		delay(10);
	}
	cmd[pos] = char(0);
	/*
    while(softSerial.available()){
      cmd+=char(softSerial.read());     // Читаем принятые от дисплея данные побайтно в строку str
      delay(10);
    }
	*/
    Serial.println(cmd);                
    //for(int i=0; i<cmd.length(); i++){ // Проходимся по каждому символу строки str
	for(int i=0; i<pos; i++){
      //----------------------------------------------
      if(memcmp(&cmd[i],"movingUp" , 8)==0){ // Если в строке str начиная с символа i находится текст "movingUp",  значит кнопка дисплея была включена
        i+=7; 
        //digitalWrite(port_direction, LOW);
        //action(distance, mySpeed, acceleration);
        //theDifferenceIsActual += distance;
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"movingDown", 10)==0){
        i+=9; 
        //digitalWrite(port_direction, HIGH);
        //state_LED_BUILTIN = !state_LED_BUILTIN;
        //digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
        //action(distance, mySpeed, acceleration);
        //theDifferenceIsActual -= distance;
      }else              
      //----------------------------------------------
      if(memcmp(&cmd[i],"light_ON", 8)==0){
        i+=7; 
        digitalWrite(port_light, HIGH);
        //state_LED_BUILTIN = true;
        //digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&cmd[i],"light_OFF", 9)==0){
        i+=8; 
        digitalWrite(port_light, LOW);
        //state_LED_BUILTIN = false;
        //digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&cmd[i],"pow_OFF", 7)==0){
        i+=6; 
        digitalWrite(port_power, LOW);
        state_power = false;
        digitalWrite(port_power_laser, LOW);
        //state_LED_BUILTIN = false;
        //digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
      }else 
      //----------------------------------------------
      if(memcmp(&cmd[i],"pow_ON", 6)==0){
        i+=5; 
        digitalWrite(port_power, HIGH);
        state_power = true;
        //------------------------------------------------------------------------------------------------------------------
        // скорее всего этот блок кода для случая, когда кнопка питания лазера уже активирована, а основного питания еще нет. При этом выход контроллера включит
        // питание лазера. Рассмотреть правильность, может быть не позволять активировать кнопку включения питания лазера без основного питания???
        // Устанавливаем состояние кнопки включения питания лазера:                  
        softSerial.print((String) "print bt1.val"+char(255)+char(255)+char(255)); // Отправляем команду дисплею: «print bt1.val» заканчивая её тремя байтами 0xFF
        while(!softSerial.available()){}                                          // Ждём ответа. Дисплей должен вернуть состояние кнопки bt1, отправив 4 байта данных, где 1 байт равен 0x01 или 0x00, а остальные 3 равны 0x00
        digitalWrite(port_power_laser, softSerial.read());       delay(10);               // Устанавливаем на выходе port_power_laser состояние в соответствии с первым принятым байтом ответа дисплея
        while(softSerial.available()){softSerial.read(); delay(10);}
        //------------------------------------------------------------------------------------------------------------------
        //delay(500);          // удалить ели все работает
        /*
		if(!state_pow_on){     // при первом включении отправить систему на верхний концевик
          focusOnTheTable();
          state_pow_on = true;
        }
		*/
      }else 
      //----------------------------------------------
      if(memcmp(&cmd[i],"las_OFF", 7)==0){
        i+=6; 
        digitalWrite(port_power_laser, LOW);
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"las_ON", 6)==0){
        i+=5;
        if(state_power){
          digitalWrite(port_power_laser, HIGH);
        }
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"EN_ROT_DEV_ON", 13)==0){
        i+=12;
        digitalWrite(port_EN_ROT_DEV, HIGH);
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"EN_ROT_DEV_OFF", 14)==0){
        i+=13;
        digitalWrite(port_EN_ROT_DEV, LOW);
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"set_dist_0.1", 12)==0){
        i+=11;
        //distance = 0.1;
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"set_dist_0.5", 12)==0){
        i+=11;
        //distance = 0.5;
      }else
      //----------------------------------------------
      if(memcmp(&cmd[i],"set_dist_1", 10)==0){
        i+=9;
        //distance = 1;
      }else
		  //----------------------------------------------
		  if(memcmp(&cmd[i],"set_dist_5", 10)==0){
			i+=9;
			//distance = 5;
      }else
		  //----------------------------------------------
		  if(memcmp(&cmd[i],"focus", 5)==0){
			i+=4;
			Serial.println(F("focus on the table!"));
			//focusOnTheTable();
      }else
		//if(cmd.equals("a0")){
		if(memcmp(&cmd[i], "a0", 2)==0){
			i+=1;
			//coordinate_X = 25;
			//coordinate_Y = 0;
			check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[0]));
		}else 
			//if(cmd.equals("a1")){
			if(memcmp(&cmd[i], "a1", 2)==0){
				i+=1;	
				//coordinate_X = 25;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[1]));
		}else 
			//if(cmd.equals("a2")){
			if(memcmp(&cmd[i], "a2", 2)==0){
				i+=1;	
				//coordinate_X = 25;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[2]));
		}else 
			//if(cmd.equals("a3")){
			if(memcmp(&cmd[i], "a3", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[3]));
		}else 
			//if(cmd.equals("a4")){
			if(memcmp(&cmd[i], "a4", 2)==0){
				i+=1;	
				//coordinate_X = 25;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("a5")){
			if(memcmp(&cmd[i], "a5", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("a6")){
			if(memcmp(&cmd[i], "a6", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("a7")){
			if(memcmp(&cmd[i], "a7", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("a8")){
			if(memcmp(&cmd[i], "a8", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("a9")){
			if(memcmp(&cmd[i], "a9", 2)==0){
				i+=1;
				//coordinate_X = 25;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[0]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("b0")){
			if(memcmp(&cmd[i], "b0", 2)==0){
				i+=1;	
				//coordinate_X = 75;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[0]));
		}else 
			//if(cmd.equals("b1")){
			if(memcmp(&cmd[i], "b1", 2)==0){
				i+=1;	
				//coordinate_X = 75;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[1]));
		}else 
			//if(cmd.equals("b2")){
			if(memcmp(&cmd[i], "b2", 2)==0){
				i+=1;	
				//coordinate_X = 75;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("b3")){
			if(memcmp(&cmd[i], "b3", 2)==0){
				i+=1;	
				//coordinate_X = 75;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[3]));
		}else 
			//if(cmd.equals("b4")){
			if(memcmp(&cmd[i], "b4", 2)==0){
				i+=1;
				//coordinate_X = 75;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("b5")){
			if(memcmp(&cmd[i], "b5", 2)==0){
				i+=1;	
				//coordinate_X = 75;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("b6")){
			if(memcmp(&cmd[i], "b6", 2)==0){
				i+=1;
				//coordinate_X = 75;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("b7")){
			if(memcmp(&cmd[i], "b7", 2)==0){
				i+=1;
				//coordinate_X = 75;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("b8")){
			if(memcmp(&cmd[i], "b8", 2)==0){
				i+=1;
				//coordinate_X = 75;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("b9")){
			if(memcmp(&cmd[i], "b9", 2)==0){
				i+=1;
				//coordinate_X = 75;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[1]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("c0")){
			if(memcmp(&cmd[i], "c0", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[0]));
		}else 
			//if(cmd.equals("c1")){
			if(memcmp(&cmd[i], "c1", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("c2")){
			if(memcmp(&cmd[i], "c2", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[2]));
		}else 
			//if(cmd.equals("c3")){
			if(memcmp(&cmd[i], "c3", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[3]));
		}else 
			//if(cmd.equals("c4")){
			if(memcmp(&cmd[i], "c4", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("c5")){
			if(memcmp(&cmd[i], "c5", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("c6")){
			if(memcmp(&cmd[i], "c6", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("c7")){
			if(memcmp(&cmd[i], "c7", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("c8")){
			if(memcmp(&cmd[i], "c8", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("c9")){
			if(memcmp(&cmd[i], "c9", 2)==0){
				i+=1;
				//coordinate_X = 125;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[2]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("d0")){
			if(memcmp(&cmd[i], "d0", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[0]));
		}else 
			//if(cmd.equals("d1")){
			if(memcmp(&cmd[i], "d1", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[1]));
		}else 
			//if(cmd.equals("d2")){
			if(memcmp(&cmd[i], "d2", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[2]));
		}else 
			//if(cmd.equals("d3")){
			if(memcmp(&cmd[i], "d3", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[3]));
		}else 
			//if(cmd.equals("d4")){
			if(memcmp(&cmd[i], "d4", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("d5")){
			if(memcmp(&cmd[i], "d5", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("d6")){
			if(memcmp(&cmd[i], "d6", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("d7")){
			if(memcmp(&cmd[i], "d7", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("d8")){
			if(memcmp(&cmd[i], "d8", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("d9")){
			if(memcmp(&cmd[i], "d9", 2)==0){
				i+=1;
				//coordinate_X = 175;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[3]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("e0")){
			if(memcmp(&cmd[i], "e0", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[0]));
		}else 
			//if(cmd.equals("e1")){
			if(memcmp(&cmd[i], "e1", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[1]));
		}else 
			//if(cmd.equals("e2")){
			if(memcmp(&cmd[i], "e2", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[2]));
		}else 
			//if(cmd.equals("e3")){
			if(memcmp(&cmd[i], "e3", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[3]));
		}else 
			//if(cmd.equals("e4")){
			if(memcmp(&cmd[i], "e4", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("e5")){
			if(memcmp(&cmd[i], "e5", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("e6")){
			if(memcmp(&cmd[i], "e6", 2)==0){
				i+=1;	
				//coordinate_X = 225;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("e7")){
			if(memcmp(&cmd[i], "e7", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("e8")){
			if(memcmp(&cmd[i], "e8", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("e9")){
			if(memcmp(&cmd[i], "e9", 2)==0){
				i+=1;
				//coordinate_X = 225;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[4]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("f0")){
			if(memcmp(&cmd[i], "f0", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[0]));
		}else
			//if(cmd.equals("f1")){
			if(memcmp(&cmd[i], "f1", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("f2")){
			if(memcmp(&cmd[i], "f2", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("f3")){
			if(memcmp(&cmd[i], "f3", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[3]));
		}else
			//if(cmd.equals("f4")){
			if(memcmp(&cmd[i], "f4", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("f5")){
			if(memcmp(&cmd[i], "f5", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("f6")){
			if(memcmp(&cmd[i], "f6", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("f7")){
			if(memcmp(&cmd[i], "f7", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("f8")){
			if(memcmp(&cmd[i], "f8", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("f9")){
			if(memcmp(&cmd[i], "f9", 2)==0){
				i+=1;
				//coordinate_X = 275;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[5]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("g0")){
			if(memcmp(&cmd[i], "g0", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[0]));
		}else
			//if(cmd.equals("g1")){
			if(memcmp(&cmd[i], "g1", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("g2")){
			if(memcmp(&cmd[i], "g2", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("g3")){
			if(memcmp(&cmd[i], "g3", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[3]));
		}else
			//if(cmd.equals("g4")){
			if(memcmp(&cmd[i], "g4", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("g5")){
			if(memcmp(&cmd[i], "g5", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("g6")){
			if(memcmp(&cmd[i], "g6", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("g7")){
			if(memcmp(&cmd[i], "g7", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("g8")){
			if(memcmp(&cmd[i], "g8", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("g9")){
			if(memcmp(&cmd[i], "g9", 2)==0){
				i+=1;
				//coordinate_X = 325;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[6]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("h0")){
			if(memcmp(&cmd[i], "h0", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[0]));
		}else
			//if(cmd.equals("h1")){
			if(memcmp(&cmd[i], "h1", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("h2")){
			if(memcmp(&cmd[i], "h2", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("h3")){
			if(memcmp(&cmd[i], "h3", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[3]));
		}else
			//if(cmd.equals("h4")){
			if(memcmp(&cmd[i], "h4", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("h5")){
			if(memcmp(&cmd[i], "h5", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("h6")){
			if(memcmp(&cmd[i], "h6", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("h7")){
			if(memcmp(&cmd[i], "h7", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("h8")){
			if(memcmp(&cmd[i], "h8", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("h9")){
			if(memcmp(&cmd[i], "h9", 2)==0){
				i+=1;
				//coordinate_X = 375;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[7]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("k0")){
			if(memcmp(&cmd[i], "k0", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[0]));
		}else
			//if(cmd.equals("k1")){
			if(memcmp(&cmd[i], "k1", 2)==0){
				i+=1;	
				//coordinate_X = 425;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("k2")){
			if(memcmp(&cmd[i], "k2", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("k3")){
			if(memcmp(&cmd[i], "k3", 2)==0){
				i+=1;	
				//coordinate_X = 425;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[3]));
		}else
			//if(cmd.equals("k4")){
			if(memcmp(&cmd[i], "k4", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("k5")){
			if(memcmp(&cmd[i], "k5", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("k6")){
			if(memcmp(&cmd[i], "k6", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("k7")){
			if(memcmp(&cmd[i], "k7", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("k8")){
			if(memcmp(&cmd[i], "k8", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("k9")){
			if(memcmp(&cmd[i], "k9", 2)==0){
				i+=1;
				//coordinate_X = 425;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[8]), pgm_read_word(&coordinate_Y_arr[9]));
		}else
			//if(cmd.equals("l0")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l0", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 0;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[0]));
		}else
			//if(cmd.equals("l1")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l1", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 50;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[1]));
		}else
			//if(cmd.equals("l2")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l2", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 100;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[2]));
		}else
			//if(cmd.equals("l3")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l3", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 150;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[3]));
		}else
			//if(cmd.equals("l4")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l4", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 200;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[4]));
		}else
			//if(cmd.equals("l5")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l5", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 250;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[5]));
		}else
			//if(cmd.equals("l6")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l6", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 300;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[6]));
		}else
			//if(cmd.equals("l7")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l7", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 350;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[7]));
		}else
			//if(cmd.equals("l8")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l8", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 400;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[8]));
		}else
			//if(cmd.equals("l9")){ // буква l маленькая (L)
			if(memcmp(&cmd[i], "l9", 2)==0){
				i+=1;
				//coordinate_X = 475;
				//coordinate_Y = 450;
				check_out_or_record(pgm_read_word(&coordinate_X_arr[9]), pgm_read_word(&coordinate_Y_arr[9]));
		}else 
		//if(cmd.equals("home")){
		if(memcmp(&cmd[i], "home", 4)==0){
			i+=3;
			mySpeed = 300;
			int8_t coord_X_tmp = 0;
			int8_t coord_Y_tmp = 0;
			departure_to_the_square(coord_X_tmp, coord_Y_tmp);
	}else
		//if(cmd.equals("program")){
		if(memcmp(&cmd[i], "program", 7)==0){
			i+=6;
			program = !program;
			Serial.println(F("programming mode activated"));
	}else 
		//if(cmd.equals("program_false")){
		if(memcmp(&cmd[i], "program_off", 11)==0){
			i+=10;
			Serial.println(F("programming mode disable"));
			program = false;
			pos_cleaningTask = 0;
			for(byte i = 0; i < 100; ++i){
				cleaningTask[i].coordinate_X_struct = -1;
				cleaningTask[i].coordinate_Y_struct = -1;
			}
	}else
		//if(cmd.equals("start")){
		if(memcmp(&cmd[i], "start", 5)==0){
			i+=4;
			
			int min_Y_1 = 550;
			int min_Y_2 = 550;
			int min_Y_3 = 550;
			int min_Y_4 = 550;
			int min_Y_5 = 550;
			int min_Y_6 = 550;
			int min_Y_7 = 550;
			int min_Y_8 = 550;
			int min_Y_9 = 550;
			int min_Y_10 = 550;
			
			int max_Y_1 = -1;
			int max_Y_2 = -1;
			int max_Y_3 = -1;
			int max_Y_4 = -1;
			int max_Y_5 = -1;
			int max_Y_6 = -1;
			int max_Y_7 = -1;
			int max_Y_8 = -1;
			int max_Y_9 = -1;
			int max_Y_10 = -1;
			
			//int coord;
			
			for(byte i = 0; i < pos_cleaningTask; ++i){
				
				if(cleaningTask[i].coordinate_X_struct == 25){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_1){
						min_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_1){
						max_Y_1 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 75){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_2){
						min_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_2){
						max_Y_2 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 125){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_3){
						min_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_3){
						max_Y_3 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 175){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_4){
						min_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_4){
						max_Y_4 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 225){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_5){
						min_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_5){
						max_Y_5 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 275){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_6){
						min_Y_6 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_6){
						max_Y_6 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 325){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_7){
						min_Y_7 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_7){
						max_Y_7 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 375){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_8){
						min_Y_8 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_8){
						max_Y_8 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 425){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_9){
						min_Y_9 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_9){
						max_Y_9 = cleaningTask[i].coordinate_Y_struct;
					}
				}
				
				if(cleaningTask[i].coordinate_X_struct == 475){
					if(cleaningTask[i].coordinate_Y_struct < min_Y_10){
						min_Y_10 = cleaningTask[i].coordinate_Y_struct;
					}
					if(cleaningTask[i].coordinate_Y_struct > max_Y_10){
						max_Y_10 = cleaningTask[i].coordinate_Y_struct;
					}
				}
			}
			//const int16_t coordinate_X_arr[] PROGMEM = {25, 75, 125, 175, 225, 275, 325, 375, 425, 475};
			//const int16_t coordinate_Y_arr[] PROGMEM = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450};
			if(max_Y_1>-1){
				//coord = 25;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[0]), min_Y_1);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[0]), max_Y_1);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_2>-1){
				//coord = 75;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[1]), min_Y_2);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[1]), max_Y_2);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_3>-1){
				//coord = 125;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[2]), min_Y_3);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[2]), max_Y_3);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_4>-1){
				//coord = 175;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[3]), min_Y_4);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[3]), max_Y_4);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_5>-1){
				//coord = 225;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[4]), min_Y_5);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[4]), max_Y_5);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_6>-1){
				//coord = 275;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[5]), min_Y_6);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[5]), max_Y_6);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_7>-1){
				//coord = 325;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[6]), min_Y_7);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[6]), max_Y_7);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_8>-1){
				//coord = 375;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[7]), min_Y_8);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[7]), max_Y_8);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_9>-1){
				//coord = 425;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[8]), min_Y_9);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[8]), max_Y_9);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
			
			if(max_Y_10>-1){
				//coord = 475;
				mySpeed = 300;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[9]), min_Y_10);
				// включить лазер
				digitalWrite(port_laser_issue_enable, HIGH);
				mySpeed = 10;
				departure_to_the_square(pgm_read_word(&coordinate_X_arr[9]), max_Y_10);
				// выключить лазер
				digitalWrite(port_laser_issue_enable, LOW);
			}
		}
    }
  }
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
