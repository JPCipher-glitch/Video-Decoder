#ifndef LIFETIME_HPP
#define LIFETIME_HPP

class Lifetime
{
private:
	bool alive = true;
	float timer = 0.f;
public:
	bool isAlive();

	void update();
	void destroy();
};

#endif
