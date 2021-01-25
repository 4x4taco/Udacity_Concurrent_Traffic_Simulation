#include <iostream>
#include <random>
#include <queue>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> lock(_mtx);
    _condvar.wait(lock, [this] {return !_queue.empty(); });

    // move back of queue to variable msg of type T 
    T msg = std::move(_queue.back());
    //delete back of queue
    _queue.pop_back();
    
    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    //lock resource to prevent data race
    std::lock_guard<std::mutex> lock(_mtx);
    
    //move msg to back of queue
    _queue.push_back(std::move(msg));

    //nofity waiting threads that data exists
    _condvar.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(1)
    {
        //sleep during each cycle for a millisecond to reduce 
        //cpu consumption
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto cur_traff_phase = _msg_queue->receive();
        if (cur_traff_phase == green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() const
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method "cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called.
    // To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    //https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device rand;
    std::mt19937 eng(rand());
    std::uniform_int_distribution<> distr(4,6);

    //initialize random integer for cycle duration
    int cycle_dur = distr(eng);

    //Initialize start time
    auto start = std::chrono::high_resolution_clock::now();

    while(true)
    {
        //Initialize timer
        auto update_timer = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count();
    
        //sleep for a millisecond to reduce cpu time
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (update_timer > cycle_dur)
        {
            //Change the color of the traffic light if it is already red
            if (_currentPhase == red)
            {
                _currentPhase ==  green;
            }

            else
            {
                _currentPhase = red;
            }
            
        }

    }
}
