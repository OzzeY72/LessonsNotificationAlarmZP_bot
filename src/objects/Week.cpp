#include "Week.hpp"

Week::Week(){setCh(false);}
Week::Week(bool ch)
{
	setCh(ch);
}
void Week::addDay(Day day)
{
	m_days.push_back(day);
}
void Week::setCh(bool ch)
{
    m_ch = ch;
}
bool Week::getCh()
{
    return m_ch;
}
boost::container::list<Day> *Week::getDays()
{
    return &m_days;
}
