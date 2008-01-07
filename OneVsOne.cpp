// OneVsOne.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "bzfsAPI.h"
#include <time.h>
#include <vector>

BZ_GET_PLUGIN_VERSION

#define ONEVSONE "1.0.3"

#define MAX_PLAYERS 2

#define DEBUG_TAG "DEBUG::OneVsOne PLUGIN::" 
#define DEBUG_LEVEL 2


// class Player
// class Match
// class Arena
//

//class Player
//{
//	private:
//
//		std::string _callsign;
//		std::map<std::string, bool> _style;
//
//	public:
//
//		// constructors
//
//		Player(){};
//
//		// getters
//		
//		std::string getCallsign(void);
//
//		// setters
//
//		void setCallsign(std::string callsign);
//		void setStyle(std::string style, bool);
//
//		// issers
//
//		bool hasStyle(std::string style);
//
//		// doers
//
//}
//
//
//class Match
//{
//
//	private:
//
//		int _maxLives;
//		bool _recording;
//
//	public:
//
//		Match(){};
//
//		// getters
//
//		int getMaxLives(void);
//
//		// setters
//
//
//
//		// issers
//
//
//
//		// doers
//		
//		void stopRecording(void);
//		void startRecording(void);
//
//
//}

typedef struct {
	int losses;
	std::string callsign;
	bool official;
	bool contest;
} OneVsOnePlayer;

std::map<int, OneVsOnePlayer> Players;

bool recording=false;

// report url

std::string url = "http://catay.be/scripts/action.php";
std::string homeurl = "http://1vs1.bzleague.com/scripts/plugin_bot.php";
//std::string url = "http://1vs1.bzleague.com/scripts/auto_match_report.php";

// default lives 
int LIVES=10;
// if "none", scores will be written to debug level 2
char LOGFILE[512]="none";

/// HELPER STUFF ///

// function returns true when a valid status is returned.

bool is_valid_status(const std::string& _data) 
{
	if (_data.size() < 2)
		return false;

	if ( _data.compare(0,2, "OK") == 0 || _data.compare(0,3, "NOK") == 0)
		return true;

	return false;
}


/// Class PlayerInfo
/// The PlayerInfo class handles player info queries
///

class PlayerInfo : public bz_URLHandler
{

	public:
			PlayerInfo() { _playerIds.clear();};
			~PlayerInfo() {};

			virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
			void setPlayerId(int playerId);

	private:
			std::vector<int> _playerIds;
};

void PlayerInfo::setPlayerId(int playerId)
{
	_playerIds.push_back(playerId);
}

void PlayerInfo::done ( const char* /*URL*/, void * data, unsigned int size, bool complete )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());

	if ( size )
	{
		std::string _data; 

		_data.append((char*)data, size);

		if (is_valid_status(_data))	
		{
			bzAPIStringList* dataList = bz_newStringList();
			dataList->tokenize(_data.c_str(), "\r\n");

			if (dataList->get(0) == "OK") 
			{
				for ( unsigned int i = 1; i < dataList->size(); i++)
				{
					bzAPIStringList* playerList = bz_newStringList();
					playerList->tokenize(dataList->get(i).c_str(), "\t", 6, false);

					bz_sendTextMessagef ( BZ_SERVER, _playerId,"Player info for %s", playerList->get(0).c_str());
					bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"zelo         : %s", playerList->get(1).c_str());
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"score        : %s", playerList->get(5).c_str());
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"matches won  : %s", playerList->get(2).c_str());
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"matches lost : %s", playerList->get(3).c_str());
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"status       : %s", playerList->get(4).c_str());
					bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");

					bz_deleteStringList(playerList);
				}
			}

			if (dataList->get(0) == "NOK")
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(1).c_str());

			bz_deleteStringList(dataList);
		}
		else
			bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");
	}

}

/// Class TopScore
/// The TopScore class handles the topscore info queries
///

class TopScore : public bz_URLHandler
{

	public:
			TopScore() { _playerIds.clear();};
			~TopScore() {};

			virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
			void setPlayerId(int playerId);

	private:
			std::vector<int> _playerIds;
};

void TopScore::setPlayerId(int playerId)
{
	_playerIds.push_back(playerId);
}

