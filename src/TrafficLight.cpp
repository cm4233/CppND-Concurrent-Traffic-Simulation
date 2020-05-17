#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <stdlib.h>
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // The method receive uses std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object is returned by the receive function. 
	std::unique_lock<std::mutex> lck(_mtx);
	_cv.wait( lck, [this]{return !_queue.empty();} );
	T message = std::move(_queue.front());
	_queue.pop_front();
	return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // The method send uses the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	std::lock_guard<std::mutex> lck(_mtx);
	_queue.push_back(std::move(msg));
	_cv.notify_one();
}


/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // infinite while-loop runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
	while(true){
    	TrafficLightPhase message = _msgQueue.receive();
    	if(message == green){ return; }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
	srand(time(NULL));//Seed for the psuedo random number generator
    while(true){
    	std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 2001 + 4000));//Random duration between 4000ms and 6000ms
    	_currentPhase = (_currentPhase == red)? green : red; //toggle traffic light
    	TrafficLightPhase message = _currentPhase;
    	_msgQueue.send(std::move(message));
    }
}
