/*
 * Project_Sudoku.c
   * Authors : Minadaki Anna		   AM:2016030108
  *			   Protopadakis Giorgos	   AM:2016030134
  * HRY411 Users Team: 12
  * Microcontroller used : ATmega16 
  * Tool:STK500
  * This project is an implementation of a sudoku solving program, using RS232 serial communication.
  * In this implementation, a 2D array is initialized for the board of the sudoku.Also, the communication of AVR with the PC is 
  * established through commands sent from the PC to AVR and back using RS232.A function for turning on specific leds while the cells of the 
  * board are being filled with values solving the sudoku (as a progress bar counting the full cells) is implemented as well.The sudoku solving algorithm 
  * is based on backtracking and is implemented in a separate function which is called every time P (play) command is received from the PC.
  * The transmission baud rate is 9600 and the set frame format 8 data bits 1 stop bit no parity.The program is using 10Mhz external crystal.
  * 
 */ 
#define F_CPU 10000000UL     //10Mhz clock
#define USART_BAUDRATE 9600 //set baudrate to 9600
#define BAUD_PRESCALE (F_CPU/16/USART_BAUDRATE)-1//calculate UBRR register value (from manual)
#define TRUE 0x01
#define FALSE 0x00
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

unsigned char sudoku[9][9]__attribute__((address(0x80070)));//2D global array 9x9 for creating the board for sudoku 

void init(){
	UCSRB = (1 << RXCIE )| (1 << RXEN ) | (1 << TXEN ); // Turn on the transmission and reception circuitry
	UCSRC = (1 << URSEL ) | (1 << UCSZ0 ) | (1 << UCSZ1 ); // Use 8- bit character sizes
	UBRRH = ((BAUD_PRESCALE)>>8); // Load upper 8- bits of the baud rate value into the high byte
	UBRRL = BAUD_PRESCALE; // Load lower 8 - bits of the baud rate value into the low byte of the
	DDRC=0xFF;//set portc as an output
	sei(); // Enable the Global Interrupt Enable flag so that interrupts can be processed
	//initialize sudoku array cells to 0
	for(unsigned char i=0;i<9;i++){
	   for (unsigned char j=0;j<9;j++){
		  sudoku[i][j]=0;
	    }
	 }
	 unsigned char* led=(unsigned char*)0x69;
	 *led=0;
	 unsigned char* xpos=(unsigned char*)0x60;
	 unsigned char* ypos=(unsigned char*)0x61;
	 *xpos=1;
	 *ypos=1;
	
	PORTC=0xFF;//initialize portc showing all leds off 

}

int main (void)
{
	init();
	while(1) // Loop forever
	{
		// Do nothing - echoing is handled by the ISR instead of in the main loop
	}
}
//receive function.Waits for the receive buffer to empty and return UDR containing the received information from the PC
unsigned char Receive(){
	while ( !(UCSRA & (1<<RXC)) ){};//check if the receive buffer is empty and loop until it is (receive complete)
		
	return UDR;
		
	
}
//transmit function.Used by the avr to transmit information to the PC from UDR after transmit buffer is emptied
void Transmit(unsigned char* answer){
	// Wait for empty transmit buffer
	while ( !( UCSRA & (1<<UDRE)) ){};
	UDR= *answer; 
	
	return;
}
//function for turning on leds while the sudoku is being solved (called in P command in the RXC interrupt handler) 
//counting the full cells and turning on the equivalent led every time.
void leds(unsigned char* led){	
	if(*led>=50){
		if(*led>=70){
			if(*led>=80){
				PORTC=0b00000000;//if the full cells are more than 80 then turn all leds on
				return;
			}else{
				PORTC=0b10000000;//if the full cells are more than 70 then turn LED0-LED6 on
				return;
			}
		}else{
			if(*led>=60){
				PORTC=0b11000000;//if the full cells are more than 60 then turn LED0-LED5 on
				return;
			}else{
				PORTC=0b11100000;//if the full cells are more than 50 then turn LED5-LED4 on
				return;
			}
		}
	}else{
		if(*led>=30){
			if(*led>=40){
				PORTC=0b11110000;//if the full cells are more than 40 then turn LED0-LED3 on
				return;
			}else{
				PORTC=0b11111000;//if the full cells are more than 30 then turn LED0-LED2 on
				return;
			}
		}else{
			if(*led>=20){
				PORTC=0b11111100;//if the full cells are more than 20 then turn LED0-LED1 on
				return;
			}else{
				PORTC=0b11111110;//if the full cells are more than 10 then turn LED0 on
				return;
			}
		}
	}
		return;}