void TopScore::done ( const char* /*URL*/, void * data, unsigned int size, bool complete )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());

	if ( size )
	{
		std::string _data; 

		_data.append((char*)data, size);

		if (is_valid_status(_data))	
		{
			bzAPIStringList* dataList = bz_newStringList();
			dataList->tokenize(_data.c_str(), "\r\n");

			if (dataList->get(0) == "OK") 
			{


				bz_sendTextMessage ( BZ_SERVER, _playerId,"Monthly points ranking");
				bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");
				bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", "Pos", "Player", "Score");
				bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", "---", "------", "-----");


				for ( unsigned int i = 1; i < dataList->size(); i++)
				{
					bzAPIStringList* topScoreList = bz_newStringList();
					topScoreList->tokenize(dataList->get(i).c_str(), "\t", 3, false);

					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", topScoreList->get(0).c_str(), 
																		topScoreList->get(1).c_str(),topScoreList->get(2).c_str());

					bz_debugMessagef ( DEBUG_LEVEL,"%-4s %-32s %-4s", topScoreList->get(0).c_str(), 
																		topScoreList->get(1).c_str(),topScoreList->get(2).c_str());
					bz_deleteStringList(topScoreList);
				}

				bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");
			}

			if (dataList->get(0) == "NOK")
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(1).c_str());

			bz_deleteStringList(dataList);
		}
		else
			bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");
	}

}

/// Class TopZelo
/// The TopZelo class handles the topzelo info queries
///

class TopZelo : public bz_URLHandler
{

	public:
			TopZelo() { _playerIds.clear();};
			~TopZelo() {};

			virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
			void setPlayerId(int playerId);

	private:
			std::vector<int> _playerIds;
};

void TopZelo::setPlayerId(int playerId)
{
	_playerIds.push_back(playerId);
}

void TopZelo::done ( const char* /*URL*/, void * data, unsigned int size, bool complete )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());

	if ( size )
	{
		std::string _data; 

		_data.append((char*)data, size);

		if (is_valid_status(_data))	
		{
			bzAPIStringList* dataList = bz_newStringList();
			dataList->tokenize(_data.c_str(), "\r\n");

			if (dataList->get(0) == "OK") 
			{
				bz_sendTextMessage ( BZ_SERVER, _playerId,"Zelo score ranking");
				bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");
				bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", "Pos", "Player", "Zelo");
				bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", "---", "------", "----");

				for ( unsigned int i = 1; i < dataList->size(); i++)
				{
					bzAPIStringList* topZeloList = bz_newStringList();
					topZeloList->tokenize(dataList->get(i).c_str(), "\t", 3, false);

					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%-4s %-32s %-4s", topZeloList->get(0).c_str(), 
																		topZeloList->get(1).c_str(),topZeloList->get(2).c_str());

					bz_deleteStringList(topZeloList);
				}

				bz_sendTextMessage ( BZ_SERVER, _playerId,"-------------------------------------------");
			}

			if (dataList->get(0) == "NOK")
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(1).c_str());

			bz_deleteStringList(dataList);
		}
		else
			bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");
	}

}


///
/// Class Report
/// The Report class handles match score reporting
///

class Report : public bz_URLHandler
{

	public:
			Report() { _playerIds.clear();};
			~Report() {};
			virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
			virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );
			void setPlayerId(int playerId);

	private:
			std::vector<int> _playerIds;


};

void Report::setPlayerId(int playerId)
{
	_playerIds.push_back(playerId);
}

void Report::done ( const char* /*URL*/, void * data, unsigned int size, bool complete )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());

	if ( size )
	{
		std::string _data; 

		_data.append((char*)data, size);

		if (is_valid_status(_data))	
		{
			bzAPIStringList* dataList = bz_newStringList();
			dataList->tokenize(_data.c_str(), "\r\n");

			if (dataList->get(0) == "OK") 
				for ( unsigned int i = 1; i < dataList->size(); i++)
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(i).c_str());

			if (dataList->get(0) == "NOK")
				for ( unsigned int i = 1; i < dataList->size(); i++)
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(i).c_str());

			bz_deleteStringList(dataList);
		}
		else
			bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");
	}
	else
		bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");

}

void Report::error ( const char* /*URL*/, int errorCode, const char * errorString )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());
	 bz_sendTextMessagef(BZ_SERVER, _playerId,"Reporting failed with errorcode = %d - %s  !!",errorCode, errorString);
}


///
/// Class Register
/// The Register class handles player registration.
///


class Register : public bz_URLHandler
{
	public:

			Register() { _playerIds.clear(); };
			~Register() {}; 

			void setPlayerId(int playerId);

			virtual void done ( const char* /*URL*/, void * data, unsigned int size, bool complete );
			virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ );

	private:
			std::vector<int> _playerIds;
};

void Register::setPlayerId(int playerId)
{
	_playerIds.push_back(playerId);
}


