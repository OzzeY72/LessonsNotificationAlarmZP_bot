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

int alarmcallback(void *data,int ac,char **av,char **acn)
{
			if(av[0] != "" && av[1] != ""){
			int *al = static_cast<int*>(data);
			*al = boost::lexical_cast<int>(av[1]);
			return *al;}
			return 0;
	
}
int getalarmcallback(void *data,int ac,char **av,char **acn)
{
			if(av[0] != "" && av[1] != ""){
			vector<long long> *al = static_cast<vector<long long>*>(data);
			al->push_back(boost::lexical_cast<long long>(av[0]));
			}
			return 0;
	
}
long long isAlarm(sqlite3 *db,long long id,int n,string astr)
{
	long long al;
	string sql = "SELECT * FROM alarm WHERE " + astr + to_string(id);
	cout << sql << endl;
	sqlite3_exec(db,sql.c_str(),alarmcallback,&al,NULL);
	if(n == 1) {
		sqlite3_stmt *st;
		sql = "DELETE FROM alarm WHERE id = " + to_string(id);
		cout << sql << endl;
		sqlite3_prepare(db,sql.c_str(),-1,&st,NULL);
		sqlite3_step(st);
	}
	return al;
}
vector<long long> getAlarm(sqlite3 *db)
{
	vector<long long> al;
	string sql = "SELECT * FROM alarm WHERE al = 1";
	cout << sql << endl;
	sqlite3_exec(db,sql.c_str(),getalarmcallback,&al,NULL);
	return al;
}
void alarm(sqlite3 *db,long long id,int al)
{
	string sql;
	sqlite3_stmt *st;
	sql = "INSERT INTO alarm (id,al) VALUES (?,?)"; 
	    //getAlarm(db);
	sqlite3_prepare(db, sql.c_str(),-1, &st,NULL);
	sqlite3_bind_int(st, 1, id);
	sqlite3_bind_int(st, 2, al);
	cout << sql << endl;
	sqlite3_step(st);
}
void getCurpara(int *cp)
{
	time_t now = time(0);	
	tm *ltm = localtime(&now);
	int hour = ltm->tm_hour+2;
	int min  = ltm->tm_min;
	int st = hour*60+min;
	if(st < 480) *cp = 0;
	else if(st >= 490 && st < 545) *cp = 1;
	else if(st >= 545 && st < 635) *cp = 2;
	else if(st >= 645 && st < 735) *cp = 3;
	else if(st >= 735 && st < 845) *cp = 4;
	else if(st >= 845 && st < 940) *cp = 5;
	else if(st >= 940 && st < 1440) *cp = 6;
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
		case 6: return "Фізична культура";
		case 7: return "Фізична культура";
	}
}


int selcallback(void *data,int ac,char **av,char **acn)
{
			if(av[0] == "") return 0;
			Lesson *lesson = static_cast<Lesson*>(data);
			lesson->setType(boost::lexical_cast<int>(av[2]));
			lesson->setGroup(boost::lexical_cast<int>(av[0]));
			lesson->setParaNumber(boost::lexical_cast<int>(av[1]));
			lesson->setCh(boost::lexical_cast<int>(av[4]));
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
	lesson->setType(-1);
	sqlite3_exec(db,sql.c_str(),selcallback,lesson,NULL);
	if(lesson->getType() == -1) return -1;
	return 0;
}
string outLesson(sqlite3 *db,Lesson *lesson)
{
	string sql = "SELECT * FROM para WHERE type = " + to_string(lesson->getType());
	cout << sql << endl;
	sqlite3_exec(db,sql.c_str(),outcallback,lesson,NULL);
	string str = "*Предмет:* " + convertType(lesson->getType()) + "\n";
	str += "*Підгрупа:* ";//"Наступна пара: \n*Підгрупа:* ";
	switch(lesson->getGroup()){case 1:str+="Тільки перша\n";break;case 2:str+="Тільки друга\n";break;
		default: str+="Обидві\n";break;}
	str+= "*Тиждень: *";
	switch(lesson->getCh()){case 1:str+="Знаменник \n";break;case 2:str+=" Чисельник\n";break;
		default: str+="Знаменник і Чисельник\n";break;}	
	str+= "*День:* " + convertDay(lesson->getDayName()) + "\n";
	str+= "*Початок:* " +  to_string(lesson->getPara()->getHour()) + ":" + to_string(lesson->getPara()->getMin()) + "\n";
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
		if(len == 6) lesson->setCh(stoi(strs.at(5)));
		else lesson->setCh(3);
	}
	return 0;
}

