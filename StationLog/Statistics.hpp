#pragma once

#include <memory>
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
	bool timeInRange(long time);

public:
	Statistics(const std::shared_ptr<IBusReader>& reader, const std::shared_ptr<ITimer>& timer);
	~Statistics();

	// 3rd
	long readData();
	long sampleCount(long duration) ;
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
	double averageDurationPlatform(int) { return -1; };
	


};


Statistics::Statistics(const std::shared_ptr<IBusReader>& reader, const std::shared_ptr<ITimer>& timer)
{
	_stationData = reader;
	_timer = timer;
	_timer->start();
}


Statistics::~Statistics()
{
}


long Statistics::readData() 
{
	Bus BusSample;

	long timeOfSample = _timer->time(); 

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

long Statistics::sampleCount(long duration)		//added 09/12/2020
{
	long Samples = 0;
	long endTime = _timer->time();
	long startTime = endTime - duration;		//takes duration as defined value from user
	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.depart >= startTime && currentBusItem.depart <= endTime)
			{
				Samples++;
			}
	}
	return Samples;
}

long Statistics::sampleTotal()
{
	return Busses.size();
}

void Statistics::clearSamples()
{
	Busses.clear();
}

long Statistics::getDuration(int timeofSample)
{
		for (Bus const& currentBusItem : Busses)
		{
			if (timeofSample == currentBusItem.depart)
			{
				return (currentBusItem.depart - currentBusItem.arrive);
			}
		}

		if (timeofSample > 86400)
		{
			throw std::out_of_range("Sample out of range");
		}

		throw std::invalid_argument("No argument given");

}

double Statistics::averageDuration(long duration)
{
	double sum = 0;
	std::vector<Bus>averageBusses;
	long endTime = _timer->time();
	long startTime = endTime - duration;
	
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
	if (averageBusses.empty())
	{
		throw std::invalid_argument("No samples!");
	}
	return sum / averageBusses.size();

	
}

long Statistics::maximumDuration(long duration)
{
	long stay = 0;
	std::vector<Bus>maxBusses;
	std::vector<long>stays;
	long endTime = _timer->time();
	long startTime = endTime - duration;

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

	if (maxBusses.empty())
	{
		throw std::invalid_argument("No samples were found!");
	}
	return stay;
}

double Statistics::averagePassengersBus(std::string busName)
{
	int counter = 0;
	double avgPassengers = 0;

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

double Statistics::averagePassengersPlatform(int platformNumber)
{
	double totalPassengers = 0;
	int counter = 0;

	for (Bus const& currentBusItem : Busses)
	{
		if (currentBusItem.platform == platformNumber)
		{
			totalPassengers += currentBusItem.passengers;
			counter++;
		}
	}
	return totalPassengers / counter;
}