void Register::done ( const char* /*URL*/, void * data, unsigned int size, bool complete )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());

	if ( size )
	{
		std::string _data; 

		_data.append((char*)data, size);

		if (is_valid_status(_data))	
		{
			bzAPIStringList* dataList = bz_newStringList();
			dataList->tokenize(_data.c_str(), "\r\n");

			if (dataList->get(0) == "OK") 
				for ( unsigned int i = 1; i < dataList->size(); i++)
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(i).c_str());

			if (dataList->get(0) == "NOK")
				for ( unsigned int i = 1; i < dataList->size(); i++)
					bz_sendTextMessagef ( BZ_SERVER, _playerId,"%s", dataList->get(i).c_str());

			bz_deleteStringList(dataList);
		}
		else
			bz_sendTextMessage (BZ_SERVER, _playerId,"no valid data was returned");
	}
}


void Register::error ( const char* /*URL*/, int errorCode, const char * errorString )
{

	int _playerId = _playerIds[0];
	_playerIds.erase(_playerIds.begin());
	 bz_sendTextMessagef(BZ_SERVER, _playerId,"Registration failed with errorcode = %d - %s  !!",errorCode, errorString);
}

class OneVsOne : public bz_EventHandler, public bz_CustomSlashCommandHandler
{
	public:
			virtual void process ( bz_EventData *eventData );
			virtual bool handle ( int playerID, bzApiString, bzApiString, bzAPIStringList*);
	protected:
			
	private:

			Register registerHandler;
			Report reportHandler;
			PlayerInfo playerInfoHandler;
			TopScore topScoreHandler;
			TopZelo topZeloHandler;

			void logRecordMatch(const char * label, int winner, int loser);
			void registerPlayer(int p, bzAPIStringList*);
			void getPlayerInfo(int p, bzAPIStringList*);
			void getTopScore(int p, bzAPIStringList*);
			void getTopZelo(int p, bzAPIStringList*);
			void showHelp(int p, bzApiString action = "all");
};


OneVsOne oneVsOne;

void OneVsOne::registerPlayer(int playerID, bzAPIStringList* params)
{

	if ( params->size() == 2 ) 
	{
		bz_PlayerRecord *playerRecord;

		playerRecord = bz_getPlayerByIndex ( playerID );

		// check if player is verified
		if ( playerRecord->verified)
		{

			std::string  registerdata;

			registerdata = std::string("action=register&callsign=") + std::string(bz_urlEncode(playerRecord->callsign.c_str())) +
									std::string("&bzid=") + std::string(bz_urlEncode(playerRecord->bzID.c_str())) +
									std::string("&email=") + std::string(bz_urlEncode(params->get(1).c_str())) +
									std::string("&ip=") + std::string(bz_urlEncode(playerRecord->ipAddress.c_str()));

			bz_debugMessagef( DEBUG_LEVEL, "DEBUG::registerPlayer = %s",registerdata.c_str());

			registerHandler.setPlayerId(playerID);
					
			bz_addURLJob(homeurl.c_str(),&registerHandler, registerdata.c_str());
		}
		else
			bz_sendTextMessagef ( BZ_SERVER, playerID,"You must be globally registered and identified to register here" );

		bz_freePlayerRecord ( playerRecord );
	}
	else
		showHelp(playerID, params->get(0));

}


void OneVsOne::getPlayerInfo(int playerID, bzAPIStringList* params) 
{

	if ( params->size() >= 2 ) 
	{
		bz_PlayerRecord *playerRecord;

		playerRecord = bz_getPlayerByIndex ( playerID );

		std::string  playerinfodata = "action=playerinfo";

		for (unsigned int i = 1; i < params->size(); i++)
			playerinfodata.append(std::string("&callsigns[]=") + std::string(bz_urlEncode(params->get(i).c_str())));

		bz_debugMessagef( DEBUG_LEVEL, "DEBUG::getPlayerInfo = %s",playerinfodata.c_str());


		playerInfoHandler.setPlayerId(playerID);
					
		bz_addURLJob(homeurl.c_str(),&playerInfoHandler, playerinfodata.c_str());

		bz_freePlayerRecord ( playerRecord );
	}
	else
		showHelp(playerID, params->get(0));
}

void OneVsOne::getTopScore(int playerID, bzAPIStringList* params) 
{

	if ( params->size() >  2 ) 
		showHelp(playerID, params->get(0));
	else
		{
			bz_PlayerRecord *playerRecord;
			std::string items = "0";

			playerRecord = bz_getPlayerByIndex ( playerID );

			if ( params->size() == 2  ) 
				items = bz_urlEncode(params->get(1).c_str());

			std::string  topscoredata = std::string("action=topscore&items=") + items;

			bz_debugMessagef( DEBUG_LEVEL, "DEBUG::getTopScore = %s",topscoredata.c_str());

			topScoreHandler.setPlayerId(playerID);
					
			bz_addURLJob(homeurl.c_str(),&topScoreHandler, topscoredata.c_str());

			bz_freePlayerRecord ( playerRecord );
		}

}


