/*H**********************************************************************
* FILENAME :   	drone_movement.h
*
* DESCRIPTION :
*       		Support code provided for the Operating Systems (SO)
*				Practical Assignment 2018.
*
* PUBLIC FUNCTIONS :
* 		double 	distance ( x1,  y1,  x2,  y2 )
*		int 	move_towards( *drone_x,  *drone_y, target_x, target_y )
*
*
*
*H*/

#ifndef SO2018_DRONE_MOVEMENT_H
#define SO2018_DRONE_MOVEMENT_H


#include <stdio.h>
#include <math.h>


#define DISTANCE 1

/*
 * Computes the distance between two points
 */
double distance(double x1, double y1, double x2, double y2);

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
int move_towards(double *drone_x, double *drone_y, double target_x, double target_y);

#endif
