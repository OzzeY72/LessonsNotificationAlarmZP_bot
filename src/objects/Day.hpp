#ifndef Ibot_OBJECTS_DAY_H_
#define Ibot_OBJECTS_DAY_H_

#include <boost/container/list.hpp>
#include "Lesson.cpp"

class Day
{
private:
	enum class DayName : int 
	{
        	MONDAY,
        	TUESDAY,
		WEDNESDAY,
		THURSDAY,
		FRIDAY,
		SATURDAY,
		SUNDAY
	};
	boost::container::list<Lesson> m_lessons;
	DayName m_dayname;

	Day();
	Day(DayName dayname);
	Day(DayName dayname, Lesson lesson);
	void addLesson (Lesson lesson);
	void setDayname(DayName dayname);

	DayName 			getDayname();
	boost::container::list<Lesson>  *getLessons();


};
#endif