void OneVsOne::getTopZelo(int playerID, bzAPIStringList* params) 
{

	if ( params->size() >  2 ) 
		showHelp(playerID, params->get(0));
	else
		{
			bz_PlayerRecord *playerRecord;
			std::string items = "0";

			playerRecord = bz_getPlayerByIndex ( playerID );

			if ( params->size() == 2  ) 
				items = bz_urlEncode(params->get(1).c_str());

			std::string  topzelodata = std::string("action=topzelo&items=") + items;

			bz_debugMessagef( DEBUG_LEVEL, "DEBUG::getTopZelo = %s",topzelodata.c_str());

			topZeloHandler.setPlayerId(playerID);
					
			bz_addURLJob(homeurl.c_str(),&topZeloHandler, topzelodata.c_str());

			bz_freePlayerRecord ( playerRecord );
		}

}

void  OneVsOne::showHelp(int playerID, bzApiString action)
{
	action.tolower();
		
	if (action == "help")
		bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso help [<action>]" );
	else if (action == "register")
		bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso register <emailaddress>" );
	else if (action == "playerinfo")
		bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso playerinfo <callsign> [<callsign> ...]" );
	else if (action == "topscore")
		bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso topscore [<items>]" );
	else if (action == "topzelo")
		bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso topzelo [<items>]" );
	else 
		{
			bz_sendTextMessage( BZ_SERVER, playerID,"Usage: /ovso <action> <params>" );
			bz_sendTextMessage( BZ_SERVER, playerID,"" );
			bz_sendTextMessage( BZ_SERVER, playerID,"actions:" );
			bz_sendTextMessage( BZ_SERVER, playerID,"" );
			bz_sendTextMessage( BZ_SERVER, playerID," help [<action>] 1vs1 help" );
			bz_sendTextMessage( BZ_SERVER, playerID," register <valid emailaddress>  1vs1 league registration" );
			bz_sendTextMessage( BZ_SERVER, playerID," playerinfo <callsign> [<callsign> ...] show 1vs1 info of a player" );
			bz_sendTextMessage( BZ_SERVER, playerID," topscore [<items>]  show the monthly player score ranking" );
			bz_sendTextMessage( BZ_SERVER, playerID," topzelo [<items>]  show the player zelo ranking" );
			bz_sendTextMessage( BZ_SERVER, playerID,"" );
		}

}

void printScore ( void )
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START printScore", DEBUG_TAG );	

	std::map<int,OneVsOnePlayer>::iterator it = Players.begin();

	std::string timeMsg;
	std::string lifeMsg;
	
	int hits, lives = 0;

	for( ; it != Players.end(); it++ ) { 

		hits = (*it).second.losses;
		lives = LIVES - (*it).second.losses;

		if ( hits == 1 ) timeMsg = "time"; 
		else timeMsg = "times";

		if ( lives == 1 ) lifeMsg = "life"; 
		else lifeMsg = "lives";

		bz_debugMessagef ( DEBUG_LEVEL, "%s key %d => %s got hit %d %s, %d %s left", 
						DEBUG_TAG, (*it).first, (*it).second.callsign.c_str(), hits, timeMsg.c_str(), lives, lifeMsg.c_str());

		bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"%s got hit %d %s, %d %s left",
						(*it).second.callsign.c_str(), hits, timeMsg.c_str(), lives, lifeMsg.c_str());
	}

	bz_debugMessagef ( DEBUG_LEVEL,"%s END printScore",DEBUG_TAG);	
}

void printBanner ( int winner, int loser )
{

 	// this makes sure that if only 1 player is playing, that player
	// always gets the 'you lose' banner.

	if ( Players.size() == MAX_PLAYERS ) {

		bz_sendTextMessagef ( BZ_SERVER, winner,"                              .__ ");
		bz_sendTextMessagef ( BZ_SERVER, winner," ___.__. ____  __ __  __  _  _|__| ____ ");
		bz_sendTextMessagef ( BZ_SERVER, winner,"<   |  |/  _ \\|  |  \\ \\ \\/ \\/ /  |/    \\ ");
		bz_sendTextMessagef ( BZ_SERVER, winner," \\___  (  <_> )  |  /  \\     /|  |   |  \\ ");
		bz_sendTextMessagef ( BZ_SERVER, winner," / ____|\\____/|____/    \\/\\_/ |__|___|  / ");
		bz_sendTextMessagef ( BZ_SERVER, winner," \\/                                   \\/ ");
	}
	
	bz_sendTextMessagef ( BZ_SERVER, loser,"                      .__");
	bz_sendTextMessagef ( BZ_SERVER, loser," ___.__. ____  __ __  |  |   ____  ______ ____");
	bz_sendTextMessagef ( BZ_SERVER, loser,"<   |  |/  _ \\|  |  \\ |  |  /  _ \\/  ___// __ \\ ");
	bz_sendTextMessagef ( BZ_SERVER, loser," \\___  (  <_> )  |  / |  |_(  <_> )___ \\\\  ___/");
	bz_sendTextMessagef ( BZ_SERVER, loser," / ____|\\____/|____/  |____/\\____/____  >\\___  >");
	bz_sendTextMessagef ( BZ_SERVER, loser," \\/                                   \\/     \\/");

}

