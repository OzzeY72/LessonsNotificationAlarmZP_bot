#include "Day.hpp"

Day::Day(){}
Day::Day(DayName dayname, Lesson lesson)
{
	setDayname(dayname);
	addLesson(lesson);
}
Day::Day(DayName dayname){setDayname(dayname);}

void Day::setDayname(DayName dayname){m_dayname = dayname;}
void Day::addLesson(Lesson lesson)
{
	m_lessons.push_back(lesson);	
}

Day::DayName Day::getDayname() {return m_dayname;}
boost::container::list<Lesson>  *Day::getLessons()  {return &m_lessons;}
	