void writeDB(Lesson *lesson, sqlite3 *db)
{
	string sql;
	sqlite3_stmt *st;
	sql = "INSERT INTO lesson (gid,day,para,type,term,calenday) VALUES (?,?,?,?,?,?)"; 
	sqlite3_prepare(db, sql.c_str(),-1, &st,NULL);
	sqlite3_bind_int(st, 1, lesson->getGroup());
	sqlite3_bind_int(st, 2, lesson->getDayName());
	sqlite3_bind_int(st, 3, lesson->getParaNumber());
	sqlite3_bind_int(st, 4, lesson->getType());
	sqlite3_bind_int(st, 5, 0);
	sqlite3_bind_int(st, 6, lesson->getCh());
	cout << sql << endl;	
	sqlite3_step(st);
}

void writeGroup(sqlite3 *db,int gid,long long id)
{
	if(gid > 0 && gid <=3)
	{
	string sql;
	sqlite3_stmt *st;
	sql = "INSERT INTO tbl1 (id,gid) VALUES (?,?)"; 
	sqlite3_prepare(db, sql.c_str(),-1, &st,NULL);
	sqlite3_bind_int(st, 1, id);
	sqlite3_bind_int(st, 2, gid);
	cout << sql << endl;
	sqlite3_step(st);
	}
}

string Next(sqlite3 *db,int current_day,int current_para,int ch,long long id,string poj)
{
    	int gid = getGid(db,id);
	string astr = "";
	if(gid > 0 && gid < 3) 
	{
		astr = " and (gid = " + to_string(gid) + " or gid = 3)";
	}
	astr += " and (calenday = " + to_string(ch) + " or " + "calenday = 3)";
	astr += " " + poj;
    	Lesson *lesson = new Lesson();
	lesson->setDayName(current_day);
	lesson->setParaNumber(current_para);
	string result="";
	while(prepareOut(db,lesson,"=",astr) == -1)
	{
		bool flag = false;
		for(int i = lesson->getParaNumber();i < 6;i++)
		{
			if(prepareOut(db,lesson,"=",astr)!=-1) {flag = true;break;}
			lesson->setParaNumber(lesson->getParaNumber()+1);
		}
		if(flag) break;
			lesson->setDayName(lesson->getDayName()+1);
			lesson->setParaNumber(1);
		if(lesson->getDayName()>=7) return "";
	}
	if(lesson->getCh() == ch || lesson->getCh() == 3) result += outLesson(db,lesson);//bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");}
	if(lesson->getGroup() != 3)
			{
				string tastr = " and (calenday = " + to_string(ch) + " or " + "calenday = 3)" + " and gid = " + to_string((2-lesson->getGroup())+1);
				tastr += " " + poj;
				if(prepareOut(db,lesson,"=",tastr) != -1){ if((lesson->getCh() == ch || lesson->getCh() == 3) && (lesson->getGroup() == gid || gid == 3 || lesson->getGroup() == 3))  result = result + "@" + outLesson(db,lesson);}
			}
	return result;
	
}
void alarmA(vector<long long> ids,Bot *bot,sqlite3 *db,int current_day,int current_para,int ch)
{
	for(auto &id : ids) cout << "\n" << id << "\n";
	for(auto it = ids.begin();it != ids.end();it++)
	{
		//string str = Next(db,current_day,current_para,ch,*it,"");
		string str = Next(db,current_day,current_para,ch,*it," and day = " + to_string(current_day) + " ");
		vector<string> strs;
		boost::split(strs,str,boost::is_any_of("@"));
		for(auto ij = strs.begin();ij != strs.end();ij++)
			{
    				try{bot->getApi().sendMessage(*it,*ij,0,0,make_shared<GenericReply>(),"MarkdownV2");}
		catch (TgException& e) {
	                 printf("error: %s\n", e.what());
         	}
			} 
	}
}