void addPlayer ( int playerId, const std::string callsign ) 
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START addPlayer :: %d , %s", DEBUG_TAG, playerId, callsign.c_str());	

	Players[playerId].callsign = callsign;
	Players[playerId].losses=0;
	Players[playerId].official=false;
	Players[playerId].contest=false;

	bz_debugMessagef ( DEBUG_LEVEL,"%s END addPlayer :: %d , %s", DEBUG_TAG, playerId, callsign.c_str());	
}

void delPlayer(int playerId) 
{ 
	bz_debugMessagef ( DEBUG_LEVEL,"%s START delPlayer :: %d", DEBUG_TAG, playerId );	

	Players.erase(playerId);

	bz_debugMessagef ( DEBUG_LEVEL,"%s END delPlayer :: %d", DEBUG_TAG, playerId );	
}

void setLoss( int playerId ) 
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START setLoss :: %d", DEBUG_TAG, playerId );	

	
	Players[playerId].losses++;

	bz_debugMessagef ( DEBUG_LEVEL,"%s END setLoss :: %d", DEBUG_TAG, playerId );	
}

int getHighestLoss() 
{

	int highestLoss=-1;

	std::map<int,OneVsOnePlayer>::iterator it = Players.begin(), stop = Players.end();
	for( ; it != stop; it++ ) { 
		if ( (*it).second.losses > highestLoss ) 
			highestLoss = (*it).second.losses;
	}

	return highestLoss;
}

int getWinner( int playerId )
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START getWinner :: %d", DEBUG_TAG, playerId );	

	int winner=-1;

	if ( Players[playerId].losses == LIVES ) {

			if ( Players.size() == 1 ) {
					winner=playerId;
			}
			else { 
					std::map<int,OneVsOnePlayer>::iterator it = Players.begin(), stop = Players.end();
					for( ; it != stop; it++ ) { 
						if ( (*it).second.losses != LIVES ) { 
							winner = (*it).first;
							break;
						}
					}
			}
	}

	bz_debugMessagef ( DEBUG_LEVEL,"%s VAR winner = %d", DEBUG_TAG, winner );	
	bz_debugMessagef ( DEBUG_LEVEL,"%s END getWinner :: %d", DEBUG_TAG, playerId );	

	return winner;
}

bool officialMatch( void ) 
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START officialMatch", DEBUG_TAG );	

	bool allPlayersOfficial = false;

	if ( Players.size() == MAX_PLAYERS ) {

		std::map<int,OneVsOnePlayer>::iterator it = Players.begin();
		for( ; it != Players.end(); it++ ) { 
			if ( (*it).second.official)  
					allPlayersOfficial = true;
			else {
					allPlayersOfficial = false;
					break;
			}
		}
	}

	bz_debugMessagef ( DEBUG_LEVEL,"%s VAR allPlayersOfficial = %d", DEBUG_TAG,allPlayersOfficial );	
	bz_debugMessagef ( DEBUG_LEVEL,"%s END officialMatch", DEBUG_TAG );	

	return allPlayersOfficial;
}

bool contestMatch() 
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START contestMatch", DEBUG_TAG );	

	bool allPlayersContest = false;

	if ( Players.size() == MAX_PLAYERS ) {

		std::map<int,OneVsOnePlayer>::iterator it = Players.begin();
		for( ; it != Players.end(); it++ ) { 
			if ( (*it).second.contest)  
					allPlayersContest = true;
			else {
					allPlayersContest = false;
					break;
			}
		}
	}

	bz_debugMessagef ( DEBUG_LEVEL,"%s VAR allPlayersContest = %d", DEBUG_TAG,allPlayersContest );	
	bz_debugMessagef ( DEBUG_LEVEL,"%s END contestMatch", DEBUG_TAG );	

	return allPlayersContest;
}

void saveScores(char * scores)
{
	bz_debugMessagef ( DEBUG_LEVEL,"%s START saveScores :: %s", DEBUG_TAG, scores );	

	if ( strcmp(LOGFILE,"none") ) {

		std::ofstream myfile;	
		myfile.open(LOGFILE, std::ios::out | std::ios::app | std::ios::binary);
		myfile << scores;
  		myfile.close();
	} else
		bz_debugMessagef ( 2,"%s SCORES :: %s", DEBUG_TAG, scores );	

	bz_debugMessagef ( DEBUG_LEVEL,"%s END saveScores :: %s", DEBUG_TAG, scores );	
}


