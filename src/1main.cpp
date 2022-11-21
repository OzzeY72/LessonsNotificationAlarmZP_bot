#include <iostream>
#include <sstream>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <tgbot/tgbot.h>
#include <string>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <sqlite/sqlite3.h>
#include "objects/Lesson.cpp"

using namespace std;
using namespace TgBot;


void getCurpara(int *cp)
{
	time_t now = time(0);	
	tm *ltm = localtime(&now);
	int hour = ltm->tm_hour;
	int min  = ltm->tm_min;
	cout << hour << " " << min << endl;
	int st = hour*60+min;
	if(st < 480) *cp = 0;
	else if(st >= 490 && st < 575) *cp = 1;
	else if(st >= 575 && st < 685) *cp = 2;
	else if(st >= 685 && st < 775) *cp = 3;
	else if(st >= 775 && st < 870) *cp = 4;
	else if(st >= 870 && st < 960) *cp = 5;
}


string convertDay(int day)
{
	switch(day)
	{
		case 1: return "Понеділок";
		case 2: return "Вівторок";
		case 3: return "Середа";
		case 4: return "Четвер";
		case 5: return "П'ятниця";
		case 6: return "Суббота";
		case 7: return "Неділя";
		default: return "Error";
	}
}
string convertType(int type)
{
	switch(type)
	{
		case 0: return "Вишмат";
		case 1: return "Основи Алгоритмізації";
		case 2: return "Архітектура";
		case 3: return "Українська мова";
		case 4: return "Історія України";
		case 5: return "English";
		case 6: return "Ф-к";
	}
}


int selcallback(void *data,int ac,char **av,char **acn)
{
			if(av[0] == "") return -1;
			Lesson *lesson = static_cast<Lesson*>(data);
			lesson->setType(boost::lexical_cast<int>(av[2]));
			lesson->setGroup(boost::lexical_cast<int>(av[0]));
			lesson->setParaNumber(boost::lexical_cast<int>(av[1]));
			cout << lesson->getType() << "\t" << lesson->getGroup() << "\t" << lesson->getParaNumber() << endl;
			return 0;
}
int outcallback(void *data,int ac,char **av,char **acn)
{
			string str(av[1]);
			Lesson *lesson = static_cast<Lesson*>(data);
			lesson->setZoomCode(boost::lexical_cast<long>(av[0]));
			lesson->setZoomPass(str);
			return 0;
}
int getcallback(void *data,int ac,char **av,char **acn)
{
			if(av[0] != "" && av[1] != ""){	
			int *gid = static_cast<int*>(data);
			*gid = boost::lexical_cast<int>(av[1]);
			return *gid;}
			return 0;
}

int prepareOut(sqlite3 *db,Lesson *lesson, char *symbol,string argstr)
{
	string sql = "SELECT * FROM lesson WHERE day = " + to_string(lesson->getDayName()) + " and para " + symbol + " " + to_string(lesson->getParaNumber()) + argstr;
	cout << sql << endl;
	sqlite3_exec(db,sql.c_str(),selcallback,lesson,NULL);
	cout << lesson->getGroup() << "\t" <<  lesson->getDayName() << "\t" << lesson->getParaNumber() << "\t" << lesson->getType() << endl;
	if(lesson->getGroup() == 0) return -1;
	return 0;
}
string outLesson(sqlite3 *db,Lesson *lesson)
{
	string sql = "SELECT * FROM para WHERE type = " + to_string(lesson->getType());
	sqlite3_exec(db,sql.c_str(),outcallback,lesson,NULL);
	cout << lesson->getZoomCode() << "\t" << *lesson->getZoomPass() << endl;
	string str = "*Предмет:* " + convertType(lesson->getType()) + "\n";
	str += "*Підгрупа:* ";//"Наступна пара: \n*Підгрупа:* ";
	switch(lesson->getGroup()){case 1:str+="Тільки перша\n";break;case 2:str+="Тільки друга\n";break;
		default: str+="Обидві\n";break;}
	str+= "*День:* " + convertDay(lesson->getDayName()) + "\n";
	str+= "*Початок:* " +  to_string(lesson->getPara()->getHour()) + ":" + to_string(lesson->getPara()->getMin()) + "\n";
	str+= "*Предмет:* " + convertType(lesson->getType()) + "\n";
	if(lesson->getZoomCode() != 0){str+= "*Zoom код:* " + to_string(lesson->getZoomCode());str+="\n";
	str+= "*Zoom пароль:* "; str+= *lesson->getZoomPass();str+="\n";}
	return str;
}
int getGid(sqlite3 *db,int id)
{
	int gid = 0;
	string sql = "SELECT * FROM tbl1 WHERE id = " + to_string(id);
	sqlite3_exec(db,sql.c_str(),getcallback,&gid,NULL);
	return gid;	
}
int parseLesson(Lesson *lesson, Message::Ptr message)
{
	vector<string> strs;
	boost::split(strs,message->text,boost::is_any_of(" "));
	int len = strs.size();
	if(len >= 5 && len <=7)
	{
		lesson->setGroup(stoi(strs.at(1)));
		lesson->setDayName(stoi(strs.at(2)));
		lesson->setType(stoi(strs.at(3)));
		lesson->setParaNumber(stoi(strs.at(4)));
		if(len == 6) lesson->setTerm(stoi(strs.at(5)));
		else lesson->setTerm(0);
	}
	return 0;
}

