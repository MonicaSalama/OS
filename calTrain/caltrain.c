

/*
 * caltrain.c
 *
 *  Created on: Nov 12, 2015
 *      Author: monica
 */


#include <pthread.h>
#include "caltrain.h"

void station_init(struct station *station) {

	station->num_pass = 0;
	station->seats_left = 0;
	station->pass_waiting_board = 0;
	pthread_mutex_init(&(station->mutex), NULL);
	//used to notify waiting passengers that there is an empty seats
	pthread_cond_init(&(station->available_seats), NULL);
	//used to notify that there is a passenger who is waiting for the boarding
	pthread_cond_init(&(station->pass_wait_board), NULL);
	//used to notify train that it's ready to leave and all passengers have boarded
	pthread_cond_init(&(station->train_on_board), NULL);

}

/*responsible : 1- to notify passeng. of the arrival of the train
                2- to leave when seats are full or all passengers have boarded*/
void station_load_train(struct station *station, int count) {
	//lock mutex
	pthread_mutex_lock(&(station->mutex));
	//no waiting passengers train will leave immediately
	if (station->num_pass == 0) {
		pthread_mutex_unlock(&(station->mutex));
	} else {
		station->seats_left = count;
		//notify all waiting pass. of available seats
		pthread_cond_broadcast(&(station->available_seats));
		//train cannot leave until either all seats are occupied or no passengers are waiting
		//and all passengers that got a seat in the train have boarded
		while (!(station->pass_waiting_board == 0
				&& (station->seats_left == 0 || station->num_pass == 0))) {
			pthread_cond_wait(&(station->train_on_board), &(station->mutex));
		}
		//in case waiting pass. < available seats
		station->seats_left = 0;
		//unlock mutex
		pthread_mutex_unlock(&(station->mutex));
	}
}

//passenger arrived to the station
void station_wait_for_train(struct station *station) {
	//lock mutex
	pthread_mutex_lock(&(station->mutex));
	//increase number of waiting pass.
	station->num_pass += 1;
	//wait for a train with available seats
	while (station->seats_left == 0) {
		pthread_cond_wait(&(station->available_seats), &(station->mutex));
	}
	//get a seat in train , decrement available seats
	station->seats_left -= 1;
	//passenger is waiting for boarding
	station->pass_waiting_board++;
	//passenger is no more waiting for a seat
	station->num_pass -= 1;
	//notify there is a passenger who is waiting to be boarded
	pthread_cond_signal(&(station->pass_wait_board));
	//unlock mutex
	pthread_mutex_unlock(&(station->mutex));

}

/*passenger got a seat and is ready to board
  responsible to notify train that all pass. have boarded*/
void station_on_board(struct station *station) {
	pthread_mutex_lock(&(station->mutex));
	while (station->pass_waiting_board == 0) {
		pthread_cond_wait(&(station->pass_wait_board), &(station->mutex));
	}
	station->pass_waiting_board -= 1;
	/* no passengers are waiting for boarding and either no seats or no passengers waiting to get seats*/
	if (station->pass_waiting_board == 0
			&& (station->num_pass == 0 || station->seats_left == 0)) {
		/*train is ready to leave*/
		pthread_mutex_unlock(&(station->mutex));
		pthread_cond_signal(&(station->train_on_board));
	}else{
		pthread_mutex_unlock(&(station->mutex));
	}
}
