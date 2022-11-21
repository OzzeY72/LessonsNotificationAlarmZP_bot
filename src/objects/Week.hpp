#pragma once
#ifndef Ibot_OBJECTS_WEEK_H_
#define Ibot_OBJECTS_WEEK_H_ 

#include "Day.cpp"

class Week
{
private:
    boost::container::list<Day> m_days;
    bool m_ch;
public:
	Week();	
	Week(bool ch);

	void addDay(Day day);
    	void setCh(bool ch);

    	bool getCh();
    	boost::container::list<Day> *getDays();
};
#endif
