#pragma once

#include <memory>
#include <iostream>
#include "IBusReader.h"
#include "FakeStationData.hpp"
#include "FakeTimer.hpp"
#include <vector>

class Statistics
{
private:
	std::shared_ptr<IBusReader> _stationData;
	std::shared_ptr<ITimer> _timer;
	std::vector<Bus> Busses;
	//private members beyond this point added on 09/01/2021
	int counter = 0;
	double totalTime = 0;
	double totalPassengers = 0;
	double avgPassengers = 0;
	long stay = 0;
	std::vector<Bus>maxBusses;
	std::vector<long>stays;
	double sum = 0;
	std::vector<Bus>averageBusses;

public:
	Statistics(const std::shared_ptr<IBusReader>& reader, const std::shared_ptr<ITimer>& timer);
	~Statistics();

	// 3rd
	long readData();
	long sampleCount(long duration);
	long sampleTotal();
	void clearSamples();
	long maximumDuration(long);
	double averageDuration(long);
	long getDuration(int timeOfSample);
	// 1st
	double averageDuration(long startTime, long endTime) { return -1; };
	long maximumDuration(int, int) { return -1; };
	long minimumDuration(int, int) { return -1; };
	double averagePassengersBus(std::string busName);
	double averagePassengersPlatform(int platformNumber);
	double averageDurationPlatform(int platformNumber);


};


Statistics::Statistics(const std::shared_ptr<IBusReader>& reader, const std::shared_ptr<ITimer>& timer)
{
	_stationData = reader;
	_timer = timer;
	_timer->start();
}


Statistics::~Statistics()
{
	//statement added 09/01/2021
	std::cout << "Destroyed Statistics!" << std::endl;
}


long Statistics::readData() 
{
	Bus BusSample; 

	long timeOfSample = _timer->time(); 

	//added 28/12/2020
	//throws an exception if the time of the sample taken is beyond the range of 24 hours
	if (timeOfSample > 86400)
	{
		throw std::out_of_range("Exceeds 24H");
	}

	if (timeOfSample <= timeOfSample - 1)
	{
		throw std::underflow_error("Underflow error");
	}

	try 
	{
		BusSample = this->_stationData->readData(timeOfSample);
	}
	catch (BusException& e) 
	{
		if (e.GetError() == BusError::NoData)
		{
			throw std::runtime_error("No Bus data at this time!");
		}
		else if (e.GetError() == BusError::Unknown)
		{
			throw std::runtime_error("Unknown error occurred when reading Bus data!");
		}
		
	}
	catch (...)
	{
		throw std::runtime_error("Things went really bad when reading Bus data!");
	}

	
	Busses.push_back(BusSample);				//added 08/12/2020

	return BusSample.depart;					// Return the second that the sample was taken at.

};

//added 09/12/2020
//endTime receives the current time from timer->time(). The time to look back over is then calculated by subtracting the duration, passed to the method, from the endTime
//and storing the result in startTime. An iteration is then performed on the Busses vector. If the current bus item's departure time is more than or equal to the startTime
//and the the current bus time's departure is less than or equal to the current Bus's departure time, increment Samples by 1. Finally, return Samples to the method. 
long Statistics::sampleCount(long duration)		
{
	long Samples = 0;
	long endTime = _timer->time();
	long startTime = endTime - duration;		//takes duration as defined value from user

	//if the duration passed exceeds 24 hours, throw an out of range exception.
	if (duration > 86400)
	{
		throw std::out_of_range("Sample was out of range!");
	}

	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.depart >= startTime && currentBusItem.depart <= endTime)
			{
				Samples++;
			}
	}
	return Samples;
}

//added 15/12/2020
//performs the vector method .size() on the Busses vector. Returns the number of elements currently contained in the vector.
long Statistics::sampleTotal()
{
	return Busses.size(); 
}

//added 15/12/2020
//performs the vector method .clear() on the Busses vector. Empties the vector of all elements. 
void Statistics::clearSamples()
{
	Busses.clear();
}

//added 21/12/2020
//a for loop iterates through the Busses vector. If the time of sample passed to the method matches the bus being iterated through departure time, return 
//the current bus item's departure time minus it's arrival time as a long. In effect, this returns the duration the bus was at the station for.
long Statistics::getDuration(int timeofSample)
{
		for (Bus const& currentBusItem : Busses)
		{
			if (timeofSample == currentBusItem.depart)
			{
				return (currentBusItem.depart - currentBusItem.arrive);
			}
		}

        //If the time of sample passed to the method exceeds 24 hours, throw an out-of-range exception.
		if (timeofSample > 86400)
		{
			throw std::out_of_range("Sample out of range");
		}

        //If no argument is passed to the method, the method will throw an invalid argument exception.
		throw std::invalid_argument("No argument given");

}

