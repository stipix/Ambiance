/*
 * main.c
 *
 *  Created on: Feb 6, 2025
 *      Author: Caitlin Bonesio
 */

/*
#include <BOARD.h>
#include <CONFIG.h>


int main(){
	BOARD_int();
	uint8_t updateFlags[UPDATELISTSIZE];
	Update_t updates[UPDATELISTSIZE];
	//Initialize all modules
	for(int i = 0; i < UPDATELISTSIZE; i++){
		if ((*InitList[i])(updates[i]) == ERROR){
			return 0;//We've crashed
		}
	}
	while(1){
		//run all module event checkers
		for(int i = 0; i < UPDATELISTSIZE; i++){
			updates[i] = *UpdateList[i]();
			if (updates[i].status == ERROR){
				return 0;//We've crashed
			}
		}
		//run all module event handlers
		for(int i = 0; i < UPDATELISTSIZE; i++){
			if(updates[i].status == 1{
				if ((*HandlerList[i])(updates[i]) == ERROR){
					return 0;//We've crashed
				}
			}
		}

	}


}
*/


