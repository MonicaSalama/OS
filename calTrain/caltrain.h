/*
 * caltrain.h
 *
 *  Created on: Nov 12, 2015
 *      Author: monica
 */

#ifndef CALTRAIN_H_
#define CALTRAIN_H_
#include <pthread.h>

#endif /* CALTRAIN_H_ */

struct station {

	//number of passengers waiting in the station
	int num_pass;
	//number of passengers that have a seat in the train and did not board yet.
	int pass_waiting_board;
	//empty seats in the train
	int seats_left;
	pthread_mutex_t mutex;
	pthread_cond_t available_seats,train_on_board ,pass_wait_board;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);