//added 22/12/2020
//endTime uses timer->time() to store the current time. startTime uses the endTime minus the duration passed to the method to calculate the time being looked back over.
//A for loop iterates through the Busses vector. If the current bus' departure time is more than or equal to the startTime and less than or equal to the endTime,
//that bus will be pushed onto a vector called averageBusses. Iteration through this vector of averageBusses adds the departure time minus the arrival times to sum.
//Finally, sum is divided by the number of busses in averageBusses and returned to the function.
double Statistics::averageDuration(long duration)
{
	long endTime = _timer->time();
	long startTime = endTime - duration;

	//If the duration passed to the method exceeds 24 hours, an out of range exception is thrown.
	if (duration > 86400)
	{
		throw std::out_of_range("Sample was out of range!");
	}
	
	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.depart >= startTime && currentBusItem.depart <= endTime)
		{
			averageBusses.push_back(currentBusItem);
		}
	}

	for (Bus const& currentBusItem : averageBusses)
	{
		sum += currentBusItem.depart - currentBusItem.arrive;
	}

	//If no busses meet the criteria defined, throw an invalid argument exception.
	if (averageBusses.empty())
	{
		throw std::invalid_argument("No samples!");
	}
	return sum / averageBusses.size();

	
}

//added 22/12/2020
//performs a similar function to averageDuration. timer->() stores current time in endTime. startTime is calculated by subtracting endTime by duration. 
//Instead of averageBusses, busses which meet the criteria are stored in a vector called maxBusses. The vector stays wherein
//the total times spent at the station by each bus in the maxBusses vector is stored. An iterator is called on the stays vector where if the current stay 
//is less than the stay at index, the stay at index becomes the current stay.
long Statistics::maximumDuration(long duration)
{
	long endTime = _timer->time();
	long startTime = endTime - duration;

	//if the duration passed to the method exceeds 24 hours, throw an exeption.
	if (duration > 86400)
	{
		throw std::out_of_range("Sample was out of range!");
	}

	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.depart >= startTime && currentBusItem.depart <= endTime)
		{
			maxBusses.push_back(currentBusItem);
		}
	}

	for (Bus const& currentBusItem : maxBusses)
	{
		stays.push_back(currentBusItem.depart - currentBusItem.arrive);
	}

	for (std::size_t index = 0; index < stays.size(); index++)
	{
		if (stay < stays[index])
		{
			stay = stays[index];
		}
	}

	//if no busses meet the criteria specified in the first for loop of the method, throw an invalid argument exception. 
	if (maxBusses.empty())
	{
		throw std::invalid_argument("No samples were found!");
	}
	return stay;
}

//added 28/12/2020
//changed char variable to std::string. For loop iterates through Busses. If the busName passed to the method matches the current bus' name, avgPassengers is added to
//using the passenger count of the current bus item. Counter is then incremented by 1. Finally, the avgPassengers is divided by the counter.
double Statistics::averagePassengersBus(std::string busName)
{
	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.name == busName)
		{
			avgPassengers += currentBusItem.passengers;
			counter++;
		}
	}
	return avgPassengers / counter;

}

//added 05/01/2021
//for loop iterates through the Busses vector. If the platform number passed to the method matches the current bus item's platform, that bus' passengers are added to
//totalPassengers. Counter is then incremented by 1. platformsFound is then set to true. [NOTE: The developer is aware of the duplication of code contained in this bool line
//but despite the best efforts, rectifying it by placing it outside the for loop wouldn't allow for compilation of main()]. Finally, totalPassengers is divided by counter and returned
//to the function.
double Statistics::averagePassengersPlatform(int platformNumber)
{
	bool platformsFound = false;

	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.platform == platformNumber)
		{
			totalPassengers += currentBusItem.passengers;
			counter++;
			platformsFound = true;
		}
	}

	//if platformsFound remains false, throw invalid argument exception.
	if (platformsFound == false)
	{
		throw std::invalid_argument("Platforms not found");
	}

	return totalPassengers / counter;
}

//added 05/01/2021
//performs in much the same way as averagePassengersPlatform(). Iterates through Busses vector. If the platformNumber passed to the method matches the current bus'
//platform number, takes the current bus' time spent at the station and adds it to totalTime. Counter then incremented by 1. platformsFound then set to true. Finally,
//returns the totalTime divided by the counter. 
double Statistics::averageDurationPlatform(int platformNumber)
{
	bool platformsFound = false;

	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.platform == platformNumber)
		{
			totalTime += currentBusItem.depart - currentBusItem.arrive;
			counter++;
			platformsFound = true;
		}
	}

	//if no platforms are found, throw an invalid argument exception.
	if (platformsFound = false)
	{
		throw std::invalid_argument("Platforms not found");
	}

	return totalTime / counter;
}