template <class T>
std::string to_string (const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

void OneVsOne::logRecordMatch(const char * label, int winner, int loser)
{
	
	time_t t = time(NULL);
	tm * now = gmtime(&t);
			
	char scores[100];
	char scores_enc[200];
	char match_date[20];

	std::string reportData;

	// save recording
	if ( recording ) {		
		char filename[512];
		snprintf( filename,512,"%s[%d]_%s[%d]_%02d%02d%02d_%02d%02d%02d.bzr",
		Players[winner].callsign.c_str(),Players[loser].losses,Players[loser].callsign.c_str(),Players[winner].losses,
		now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
		bz_saveRecBuf( filename );
		bz_stopRecBuf();
		bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"Match has been recorded as %s ", filename );
	} 

	sprintf(match_date, "%02d-%02d-%02d %02d:%02d:%02d",
			now->tm_year+1900,
			now->tm_mon+1,
			now->tm_mday,
			now->tm_hour,
			now->tm_min,
			now->tm_sec);

	// get bzid's ... dirty
	
	bz_PlayerRecord *playerRecord;

	playerRecord = bz_getPlayerByIndex ( winner );

	bzApiString wbzid = playerRecord->bzID;

	bz_freePlayerRecord ( playerRecord );

	playerRecord = bz_getPlayerByIndex ( loser );

	bzApiString lbzid = playerRecord->bzID;

	bz_freePlayerRecord ( playerRecord );

	//format scores 
	sprintf( scores,"%s\t%s\t%s\t-\t%s\t%d\t-\t%d\t%s\t-\t%s\n",
						label,match_date,Players[winner].callsign.c_str(),
						Players[loser].callsign.c_str(),Players[loser].losses,Players[winner].losses,
						wbzid.c_str(),lbzid.c_str());

	reportData = std::string ("action=report") + std::string("&type=") + std::string(bz_urlEncode(label)) + std::string("&date=") + 
					std::string(bz_urlEncode(match_date)) + std::string("&winner=") + 
					std::string(bz_urlEncode(Players[winner].callsign.c_str())) + 
					std::string("&loser=") + std::string(bz_urlEncode(Players[loser].callsign.c_str())) + 
					std::string("&winner_score=") + to_string(Players[loser].losses) + 
					std::string("&loser_score=") + to_string(Players[winner].losses) +
					std::string("&wbzid=") + std::string(wbzid.c_str()) +
					std::string("&lbzid=") + std::string(lbzid.c_str());

	saveScores( scores );

	bz_debugMessagef ( DEBUG_LEVEL,"%s VAR :: reportData = %s", DEBUG_TAG, reportData.c_str());	

	if ( strcasecmp ( label,"official" ) == 0 ) 
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS,"The game has been logged as an official match");

	if ( strcasecmp ( label,"contest" ) == 0 ) 
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS,"The game has been logged as a contest match");

	// do reporting over http

	reportHandler.setPlayerId(BZ_ALLUSERS);
	bz_addURLJob(homeurl.c_str(), &reportHandler, reportData.c_str());

}


