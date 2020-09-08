/*C**********************************************************************
* FILENAME :   	drone_movement.c
*
* DESCRIPTION :
*       		Support code provided for the Operating Systems (SO)
*				Practical Assignment 2018.
*
* PUBLIC FUNCTIONS :
* 		double 	distance ( x1,  y1,  x2,  y2 )
*		int 	move_towards ( *drone_x,  *drone_y, target_x, target_y )
*
*
*
*C*/

#include "drone_movement.h"

/*
 * Computes the distance between two points
 */
double distance(double x1, double y1, double x2, double y2){
     return sqrtf( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}

/*
 * Advances the drone 1 step towards the target point.
 *
 * You should provide the address for the coordinates of your drone,
 * in order for them to be updated, according to the following cases
 *
 *
 * returns  1 - in case it moved correctly towards the target
 *				drone_x, drone_y will be updated
 *
 * returns  0 - in case it moved and reached the target
 *				drone_x, drone_y will be updated
 *
 * returns -1 - in case it was already in the target
 *				drone_x, drone_y are NOT updated
 * returns -2 - in case there is an error
 *				drone_x, drone_y are NOT updated
 */

int move_towards(double *drone_x, double *drone_y, double target_x, double target_y){

	// if one of the coords is negative, there is an error.
	if( *drone_x < 0 ||  *drone_y < 0 ||  target_x < 0 ||  target_y < 0){
		return -2;
	}

	// if it is on the target, does not move
	if( (*drone_x == target_x) && (*drone_y ==  target_y)){
		return -1;
	}

	// if distance < 1, move to target, return 0
	if( distance(*drone_x, *drone_y,  target_x,  target_y) <= 1){
		*drone_x = target_x;
		*drone_y = target_y;
		return 0;
	}


	// obtain the angle, usign arc tangent.
	double angle = atan2(target_y - *drone_y, target_x - *drone_x);


    *drone_x = (*drone_x) + (cos(angle) * DISTANCE);
    *drone_y = (*drone_y) + (sin(angle) * DISTANCE);


    return 1;
}
