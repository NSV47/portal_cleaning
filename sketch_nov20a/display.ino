void controlFromTheDisplay(){
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
			if(memcmp(&cmd[i],"movingUp" , 8)==0){ // Если в строке str начиная с символа i находится текст "movingUp",  значит кнопка дисплея была включена
				i+=7; 
				//digitalWrite(port_direction, LOW);
				//action(distance, mySpeed, acceleration);
				//theDifferenceIsActual += distance;
			}else
			if(memcmp(&cmd[i],"movingDown", 10)==0){
				i+=9; 
				//digitalWrite(port_direction, HIGH);
				//state_LED_BUILTIN = !state_LED_BUILTIN;
				//digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
				//action(distance, mySpeed, acceleration);
				//theDifferenceIsActual -= distance;
			}else              
			if(memcmp(&cmd[i],"light_ON", 8)==0){
				i+=7; 
				digitalWrite(port_light, HIGH);
				//state_LED_BUILTIN = true;
				//digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
			}else 
			if(memcmp(&cmd[i],"light_OFF", 9)==0){
				i+=8; 
				digitalWrite(port_light, LOW);
				//state_LED_BUILTIN = false;
				//digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
			}else 
			if(memcmp(&cmd[i],"pow_OFF", 7)==0){
				i+=6; 
				digitalWrite(port_power, LOW);
				state_power = false;
				digitalWrite(port_power_laser, LOW);
				//state_LED_BUILTIN = false;
				//digitalWrite(LED_BUILTIN, state_LED_BUILTIN);
			}else 
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
			if(memcmp(&cmd[i],"las_OFF", 7)==0){
				i+=6; 
				digitalWrite(port_power_laser, LOW);
			}else
			if(memcmp(&cmd[i],"las_ON", 6)==0){
				i+=5;
				if(state_power){
				  digitalWrite(port_power_laser, HIGH);
				}
			}else
			if(memcmp(&cmd[i],"EN_ROT_DEV_ON", 13)==0){
				i+=12;
				digitalWrite(port_EN_ROT_DEV, HIGH);
			}else
			if(memcmp(&cmd[i],"EN_ROT_DEV_OFF", 14)==0){
				i+=13;
				digitalWrite(port_EN_ROT_DEV, LOW);
			}else
			if(memcmp(&cmd[i],"set_dist_0.1", 12)==0){
				i+=11;
				//distance = 0.1;
			}else
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
