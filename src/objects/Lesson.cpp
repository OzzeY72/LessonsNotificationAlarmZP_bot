#include "Lesson.hpp"

using namespace std;

Lesson::Lesson()
{
	this->setGroup(3);
	this->setPara(14,55);
}
Lesson::Lesson(int npara,int type)
{
	this->setGroup(3);
	this->setParaNumber(npara);
	this->setType(type);
	this->setTerm(true);
}
Lesson::Lesson(int group,int npara, int type, bool term,long zcode,string zpass)
{
	this->setParaNumber(npara);
	this->setGroup(group);
	this->setType(type);
	this->setTerm(term);
	this->setZoomCode(zcode);
	this->setZoomPass(zpass);
}

void Lesson::setGroup(int group){m_group=group;}
void Lesson::setPara(Ltime para){m_para=para;}
void Lesson::setPara(short hour,short min){m_para.setHour(hour);m_para.setMin(min);}
void Lesson::setType(int type){m_type=type;}
void Lesson::setTerm(bool term){m_persistent=term;}
void Lesson::setZoomCode(long zcode){m_zcode = zcode;}
void Lesson::setZoomPass(string zpass){m_zpass = zpass;}
void Lesson::setParaNumber(int npara){
	m_npara = npara;
	switch(m_npara)
	{
		case 1: setPara(8,0);break;
		case 2: setPara(9,35);break;
		case 3: setPara(11,25);break;
		case 4: setPara(12,55);break;
		case 5: setPara(14,30);break;
		case 6: setPara(16,0);break;
		default:break;
	}

}
void Lesson::setDayName(int day){m_dayname = day;}
void Lesson::setOutput(string output){m_output = output;}
void Lesson::setCh(int ch){m_ch = ch;}
int Lesson::getGroup() {return m_group;}
Ltime *Lesson::getPara()  {return &m_para;}
int  Lesson::getType()  {return m_type;}
int Lesson::getParaNumber(){return m_npara;}
bool  Lesson::getTerm()  {return m_persistent;}
long Lesson::getZoomCode(){return m_zcode;}
string *Lesson::getZoomPass(){return &m_zpass;}
int Lesson::getDayName(){return m_dayname;}
string *Lesson::getOutput(){return &m_output;}
int Lesson::getCh(){return m_ch;}
