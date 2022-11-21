#ifndef Ibot_OBJECTS_LESSON_H_
#define Ibot_OBJECTS_LESSON_H_
#include <ctime>
#include <iostream>

using namespace std;

class Ltime
{
private:
	short m_hour;
	short m_min;
public:
	Ltime(){setHour(14);setMin(0);}
	Ltime(short hour,short min)
	{
		setHour(hour);
		setMin(min);
	}
	void setHour(short hour){m_hour = (hour < 24) ? hour : 0;}
	void setMin(short min){  m_min  = (min  < 60) ? min  : 0;}
	short getHour(){return m_hour;}
	short getMin(){return m_min;}
};
class Lesson
{
public: enum Group
	{
		FIRST,
		SECOND,
		BOTH
	};
	enum Type
	{
		MATH,
		ALGORITM,
		ARCHITECTURE,
		UKRANIAN,
		HISTORY,
		ENGLISH,
		PE
	};
private:
	int m_group;
	Ltime m_para;
	int  m_type;
	int   m_dayname;
	int   m_npara;
	bool m_persistent;
	long m_zcode;	
	string m_zpass;
	string m_output;
	int m_ch;
public:
	Lesson();
	Lesson(int npara,int type);
	Lesson(int group,int npara,int type,bool term,long zcode,string zpass);
	void setCh(int ch);
	void setGroup(int group);
	void setPara(Ltime para);
	void setPara(short hour,short min);
	void setType(int type);
	void setTerm(bool term);
	void setZoomCode(long zcode);
	void setZoomPass(string zpass);
	void setParaNumber(int npara);
	void setDayName(int day);
	void setOutput(string output);

	int getGroup();
	Ltime *getPara();
	int getType();
	bool getTerm();
	int getParaNumber();
	long getZoomCode();
	string *getZoomPass();
	int getDayName();
	string *getOutput();
	int getCh();
};
#endif
