#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "observer.hpp" 
#include <vector>

class Command
{
private:
	std::vector<IObserver*> obsList;
public:
	void attach(IObserver* obs);
	void notify();

	void createCommand();
	void update();
};

#endif