/*function for the sudoku solving algorithm called by P command in RXC interrupt handler.
  finds the empty cells in global sudoku unsolved array and calculates the correct value for each
  cell using backtracking algorithm.The function leds() is called as the algorithm runs
  to turn on the equivalent leds as the sudoku cells are being filled.If the cells are all full then the function returns 1 as a success.
*/
unsigned char solveSudoku (){
	unsigned char* led=(unsigned char*)0x69;//pointer for led counter
	unsigned char* result=(unsigned char*)0x6A;
	*result=FALSE;//variable for checking if a number is suitable for each cell of the sudoku array 
	unsigned char x,y,i,j;
	//go through the whole 9x9 array
	for( i=0;i<9;i++){
		for ( j=0;j<9;j++){
			if (sudoku[i][j]==0){//if an empty cell is detected then save i and j in variables x and y for later use
				x=i;
				y=j;
				i=9;//turn i value to 9 to break the outer loop
			    break;//break the inner loop
			 }
			 
		}
		
	 }
	  
	if(i!=10 ){//this is the case where all the sudoku cells are full, so the program has not entered the if case above to turn the i variable to 9.(the statement compares i to 10 as the i++ happens in the end of the for loop
				//so when i is set to 9 inside the if statement above, it exits as 10 from the outer loop).
		return TRUE;// if this is the case then the sudoku is solved and the function returns
    }		
	
			
	//calculate the beginning of 3*3 sub array for x and y position of sudoku array saved from the for loops above
	unsigned char subX=(x/3) * 3;//floor of x/3 multiplied by 3
	unsigned char subY=(y/3) * 3;//floor of y/3 multiplied by 3
				
	for (unsigned char num=1;num<10;++num){//a loop for checking if any of the numbers 1-9 can be saved as a solution in the sudoku cell. 
		for (unsigned char k=0;k<9;++k){
			//check if the num equals any of the values existing in the specific row or column for the given x and y 
			if (sudoku[k][y]==num || sudoku[x][k]==num || sudoku[subX + (k%3)][subY + (k/3)] ==num){//or in the sub array starting from x and y positions calculated above(subX,subY) 
					*result=TRUE;//pointer variable for checking if the num exists in the row or in the column or in the 3*3 sub array
					break;//break for saving time as the current num is not a solution for the cell
			}
		}			
		if (*result==FALSE){//if the variable is false that means that the num does not exist in the row, column or 3*3 sub array
			sudoku[x][y]=num;//save the num in the cell of sudoku
			*led=*led+1;//increment the led variable for the new entry in the sudoku array
			leds(led);//and call led function to turn on the equivalent leds
			if (solveSudoku()){//call solveSudoku function again to continue the algorithm for the next empty cell and check if the sudoku is complete
				return TRUE;//return the function if sudoku is solved
			}
			sudoku[x][y]=0;//undo the last entry in the sudoku array if solveSudoku returns false
			*led=*led-1;//update the led variable
			leds(led);//and call the leds function to turn on the equivalent leds
		}
		*result=FALSE;//re initialize result variable to check the next num 
	}
	return FALSE;//no num is a suitable solution for the current cell so return false to undo the last entry
	
}


 
//RXC interrupt handler function
ISR ( USART_RXC_vect ){  
	unsigned char* xpos=(unsigned char*)0x60;//pointer for modifying values to be used as index x in sudoku array (for transmitting results in T and S commands)
	unsigned char* ypos=(unsigned char*)0x61;//pointer for modifying values to be used as index y in sudoku array (for transmitting results in T and S commands)
	unsigned char* answer=(unsigned char* )0x62;//pointer for storing the answers from AVR to PC (and passed as a parameter in Transmit function)
	*answer=0; //also used as index to p pointer for storing contents of UDR in SRAM 
	unsigned char* data =(unsigned char* )0x63;//pointer used for storing more than 1 bytes (read from UDR)
	unsigned char* led=(unsigned char*)0x69;
	
	while(1){   //infinite loop that breaks when LF is received from UDR
		data[*answer]=Receive();//call receive function and store the returned value from UDR in sram addresses 0x63-0x68(depending on the command sent from the PC)
		if (data[*answer] == 0x0A) {//if the value is LF in ASCII then
			break;//and stop the loop
		}else {//if the value in not LF
			*answer=*answer+1;//increment the index of the array so that the next value can be read and stored
		  }
					
	}
			
	//FROM THIS POINT ON IF THE DATA ARRAY COINTAINS SOME VALUES FOLLOWED BY CRLF THESE VALUES ARE
	//COMPARED WITH THE ASCII VALUES OF THE REQUIRED COMMAND SET AND EXECUTE ACCORDINGLY.
	
	if ((data[0]==0x41) && (data[1]==0x54)) { //AT COMMAND (SIMPLY RESPONDS OK<CR><LF>)
		 if (data[2]==0x0D && data[3]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted.
			*answer=0x4F;     //0 in ASCII
			Transmit(answer);
			*answer=0x4B;	 //K in ASCII
			Transmit(answer);
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
			*answer=0x0A;	 //LF in ASCII
			Transmit(answer);
		 }
		   
	}else if(data[0]==0x43){			  //C COMMAND (reinitialize all sudoku array cells to 0 and turns all leds off.When done, transmits OK<CR><LF>)
	  if (data[1]==0x0D && data[2]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
		
		*answer=0x4F;   //0 in ASCII
		Transmit(answer);
		*answer=0x4B;	 //K in ASCII
		Transmit(answer);
		*answer=0x0D;	 //CR in ASCII
		Transmit(answer);
		*answer=0x0A;	 //LF in ASCII
		Transmit(answer);
		
		//go through the whole 9x9 array
		for(int i=0;i<9;i++){
	      for (int j=0;j<9;j++){
		     sudoku[i][j]=0;//reinitialize sudoku array to 0
			 }	
	     }
		 *led=0; //reinitialize led counter
		PORTC=0xFF;//turn all leds off		
		*xpos=1;
		*ypos=1;
	  }
	}else if(data[0]==0x4E){				//N COMMAND (followed by 3 characters containing the x and y position in sudoku array and a value to be stored there.<CR><LF> follows.)
										    //This is the command that fills the array with values when downloading an unsolved board.
		if (data[4]==0x0D && data[5]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
			data[1]&=0x0F;//mask x position with 0x0F to retrieve the decimal equivalent and store it back in the data cell
			data[2]&=0x0F;//mask y position with 0x0F to retrieve the decimal equivalent and store it back in the data cell
			data[3]&=0x0F;//mask the actual value with 0x0F to retrieve the decimal equivalent and store it back in the data cell
			sudoku[(data[1]-1)][(data[2]-1)]=data[3];//store the masked value in the masked position in sudoku array.
			
			if(data[3]!=0){//if the value stored in the sudoku array is different to zero
				*led=*led+1;//then a new number has been stored so update the led counter 
				leds(led);//and call leds function to turn on the equivalent leds
			}
			
		    *answer=0x4F;   //0 in ASCII
		    Transmit(answer);
		    *answer=0x4B;	 //K in ASCII
			Transmit(answer);
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
		    *answer=0x0A;	 //LF in ASCII
			Transmit(answer);
	    }
	}else if(data[0]==0x50 ){//P COMMAND 
											//FOR P COMMAND:this is the command where the sudoku is solved.It is called after the unsolved board 
											//has been downloaded, it transmits OK<CR><LF>, then it runs the algorithm to solve the sudoku(not implemented yet)
											//and when all the cells of the board are filled, it transmits D<CR><LF> meaning the calculations are done and the 
											//board is ready.
		if (data[1]==0x0D && data[2]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
			
			*answer=0x4F;   //0 in ASCII
			Transmit(answer);
			*answer=0x4B;	 //K in ASCII
			Transmit(answer);
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
			*answer=0x0A;	 //LF in ASCII
			Transmit(answer);
		
			if(solveSudoku()){//if the function for sudoku solver returns true meaning that the sudoku board has been solved successfully
				*answer=0x44;   //D in ASCII
				Transmit(answer);
				*answer=0x0D;	 //CR in ASCII
				Transmit(answer);
				*answer=0x0A;	 //LF in ASCII
				Transmit(answer);
			}
		//initialize variables x and y for use in S and T commands.
		xpos[0]=1;
		ypos[0]=1;
	 }
	}else if (data[0]==0x44){				//D(debug) COMMAND (followed by 2 characters containing the x and y position in sudoku array.Using this position the value of that
											//sudoku cell is transmitted back to the PC. The answer is in the form of: N<X><Y><VALUE><CR><LF>.)
	   if (data[3]==0x0D && data[4]==0x0A){ //check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
			*answer=0x4E; //N IN ASCII
			Transmit(answer);
			*answer=data[1];//transmit x position as read from UDR
			Transmit(answer);
			*answer=data[2];//transmit y position as read from UDR
			Transmit(answer);
			data[1]&=0x0F;//mask x position with 0x0F to retrieve the decimal equivalent and store it back in the data cell
			data[2]&=0x0F;//mask y position with 0x0F to retrieve the decimal equivalent and store it back in the data cell
			*answer=(sudoku[(data[1]-1)][(data[2]-1)])+48;
			Transmit(answer);//transmit the value from the position of the board that was read from the command and masked above
											  // (+48 is added, as the value is stored in sudoku array in decimal, and the numbers 0-9 start from 
											  //decimal 48 in ASCII.)
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
			*answer=0x0A;	 //LF in ASCII
			Transmit(answer);
	   }
		
	}else if(data[0]==0x53){                //S COMMAND (sends the first sudoku cell result in the format of: N<X><Y><VALUE><CR><LF>, after it is solved (after AVR has sent done to PC).)
		
		if(data[1]==0x0D && data[2]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
			*answer=0x4E;     //N IN ASCII
			Transmit(answer);
			*answer=xpos[0]+48; //transmit xposition initialized to 1 to show the cell 11 of sudoku array and added 48 to reach the ASCII encoding of 0-9 decimal numbers.
			Transmit(answer); 
			*answer=ypos[0]+48; //transmit yposition initialized to 1 to show the cell 11 of sudoku array and added 48 to reach the ASCII encoding of 0-9 decimal numbers.
			Transmit(answer);
			*answer=(sudoku[(xpos[0]-1)][(ypos[0]-1)])+48; //transmit the value in 11 position in sudoku array and added 48 to reach the ASCII encoding of 0-9 decimal numbers.
			Transmit(answer);
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
			*answer=0x0A;	 //LF in ASCII
			Transmit(answer);
		}
	}else if(data[0]==0x54 ){				//T COMMAND (sends the next value of the sudoku cell, called until all the board is transmitted)
		if(data[1]==0x0D && data[2]==0x0A){//check if <CR><LF> has been received.if not, no answer is transmitted and no execution is done.
				ypos[0]=ypos[0]+1; //increase y position as the position 11 has been displayed from S COMMAND, so the transmission starts from cell 12 of sudoku array.
			    *answer=0x4E;  //N IN ASCII
			    Transmit(answer);
				*answer=xpos[0]+48;//transmit xposition to show the value of sudoku array in this position and added 48 to reach the ASCII encoding of 0-9 decimal numbers.
				Transmit(answer);
				*answer=ypos[0]+48;//transmit yposition to show the value of sudoku array in this position and added 48 to reach the ASCII encoding of 0-9 decimal numbers.
				Transmit(answer);
				*answer=(sudoku[(xpos[0]-1)][(ypos[0]-1)])+48;//transmit the value of the positions above 
				Transmit(answer);
				*answer=0x0D;	 //CR in ASCII
				Transmit(answer);
				*answer=0x0A;	 //LF in ASCII
				Transmit(answer);
				if(ypos[0]==9){//when y position reaches the value 9
					if(xpos[0]==9){//if x position has reached the value 9 as well, that means that the whole board has been transmitted, so the answer D<CR><LF> is transmitted to PC.
						*answer=0x44; //D
						Transmit(answer);
						*answer=0x0D;	 //CR in ASCII
						Transmit(answer);
						*answer=0x0A;	 //LF in ASCII
						Transmit(answer);
						//reinitialize yposition and xposition in the right values so that after exiting this case, their values are 1 1.
						ypos[0]=1;
						xpos[0]=0;
					}
					xpos[0]=xpos[0]+1;//if xposition has not reached the value 9 then increase the value of xposition.
					ypos[0]=0;//set yposition to 0, so that when T is called again to transmit the next value, it takes the value 1 before transmitting.
				}	
			}
	
		}
		if(data[0]==0x42){ //B COMMAND
			*answer=0x4F;   //0 in ASCII
			Transmit(answer);
			*answer=0x4B;	 //K in ASCII
			Transmit(answer);
			*answer=0x0D;	 //CR in ASCII
			Transmit(answer);
			*answer=0x0A;	 //LF in ASCII
			Transmit(answer);

	     }
}					