void writeDB(Lesson *lesson, sqlite3 *db)
{
	cout << lesson->getGroup() << "\t" << lesson->getDayName() << "\t" << lesson->getParaNumber() << "\t" << lesson->getType() << endl;
	string sql;
	sqlite3_stmt *st;
	sql = "INSERT INTO lesson (gid,day,para,type,term,calenday) VALUES (?,?,?,?,?,?)"; 
	sqlite3_prepare(db, sql.c_str(),-1, &st,NULL);
	sqlite3_bind_int(st, 1, lesson->getGroup());
	sqlite3_bind_int(st, 2, lesson->getDayName());
	sqlite3_bind_int(st, 3, lesson->getParaNumber());
	sqlite3_bind_int(st, 4, lesson->getType());
	sqlite3_bind_int(st, 5, int(lesson->getTerm()));
	sqlite3_bind_int(st, 6, 0);
	
	sqlite3_step(st);
}

void writeGroup(sqlite3 *db,int gid,long id)
{
	if(gid > 0 && gid <=3)
	{
	string sql;
	sqlite3_stmt *st;
	sql = "INSERT INTO tbl1 (id,gid) VALUES (?,?)"; 
	sqlite3_prepare(db, sql.c_str(),-1, &st,NULL);
	sqlite3_bind_int(st, 1, id);
	sqlite3_bind_int(st, 2, gid);

	sqlite3_step(st);
	}
}

