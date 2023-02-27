
//display_functions.c

void move_ufo(int direction){ // will accept input to decide whiche direction to move. 

	if (direction < 0){		//move upwards
		int timeOutCount = 0;
		int blanksBelow = 128;	// to create blanks below the ship as it leaves the lane
		int blanksAbove = 127;	// to create blanks above the ship as it spawns in
		int shiftAbove = 7;		// how much to shift the ufo template when spawning
		int shiftBelow = 1;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i;					// a counter
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		
			

		while (j > 0){	// loops 8 times. 1 for each pixel in a page.

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);	
				temp1[i] = ((ufo[i] >> shiftBelow)| blanksBelow);	
			}

			int w = 0;
			for( w ; w < 19 ; w++){	// this for loop copies out temp value into the map at the right lane
				gameMap[(characterLane*128) + (10 + w)] = ((gameMap[(characterLane*128) + (10 + w)] | 255/* or 255 to reset the image*/ )
				& temp0[w]);
				gameMap[((characterLane + 1)*128) + (10 + w)] = ((gameMap[((characterLane + 1)*128) + (10 + w)] | 255 )	& temp1[w]);
			} 
			blanksAbove = (blanksAbove /2); 	// decrements how much blank space should be used above
			blanksBelow = ((blanksBelow/2) + 128);// increments how much blank space should be used below 
			shiftAbove -- ;						// decrements how much of the ufo template to remove above
			shiftBelow ++;						// increments how much of the ufo template to remove below
			j --;								// counts itterations. 

			//display_update();
			display_image(0, gameMap);

			delayTest = 0;
			while (delayTest < 100000){
				delayTest ++;
			}
		}

	}
	if (direction > 0){		//move downwards
		int timeOutCount = 0;
		int blanksBelow = 254;	// to create blanks below the ship as it leaves the lane
		int blanksAbove = 128;	// to create blanks above the ship as it spawns in
		int shiftAbove = 1;		// how much to shift the ufo template when spawning
		int shiftBelow = 7;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i;					// a counter
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		
			

		while (j > 0){	// loops 8 times. 1 for each pixel in a page. //! ska skrivas om för att flytta neråt!!

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);
				temp1[i] = ((ufo[i] >> shiftBelow)| blanksBelow);	
			}

			int w = 0;
			for( w ; w < 19 ; w++){	// this for loop copies out temp value into the map at the right lane
				gameMap[((characterLane - 1)*128) + (10 + w)] = ((gameMap[((characterLane - 1)*128) + (10 + w)] | 255/* or 255 to reset the image*/ )
				& temp0[w]);

				
				gameMap[((characterLane + 1)*128) + (10 + w)] = ((gameMap[((characterLane + 1)*128) + (10 + w)] | 255 )	& temp1[w]);
			} 
			blanksAbove = ((blanksAbove*2)+1); 	// decrements how much blank space should be used above
			blanksBelow = (blanksBelow*2);	// increments how much blank space should be used below 
			shiftAbove ++;						// decrements how much of the ufo template to remove above
			shiftBelow --;						// increments how much of the ufo template to remove below
			j --;								// counts itterations. 

			//display_update();
			display_image(0, gameMap);

			delayTest = 0;
			while (delayTest < 100000){
				delayTest ++;
			}
		}
	}
	return;
}

void showUfo(void){	// funktion för att ladda in ufot i game map.
	int w = 0;
	for( w ; w < 19 ; w++){
		gameMap[(characterLane*128) + (10 + w)] = (gameMap[(characterLane*128) + (10 + w)] & ufo[w]);
	} 
	
	//display_update();
	display_image(0, gameMap);
    return:
}

void setup_gameMap(void){
	int row;
	int column;
	for(row = 0 ; row < 4 ; row++){			//this loop is for filling the map with things.
		for(column = 0 ; column < 128 ; column++){			
				gameMap[(row*128) + column] = 255;		
		}
	display_update();
	showUfo();
	}
}