#ifndef OBSERVER_HPP
#define OBSERVER_HPP

struct IPacket;

class IObserver
{
private:
public:
	virtual void call(IPacket* packet) = 0;
};

#endif