int main() {
    Bot bot("5698184782:AAEg-543MH1j18AYrvYlmiD34jB3BQfOyFE");
    sqlite3 *db;
    sqlite3_open("bot.db",&db);
    time_t now = time(0);
    bool ch =false;
    int current_day = (localtime(&now)->tm_yday%7)+6;
    cout << localtime(&now)->tm_yday << endl;
    cout << "Today " << current_day << endl;
    int tmph = localtime(&now)->tm_hour;
    int current_para = 0;
    getCurpara(&current_para);
    cout << "Para" << current_para;
    if((localtime(&now)->tm_yday/7)%2 == 0) ch = true;
    else ch = false;   
    vector<BotCommand::Ptr> commands;
    BotCommand::Ptr cm1(new BotCommand);
    cm1->command = "start";
    cm1->description = "Команда для розпочатку роботи";
    commands.push_back(cm1);
	
    BotCommand::Ptr cm2(new BotCommand);
    cm2->command = "help";
    cm2->description = "Вивести всі можлииві команди";
    commands.push_back(cm2);

    BotCommand::Ptr cm3(new BotCommand);
    cm3->command = "set_group";
    cm3->description = "Команда для вказання підгруппи";
    commands.push_back(cm3);

    BotCommand::Ptr cm4(new BotCommand);
    cm4->command = "add_lesson";
    cm4->description = "Команда для створення пари";
    commands.push_back(cm4);

    BotCommand::Ptr cm5(new BotCommand);
    cm5->command = "week_type";
    cm5->description = "Який тиждень";
    commands.push_back(cm5);

    BotCommand::Ptr cm6(new BotCommand);
    cm6->command = "next";
    cm6->description = "Показати наступну пару";
    commands.push_back(cm6);
    
    BotCommand::Ptr cm7(new BotCommand);
    cm7->command = "add_lesson";
    cm7->description = "Додати пару формат: <гр> <дн> <нзв пр> <номер пр> <тм ?> <день>";
    commands.push_back(cm7);

    BotCommand::Ptr cm8(new BotCommand);
    cm8->command = "show_day";
    cm8->description = "Вивести пари на день";
    commands.push_back(cm8);

    bot.getApi().setMyCommands(commands);
    vector<BotCommand::Ptr> vectCmd;
    vectCmd = bot.getApi().getMyCommands();

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Привіт, цей бот показує розклад пар для 6.1212-пзс, та може попереджувати про початок занять");  
    bot.getApi().sendMessage(message->chat->id, "Отримати список команд: /help");
    });
    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        vector<BotCommand::Ptr> vectCmd;
	vectCmd = bot.getApi().getMyCommands();
	string s = "";
	for(auto it = vectCmd.begin();it != vectCmd.end();it++)
	{
	    auto obj = *it;
	    s +=  "/" + obj->command + "\t" + obj->description + "\n";
	}	
	    bot.getApi().sendMessage(message->chat->id, s );
	});
    //NEXT
    bot.getEvents().onCommand("next", [&db,&bot,&current_day,&current_para](Message::Ptr message)
    {	
    	int gid = getGid(db,message->chat->id);
	string astr = "";
	if(gid > 0 && gid < 3) 
	{
		astr = " and (gid = " + to_string(gid) + " or gid = 3)";
	}
    	Lesson *lesson = new Lesson();
	lesson->setDayName(1);
	lesson->setParaNumber(current_para-1);
    	prepareOut(db,lesson,">",astr);
	cout << convertType(lesson->getType()) << endl;
	bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");
	});
    //SHOW_DAY
    bot.getEvents().onCommand("show_day", [&db,&bot,&current_day,&current_para](Message::Ptr message)
    {
    	int num = 0;
	vector<string> strs;
	boost::split(strs,message->text,boost::is_any_of(" "));
	int len = strs.size();
	if(len == 1) num = current_day;
       	else num = stoi(strs.at(1));	

    	int gid = getGid(db,message->chat->id);
	string astr = "";
	if(gid > 0 && gid < 3) 
	{
		astr = " and (gid = " + to_string(gid) + " or gid = 3)";

	}
    	for(int i = 0; i <= 6;i++) 
	{
		Lesson *lesson = new Lesson();
		(num > 0 && num <=7) ? lesson->setDayName(num) : lesson->setDayName(current_day);
		lesson->setParaNumber(i);
		lesson->setGroup(0);
		if(prepareOut(db,lesson,"=",astr) != -1)
		{	
			bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");
		}
	}
	//bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");
});
	//WEEK_TYPE
    bot.getEvents().onCommand("week_type", [&ch,&bot](Message::Ptr message)
    {	
        bot.getApi().sendMessage(message->chat->id, ((ch) ? "Чисельник" : "Знаменник"));
    });
	//ADD_LESSON
    bot.getEvents().onCommand("add_lesson", [&bot,&db](Message::Ptr message)
    {
	Lesson *lesson = new Lesson();	
	if(parseLesson(lesson,message) == -2) {bot.getApi().sendMessage(message->chat->id, "Не правильно введена команда");}
	else writeDB(lesson,db);
    });
	//SET_GROUP
    bot.getEvents().onCommand("set_group", [&bot,&db](Message::Ptr message)
    {
    	string outstr = "";
	Lesson *lesson = new Lesson();
	vector<string>	strs;
	boost::split(strs,message->text,boost::is_any_of(" "));
	if(strs.size() == 2)
	{
		outstr = "Группу встановленно: " + strs.at(1);
		writeGroup(db,stoi(strs.at(1)),message->chat->id);
	}
	else outstr = "Не правильно введена команда";
        bot.getApi().sendMessage(message->chat->id, outstr);
	writeDB(lesson,db);
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgException& e) {
        printf("error: %s\n", e.what());
    }
    sqlite3_close(db);
    return 0;
}