int main() {
    Bot /*bot("5730048014:AAHA6-j3A-3tWcL9zl6yFRkjpI8yIkW_naE");*/bot("5698184782:AAEg-543MH1j18AYrvYlmiD34jB3BQfOyFE");
    sqlite3 *db;
    sqlite3_open("bot.db",&db);
    vector<long long> ids = getAlarm(db);
    time_t now = time(0);
    int ch =0;
    int current_day = localtime(&now)->tm_wday;
    cout << localtime(&now)->tm_yday << endl;
    cout << "Today " << current_day << endl;
    int tmph = localtime(&now)->tm_hour;
    int current_para = 0;
    getCurpara(&current_para);
    cout << "Para" << current_para;
    int tmtmtmint = localtime(&now)->tm_yday;
    if(((tmtmtmint+5)/7)%2 == 0) {ch = 1;cout << "\n NAME\n";}
    else {ch = 2;}   
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
    cm7->description = "Додати пару формат: <група(3-всі)> <день> <назва пари> <номер пари> <Ч/З(3 для обох)>";
    commands.push_back(cm7);

    BotCommand::Ptr cm8(new BotCommand);
    cm8->command = "show_day";
    cm8->description = "Вивести пари на день";
    commands.push_back(cm8);

    BotCommand::Ptr cm9(new BotCommand);
    cm9->command = "alarm";
    cm9->description = "Дозволити/Відключити попередження про початок пари";
    commands.push_back(cm9);

    bot.getApi().setMyCommands(commands);
    vector<BotCommand::Ptr> vectCmd;
    vectCmd = bot.getApi().getMyCommands();

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        try{bot.getApi().sendMessage(message->chat->id, "Привіт, цей бот показує розклад пар для 6.1212-пзс, та може попереджувати про початок занять");  }
      catch (TgException& e) {
        printf("error: %s\n", e.what());
    }
    try{bot.getApi().sendMessage(message->chat->id, "Отримати список команд: /help");}
    catch (TgException& e) {
        printf("error: %s\n", e.what());
    }
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
	    try{bot.getApi().sendMessage(message->chat->id, s );}
    	    catch (TgException& e) {
        	printf("error: %s\n", e.what());
    	    }
	});
    //ALLOW_ALARM
    bot.getEvents().onCommand("alarm", [&bot,&db](Message::Ptr message) {
	long long alst = isAlarm(db,message->chat->id,1," id = ");
	alarm(db,message->chat->id,!alst);
	try{bot.getApi().sendMessage(message->chat->id,(!alst) ? "Сповіщення встановелно" : "Сповіщення відключено");	}
     	catch (TgException& e) {
        	printf("error: %s\n", e.what());
    	}
    });
    //NEXT
    bot.getEvents().onCommand("next", [&db,&bot,&current_day,&current_para,&ch](Message::Ptr message)
    {
	vector<string> strs;
	string next_string = Next(db,current_day,current_para,ch,message->chat->id,"");
	if(next_string.find("@") != std::string::npos) 
		boost::split(strs,next_string,boost::is_any_of("@"));
	else strs.push_back(next_string);
		for(auto it = strs.begin();it != strs.end();++it)
		{
    			try{bot.getApi().sendMessage(message->chat->id,*it,0,0,make_shared<GenericReply>(),"MarkdownV2");}
			catch (TgException& e) {
	                	 printf("error: %s\n", e.what());
         		}
		}
    });
    //SHOW_DAY	
    bot.getEvents().onCommand("show_day", [&db,&bot,&current_day,&current_para,&ch](Message::Ptr message)
    {
    	int num = 0;
	vector<string> strs;
	boost::split(strs,message->text,boost::is_any_of(" "));
	int len = strs.size();
	if(len == 1) num = current_day;
       	else {num = (strs.at(1) != "") ? stoi(strs.at(1)) : current_day;}

    	int gid = getGid(db,message->chat->id);
	if(!gid) gid = 3;
	string astr = "";
	if((gid > 0 && gid < 3)) 
	{
		astr = " and (gid = " + to_string(gid) + " or gid = 3)";

	}
	astr += " and (calenday = " + to_string(ch) + " or " + "calenday = 3)";
    	for(int i = 0; i <= 6;i++) 
	{
		Lesson *lesson = new Lesson();
		(num > 0 && num <=7) ? lesson->setDayName(num) : lesson->setDayName(current_day);
		lesson->setParaNumber(i);
		lesson->setGroup(0);
		if(prepareOut(db,lesson,"=",astr) != -1)
		{
			if(lesson->getCh() == ch || lesson->getCh() == 3){try{bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");}	
   				catch (TgException& e) {
        				printf("error: %s\n", e.what());
    				}
			}
			if(lesson->getGroup() != 3)
			{
				string tastr = astr + " and gid = " + to_string((2-lesson->getGroup())+1);
				try{if(prepareOut(db,lesson,"=",tastr) != -1){ if((lesson->getCh() == ch || lesson->getCh() == 3) && (lesson->getGroup() == gid || gid == 3 || lesson->getGroup() == 3)) bot.getApi().sendMessage(message->chat->id,outLesson(db,lesson),0,0,make_shared<GenericReply>(),"MarkdownV2");}}
   				catch (TgException& e) {
        				printf("error: %s\n", e.what());
    				}
			}
		}
	}
	});
	//WEEK_TYPE
    bot.getEvents().onCommand("week_type", [&ch,&bot](Message::Ptr message)
    {	
        try{bot.getApi().sendMessage(message->chat->id, ((ch == 2 ) ? "Чисельник" : "Знаменник"));}
    	catch (TgException& e) {
        	printf("error: %s\n", e.what());
    	}
    });
	//ADD_LESSON
    bot.getEvents().onCommand("add_lesson", [&bot,&db](Message::Ptr message)
    {
	Lesson *lesson = new Lesson();	
	if(parseLesson(lesson,message) == -2) {try{bot.getApi().sendMessage(message->chat->id, "Не правильно введена команда");}
    	catch (TgException& e) {
        	printf("error: %s\n", e.what());
    	}
	}
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
        try{bot.getApi().sendMessage(message->chat->id, outstr);}
    	catch (TgException& e) {
        	printf("error: %s\n", e.what());
    	}
	writeDB(lesson,db);
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
	bool flg = false;
        TgLongPoll longPoll(bot);
        while (true) {
    	    time_t now = time(0);
            getCurpara(&current_para);
    	    current_day = localtime(&now)->tm_wday;
    	    int tmtmtmint = localtime(&now)->tm_yday;
	    if(((tmtmtmint+5)/7)%2 == 0) {ch = 1;}
    	    else {ch = 2;}   
	    tm *ltm = localtime(&now);
	    int hour = ltm->tm_hour+2;
	    int min  = ltm->tm_min;
	    if(hour == 0 and min == 0) cout << "Day: " << current_day << "\t Para: " << current_para << "\t Chsl: " << ch << endl;
	    cout << hour << ":" << min << ":" << ltm->tm_sec << endl;
	    int st = hour*60+min;
	    if(st == 470 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);	
	    } else if(st == 471) flg = false;

	    if(st == 565 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);
	    } else if(st == 566) {flg = false;}
	    if(st == 675 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);	
	    } else if(st == 676) flg = false;

	    if(st == 765 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);	
	    } else if(st == 766) flg = false;

	    if(st == 860 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);
	    } else if(st == 861) flg = false;
	    if(st == 950 && !flg)
	    {
	    	flg = true;
		ids = getAlarm(db);
		alarmA(ids,&bot,db,current_day,current_para,ch);	
	    } else if(st == 950) flg = false;
	    
            longPoll.start();
        }
    } catch (TgException& e) {
        printf("error: %s\n", e.what());
    }
    sqlite3_close(db);
    return 0;
}