void OneVsOne::process ( bz_EventData *eventData )
{

	if (eventData->eventType == bz_ePlayerJoinEvent) {

		bz_debugMessagef ( DEBUG_LEVEL,"%s START EVENT :: bz_ePlayerJoinEvent", DEBUG_TAG );	

		bz_PlayerJoinPartEventData *joinData = (bz_PlayerJoinPartEventData*)eventData;

		bz_debugMessagef ( DEBUG_LEVEL,"%s VAR :: playerID = %d , callsign = %s", 
						DEBUG_TAG, joinData->playerID, joinData->callsign.c_str() );	
			
		// revoking superkill perms makes sure admin/cops don't abuse it
		// when a match is in progress ;)
		if ( bz_hasPerm ( joinData->playerID, "superkill" )) {
			bz_revokePerm ( joinData->playerID, "superkill" );

			bz_debugMessagef ( DEBUG_LEVEL,"%s INFO :: revokePerm superkill, playerID = %d, callsign = %s", 
						DEBUG_TAG, joinData->playerID, joinData->callsign.c_str() );	
		}

		if (joinData->team != eObservers) {
			addPlayer(joinData->playerID,joinData->callsign.c_str());
		}

		bz_debugMessagef ( DEBUG_LEVEL,"%s END EVENT :: bz_ePlayerJoinEvent", DEBUG_TAG );	
	}

	if (eventData->eventType == bz_ePlayerPartEvent) {

		bz_debugMessagef ( DEBUG_LEVEL,"%s START EVENT :: bz_ePlayerPartEvent", DEBUG_TAG );	

		bz_PlayerJoinPartEventData *partData = (bz_PlayerJoinPartEventData*)eventData;
			
		bz_debugMessagef ( DEBUG_LEVEL,"%s VAR :: playerID = %d , callsign = %s", 
						DEBUG_TAG, partData->playerID, partData->callsign.c_str() );	

		if ( partData->team != eObservers ) {
			delPlayer ( partData->playerID );

			if ( recording )
				bz_stopRecBuf ();
		}

		bz_debugMessagef ( DEBUG_LEVEL,"%s END EVENT :: bz_ePlayerPartEvent", DEBUG_TAG );	
	}

	if (eventData->eventType == bz_ePlayerDieEvent) {

		bz_debugMessagef ( DEBUG_LEVEL,"%s START EVENT :: bz_ePlayerDieEvent", DEBUG_TAG );	

		if ( !Players.empty () ) {

			bz_PlayerDieEventData *dieData = (bz_PlayerDieEventData*)eventData;	

			setLoss(dieData->playerID);

			printScore();

			if (getWinner(dieData->playerID) != -1) {

				int loser=dieData->playerID;
				int winner=getWinner(dieData->playerID);

				bz_gameOver (winner, eNoTeam);

				printBanner(winner, loser);

				if (officialMatch()) 
					logRecordMatch("official",winner,loser);

				if (contestMatch()) 
					logRecordMatch("contest",winner,loser);


				// remove all players from the list
				delPlayer ( loser );
				delPlayer ( winner );
			}
		}

		bz_debugMessagef ( DEBUG_LEVEL,"%s END EVENT :: bz_ePlayerDieEvent", DEBUG_TAG );	
	}

	if (eventData->eventType == bz_eSlashCommandEvent) {

		bz_debugMessagef ( DEBUG_LEVEL,"%s START EVENT :: bz_eSlashCommandEvent", DEBUG_TAG );	

		bz_SlashCommandEventData *slashCommandData = (bz_SlashCommandEventData*)eventData;	

		bz_debugMessagef ( DEBUG_LEVEL,"%s VAR :: from = %d , message = %s", 
						DEBUG_TAG, slashCommandData->from, slashCommandData->message.c_str() );	

		if (strcasecmp(slashCommandData->message.c_str(),"/superkill") == 0) {
			if ( officialMatch () || contestMatch () )  {
				bz_sendTextMessagef ( BZ_SERVER,slashCommandData->from,
								"You can't perform /superkill during an official or contest match" );
			}
			else bz_superkill();
		}

		bz_debugMessagef ( DEBUG_LEVEL,"%s END EVENT :: bz_eSlashCommandEvent", DEBUG_TAG );	
	}

}

