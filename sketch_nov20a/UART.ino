







void controlUart(){                          // Эта функция позволяет управлять системой из монитора порта
  if (Serial.available()) {         // есть что на вход
    char cmd[15];
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
			action(distance, movementSpeed, acceleration);
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
    }else 
		if (memcmp(&cmd[i], "sp", 2)==0) {
		  i+=1;
		  Serial.println(F("введите скорость"));      
		  movementSpeed = readdata();
		  Serial.print(F("скорость: "));
		  Serial.println(movementSpeed);
		  Serial.print(F("T: "));
		  uint16_t T_tmp = (float(1.0f/(1600L/screwPitch))*1000000L)/movementSpeed;
		  Serial.println(T_tmp);
	}else 
		if (memcmp(&cmd[i], "acc", 3)==0) {
		  i+=2;
		  Serial.println(F("введите ускорение"));       
		  acceleration = readdata();
		  Serial.print(F("acceleration: "));
		  Serial.println(acceleration);
    }else 
		if(memcmp(&cmd[i], "up", 2)==0){
			i+=1;
			Serial.println(F("Движение к нулю"));

			if(axis_global==char(88)){
				digitalWrite(port_direction_X, LOW);
				action(char(88), distance_global, movementSpeed, acceleration);
			}else
				if(axis_global==char(89)){
					digitalWrite(port_direction_Y, LOW);
					action(char(89), distance_global, movementSpeed, acceleration);
				}else
					if(axis_global==char(90)){
						digitalWrite(port_direction_Z, LOW);
						action(char(90), distance_global, movementSpeed, acceleration);
					}
    }else 
    if(memcmp(&cmd[i], "down", 4)==0){
      i+=3;
      Serial.println(F("Поехали!"));
      if(axis_global==char(88)){
        digitalWrite(port_direction_X, HIGH);
        action(char(88), distance_global, movementSpeed, acceleration);
      }else
        if(axis_global==char(89)){
          digitalWrite(port_direction_Y, HIGH);
          action(char(89), distance_global, movementSpeed, acceleration);
          }else
            if(axis_global==char(90)){
            digitalWrite(port_direction_Z, HIGH);
            //float special_dist_for_Z = distance_global*2; // На Z оси другой шаг винта. Лавинообразный эффект. Комментарий в action
            action(char(90), distance_global, movementSpeed, acceleration);
            }
    }else 
    //if(cmd.equals("focus")){
    if(memcmp(&cmd[i], "focus", 5)==0){
      i+=4;
      Serial.println(F("focus!"));
      //focusOnTheTable();
      //digitalWrite(port_direction, LOW);
      //action(500, movementSpeed, acceleration);
      //digitalWrite(port_direction, HIGH);
      //action(100, movementSpeed, acceleration);
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
      movementSpeed = 30;
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
    //if(cmd.equals("start")){
    if(memcmp(&cmd[i], "start", 5)==0){
	i+=4;
		  
	int16_t arr_of_min_Y[cleaningSpeed] = {550, 550, 550, 550, 550, 550, 550, 550, 550, 550};
	int16_t arr_of_max_Y[cleaningSpeed] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	calculation_of_the_working_rectangle(arr_of_min_Y, arr_of_max_Y);
	trajectory_movement(arr_of_min_Y, arr_of_max_Y);
      
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
