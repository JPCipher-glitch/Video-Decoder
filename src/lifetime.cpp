#include "lifetime.hpp"

bool Lifetime::isAlive()
{
	return alive;
}

void Lifetime::update()
{

}

void Lifetime::destroy()
{
	alive = false;
}