bool OneVsOne::handle ( int playerID, bzApiString cmd, bzApiString msg, bzAPIStringList* cmdParams )
{

	// transfrom to lowercase
	cmd.tolower();
	msg.tolower();

	if ( strcasecmp ( cmd.c_str(), "official") == 0 ) {

		bz_PlayerRecord *playerRecord;
	  	playerRecord = bz_getPlayerByIndex( playerID );

		if ( playerRecord->team == eObservers ) {
			bz_sendTextMessagef ( BZ_SERVER, playerID,
							"Observers obviously can't play official matches ;)" );
			return true;
		}

		
		if ( ! playerRecord->globalUser ) {
			bz_sendTextMessagef ( BZ_SERVER, playerID,"You must be registered and identified to play official matches" );
			return true;
		}

		if ( Players[playerID].losses != 0 ) { 
			bz_sendTextMessagef ( BZ_SERVER, playerID,"You have to declare the match official before you start playing" );
			return true;
		}

		if ( ! contestMatch () ) {
			Players[playerID].official = true;	
			Players[playerID].contest = false;	
		}				
		else {
				bz_sendTextMessagef ( BZ_SERVER, playerID,"There is already a contest match in progress ..." );
				return true;
		}

		bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"%s declared to play an official match",playerRecord->callsign.c_str() );

		if ( officialMatch () ) {
			bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"All current players asked to play official." );
			if ( recording )
				bz_stopRecBuf ();

			recording = bz_startRecBuf ();
		}

		bz_freePlayerRecord ( playerRecord );

		return true;
	}

	if ( strcasecmp ( cmd.c_str(), "contest" ) == 0) {

		bz_PlayerRecord *playerRecord;
	  	playerRecord = bz_getPlayerByIndex ( playerID );

		if (playerRecord->team == eObservers) {
			bz_sendTextMessagef ( BZ_SERVER, playerID,
							"Observers obviously can't play contest matches ;)" );
			return true;
		}

		
		if ( ! playerRecord->globalUser ) {
			bz_sendTextMessagef ( BZ_SERVER, playerID,"You must be registered and identified to play contest matches" );
			return true;
		}

		if ( Players[playerID].losses != 0 ) { 
			bz_sendTextMessagef ( BZ_SERVER, playerID,"You have to declare the match a contest before you start playing");
			return true;
		}

		if ( ! officialMatch () ) {
			Players[playerID].contest = true;	
			Players[playerID].official = false;	
		}
		else {
				bz_sendTextMessagef ( BZ_SERVER, playerID,"There is already an official match in progress ..." );
				return true;
		}

		bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"%s declared to play a contest match",playerRecord->callsign.c_str() );

		if ( contestMatch () ) {
			bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"All current players asked to play a contest match." );
			if ( recording )
				bz_stopRecBuf ();

			recording = bz_startRecBuf ();
		}

		bz_freePlayerRecord ( playerRecord );

		return true;
	}

	if ( strcasecmp ( cmd.c_str(), "setlives" ) == 0) {

		if (! bz_hasPerm(playerID,"SETLIVES")) {
        	bz_sendTextMessagef (BZ_SERVER, playerID, "You do not have permission to run the setlives command");
        	return true;
 	 	}

		if ( cmdParams->size() == 1 ) {
			
			int lives = 0;
			std::istringstream iss(cmdParams->get(0).c_str());

			if (( iss >> lives)) {

				bz_PlayerRecord *playerRecord;

	  			playerRecord = bz_getPlayerByIndex ( playerID );

				if ( getHighestLoss() < lives && lives > 0 ) {
					LIVES=lives;
					bz_sendTextMessagef ( BZ_SERVER, BZ_ALLUSERS,"Life count set to %d by %s.", LIVES, playerRecord->callsign.c_str() );
				}
				else
					bz_sendTextMessagef ( BZ_SERVER, playerID,"Life count should be higher then highest player hit count.");

				bz_freePlayerRecord ( playerRecord );

				return true;
			}

		}

		bz_sendTextMessagef ( BZ_SERVER, playerID,"Usage: /setlives <life count>" );

				return true;
	}

	if ( cmd == "ovso") {

		if ( cmdParams->size() >= 1 ) {

			bzApiString action = cmdParams->get(0);
			action.tolower();

			if ( action == "help") {
				if ( cmdParams->size() == 2 )
					showHelp(playerID,cmdParams->get(1));
				else showHelp(playerID);

				return true;
			}
			
			if ( action == "register") {
				registerPlayer(playerID, cmdParams);
				return true;
			}

			if ( action == "playerinfo" ) {
				bzAPIStringList *p = bz_newStringList();
				p->tokenize(msg.c_str(), " ", 0, true);
				getPlayerInfo(playerID, p);
				bz_deleteStringList(p);
				return true;
			}

			if ( action == "topscore") {
				getTopScore(playerID, cmdParams);
				return true;
			}

			if ( action == "topzelo") {
				getTopZelo(playerID, cmdParams);
				return true;
			}

		}

		showHelp(playerID);
	}

	return true;
}

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bzAPIStringList* cmdLine = bz_newStringList();

	cmdLine->tokenize(commandLine, ",", 2, false);

	if (cmdLine->size() == 2) {

		LIVES=atoi(cmdLine->get(0).c_str());
		strcpy(LOGFILE,cmdLine->get(1).c_str());
	}
	else if (cmdLine->size() == 1) 
			LIVES=atoi(cmdLine->get(0).c_str());

	bz_registerCustomSlashCommand ("official", &oneVsOne);
	bz_registerCustomSlashCommand ("contest", &oneVsOne);
	bz_registerCustomSlashCommand ("setlives", &oneVsOne);
	bz_registerCustomSlashCommand ("ovso", &oneVsOne);

	bz_registerEvent(bz_ePlayerJoinEvent, &oneVsOne);
	bz_registerEvent(bz_ePlayerPartEvent, &oneVsOne);
	bz_registerEvent(bz_ePlayerDieEvent, &oneVsOne);
	bz_registerEvent(bz_eSlashCommandEvent,&oneVsOne); 

	bz_debugMessage( DEBUG_LEVEL, "OneVsOne plugin loaded" );

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand ("official");
	bz_removeCustomSlashCommand ("contest");
	bz_removeCustomSlashCommand ("setlives");
	bz_removeCustomSlashCommand ("ovso");

	bz_removeEvent(bz_ePlayerJoinEvent, &oneVsOne);
	bz_removeEvent(bz_ePlayerPartEvent, &oneVsOne);
	bz_removeEvent (bz_ePlayerDieEvent, &oneVsOne);
	bz_removeEvent(bz_eSlashCommandEvent, &oneVsOne); 

	bz_debugMessage( DEBUG_LEVEL, "OneVsOne plugin unloaded" );

	return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
