#include <OpenTptInternet.h>
#include <Dialogs.h>
#include <OpenTransport.h>
#include <OpenTptLinks.h>
#include <OpenTptConfig.h>
#include <NameRegistry.h>
#include <QuickTimeComponents.h>

#include "Toolbox.h"
#include "Globals.h"
#include "Common.h"
#include "Network.h"
#include "Players.h"
#include "Graphics.h"
#include "Creatures.h"
#include "FileIO.h"
#include "Resource.h"
#include "Skills.h"
#include "Items.h"
#include "Npc.h"
#include "Shops.h"
#include "Splash.h"
#include "FX.h"
#include "Async Sound.h"
// #include "Floater.h"
#include "Quest.h"
#include "GameText.h"
// #include "implode.h"
#include "Shell.h"
#include "Updater.h"
#include "GuildHalls.h"
#include <stdio.h>

#define							kNextAction						        5		//30

#ifdef _SERVER
#define							kMaxHandlerMessageCount				5000
#define							kMaxHandlerMessageSize				1000
#endif

#ifndef _SERVER
#define							kMaxHandlerMessageCount				200
#define							kMaxHandlerMessageSize				5000
#endif

static Str31				kJoinDialogLabel = "\pChoose a Game:";

//extern NMBoolean  gInNetSprocketMode;

#ifdef _SERVER
int																monsterList[kMaxScreenElements];
int																containerList[kMaxScreenElements];
int																tracksList[kMaxScreenElements];
int																bloodStainList[kMaxScreenElements];
int																fxList[kMaxScreenElements];
UInt8															playerData									[kMaxPlayers][kDataSize];
UInt8															outData											[kDataSize];
int																numberOfPlayersOnline;
int																maxPlayersOnline;
long															numberOfLogoffs;
long															serverFull;
int																maxMessageSize;
long															gracefulLogoffs;
long															failedLogons;
long															numberOfLogons;
int																refreshFrequency;
UInt32														waitingForAuthorization			[kMaxPlayers];
unsigned long											authorizationTimeOut				[kMaxPlayers];
UInt16														fxTrailRow									[100];
UInt16														fxTrailCol									[100];
int																messagesReceived;
int																messagesSent;
long															bytesSent;
long															bytesReceived;
long															exceededNT;
long															skillGain;
int																nbRefresh;
#endif

Str255															gContextString = "\pOberin Context";
UInt8                               myQueue[kMaxHandlerMessageCount][kMaxHandlerMessageSize];
int                                 queueWritePtr;
int                                 queueReadPtr;
int                                 newestRefresh;
TBoolean														hostRunning;
unsigned long						nextHelpTime;

unsigned long						currentlyTracking;
UInt8								monstersTracked;
int									trackingRow;
int									trackingCol;
UInt8								localTracking[kMaxTracked][3];

static Str31												gameName;
static Str31												password;
Str31																playerName;

char                                playerNames[kMaxPlayers][kStringLength];//[15];
UInt32                              playerIDs[kMaxPlayers];
UInt32                              myID;
UInt32                              chatMessageID;
UInt32                              soundMessageID;

UInt32															lastRefreshMessageTime;
UInt32															lastRefreshTick;

char																myPassword[kStringLength];//[15];
char                                localContainerName[kStringLength];//[15];

int																	inventoryCheckSumFailures;
UInt8																localChess[96];

int																	waitingFor[10];
unsigned long												reSendAt[10];

long																lastJoin; // beta variable
int																	maxDataSize=0; // beta variable
int																	lastMessageSize=0;
UInt32															previousMessageID[10];

extern TBoolean				gUpdating;
extern unsigned long 		currentDate;
extern unsigned long 		lastCalendarCheck;
extern UInt8				localEffectsDestination	[kMaxLocalFX];
extern WindowRef			inventoryWindow;
extern WindowRef			bankWindow;
extern int					localSpat;

SkillsRefreshMessage								gSkillsRefreshMessage;
SkillChangeMessage									gSkillChangeMessage;
PlayerTargetResourceMessage					gPlayerTargetResourceMessage;
MakeMessage													gMakeMessage;
SpatMessage										gSpatMessage;
InventoryRefreshMessage							gInventoryRefreshMessage;
InventoryAddMessage									gInventoryAddMessage;
InventoryDropMessage								gInventoryDropMessage;
GoldAddMessage											gGoldAddMessage;
InventoryDeleteMessage							gInventoryDeleteMessage;
ArmMessage													gArmMessage;
ArmRefreshMessage										gArmRefreshMessage;
ItemTransferMessage									gItemTransferMessage;
GiveGoldMessage											gGiveGoldMessage;
GoldRemoveMessage										gGoldRemoveMessage;
ContainerContentsMessage						gContainerContentsMessage;
TakeFromListMessage									gTakeFromListMessage;
ShopMessage													gShopMessage;
BuyFromListMessage									gBuyFromListMessage;
IDMessage														gIDMessage;
NightVisionMessage									gNightVisionMessage;
DisplayMessage											gDisplayMessage;
AlertMessage												gAlertMessage;
ChessRefreshMessage									gChessRefreshMessage;
ChessMoveMessage									  gChessMoveMessage;
PathMessage                         gPathMessage;
RequestPlayerNameMessage            gRequestPlayerNameMessage;
HallRequestMessage					gHallRequestMessage;
CalendarRequestMessage				gCalendarRequestMessage;
CalendarInfosMessage				gCalendarInfosToClientMessage;
UDPMessage                          gUDPMessage;
GotUDPMessage                       gUDPOKMessage;
GMMessage                           gGMMessage;
PlayerClearMessage                  gPlayerClearMessage;
NSpPlayerLeftMessage                gNSpPlayerLeftMessage;
RemoveFromPartyMessage              gRemoveFromPartyMessage;
RequestQuestDescriptionMessage      gRequestQuestDescriptionMessage;
RepairMessage                       gRepairMessage;
ItemRepairedMessage                 gItemRepairedMessage;
PetNameRefreshMessage               gPetNameRefreshMessage;
MacMessage                          gMacMessage;
ClientCommandMessage				gClientCommandMessage;
SortMessage							gSortMessage;
SpeedMessage						gSpeedMessage;
ClearSpatMessage					gPlayerClearSpatMessage;

PlayerRefreshMessage2								gPlayerRefreshMessage2;
TextToServerMessage2							  gTextToServerMessage2;
BugTextToServerMessage2				      gBugTextToServerMessage2;
TextToClientMessage2								gTextToClientMessage2;
VersionToClientMessage							gVersionToClientMessage;
PetNameMessage2									    gPetNameMessage2;
QuestDescriptionMessage2						gQuestDescriptionMessage2;
LocateMessage2									    gLocateMessage2;
PlayerNameMessage2				          gPlayerNameMessage2;
BroadcastTextToServerMessage2				gBroadcastTextToServerMessage2;
SystemTextToClientMessage2					gSystemTextToClientMessage2;
MeetToServerMessage2								gMeetToServerMessage2;
AddToPartyMessage2									gAddToPartyMessage2;
GuildToServerMessage2								gGuildToServerMessage2;
GiftToServerMessage2								gGiftToServerMessage2;
ExpToServerMessage2								  gExpToServerMessage2;
AnnounceToServerMessage2						gAnnounceToServerMessage2;
RExpToServerMessage2							gRExpToServerMessage2;
WhisperToServerMessage2							gWhisperToServerMessage2;
SummonToServerMessage2							gSummonToServerMessage2;
SignToServerMessage2								gSignToServerMessage2;
MsayToServerMessage2								gMsayToServerMessage2;
NoteToServerMessage2								gNoteToServerMessage2;
HallToServerMessage2								gHallToServerMessage2;
HallToClientMessage								gHallToClientMessage;
LogonTextToServerMessage2				    gLogonTextToServerMessage2;
MsgToServerMessage2								  gMsgToServerMessage2;
CompressedTextToClientMessage2			gCompressedTextToClientMessage2;
HallGuildToServerMessage2				gHallGuildToServerMessage2;
NameChangeToServerMessage2						gNameChangeToServerMessage2;
CustomTitleToServerMessage2				gCustomTitleToServerMessage2;
TrackingInfoToClientMessage2			gTrackingInfosToClientMessage2;

UInt8																inData											[kDataSize];
UInt8																testData										[kDataSize];
UInt16															inRow;
UInt16															inCol;
TBoolean														gInCallback;
static PlayerActionMessage					gPlayerActionMessage;
static PlayerTargetMessage					gPlayerTargetMessage;

UInt8                               original[5000];
UInt8                               packed[5000];
UInt8                               unpacked[5000];
char                                work[5000];
long                                gReadBytesLeft = 0;
long                                gWriteBytes;
//Ptr 																WorkBuff;           //buffer for compress/decompress
char																*globalRead;
char																*globalWrite;
int																	previousLocationValue;
TBoolean                            showLogonMessage;
char                                logonMessage1[kStringLength];
char                                logonMessage2[kStringLength];
char                                logonMessage3[kStringLength];

static AuthorizeMessage							gPlayerAuthorizeMessage;

Boolean 														gHost = false;
NSpGameReference										gNetGame = nil;

unsigned long												nextRefresh;
unsigned long												nextTextRefresh;
unsigned long												nextAction;
unsigned long												nextStep;
unsigned long												nextRender;
TBoolean														canStep;

TBoolean														gotFirstRefresh;
int																	targetFrame;
TBoolean														animateStep=false;
TBoolean														playerDead;
TBoolean														inTown;
TBoolean                            gotInventoryRefresh;
TBoolean                            gotSkillsRefresh;
TBoolean                            gotArmsRefresh;

UInt8								localSpeed;

int																	dataReSendError; // beta

int																	gameTime;
int                                 magicLoreTarget;
char                                macID[kStringLength];

char									              startDateAndTime[kStringLength]; //[128];
TBoolean                            gotTermination;
TBoolean                            reconnected;
TBoolean                            attemptingReconnect;
TBoolean                            thisIsReconnect;

UInt32								ToRemove[5000];

extern TBoolean											logText;

extern inventoryType								bankInventory								[kMaxInventorySize];
extern UInt8				                chessBoard									[kMaxChessBoards][96];
extern UInt16												state												[kMaxScreenElements];
extern TBoolean											morphed											[kMaxScreenElements];
extern TBoolean											poisoned										[kMaxScreenElements];
extern TBoolean											cursed											[kMaxScreenElements];
extern TBoolean											afk										[kMaxScreenElements];
extern TBoolean											questChar										[kMaxScreenElements];
extern TBoolean                     playerOnline                [kMaxScreenElements];
extern TBoolean											stealthChar									[kMaxScreenElements];
extern TBoolean											regenerating									[kMaxScreenElements];
extern TBoolean											gmChar										  [kMaxScreenElements];
extern TBoolean											isPet										    [kMaxScreenElements];
extern UInt16												containerState							[kMaxScreenElements];
extern UInt16												tracksState							[kMaxScreenElements];
extern UInt16												bloodStainState							[kMaxScreenElements];
extern UInt8												human												[kMaxScreenElements][19];
extern UInt16												creatureMap									[kMapWidth][kMapHeight];
extern UInt16												containerMap								[kMapWidth][kMapHeight];
extern UInt16												tracksMap								[kMapWidth][kMapHeight];
extern UInt8								tracksID					      [kMaxTracks];
extern inventoryType								container										[kMaxContainers][kMaxInventorySize];
extern char								          containerName               [kMaxContainers][48];//[15];
extern char								          tracksNote               [kMaxContainers][100];
extern char								          containerGift               [kMaxContainers][48];//[15];
extern char								          tracksGift               [kMaxContainers][48];
extern unsigned long				        containerDecayTime	        [kMaxContainers];
extern unsigned long				        tracksDecayTime	        [kMaxTracks];
extern Boolean				              containerGM	                [kMaxContainers];
extern Boolean											Done;

extern creatureType									creature										[kMaxCreatures];
extern UInt8												creatureDirection						[kMaxScreenElements];
extern UInt8												creatureFacing							[kMaxScreenElements];
extern UInt8												creatureHealth							[kMaxScreenElements];
extern UInt8												creatureMp								[kMaxScreenElements];
extern UInt8												creatureAttacking						[kMaxScreenElements];
extern int  												creatureID						      [kMaxScreenElements];
extern creatureControls							creatureInfo								[kMaxCreatureTypes];
extern TBoolean											canWalkOn										[kMaxIcons];
extern TBoolean											canFlyOver									[kMaxIcons];
extern UInt16												map													[kMapWidth][kMapHeight];
extern UInt16												playerRow;
extern UInt16												playerCol;
extern UInt16												mapToScreenRow							[kMaxScreenElements];
extern UInt16												mapToScreenCol							[kMaxScreenElements];
extern int													frame;
extern int													xOffset;
extern int													yOffset;
extern int													currentInfoDisplay;
extern char													message[kStringLength];//[128];
extern char													itemName[kStringLength];//[48];
extern playerLocalSkillsType				localSkills									[kMaxSkills];
extern int													targetMode;
extern TBoolean											inventorySlotArmed					[kMaxInventorySize];
extern TBoolean                     gmCanSpawn                  [kMaxContainerTypes];
extern TBoolean                     gmProp                  		[kMaxContainerTypes];
extern char													spIP[kStringLength];
extern TBoolean											sign[kMaxIcons];

extern UInt8												localStrength;
extern UInt8												localIntelligence;
extern UInt8												localDexterity;
extern UInt8												localArmorRating;
extern UInt8												localHitPoints;
extern UInt8												localMagicPoints;
extern UInt8												localStamina;
extern UInt16												localWeight;
extern UInt8												localLevel;
extern UInt8												localClass;
extern UInt16												localHunger;
extern itemType											itemList										[kMaxItems];
extern char													chatText										[kMaxChatSlots][kStringLength];//[64];
extern unsigned long								chatDecay										[kMaxChatSlots];
extern UInt16												chatCreatureID							[kMaxChatSlots];
//extern UInt16												chatReverseLookup						[kMaxChatSlots];
extern npcType											npc													[kMaxNPCs];
extern TBoolean                     updateMapView;

extern int                 localPetStatus          [kMaxPets];
extern int                 localPetType            [kMaxPets];
extern char                localPetName            [kMaxPets][kStringLength];
extern int                 localPetLoyalty         [kMaxPets];

extern UInt8				localRightHand;
extern UInt8				localLeftHand;
extern UInt8				localLegs;
extern UInt8				localTorso;
extern UInt8				localFeet;
extern UInt8				localArms;
extern UInt8				localRightFinger;
extern UInt8				localLeftFinger;
extern UInt8				localHead;
extern UInt8				localHands;
extern UInt8				localNeck;
extern UInt8				localWaist;

extern int                 chessBoardRow1[kMaxChessBoards];
extern int                 chessBoardCol1[kMaxChessBoards];
extern int                 chessBoardRow2[kMaxChessBoards];
extern int                 chessBoardCol2[kMaxChessBoards];

extern int                          ginsengSpawn;
extern int                          lotusFlowerSpawn;
extern int                          mandrakeRootSpawn;
extern int                          garlicSpawn;
extern int                          volcanicAshSpawn;
extern int                          mushroomSpawn;//Dave
//extern int                          eggSpawn;//Dave

extern int                 friend1;
extern int                 friend2;
extern int                 friend3;
extern int                 friend4;

extern int					targetSlot;	// used for dropping/giving multiple items
extern int					targetCount;	// used for dropping/giving multiple items
extern int					playerAction;	// used for dropping multiple items

extern Movie				theMusic;
extern TBoolean			musicOn;
extern int          currentQuestSlot;
extern int          tbQuestWindow;

extern TBoolean							standingInShop;
extern int									flip; // beta
extern int									refreshTime; // beta
extern int                  stepFrame;
extern int                  stepDirection;
extern unsigned long        nextRenderStep1;
extern unsigned long        nextRenderStep2;
extern unsigned long        nextRenderStep3;
extern unsigned long        nextRenderStep4;
//extern TRect                currentMainWindowsRect;

extern WindowPtr						idWindow;
extern TRect                idWindowRect;
extern int                  tbIDWindow;

extern TBoolean             windowVisible[kMaxWindows];
extern WindowPtr            logonWindow;
extern int                  tbLogOnWindow;
extern int									localTarget;
extern TBoolean             friendOnline[16];
extern TBoolean             friendSharing[16];
extern char					        localParty[16][kStringLength]; //[15];
extern TBoolean             skipOldMessages;

extern char                 questDescription[1024];
extern int                  questGold;
extern int                  questExperience;
extern int                  questItem;
extern int                  questCount;
extern int                  tbMakeWindow;
extern questType						quest[kMaxQuests];
extern TBoolean						  chimeOn;
extern int									serverRestarts;
extern FSSpec				AppFSSpec;

extern int          tbMapWindow;
extern int          tbInventoryWindow;
extern int			tbCalendarWindow;
extern int          tbPackContentsWindow;
extern int          tbShopContentsWindow;
extern int          tbSellListWindow;
extern int          tbIDWindow;
extern int          tbMakeWindow;
extern int          tbSkillsWindow;
extern int          tbStatsWindow;
extern int          tbChatWindow;
extern int          tbCharacterWindow;
extern int          tbChessWindow;
extern int          tbLogOnWindow;
extern int          tbBankWindow;
extern int          tbPartyWindow;
extern int          tbQuestWindow;
extern int          tbPetsWindow;
extern int          tbAboutWindow;
extern char					ban											[1000][18];

//extern UInt8              chatLookUpTable   [kMaxCreatures];
extern char								chatString        [kMaxChatSlots][kStringLength];	// saved
extern UInt16							chatCreatureID    [kMaxChatSlots];	// saved
extern char								buildString[128];
extern int								guildHallsPtr;
extern int								guildMap[kMapWidth][kMapHeight];
extern guildHallType					guildHalls[kMaxGuildHalls];
extern UInt8							localHallsIDs[kMaxGuildHalls];
extern int								maxSkillLevel[7][kMaxSkills];
extern TRect							skillsWindowRect;
extern WindowPtr						skillsWindow;
extern TPoint       					windowPosition[kMaxWindows];
extern TBoolean							thisIsFirstRefresh;

//char						pcRoll[kStringLength];
//int                 			rollChance; // Dave

struct Address8022
{
	OTAddressType	fAddrFamily;
	UInt8			fHWAddr[k48BitAddrLength];
	UInt16			fSAP;
	UInt8			fSNAP[k8022SNAPLength];
};

typedef struct Address8022 Address8022;


// ----------------------------------------------------------------- //
void nw_RestartServer(void)
// ----------------------------------------------------------------- //

{
	int									i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_RestartServer");
#endif

	fi_WriteToErrorLog("Attempting to restart server.");

	nw_ShutdownNetworking();

	for (i=1;i<kMaxPlayers;i++)
		if (player[i].active)
		{
			fi_Logout(i);

			if (gNetGame)
				nw_RemovePlayer(player[i].id);

			pl_ClearPlayerRecord(i);

#ifdef _SERVERONLY
			gr_DrawServerNameAt(i);

#endif
		}

	queueWritePtr =	0;

	queueReadPtr 	=	0;

	if (nw_DoHost()!=noErr)
	{
		fi_WriteToErrorLog("Unable to restart server.");
		tb_AlertMessage("Unable to restart server.");
		fi_CloseGMLog();
		tb_Halt();
	}

	fi_WriteToErrorLog("Server restart successful.");

#ifdef _SERVERONLY
	serverRestarts++;
#endif

}

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
TBoolean nw_IsGameOkay(void)
// ----------------------------------------------------------------- //

{
	EventRecord						
	;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_IsGameOkay");
#endif

	if (attemptingReconnect)
	{
		fi_WriteToErrorLog("nw_IsGameOkay - attemptingReconnect");
		return (false);
	}

	if ((gNetGame==NULL) && (onLine==true) && (attemptingReconnect==false))
	{
		gr_AddText("Connection lost.  Attempting to reconnect...",true,true);
		fi_WriteToErrorLog("Dropped ... attempting reconnect.");
		nw_ShutdownNetworking();
		onLine=false;
		reconnected=false;
		onLine=true;
		attemptingReconnect=true;

		if (nw_DoJoin()!=0)
			sh_EndSession();

		return (false);
	}

	//fi_WriteToErrorLog("nw_IsGameOkay - exiting true");
	return (true);

}

#endif

// ----------------------------------------------------------------- //
void nw_GetStartDate(void)
// ----------------------------------------------------------------- //

{
	DateTimeRec						Date;
	unsigned long					Secs;
	char									temp[kStringLength]; //[50];
	long                  t;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetStartDate");
#endif

	strcpy(startDateAndTime,"Start Up:   ");

	t = ReadDateTime(&Secs);
	SecondsToDate(Secs, &Date);

	tb_IntegerToString(Date.month,temp);
	strcat(startDateAndTime,temp);

	strcat(startDateAndTime,"/");

	tb_IntegerToString(Date.day,temp);
	strcat(startDateAndTime,temp);

	strcat(startDateAndTime,"/");

	tb_IntegerToString(Date.year,temp);
	strcat(startDateAndTime,temp);

	strcat(startDateAndTime," at ");

	tb_IntegerToString(Date.hour,temp);
	strcat(startDateAndTime,temp);

	strcat(startDateAndTime,":");

	tb_IntegerToString(Date.minute,temp);
	strcat(startDateAndTime,temp);

	strcat(startDateAndTime,":");

	tb_IntegerToString(Date.second,temp);
	strcat(startDateAndTime,temp);

}

// ----------------------------------------------------------------- //
static pascal Boolean MessageHandler(NSpGameReference /*inGameRef*/, NSpMessageHeader *inMessage, void */*inContext*/)
// ----------------------------------------------------------------- //

{
	//char                  errorMessage[kStringLength];//[64];
	//char                  errorNumber[kStringLength];//[32];
	//char									*myIP;

// if (inGameRef!=gNetGame)
//   {
//     fi_WriteToErrorLog("Game reference mismatch");
//     return (false);
//   }

	//#ifdef _FUNCTIONTRACEDEBUGGING
	//fi_WriteToErrorLog("MessageHandler");
	//#endif

	if (inMessage==NULL)
	{
		// fi_WriteToErrorLog("Null Message");
		tb_Beep(0);
		return (false);
	}

	if (inMessage->messageLen>=kMaxHandlerMessageSize)
	{
		tb_Beep(0);
		tb_Beep(0);
		// strcpy(errorMessage,"Message too big: ");
		// tb_IntegerToString(inMessage->messageLen,errorNumber);
		// strcat(errorMessage,errorNumber);
		// fi_WriteToErrorLog(errorMessage);
		return (false);
	}

	if (inMessage->messageLen<=0)
	{
		tb_Beep(0);
		tb_Beep(0);
		tb_Beep(0);
		//fi_WriteToErrorLog("Zero Message Length");
		return (false);
	}

	gInCallback=true;

	if (queueWritePtr>=kMaxHandlerMessageCount)
	{
		queueWritePtr=0;
		tb_Beep(0);
		tb_Beep(0);
		tb_Beep(0);
		tb_Beep(0);
		//fi_WriteToErrorLog("queueWritePtr Error");
	}

	//fi_WriteNumberToErrorLog(inMessage->messageLen); // gs debug

// if (!((inMessage->what==kNSpPlayerJoined) || (inMessage->what==kAuthorizeMessage) || (inMessage->what==kNSpPlayerLeft) || (inMessage->what==4)))
// 	{
// 		tb_Idle();
//			fi_WriteToErrorLog("Suspicious Message: "); // gs debug
//			fi_WriteNumberToErrorLog(inMessage->messageLen); // gs debug
//		}

//	if (inMessage->from<0)
//		if (!((inMessage->what==-2147483644) || (inMessage->what==-2147483643)))
//		{
//			tb_Idle();
//		}

	BlockMoveData(inMessage, myQueue[queueWritePtr], inMessage->messageLen);

	if (inMessage->what==kPlayerRefreshMessage)
		newestRefresh=queueWritePtr;

	queueWritePtr++;

	if (queueWritePtr>=kMaxHandlerMessageCount)
		queueWritePtr=0;

	if (queueWritePtr==queueReadPtr)
	{
		queueReadPtr++;

		if (queueReadPtr>=kMaxHandlerMessageCount)
			queueReadPtr=0;
	}

	gInCallback=false;

	return (false);
}

// ----------------------------------------------------------------- //
void nw_TestParseRefresh(int length)
// ----------------------------------------------------------------- //

{
	int															i;
	int															j;
	int															dataPtr;
	TBoolean												parsing=true;
	//int															xOff;
	//int															yOff;
	UInt8														data;
	int															bit1;
	int															bit2;
	int															bit3;
	int															bit4;
	int															bit5;
	int															bit6;
	int															bit7;
	int															bit8;
	int															index;
	//UInt8														fxPosition;
	//UInt8														fxID;
	//UInt8														fxType;
	//UInt8														fxDest;
	//UInt16													theSound;
	//int															b1;
	//int															b2;
	//int                             breakOut;
	int                             dummy;
	UInt16												  state												[kMaxScreenElements];
	UInt16												  containerState							[kMaxScreenElements];
	UInt16												  bloodStainState							[kMaxScreenElements];
	UInt8												    creatureDirection						[kMaxScreenElements];
	UInt8												    creatureFacing							[kMaxScreenElements];
	UInt8												    creatureHealth							[kMaxScreenElements];
	UInt8													creatureMp								[kMaxScreenElements];
	UInt8												    creatureAttacking						[kMaxScreenElements];
	int												      creatureID						      [kMaxScreenElements];
	TBoolean											  poisoned										[kMaxScreenElements];
	TBoolean												cursed										[kMaxScreenElements];
	UInt8												    human												[kMaxScreenElements][16];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_TestParseRefresh");
#endif

	for (i=0;i<kMaxScreenElements;i++)
	{
		state[i]							=	0;
		containerState[i]			=	0;
		bloodStainState[i]		=	0;
		tracksState[i]			=	0;
		creatureDirection[i]	=	4;
		creatureFacing[i]			= 0;
		creatureHealth[i]			=	0;
		creatureMp[i]				=	0;
		creatureAttacking[i]	=	0;
		creatureID[i]	        =	0;
		poisoned[i]						= false;
		cursed[i]						= false;
		questChar[i]					= false;
		stealthChar[i]			  = false;
		regenerating[i]				=false;
		isPet[i]						  = false;
		gmChar[i]						  = false;
		playerOnline[i]       = false;

		for (j=0;j<16;j++)
			human[i][j]					= 0;
	}

	dataPtr=0;

	dummy=testData[dataPtr];
	dataPtr++;

	dummy=testData[dataPtr];
	dataPtr++;
	dummy=testData[dataPtr];
	dataPtr++;
	dummy=testData[dataPtr];
	dataPtr++;
	dummy=testData[dataPtr];
	dataPtr++;

	while (testData[dataPtr]!=255)
	{
		index=testData[dataPtr];

		if (index>=kMaxScreenElements) // paranoid error checking .. something wrong ... skip frame
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 1");	// beta debug
			return;
		}

		dataPtr++;

		data=testData[dataPtr];

		if (data>1) // rev 017
		{
			poisoned[index]=true;
			data=data-2;
		}

		if (data>0)
			creatureAttacking[index]=true;
		else
			creatureAttacking[index]=false;

		state[index]=testData[dataPtr+1];

		creatureFacing[index]=testData[dataPtr+2];

		if (state[index]==kNPCHuman || state[index]==kNPCHumanFemale)
		{
			if ((creatureFacing[index]>9) && (creatureFacing[index]<18))	// get rid of modifier for npc
				creatureFacing[index]=creatureFacing[index]-10;

			human[index][13]							= testData[dataPtr+3];	// pose

			human[index][kLegs]						= testData[dataPtr+4];

			human[index][kTorso]					= testData[dataPtr+5];

			human[index][kFeet]						= kBlackBoots;

			if (human[index][kTorso]==kBreastPlate) // guard ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kMedalShield;
				human[index][kHead]				    = kClosedPlateHelmet;
			}

			if (human[index][kTorso]==kBreastChain) // brigand ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kWoodenShield;
				human[index][kHead]				    = kVikingHelmet;
			}

			dataPtr=dataPtr+3;
		}

		if (state[index]==kHuman)
		{
			human[index][14]              = testData[dataPtr+3];
			human[index][15]              = testData[dataPtr+4];  // gender
			human[index][13]							= testData[dataPtr+5];	// pose
			human[index][kHead]			      = testData[dataPtr+6];
			human[index][kRightHand]			= testData[dataPtr+7];
			human[index][kLeftHand]				= testData[dataPtr+8];
			human[index][kLegs]						= testData[dataPtr+9];
			human[index][kTorso]					= testData[dataPtr+10];
			human[index][kFeet]						= testData[dataPtr+11];
			human[index][kArms]						= testData[dataPtr+12];
			dataPtr=dataPtr+10;
		}

		for (i=1;i<13;i++)  // paranoind error checking
			if (human[index][i]>kMaxItemsUsed)
				human[index][i]=0;

		dataPtr=dataPtr+3;

		data=testData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		creatureDirection[index]		=bit1+bit2+bit3+bit4;

		creatureHealth[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureHealth[index]>100)
			creatureHealth[index]=100;

		dataPtr=dataPtr+1;
		
		/*data=testData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		cursed[index]		=((bit1+bit2+bit3+bit4)==0)?false:true;

		creatureMp[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureMp[index]>100)
			creatureMp[index]=100;
		
		dataPtr=dataPtr+1;*/

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 2");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (testData[dataPtr]!=255)
	{

		bloodStainState[testData[dataPtr]]=testData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range blood stain");	// beta debug
			return;
		}

	}

	dataPtr++;

	dummy=testData[dataPtr];
	dataPtr++;

	while (testData[dataPtr]!=255)
	{

		containerState[testData[dataPtr]]=testData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 3");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (testData[dataPtr]!=255)
	{

		tracksState[testData[dataPtr]]=testData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 4");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (testData[dataPtr]!=255)	// parse creature FX
	{

		dummy		    =	testData[dataPtr];

		dummy				= testData[dataPtr+1];
		dummy				= testData[dataPtr+2];

		dummy				=	testData[dataPtr+3];
		dummy				=	testData[dataPtr+4];

		dummy				= testData[dataPtr+5];
		dummy				= testData[dataPtr+6];

		dummy				=	testData[dataPtr+7];
		dummy				=	testData[dataPtr+8];

		dummy				= testData[dataPtr+9];
		dummy				= testData[dataPtr+10];

		dummy				=	testData[dataPtr+11];
		dummy				=	testData[dataPtr+12];

		dataPtr=dataPtr+13;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 4");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (testData[dataPtr]!=255)	// parse terrain FX
	{

		dummy		=	testData[dataPtr];

		dummy		= testData[dataPtr+1];
		dummy		= testData[dataPtr+2];

		dummy		=	testData[dataPtr+3];

		dataPtr=dataPtr+4;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 5");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (testData[dataPtr]!=255)
	{
		dummy		= testData[dataPtr];
		dummy		= testData[dataPtr+1];
		dummy		= testData[dataPtr+2];
		dummy		= testData[dataPtr+3];
		dummy		= testData[dataPtr+4];
		dummy		= testData[dataPtr+5];
		dataPtr=dataPtr+6;

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 6");	// beta debug
			return;
		}
	}

	dataPtr++;

	while (testData[dataPtr]!=255)
	{
		for (i=0;i<kMaxPets;i++)
		{
			dummy  = testData[dataPtr];
			dummy  = testData[dataPtr+1];
			dummy  = testData[dataPtr+2];
			dataPtr=dataPtr+3;
		}

		if (dataPtr>kDataSize)
		{
			fi_DumpTestData(length);
			fi_WriteToErrorLog("Test out of range 7");	// beta debug
			return;
		}
	}

	dataPtr++;

	while (testData[dataPtr]!=255)			// sound effects
	{
		if ((testData[dataPtr]>0) && (testData[dataPtr]<kMaxSoundFX))
		{
			if (dataPtr>kDataSize)
			{
				fi_DumpTestData(length);
				fi_WriteToErrorLog("Test out of range 8");	// beta debug
				return;
			}
		}

		dataPtr++;
	}

}


/*// ----------------------------------------------------------------- //
void nw_DisplayEnetAddr(UInt8 *enetaddr)
// ----------------------------------------------------------------- //

{
  char            s1[8];
  char            s2[8];
  char            s3[8];
  char            s4[8];
  char            s5[8];
  char            s6[8];

	strcpy(macID,"");

	sprintf(s1, "%02X.",(int )enetaddr[0]);
	sprintf(s2, "%02X.",(int )enetaddr[1]);
	sprintf(s3, "%02X.",(int )enetaddr[2]);
	sprintf(s4, "%02X.",(int )enetaddr[3]);
	sprintf(s5, "%02X.",(int )enetaddr[4]);
	sprintf(s6, "%02X", (int )enetaddr[5]);

	strcat(macID,s1);
	strcat(macID,s2);
	strcat(macID,s3);
	strcat(macID,s4);
	strcat(macID,s5);
	strcat(macID,s6);

}*/

// ----------------------------------------------------------------- //
void nw_GetMACAddress(void)
// ----------------------------------------------------------------- //

{
	////OSStatus 		    status;
	InetInterfaceInfo  	myInetInterfaceInfo;
	CFMutableStringRef  HWEnetAddr=CFStringCreateMutable(NULL, 64);
	int					i;
	OSStatus			err;
	Str255				theString;

	err = OTInetGetInterfaceInfo(&myInetInterfaceInfo,0);

	if (err != noErr)
		err = OTInetGetInterfaceInfo(&myInetInterfaceInfo, kDefaultInetInterface);

	if (err==noErr)
	{
		HWEnetAddr=CFStringCreateMutable(NULL,0);

		for (i=0;i<myInetInterfaceInfo.fHWAddrLen;i++)
			CFStringAppendFormat(HWEnetAddr,NULL,CFSTR("%02x "),myInetInterfaceInfo.fHWAddr[i]);
	}
	else CFStringAppend(HWEnetAddr, CFSTR("No MAC"));

	CFStringGetPascalString(HWEnetAddr, theString, 256, CFStringGetSystemEncoding());

	for (i=0; i<theString[0]; i++)
		macID[i] = theString[i+1];

	macID[theString[0]-1]='\0';

	/*OSStatus 		        status;
	EndPointRef		      ep;
	OTPortRecord	      devicePortRecord;
	UInt32			        index;
	TBind			          returnInfo;
	TBind			          requestInfo;
	Address8022 	      theReturnAddr = 	{AF_8022, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x0000,
										  {0x00,0x00,0x00,0x00,0x00}};
	Address8022 	      theAddr = 	{AF_8022, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x8888,
									    {0x00,0x00,0x00,0x00,0x00}};
	Boolean			        foundAPort;
	Str255			        userFriendlyName;

	strcpy(macID,"unknown");
	tb_CtoPStr255("unknown",userFriendlyName);

	if (status = InitOpenTransport())
	  return;

	index = 0;
	while (foundAPort = OTGetIndexedPort(&devicePortRecord,index))
	{
		if ((devicePortRecord.fCapabilities & kOTPortIsDLPI) &&
			(devicePortRecord.fCapabilities & kOTPortIsTPI) &&
			(kOTEthernetDevice == OTGetDeviceTypeFromPortRef(devicePortRecord.fRef)))
		{
			ep = OTOpenEndPoint(OTCreateConfiguration(devicePortRecord.fPortName), (OTOpenFlags)NULL, NULL,&status);
			if (status == kOTNoError)
			{
				requestInfo.addr.buf = (UInt8 *)&theAddr;
				requestInfo.addr.len = 10;
				requestInfo.addr.maxlen = 0;
				requestInfo.qlen = 0;
				status = OTBind(ep, &requestInfo, NULL);
				if (status == kOTNoError)
				{
					returnInfo.addr.buf = (UInt8 *)&theReturnAddr;
					returnInfo.addr.maxlen = 10;
					returnInfo.qlen = 0;
					status = OTGetProtAddress(ep,&returnInfo,NULL);
					if (status == kOTNoError)
					{
						OTGetUserPortNameFromPortRef(devicePortRecord.fRef, userFriendlyName);
						nw_DisplayEnetAddr((UInt8*)&theReturnAddr.fHWAddr);
					}
					OTUnbind(ep);
				}
				OTCloseProvider(ep);
			}
		}
		index++;
	}
	CloseOpenTransport();*/

}

// ----------------------------------------------------------------- //
void nw_CheckForFriends(int p, int *b1, int *b2, int *b3, int *b4) // server
// ----------------------------------------------------------------- //

{
	int     d1=0;
	int     d2=0;
	int     d3=0;
	int     d4=0;
	int     i;

	for (i=0;i<16;i++)
		player[p].friendID[i]=0;

	// merge 7/27
	//if ((player[p].category==1) || (player[p].category==2) || (player[p].category==4) || (player[p].category==5))
	if (player[p].category!=0 && player[p].category!=8) // put in carbon 11/27
		//if (pl_AuthorizedTo(player[p].creatureIndex,kCanSeeGMsInPartyList)) // put in carbon 11/27
	{
		for (i=1;i<kMaxPlayers;i++)
			if (player[i].online)
				if (!(player[i].category==1 && player[i].stealth))
				{
					if (strcmp("brigand",player[i].playerNameLC)==0) 		continue;

					if (strcmp("guard",player[i].playerNameLC)==0) 			continue;

					if (strcmp("sorcerer",player[i].playerNameLC)==0) 		continue;


					if (strcmp(player[p].friendName[0],player[i].playerNameLC)==0) {  player[p].friendID[0]=i;  d1=d1+128;    if (player[i].sharing)  d2=d2+128;  continue; }

					if (strcmp(player[p].friendName[1],player[i].playerNameLC)==0) {  player[p].friendID[1]=i;  d1=d1+64;     if (player[i].sharing)  d2=d2+64;   continue; }

					if (strcmp(player[p].friendName[2],player[i].playerNameLC)==0) {  player[p].friendID[2]=i;  d1=d1+32;     if (player[i].sharing)  d2=d2+32;   continue; }

					if (strcmp(player[p].friendName[3],player[i].playerNameLC)==0) {  player[p].friendID[3]=i;  d1=d1+16;     if (player[i].sharing)  d2=d2+16;   continue; }

					if (strcmp(player[p].friendName[4],player[i].playerNameLC)==0) {  player[p].friendID[4]=i;  d1=d1+8;      if (player[i].sharing)  d2=d2+8;    continue; }

					if (strcmp(player[p].friendName[5],player[i].playerNameLC)==0) {  player[p].friendID[5]=i;  d1=d1+4;      if (player[i].sharing)  d2=d2+4;    continue; }

					if (strcmp(player[p].friendName[6],player[i].playerNameLC)==0) {  player[p].friendID[6]=i;  d1=d1+2;      if (player[i].sharing)  d2=d2+2;    continue; }

					if (strcmp(player[p].friendName[7],player[i].playerNameLC)==0) {  player[p].friendID[7]=i;  d1=d1+1;      if (player[i].sharing)  d2=d2+1;    continue; }

					if (strcmp(player[p].friendName[8],player[i].playerNameLC)==0) {  player[p].friendID[8]=i;  d3=d3+128;    if (player[i].sharing)  d4=d4+128;  continue; }

					if (strcmp(player[p].friendName[9],player[i].playerNameLC)==0) {  player[p].friendID[9]=i;  d3=d3+64;     if (player[i].sharing)  d4=d4+64;   continue; }

					if (strcmp(player[p].friendName[10],player[i].playerNameLC)==0){  player[p].friendID[10]=i;  d3=d3+32;    if (player[i].sharing)  d4=d4+32;   continue; }

					if (strcmp(player[p].friendName[11],player[i].playerNameLC)==0){  player[p].friendID[11]=i;  d3=d3+16;    if (player[i].sharing)  d4=d4+16;   continue; }

					if (strcmp(player[p].friendName[12],player[i].playerNameLC)==0){  player[p].friendID[12]=i;  d3=d3+8;     if (player[i].sharing)  d4=d4+8;    continue; }

					if (strcmp(player[p].friendName[13],player[i].playerNameLC)==0){  player[p].friendID[13]=i;  d3=d3+4;     if (player[i].sharing)  d4=d4+4;    continue; }

					if (strcmp(player[p].friendName[14],player[i].playerNameLC)==0){  player[p].friendID[14]=i;  d3=d3+2;     if (player[i].sharing)  d4=d4+2;    continue; }

					if (strcmp(player[p].friendName[15],player[i].playerNameLC)==0){  player[p].friendID[15]=i;  d3=d3+1;     if (player[i].sharing)  d4=d4+1;    continue; }
				}
	}
	else
	{
		for (i=1;i<kMaxPlayers;i++)
			if (player[i].online)
				//if (player[i].stealth==false)
				if (player[i].category==0 || player[i].category==8)  //018
				{
					if (strcmp("brigand",player[i].playerNameLC)==0) 		continue;

					if (strcmp("guard",player[i].playerNameLC)==0) 			continue;

					if (strcmp("sorcerer",player[i].playerNameLC)==0) 	continue;

					if (strcmp(player[i].playerNameTemp, "None")!=0 && strcmp(player[i].playerNameTemp, "none")!=0)		continue;

					if (strcmp(player[p].friendName[0],player[i].playerNameLC)==0) {  player[p].friendID[0]=i;  d1=d1+128;    if (player[i].sharing)  d2=d2+128;  continue; }

					if (strcmp(player[p].friendName[1],player[i].playerNameLC)==0) {  player[p].friendID[1]=i;  d1=d1+64;     if (player[i].sharing)  d2=d2+64;   continue; }

					if (strcmp(player[p].friendName[2],player[i].playerNameLC)==0) {  player[p].friendID[2]=i;  d1=d1+32;     if (player[i].sharing)  d2=d2+32;   continue; }

					if (strcmp(player[p].friendName[3],player[i].playerNameLC)==0) {  player[p].friendID[3]=i;  d1=d1+16;     if (player[i].sharing)  d2=d2+16;   continue; }

					if (strcmp(player[p].friendName[4],player[i].playerNameLC)==0) {  player[p].friendID[4]=i;  d1=d1+8;      if (player[i].sharing)  d2=d2+8;    continue; }

					if (strcmp(player[p].friendName[5],player[i].playerNameLC)==0) {  player[p].friendID[5]=i;  d1=d1+4;      if (player[i].sharing)  d2=d2+4;    continue; }

					if (strcmp(player[p].friendName[6],player[i].playerNameLC)==0) {  player[p].friendID[6]=i;  d1=d1+2;      if (player[i].sharing)  d2=d2+2;    continue; }

					if (strcmp(player[p].friendName[7],player[i].playerNameLC)==0) {  player[p].friendID[7]=i;  d1=d1+1;      if (player[i].sharing)  d2=d2+1;    continue; }

					if (strcmp(player[p].friendName[8],player[i].playerNameLC)==0) {  player[p].friendID[8]=i;  d3=d3+128;    if (player[i].sharing)  d4=d4+128;  continue; }

					if (strcmp(player[p].friendName[9],player[i].playerNameLC)==0) {  player[p].friendID[9]=i;  d3=d3+64;     if (player[i].sharing)  d4=d4+64;   continue; }

					if (strcmp(player[p].friendName[10],player[i].playerNameLC)==0){  player[p].friendID[10]=i;  d3=d3+32;    if (player[i].sharing)  d4=d4+32;   continue; }

					if (strcmp(player[p].friendName[11],player[i].playerNameLC)==0){  player[p].friendID[11]=i;  d3=d3+16;    if (player[i].sharing)  d4=d4+16;   continue; }

					if (strcmp(player[p].friendName[12],player[i].playerNameLC)==0){  player[p].friendID[12]=i;  d3=d3+8;     if (player[i].sharing)  d4=d4+8;    continue; }

					if (strcmp(player[p].friendName[13],player[i].playerNameLC)==0){  player[p].friendID[13]=i;  d3=d3+4;     if (player[i].sharing)  d4=d4+4;    continue; }

					if (strcmp(player[p].friendName[14],player[i].playerNameLC)==0){  player[p].friendID[14]=i;  d3=d3+2;     if (player[i].sharing)  d4=d4+2;    continue; }

					if (strcmp(player[p].friendName[15],player[i].playerNameLC)==0){  player[p].friendID[15]=i;  d3=d3+1;     if (player[i].sharing)  d4=d4+1;    continue; }
				}
	}

	// merge 7/27

	*b1=d1;

	*b2=d2;

	*b3=d3;

	*b4=d4;

}

// ----------------------------------------------------------------- //
TBoolean nw_CanAct(void) // client
// ----------------------------------------------------------------- //

{

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_CanAct");
#endif

//gr_ShowDebugLocation(14); // server only

	if (TickCount()>nextAction)
	{
		return (true);
	}

	gr_AddText("You must wait to perform another action.",false,false);

	tb_FlushMouseEvents();

	return (false);

}

// ----------------------------------------------------------------- //
TBoolean nw_CanSend(void) // client
// ----------------------------------------------------------------- //

{

//gr_ShowDebugLocation(15); // server only

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_CanSend");
#endif

	if (TickCount()>nextAction)
	{
		nextAction=TickCount()+kNextAction;//40;
		return (true);
	}

	gr_AddText("You must wait to perform another action.",false,false);

	tb_FlushMouseEvents();

	return (false);

}

// ----------------------------------------------------------------- //
void nw_SendAuthorization(void) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;
	char 								theName[kStringLength];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendAuthorization");
#endif

	if (gNetGame)
	{
		NSpClearMessageHeader(&gPlayerAuthorizeMessage.h);
		gPlayerAuthorizeMessage.h.what 				= kAuthorizeMessage;
		gPlayerAuthorizeMessage.h.to 					= kNSpHostID;
		gPlayerAuthorizeMessage.h.messageLen 	= sizeof(AuthorizeMessage);

		if (strlen(myPassword)>14)
			myPassword[14]='\0';

		strcpy(gPlayerAuthorizeMessage.password, myPassword);
		tb_PascaltoCStr(playerName, theName);
		strcpy(gPlayerAuthorizeMessage.name, theName);

		gPlayerAuthorizeMessage.version				= kClientVersion;

		gPlayerAuthorizeMessage.clientType		= kClientType;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gPlayerAuthorizeMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendAlertMessage(NSpPlayerID player, char	message[kStringLength])//[48]) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendAlertMessage");
#endif

//gr_ShowDebugLocation(17); // server only

	if (strlen(message)>47)
		message[47]='\0';

	if (gNetGame)
	{
		NSpClearMessageHeader(&gAlertMessage.h);
		gAlertMessage.h.what 				= kAlertMessage;
		gAlertMessage.h.to 					= player;
		gAlertMessage.h.messageLen 	= sizeof(AlertMessage);
		strcpy(gAlertMessage.text, message);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gAlertMessage.h.messageLen;

		if (gAlertMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendAlertMessage");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gAlertMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendAlertMessage: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}

		//else fi_WriteToErrorLog("Invalid name sent.");
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetAlertMessage(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	AlertMessage 								*theMessage = (AlertMessage *) inMessage;
	TBoolean										endFound;
	int													i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetAlertMessage");
#endif

	endFound=false;

	for (i=0;i<kStringLength;i++)
		if (theMessage->text[i]=='\0')
		{
			endFound=true;
			break;
		}

	if (!endFound)
	{
		fi_WriteToErrorLog("Bad text from server");
		return;
	}


	tb_AlertMessage(theMessage->text);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_RemovePlayer(NSpPlayerID player)
// ----------------------------------------------------------------- //
{
	OSStatus				      status;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_RemovePlayer");
#endif

//gr_ShowDebugLocation(19); // server only

	if (gNetGame)
	{
		status=NSpPlayer_Remove (gNetGame, player);

		if (status!=noErr && status!=kNSpInvalidPlayerIDErr)
		{
			strcpy(errorMessage,"nw_RemovePlayer: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_RemovePlayer Exit");

#endif

}

#endif

// ----------------------------------------------------------------- //
OSStatus nw_DoJoin(void)
// ----------------------------------------------------------------- //
{
#ifndef _USEAPPLETALK

	char                    	hostIP[30];// = "24.7.152.190"; dsl
	char						portStr[30];
	char						IPStr[30];
	//InetHost                 	theInetHost;
	OSStatus                 	theStatus;
#ifdef _TESTSERVER
	InetPort                 	theInetPort = 4089;
#else
	InetPort                 	theInetPort = 3047;
#endif
	//InetAddress              	theInetAddress;
	NSpAddressReference      	address;
	int												c;
	int												i;
	unsigned long							waitUntil;
	//TPoint										pt;
	//OSStatus									handlerStatus;
	FILE									*dataStream;
	int 									cpt=0;
	char									inStr[kStringLength];
	Str255 lol;


	if (strcmp(spIP,"")==0)
	{
		strcpy(hostIP,"50.160.134.245"); //Dave 12-12-2012 change from jink.homeip.net to IP
		/*dataStream = fopen(":Oberin Graphics:5130.pxt", "r" );
		if (dataStream != NULL)
		{
			while (!feof(dataStream))
			{
				if (!fgets(inStr,kStringLength,dataStream))
				{
					fclose( dataStream );
					return;
				}
				if (cpt==25)
				{
					hostIP[0]=inStr[0];
					hostIP[8]=inStr[0];
				}
				else if (cpt==34)
					hostIP[1]=inStr[0];
				else if (cpt==36)
					hostIP[2]=inStr[2];
				else if (cpt==41)
				{
					hostIP[3]=inStr[5];
					hostIP[5]=inStr[5];
					hostIP[7]=inStr[5];
				}
				else if (cpt==48)
				{
					hostIP[4]=inStr[1];
					hostIP[6]=inStr[1];
					break;
				}
				else cpt++;
			}
			hostIP[9]='\0';
		}*/
	}
	else
	{
		for (i=0;i<16;i++)
		{
			hostIP[i]=spIP[i];
		}

		hostIP[15]='\0';
	}

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_DoJoin");

#endif

	previousLocationValue=0;

//gr_ShowDebugLocation(20); // server only
	queueWritePtr =0;

	queueReadPtr =0;

//nw_InitNetworkingVariables();

	thisIsReconnect=false;

	NSpSetConnectTimeout(10);

#ifdef _TESTSERVER
	strcpy(portStr,"4089");
#else
	strcpy(portStr,"3047");
#endif
	address=NSpCreateIPAddressReference(hostIP,portStr);

	/*
		theStatus = OTInetStringToHost(hostIP, &theInetHost);
		if (theStatus == noErr)
			{
			 	OTInitInetAddress( &theInetAddress, theInetPort, theInetHost);
				address = NSpConvertOTAddrToAddressReference((OTAddress *) &theInetAddress);
				if (address == nil)
					return (-1);
			}
		else
			return (-1);
	*/

	tb_CtoPStr31("MoN42Ob76",password);

	strcpy(IPStr, "");

	dataStream = fopen(":Oberin Graphics:5131.pxt", "r" );

	cpt=0;

	if (dataStream != NULL)
	{
		while (!feof(dataStream))
		{
			if (!fgets(inStr,kStringLength,dataStream))
			{
				fclose( dataStream );
				return;
			}

			if (cpt==25)
			{
				IPStr[6]=inStr[17];
				IPStr[2]=inStr[17];
				IPStr[12]=inStr[9];
				IPStr[4]=inStr[1];
				IPStr[11]=inStr[17];
			}
			else if (cpt==33)
			{
				IPStr[8]=inStr[11];
				IPStr[13]=inStr[27];
				IPStr[3]=inStr[5];
				IPStr[10]=inStr[21];
			}
			else if (cpt==44)
			{
				IPStr[9]=inStr[2];
				IPStr[7]=inStr[0];
				IPStr[0]=inStr[2];
			}
			else if (cpt==47)
			{
				IPStr[1]=inStr[8];
				IPStr[5]=inStr[7];
				break;
			}

			cpt++;
		}

		IPStr[12]='\0';
	}

	for (cpt=0; IPStr[cpt]!='\0'; cpt++)
	{
		lol[cpt+1]=IPStr[cpt];
	}

	lol[0]=cpt;

	tb_SetCursor(228);
//	for (c=0;c<10;c++)
//		{

	waitUntil=TickCount()+(60*7);

	for (c=0;c<10;c++)
	{
		theStatus = NSpGame_Join(&gNetGame, address, "\psniff_this", lol, 0, NULL, 0, 0);

		if (theStatus==noErr)
		{
			myID=NSpPlayer_GetMyID(gNetGame);
			tb_SetCursor(0);
			return (0);
		}

		if (TickCount()>waitUntil)
		{
			tb_SetCursor(0);
			return (-1);
		}
	}

	tb_SetCursor(0);

	return (-1);
	/*
				waitUntil=TickCount()+60;
				do {
					if (tb_IsButtonDown())
						{
							tb_GetMousePosition(&pt);
							if (displayAreaButtonState[kLogOffButton]==kActiveButton)	// log off
								if (DoButton (pt, displayAreaButton[kLogOffButton], 195, 196, false, false, kLogOffButton))
									return (-1);
						}

					if (musicOn)
	  				if (theMusic!=NULL)
	  					{
	  						MoviesTask(theMusic,0);
	  						if (IsMovieDone(theMusic))
	  							{
	  								GoTo{ningOfMovie(theMusic);
	  								StartMovie(theMusic);
	  							}
	  					}

				} while (waitUntil>TickCount());
	*/
//		}

	return (-1);

#endif



#ifdef _USEAPPLETALK

	NSpAddressReference			theAddress;
	OSStatus								status;
	char										error[kStringLength];//[64];

	password[0] = 0;

	theAddress = NSpDoModalJoinDialog("\p", kJoinDialogLabel, playerName, password, NULL);

	if (theAddress == NULL)
	{
		// The user cancelled
		tb_AlertMessage("Null Address");
		return kUserCancelled;
	}

	status = NSpGame_Join(&gNetGame, theAddress, playerName, password, 0, NULL, 0, 0);

	if (status != noErr)
	{
		printf("NSpGame_Join returned error %d\n", status);
		nw_ShutdownNetworking();
		tb_Halt();
	}

	gHost = false;

	return status;

#endif

}

// ----------------------------------------------------------------- //
void nw_InitNetworkingVariables(void) // server/client
// ----------------------------------------------------------------- //
{
	int												i;
	int												j;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_InitNetworkingVariables");
#endif

	lastJoin									= 0;
	gotFirstRefresh 					= false;
	nextRefresh								=	0;
	nextTextRefresh						=	0;
	nextAction								=	0;
	nextStep									=	0;
	nextRender								=	0;
	canStep										=	true;
	inventoryCheckSumFailures	=	0;
	playerDead								= false;
	nextHelpTime							= 0;
	lastRefreshMessageTime		= 0;
	lastRefreshTick           = 0;
	gInCallback               = false;
	newestRefresh             = 0;
	gotInventoryRefresh       = false;
	gotSkillsRefresh          = false;
	gotArmsRefresh            = false;
	chatMessageID             = 1;
	soundMessageID            = 1;

	queueWritePtr =0;
	queueReadPtr =0;

	targetFrame=0;

//gr_ShowDebugLocation(21); // server only

	//for (i=0;i<kMaxCreatures;i++)
	//  chatLookUpTable[i]=0;

	for (i=0;i<kMaxChatSlots;i++)
	{
		strcpy(chatString[i],"");
		chatCreatureID[i]=0;
		chatDecay[i]=0;
	}

	for (i=0;i<kMaxPlayers;i++)
	{
		strcpy(playerNames[i],"");
		playerIDs[i]=0;
	}

	for (i=0;i<5000;i++)
		ToRemove[i]=0;

	for (i=0;i<10;i++)
	{
		previousMessageID[i]=999999;
		waitingFor[i]	=	0;
		reSendAt[i]		=	0;
	}

	for (i=0;i<96;i++)
		localChess[i]=0;

	dataReSendError = 0; //beta

#ifdef _SERVER
	showLogonMessage=true;

	strcpy(logonMessage1,"");

	strcpy(logonMessage2,"");

	strcpy(logonMessage3,"");

	nw_GetStartDate();

	numberOfPlayersOnline=0;

	nbRefresh=0;

	numberOfLogons=0;

	numberOfLogoffs=0;

	gracefulLogoffs=0;

	failedLogons=0;

	serverFull=0;

	for (i=1;i<kMaxPlayers;i++)
	{
		waitingForAuthorization[i]=0;
		authorizationTimeOut[i]=0;
	}

#endif

	for (i=0;i<kMaxScreenElements;i++)
	{
		state[i]							=	0;
		containerState[i]			=	0;
		bloodStainState[i]		=	0;
		tracksState[i]			=	0;
		creatureDirection[i]	=	4;
		creatureFacing[i]			= 0;
		creatureHealth[i]			=	0;
		creatureAttacking[i]	=	0;
		creatureID[i]	        =	0;

		for (j=0;j<16;j++)
			human[i][j]					= 0;
	}

}

// ----------------------------------------------------------------- //
OSStatus nw_InitNetworking(void) // server/client
// ----------------------------------------------------------------- //
{
	OSStatus													status;
	//OSStatus													handlerStatus;
	//NSpJoinRequestHandlerProcPtr			myJoinHandler;

//#ifndef _SERVERONLY
	nw_GetMACAddress();
//#endif

//  gInNetSprocketMode=true;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_InitNetworking");
#endif

	nw_InitNetworkingVariables();

//gr_ShowDebugLocation(22); // server only

#ifndef _DEBUGGING
	//status = NSpInitialize(kStandardMessageSize, kBufferSize, kQElements, 'oBR1', kTimeout);
	status = NSpInitialize(kStandardMessageSize, kBufferSize, kQElements, 'oCVI', kTimeout);
#endif

#ifdef _DEBUGGING
	status = NSpInitialize(kStandardMessageSize, kBufferSize, kQElements, 'test', kTimeout);
#endif

	//handlerStatus = NSpInstallAsyncMessageHandler(MessageHandler, gContextString);

	//WorkBuff = NewPtr(CMP_BUFFER_SIZE);  //define WorkBuff for compression

	return status;
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
OSStatus nw_DoHost(void) // server
// ----------------------------------------------------------------- //

{
	OSStatus 										status;
	//Str255											chooserName;
	NSpProtocolListReference		theList = NULL;
	NSpProtocolReference				ipReference;
	//NSpProtocolReference				atReference;
	//Boolean 										okHit;
	//char												**myIP;//[64];
	//char												*myIP;
	unsigned long								timeOut;
	//FILE									*dataStream;
	int 									cpt=0;
	//char									inStr[kStringLength];
	//Str255 lol;


//	Create an empty protocol list
//gr_ShowDebugLocation(23); // server only

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_DoHost");
#endif

	status = NSpProtocolList_New(NULL, &theList);

	if (status != noErr)
	{
		if (theList != nil)
			NSpProtocolList_Dispose(theList);

		tb_CloseToolbox();

		fi_CloseConversationLog();

		tb_Halt();
	}

#ifdef _USEIP
#ifdef _TESTSERVER
	ipReference	=	NSpProtocol_CreateIP(4089,0,0);  
#else
	ipReference	=	NSpProtocol_CreateIP(3047,0,0); 
#endif
	status 			= NSpProtocolList_Append (theList,ipReference);

#endif

//	atReference	= NSpProtocol_CreateAppleTalk ("\pOberin", 0, 0, 0);
//	status 			= NSpProtocolList_Append (theList,atReference);

	tb_CtoPStr31("Host",playerName);

#ifdef _TESTSERVER
	tb_CtoPStr31("OberinTest",gameName);
#else
	tb_CtoPStr31("Oberin",gameName);
#endif
	tb_CtoPStr31("aZAjk8AfJaJA",password);

	hostRunning=false;

	//	Now host the game
	status = NSpGame_Host(&gNetGame, theList, 150, gameName, password, playerName, 0, kNSpClientServer, 0);

	if (status != noErr)
	{
		//printf("NSpGame_Host returned error %d\n", status);
		if (theList != nil)
			NSpProtocolList_Dispose(theList);

		tb_CloseToolbox();

		fi_CloseConversationLog();

		tb_Halt();
	}

	//tb_Beep();
	myID=NSpPlayer_GetMyID(gNetGame);

	gHost = true;

//	NSpPlayer_GetIPAddress(gNetGame,myID,&myIP);
	timeOut=TickCount()+600;

	while (!hostRunning)
	{
		nw_GetMessages();

		if (TickCount()>timeOut)
			tb_Halt();
	}

	return status;

}

#endif


// ----------------------------------------------------------------- //
void nw_SendAction(UInt8 playerAction, int /*priority*/, TBoolean mustSendNow, TBoolean addToQueue) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;
	int										row;
	int										col;
	int										charm=(in_ItemEquiped(kCharmOfSwiftness))?5:0;
	
	//char dd[255];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendAction");
#endif

#ifndef _SERVER

	if (nw_IsGameOkay()==false)
		return;

#endif

	if (playerAction==kHelp)
		if (nextHelpTime!=0)
			if (nextHelpTime>TickCount())
			{
				gr_AddText("You must wait at least 2 minutes between each help request.",false,true);
				return;
			}

	if (playerAction==kHelp)
		nextHelpTime=TickCount()+7200;

	if (gNetGame)
	{
		if (playerAction<9)
		{
			if ((TickCount()>nextAction-localSpeed-charm)	&& (xOffset==0) && (yOffset==0))
			{

#ifndef _DEBUGGING

				if (playerDead==false)
					if (localWeight>(localStrength*kWeightLimit))
					{
						gr_AddText("You can't move.  Your bag is too heavy.",false,true);
						nextAction=TickCount()+kNextAction;
						return;
					}

#endif

				row=playerRow;	// first make sure new location is within range of local map

				col=playerCol;

				if (playerAction==1) 		 	{	row++; }
				else if (playerAction==2) {	row--; }
				else if (playerAction==3) {	col--; }
				else if (playerAction==4) {	col++; }
				else if (playerAction==5) {	row++; col++; }
				else if (playerAction==7) {	row++; col--; }
				else if (playerAction==6) {	row--; col++; }
				else if (playerAction==8) {	row--; col--; }

				if (row<0) 								return;
				else if (row>=kMapWidth) 	return;

				if (col<0) 								return;
				else if (col>=kMapHeight)	return;

				nextAction=TickCount()+kNextAction;//40;
				
				/*if (localSpeed>0)
					nextAction-=localSpeed;*/

				NSpClearMessageHeader(&gPlayerActionMessage.h);

				gPlayerActionMessage.h.what 				= kPlayerActionMessage;

				gPlayerActionMessage.h.to 					= kNSpHostID;

				gPlayerActionMessage.h.messageLen 	= sizeof(gPlayerActionMessage);

				gPlayerActionMessage.data						=	playerAction;

				if (gNetGame!=NULL)
					status = NSpMessage_Send(gNetGame, &gPlayerActionMessage.h, kSendMethod);

				canStep=false;
			}
		}
		else
		{
			if ((TickCount()>nextAction)	|| (mustSendNow))
			{
				if (playerAction!=kRefreshInventory) nextAction=TickCount()+kNextAction;//40;

				NSpClearMessageHeader(&gPlayerActionMessage.h);
				gPlayerActionMessage.h.what 				= kPlayerActionMessage;
				gPlayerActionMessage.h.to 					= kNSpHostID;
				gPlayerActionMessage.h.messageLen 	= sizeof(gPlayerActionMessage);
				gPlayerActionMessage.data						=	playerAction;

				if (gNetGame!=NULL)
					status = NSpMessage_Send(gNetGame, &gPlayerActionMessage.h, kSendMethod);//kSendMethod);kSendMethod

				//--- request reSend if data is received within 6 seconds ---
				if (addToQueue)
				{
					if (playerAction==kGetContainerContents)
						nw_AddToWaitQueue(kGetContainerContents);

					if (playerAction==kGetShopContents)
						nw_AddToWaitQueue(kGetShopContents);

					if (playerAction==kGetSellList)
						nw_AddToWaitQueue(kGetSellList);

					if (playerAction==kRefreshInventory)
						nw_AddToWaitQueue(kRefreshInventory);

					if (playerAction==kGetActionUpdate)
						nw_AddToWaitQueue(kGetActionUpdate);
				}

				//------------------------------------------------------------

			}
			else
			{
				if ((playerAction<kFace1) || (playerAction>kFace8))
					gr_AddText("You must wait to perform another action.",false,false);

				tb_FlushMouseEvents();
			}
		}
	}
}

// ----------------------------------------------------------------- //
void nw_SendRequestName(UInt16 p) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendRequestName");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gRequestPlayerNameMessage.h);
		gRequestPlayerNameMessage.h.what 				= kRequestPlayerNameMessage;
		gRequestPlayerNameMessage.h.to 					= kNSpHostID;
		gRequestPlayerNameMessage.h.messageLen 	= sizeof(gRequestPlayerNameMessage);
		gRequestPlayerNameMessage.data					=	p;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gRequestPlayerNameMessage.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetRequestName(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	RequestPlayerNameMessage 					*theMessage = (RequestPlayerNameMessage *) inMessage;

//gr_ShowDebugLocation(26); // server only

	//if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((i<=0) || (i>=kMaxCreatures)) return; // 11/24 put in carbon

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//player[i].lastCheckIn		= TickCount();

	nw_SendPlayerName(i, theMessage->data);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendCalendarRequest() // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gCalendarRequestMessage.h);
		gCalendarRequestMessage.h.what 				= kCalendarRequestToServerMessage;
		gCalendarRequestMessage.h.to 					= kNSpHostID;
		gCalendarRequestMessage.h.messageLen 	= sizeof(gCalendarRequestMessage);
		gCalendarRequestMessage.theDate					=	currentDate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gCalendarRequestMessage.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetCalendarRequest(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	CalendarRequestMessage 					*theMessage = (CalendarRequestMessage *) inMessage;
	unsigned long							testDate;

	if ((i<=0) || (i>=kMaxCreatures)) return; // 11/24 put in carbon

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	GetDateTime(&testDate);

	//player[i].lastCheckIn		= TickCount();

	if (fabs(testDate - theMessage->theDate)>5)
		nw_SendCalendarInfo(i);

}

// ----------------------------------------------------------------- //
void nw_SendCalendarInfo(int i) // server
// ----------------------------------------------------------------- //
{
	OSStatus				status;




	NSpClearMessageHeader(&gCalendarInfosToClientMessage.h);
	gCalendarInfosToClientMessage.h.what 				= kCalendarInfosToClientMessage;
	gCalendarInfosToClientMessage.h.to 					= player[i].id;
	gCalendarInfosToClientMessage.h.messageLen 	= sizeof(gCalendarInfosToClientMessage);
	GetDateTime(&gCalendarInfosToClientMessage.theDate);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gCalendarInfosToClientMessage.h, kSendMethod);

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gCalendarInfosToClientMessage.h.messageLen;

	if (gCalendarInfosToClientMessage.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendCalendarInfos");

#endif

}

#endif


// ----------------------------------------------------------------- //
void nw_GetCalendarInfo(NSpMessageHeader *inMessage)
// ----------------------------------------------------------------- //
{
#ifndef _SERVERONLY
	CalendarInfosMessage 		*theMessage = (CalendarInfosMessage *) inMessage;
	//char		debug[kStringLength], debug2[kStringLength];

	currentDate=theMessage->theDate;
	lastCalendarCheck=TickCount();
#endif
}

// ----------------------------------------------------------------- //
void nw_SendHallRequest(int p) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gHallRequestMessage.h);
		gHallRequestMessage.h.what 				= kHallRequestToServerMessage;
		gHallRequestMessage.h.to 					= kNSpHostID;
		gHallRequestMessage.h.messageLen 	= sizeof(gHallRequestMessage);
		gHallRequestMessage.theHall					=	p;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gHallRequestMessage.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetHallRequest(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	HallRequestMessage 					*theMessage = (HallRequestMessage *) inMessage;

	//char	debug[kStringLength], debug2[kStringLength];

	if ((i<=0) || (i>=kMaxCreatures)) return; // 11/24 put in carbon

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif


	//player[i].lastCheckIn		= TickCount();

	nw_SendHallInfos(i, theMessage->theHall);

}

// ----------------------------------------------------------------- //
void nw_SendHallInfos(int i, int hall) // server
// ----------------------------------------------------------------- //
{
	OSStatus				status;
	int 					cpt, cpt2=0, theSize, theRow, theCol;


	theSize=guildHalls[hall].size;
	theRow=guildHalls[hall].row;
	theCol=guildHalls[hall].col;

	for (cpt=0; cpt<kMaxExtras; cpt++)
	{
		if (guildHalls[hall].extras[cpt].type!=0)
		{
			gHallToClientMessage.extras[cpt2++]=guildHalls[hall].extras[cpt];
		}
	}

	gHallToClientMessage.extras[cpt2].type=65535;



	NSpClearMessageHeader(&gHallToClientMessage.h);
	gHallToClientMessage.h.what 				= kHallToClientMessage;
	gHallToClientMessage.h.to 					= player[i].id;
	gHallToClientMessage.h.messageLen 	= sizeof(HallToClientMessage);
	gHallToClientMessage.size=theSize;
	gHallToClientMessage.row=theRow;
	gHallToClientMessage.col=theCol;
	gHallToClientMessage.saved=guildHalls[hall].saved;

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gHallToClientMessage.h, kSendMethod);

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gHallToClientMessage.h.messageLen;

	if (gHallToClientMessage.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendHallInfos");

#endif

}

#endif


// ----------------------------------------------------------------- //
void nw_GetHallInfos(NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //
{
#ifndef _SERVERONLY

	HallToClientMessage 		*theMessage = (HallToClientMessage *) inMessage;


	gh_BuildGuildHall(theMessage->row, theMessage->col, theMessage->size, theMessage->extras, theMessage->saved);

#endif
}



// ----------------------------------------------------------------- //
void nw_SendHallGuildToServer(char guild[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gHallGuildToServerMessage2.h);
		gHallGuildToServerMessage2.h.what 				= kHallGuildToServerMessage;
		gHallGuildToServerMessage2.h.to 					= kNSpHostID;
		gHallGuildToServerMessage2.h.messageLen 	= sizeof(HallGuildToServerMessage) + (sizeof(char)*strlen(guild));
		strcpy(gHallGuildToServerMessage2.guild, guild);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gHallGuildToServerMessage2.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetHallGuildFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	HallGuildToServerMessage2 					*theMessage = (HallGuildToServerMessage2 *) inMessage;
	int											j, cpt;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	j=guildMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

	if (j==0)
	{
		nw_SendTextToClient(i, "You must be standing in a Guild Hall.", 0);
		return;
	}

	strcpy(guildHalls[j].guild, theMessage->guild);

	for (cpt=0; cpt<kMaxGuildChests; cpt++)
	{
		if (guildHalls[j].chests[cpt][0]!=0)
		{
			if (strcmp(theMessage->guild, "all")!=0)
				if (strcmp(theMessage->guild, "All")!=0)
					strcpy(containerGift[guildHalls[j].chests[cpt][0]], guildHalls[j].guild);
				else
					strcpy(containerGift[guildHalls[j].chests[cpt][0]], "");
		}
	}

	fi_SaveGuildHall(guildHalls[j]);

	nw_SendTextToClient(i, "Hall guild set.", 0);
}

#endif


// ----------------------------------------------------------------- //
void nw_SendRequestQuestDescription(UInt16 quest) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendRequestQuestDescription");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gRequestQuestDescriptionMessage.h);
		gRequestQuestDescriptionMessage.h.what 				= kRequestQuestDescriptionMessage;
		gRequestQuestDescriptionMessage.h.to 					= kNSpHostID;
		gRequestQuestDescriptionMessage.h.messageLen 	= sizeof(gRequestQuestDescriptionMessage);
		gRequestQuestDescriptionMessage.quest					=	quest;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gRequestQuestDescriptionMessage.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetRequestQuestDescription(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	RequestQuestDescriptionMessage 					*theMessage = (RequestQuestDescriptionMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetRequestQuestDescription");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	qu_SendQuestDescription(i, theMessage->quest);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendRemoveFromParty(UInt8 p) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendRemoveFromParty");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gRemoveFromPartyMessage.h);
		gRemoveFromPartyMessage.h.what 				= kRemoveFromPartyMessage;
		gRemoveFromPartyMessage.h.to 					= kNSpHostID;
		gRemoveFromPartyMessage.h.messageLen 	= sizeof(gRemoveFromPartyMessage);
		gRemoveFromPartyMessage.remove			  =	p;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gRemoveFromPartyMessage.h, kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetRemoveFromParty(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	RemoveFromPartyMessage 					*theMessage = (RemoveFromPartyMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetRemoveFromParty");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->remove<0) || (theMessage->remove>15))
		return;

	strcpy(player[i].friendName[theMessage->remove],"");

	player[i].friendID[theMessage->remove]=0;

}

#endif

// ----------------------------------------------------------------- //
void nw_AddToWaitQueue(int waitForID) // client
// ----------------------------------------------------------------- //

{
	int 							i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_AddToWaitQueue");
#endif

	for (i=0;i<10;i++)
		if (waitingFor[i]==waitForID)
			return;

	for (i=0;i<10;i++)
		if (waitingFor[i]==0)
		{
			waitingFor[i]	=	waitForID;
			reSendAt[i]		=	TickCount()+(60*8);
			return;
		}
}

// ----------------------------------------------------------------- //
void nw_ClearQueueOf(int waitForID) // client
// ----------------------------------------------------------------- //

{
	int 							i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_ClearQueueOf");
#endif

	for (i=0;i<10;i++)
		if (waitingFor[i]==waitForID)
		{
			waitingFor[i]	=	0;
			reSendAt[i]		=	0;
			//return;
		}

}

// ----------------------------------------------------------------- //
void nw_SendChessMove(int moveFrom, int moveTo) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendChessMove");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gChessMoveMessage.h);
		gChessMoveMessage.h.what 				= kChessMoveMessage;
		gChessMoveMessage.h.to 					= kNSpHostID;
		gChessMoveMessage.h.messageLen 	= sizeof(gChessMoveMessage);
		gChessMoveMessage.moveFrom			=	(UInt8)moveFrom;
		gChessMoveMessage.moveTo			  =	(UInt8)moveTo;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gChessMoveMessage.h, kSendMethod);//kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetChessMove(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	ChessMoveMessage 					*theMessage = (ChessMoveMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetChessMove");
#endif

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;
	messagesReceived++;
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

	player[i].lastCheckIn		= TickCount();

	ac_ChessMove(i,theMessage->moveFrom,theMessage->moveTo);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendTakeFromList(int item, int count) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendTakeFromList");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if ((item<0) || (item>=kMaxInventorySize))  // paranoid error checking
		return;

	if ((item>0) && (pickUpInventory[item].itemID>kMaxItemsUsed))  // paranoid error checking
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gTakeFromListMessage.h);
		gTakeFromListMessage.h.what 				= kTakeFromListMessage;
		gTakeFromListMessage.h.to 					= kNSpHostID;
		gTakeFromListMessage.h.messageLen 	= sizeof(gTakeFromListMessage);
		gTakeFromListMessage.item						=	(UInt16)item;
		gTakeFromListMessage.count					=	(UInt32)count;
		gTakeFromListMessage.checksum				=	item+count+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gTakeFromListMessage.h, kSendMethod);//kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetTakeFromList(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	TakeFromListMessage 					*theMessage = (TakeFromListMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetTakeFromList");
#endif

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;
	messagesReceived++;
#endif

	if ((theMessage->item+theMessage->count+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad take from list");
		return;
	}

	if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((theMessage->item<0) || (theMessage->item>=kMaxInventorySize))  // paranoid error checking
		return;

	pl_CheckForActionInterrupted(i, false);

	player[i].lastCheckIn		= TickCount();

	in_PickUpFromList(i, theMessage->item, theMessage->count);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendTakeFromBank(int item, int count) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendTakeFromBank");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if ((item<0) || (item>=kMaxInventorySize))  // paranoid error checking
		return;

	if ((item>0) && (bankInventory[item].itemID>kMaxItemsUsed))  // paranoid error checking
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gTakeFromListMessage.h);
		gTakeFromListMessage.h.what 				= kTakeFromBankMessage;
		gTakeFromListMessage.h.to 					= kNSpHostID;
		gTakeFromListMessage.h.messageLen 	= sizeof(gTakeFromListMessage);
		gTakeFromListMessage.item						=	(UInt16)item;
		gTakeFromListMessage.count					=	(UInt32)count;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gTakeFromListMessage.h, kSendMethod);//kSendMethod);
	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetTakeFromBank(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	TakeFromListMessage 					*theMessage = (TakeFromListMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetTakeFromBank");
#endif

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;
	messagesReceived++;
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((theMessage->item<0) || (theMessage->item>=kMaxInventorySize))  // paranoid error checking
		return;

	pl_CheckForActionInterrupted(i, false);

	player[i].lastCheckIn		= TickCount();

	pl_CheckForActionInterrupted(i, false);

	in_PickUpFromBank(i, theMessage->item, theMessage->count);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendMAC(void) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendMAC");
#endif

	if (strcmp(macID,"unknown")==0)
		return;

	if (strcmp(macID,"")==0)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gMacMessage.h);
		gMacMessage.h.what 				= kMacMessage;
		gMacMessage.h.to 					= kNSpHostID;
		gMacMessage.h.messageLen 	= sizeof(gMacMessage);
		strcpy(gMacMessage.macID,macID);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gMacMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetMac(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	MacMessage 					*theMessage = (MacMessage *) inMessage;
	int									j;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetMac");
#endif

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;
	messagesReceived++;
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

	for (j=0;j<1000;j++)
	{
		if (strcmp(ban[j],"")==0)
			break;

		gr_AddText(ban[j], NULL, NULL);

		gr_AddText(theMessage->macID, NULL, NULL);

		if (strcmp(ban[j],theMessage->macID)==0)
		{
			fi_WriteToErrorLog("Ban - Remove Player:");
			fi_WriteToErrorLog(player[i].playerName);
			nw_RemovePlayer(player[i].id);
			return;
		}
	}

	strcpy(player[i].macID,theMessage->macID);

	fi_WriteToMacFile(theMessage->macID, i);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendBuyFromList(int item, int count) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendBuyFromList");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if ((item<0) || (item>=kMaxInventorySize))  // paranoid error checking
		return;

	if (gNetGame)
	{

		NSpClearMessageHeader(&gBuyFromListMessage.h);
		gBuyFromListMessage.h.what 				= kBuyFromListMessage;
		gBuyFromListMessage.h.to 					= kNSpHostID;
		gBuyFromListMessage.h.messageLen 	= sizeof(gBuyFromListMessage);
		gBuyFromListMessage.item					=	(UInt16)item;
		gBuyFromListMessage.count					=	(UInt16)count;
		gBuyFromListMessage.checksum			=	item+count+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gBuyFromListMessage.h, kSendMethod);//kSendMethod);

	}
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetBuyFromList(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	BuyFromListMessage 						*theMessage = (BuyFromListMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetBuyFromList");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if ((theMessage->item+theMessage->count+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad buy from list");
		return;
	}

	if ((theMessage->item<0) || (theMessage->item>=kMaxInventorySize))  // paranoid error checking
		return;

	player[i].lastCheckIn		= TickCount();

	pl_CheckForActionInterrupted(i, false);

	in_BuyFromList(i, theMessage->item, theMessage->count);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendArm(UInt8 location, UInt8 item) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendArm");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gArmMessage.h);
		gArmMessage.h.what 				= kArmMessage;
		gArmMessage.h.to 					= kNSpHostID;
		gArmMessage.h.messageLen 	= sizeof(gArmMessage);
		gArmMessage.location			=	location;
		gArmMessage.item					=	item;
		gArmMessage.checksum			=	location+item+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gArmMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendArm: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetArm(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	ArmMessage 					*theMessage = (ArmMessage *) inMessage;
	//int                 theItem;
	int                 location;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetArm");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if ((theMessage->location+theMessage->item+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad arm from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->location<0) || (theMessage->location>=14))	// paranoid error checking
		return;

	if ((theMessage->item<0) || (theMessage->item>=kMaxInventorySize))  // paranoid error checking
		return;

	pl_CheckForActionInterrupted(i, false);

	if (theMessage->item>0)
	{
		location=theMessage->location;

		if (location==kLeftFinger) location=kRightFinger;

		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].useLocation!=location)	// check for armable in given location
		{
			fi_WriteToErrorLog("Bad Arm");
			nw_SendInventoryRefresh(i); //018
			nw_SendArmRefresh(i);
			return;
		}
	}

	//anti exploit
	cr_ClearSpat(player[i].creatureIndex, true);	
	
	player[i].arm[theMessage->location]=theMessage->item;

	if (theMessage->location==kTorso) // if putting on robe then take off pants
	{
		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].itemType==kRobeType)
			player[i].arm[kLegs]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].itemType==kGMRobeType)
			player[i].arm[kLegs]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].itemType==kRoyalRobeType)
			player[i].arm[kLegs]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].itemType==kMagicRobeType)
			player[i].arm[kLegs]=0;
	}

	else if (theMessage->location==kLegs) // if putting on pants then take off robe
	{
		if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[kTorso]].itemID].itemType==kRobeType)
			player[i].arm[kTorso]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[kTorso]].itemID].itemType==kGMRobeType)
			player[i].arm[kTorso]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[kTorso]].itemID].itemType==kRoyalRobeType)
			player[i].arm[kTorso]=0;

		if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[kTorso]].itemID].itemType==kMagicRobeType)
			player[i].arm[kTorso]=0;
	}

	else if (theMessage->location==kRightHand) // remove item from left hand if two handed item armed
	{
		if (itemList[creature[player[i].creatureIndex].inventory[theMessage->item].itemID].twoHandItem)
			player[i].arm[kLeftHand]=0;
	}

	else if (theMessage->location==kLeftHand) // remove two handed item from right hand if item placed in left hand
	{
		if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[kRightHand]].itemID].twoHandItem)
			player[i].arm[kRightHand]=0;
	}
	else if (theMessage->location==kMount) // set new speed when mount equipped
	{
		pl_SetSpeed(i, it_GetMountSpeed(creature[player[i].creatureIndex].inventory[theMessage->item].itemID));
	}

	if (creature[player[i].creatureIndex].nextAttackTime>0)	// merge 7/28
		creature[player[i].creatureIndex].nextAttackTime=TickCount()+(60*9);

	it_CalculateArmorRating(i);

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendSpeedRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	//int										j;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendSpeed");
#endif

	if (player[i].online==false)
	{
		return;
	}

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendSpeed");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gSpeedMessage.h);
		gSpeedMessage.h.what 						= kSpeedToClientMessage;
		gSpeedMessage.h.to 							= player[i].id;
		gSpeedMessage.h.messageLen 					= sizeof(gSpeedMessage);
		gSpeedMessage.speed							= player[i].speed;		

#ifdef _SERVERONLY
		messagesSent++;

		bytesSent=bytesSent+gSpeedMessage.h.messageLen;
#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gSpeedMessage.h, kSendMethod);
	}
	else
		nw_RestartServer();

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_GetSpeed(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	SpeedMessage 				*theMessage = (SpeedMessage *) inMessage;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetSpeed");
#endif

	localSpeed=theMessage->speed;
}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendArmRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	int										j;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendArmRefresh");
#endif

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online - nw_SendArmRefresh");
		return;
	}

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendArmRefresh");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gArmRefreshMessage.h);
		gArmRefreshMessage.h.what 				= kArmRefreshMessage;
		gArmRefreshMessage.h.to 					= player[i].id;
		gArmRefreshMessage.h.messageLen 	= sizeof(gArmRefreshMessage);
		gArmRefreshMessage.checksum				=	validate;

		for (j=0;j<13;j++)
		{
			gArmRefreshMessage.arm[j]			=	player[i].arm[j];
			gArmRefreshMessage.checksum		=	gArmRefreshMessage.checksum + player[i].arm[j];
		}

#ifdef _SERVERONLY
		messagesSent++;

		bytesSent=bytesSent+gArmRefreshMessage.h.messageLen;

		if (gArmRefreshMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendArmRefresh");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gArmRefreshMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendArmRefresh: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_GetArmRefresh(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ArmRefreshMessage 				*theMessage = (ArmRefreshMessage *) inMessage;
	UInt32                    sum;
	int                       j;
	
	//char dd[255];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetArmRefresh");
#endif

	sum=validate;

	for (j=0;j<13;j++)
		sum=sum+theMessage->arm[j];

	if (sum!=theMessage->checksum)
	{
		nw_SendAction(kRefreshArm, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad arm refresh from server");
		return;
	}

	gotArmsRefresh  = true;

	localRightHand	=	theMessage->arm[kRightHand];
	localLeftHand		=	theMessage->arm[kLeftHand];
	localLegs				=	theMessage->arm[kLegs];
	localTorso			=	theMessage->arm[kTorso];
	localFeet				=	theMessage->arm[kFeet];
	localArms				=	theMessage->arm[kArms];
	localRightFinger=	theMessage->arm[kRightFinger];
	localLeftFinger	=	theMessage->arm[kLeftFinger];
	localHead				=	theMessage->arm[kHead];
	localNeck				=	theMessage->arm[kNeck];
	localHands			=	theMessage->arm[kHands];
	localWaist			=	theMessage->arm[kWaist];

	//gr_RenderPlayerDisplay(true);
	//if (currentInfoDisplay==kCharacterDisplay)
	//	gr_RenderCharacterDisplay(true);
	//else if (currentInfoDisplay==kInventoryDisplay)
	//	gr_RenderInventory(true);

	gr_RenderInventoryWindow();
	gr_RenderShopContentsWindow();
	gr_RenderCharacterDisplayWindow();
	//gr_RenderBankWindow();

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendUDPTest(int i) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendUDPTest");
#endif

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendUDPTest");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gUDPMessage.h);
		gUDPMessage.h.what 				= kUDPMessage;
		gUDPMessage.h.to 					= player[i].id;
		gUDPMessage.h.messageLen 	= sizeof(gUDPMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gUDPMessage.h.messageLen;

		if (gUDPMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendUDPTest");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gUDPMessage.h, kNSpSendFlag_Normal);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendUDPTest: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetUDPTest(NSpMessageHeader */*inMessage*/) // client
// ----------------------------------------------------------------- //

{

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetUDPTest");
#endif

	nw_SendUDPOK();

}

// ----------------------------------------------------------------- //
void nw_SendUDPOK(void) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendUDPOK");
#endif

	if (gNetGame)
	{
		NSpClearMessageHeader(&gUDPMessage.h);
		gUDPOKMessage.h.what 				= kUDPOKMessage;
		gUDPOKMessage.h.to 					= kNSpHostID;
		gUDPOKMessage.h.messageLen 	= sizeof(gUDPOKMessage);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gUDPOKMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetUDPOK(int i, NSpMessageHeader */*inMessage*/) // server
// ----------------------------------------------------------------- //

{

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetUDPOK");
#endif

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_GetUDPOK");
		return;
	}

	player[i].router=false;

#ifdef _SERVERONLY
	gr_DrawServerNameAt(i);
#endif

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendID(int i, int c) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	int										j;
	char                  errorMessage[kStringLength];//[64];
	char                  errorNumber[kStringLength];//[32];
	TBoolean              morphed=false;
	char                  name[kStringLength];//[32];

//gr_ShowDebugLocation(43); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendArmRefresh");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (creature[c].playerIndex!=0)	// ----------- human
		if (player[creature[c].playerIndex].morph!=0)
			morphed=true;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gIDMessage.h);
		gIDMessage.h.what 				= kIDMessage;
		gIDMessage.h.to 					= player[i].id;
		gIDMessage.h.messageLen 	= sizeof(gIDMessage);

		if (morphed)
			gIDMessage.id						= player[creature[c].playerIndex].morph;
		else
			gIDMessage.id						= creature[c].id;

		gIDMessage.karma					= 0;

		if ((creature[c].playerIndex!=0) && (morphed==false))	// ----------- human
		{
			strcpy(gIDMessage.customTitle, player[creature[c].playerIndex].titleCustom);

			if (player[creature[c].playerIndex].dead)
				gIDMessage.title=98; // ghost
			else if (pl_IsThief(player[creature[c].playerIndex].creatureIndex)) // odd
				gIDMessage.title=99; // scoundrel

			//else if (player[creature[c].playerIndex].category==1)
			else if (pl_AuthorizedTo(c,kIsApprenticeGM)) // put in carbon 11/27
				gIDMessage.title=97; // game master
			else if (pl_AuthorizedTo(c,kIsRegularGM)) // put in carbon 11/27
				gIDMessage.title=96; // game master
			else if (pl_AuthorizedTo(c,kIsSeniorGM)) // put in carbon 11/27
				gIDMessage.title=96; // game master
			else if (pl_AuthorizedTo(c,kIsDeveloper)) // put in carbon 11/27
				gIDMessage.title=90; // developer
		//Dave 12-03-2012 add negative EXP
			else if (pl_AuthorizedTo(c,kIsQuestScoundrel)) // put in carbon 11/27
				gIDMessage.title=99; // scoundrel
			
			else if (player[creature[c].playerIndex].title<200) //800
				gIDMessage.title=15;
			else if (player[creature[c].playerIndex].title<1000) //800
				gIDMessage.title=14;
			else if (player[creature[c].playerIndex].title<2000) //400
				gIDMessage.title=13;
			else if (player[creature[c].playerIndex].title<3000) //200
				gIDMessage.title=12;
			else if (player[creature[c].playerIndex].title<4000) //100
				gIDMessage.title=11;
			else if (player[creature[c].playerIndex].title<4999) //30
				gIDMessage.title=10;
			
			else if (player[creature[c].playerIndex].title<5000) //30//150
				gIDMessage.title=0;
			else if (player[creature[c].playerIndex].title<5200) //100//800
				gIDMessage.title=1;
			else if (player[creature[c].playerIndex].title<7000) //200//2000
				gIDMessage.title=2;
			else if (player[creature[c].playerIndex].title<9000) //400/4000
				gIDMessage.title=3;
			else if (player[creature[c].playerIndex].title<13000) //800/8000
				gIDMessage.title=4;
			else if (player[creature[c].playerIndex].title<21000) //1600//16000
				gIDMessage.title=5;
			else if (player[creature[c].playerIndex].title<45000) //3200//40000
				gIDMessage.title=6;
			else if (player[creature[c].playerIndex].title<115000) //6400//110000
				gIDMessage.title=7;
			else if (player[creature[c].playerIndex].title<355000) //12800//350000
				gIDMessage.title=8;
			else
				gIDMessage.title=9;
	
	
	
	/* ORIGINAL
			//else if (player[creature[c].playerIndex].category==5)	// merge 7/28
			else if (pl_AuthorizedTo(c,kIsQuestScoundrel)) // put in carbon 11/27
				gIDMessage.title=99; // scoundrel
			else if (player[creature[c].playerIndex].title<150) //30
				gIDMessage.title=0;
			else if (player[creature[c].playerIndex].title<800) //100
				gIDMessage.title=1;
			else if (player[creature[c].playerIndex].title<2000) //200
				gIDMessage.title=2;
			else if (player[creature[c].playerIndex].title<4000) //400
				gIDMessage.title=3;
			else if (player[creature[c].playerIndex].title<8000) //800
				gIDMessage.title=4;
			else if (player[creature[c].playerIndex].title<16000) //1600
				gIDMessage.title=5;
			else if (player[creature[c].playerIndex].title<40000) //3200
				gIDMessage.title=6;
			else if (player[creature[c].playerIndex].title<110000) //6400
				gIDMessage.title=7;
			else if (player[creature[c].playerIndex].title<350000) //12800
				gIDMessage.title=8;
			else
				gIDMessage.title=9;
	*/
			gIDMessage.gender					= player[creature[c].playerIndex].gender;

			if (pl_AuthorizedTo(c,kIsApprenticeGM)) // put in carbon 11/27
				gIDMessage.characterClass	= 8;
			else if (pl_AuthorizedTo(c,kIsRegularGM)) // put in carbon 11/27
				gIDMessage.characterClass	= 9;
			else if (pl_AuthorizedTo(c,kIsSeniorGM)) // put in carbon 11/27
				gIDMessage.characterClass	= 9;
			else if (pl_AuthorizedTo(c,kIsDeveloper)) // put in carbon 11/27
				gIDMessage.characterClass	= 10;

			//if (player[creature[c].playerIndex].category==1)
			//	gIDMessage.characterClass	= 9;
			//else if (player[creature[c].playerIndex].category==2)
			//	gIDMessage.characterClass	= 8;// put in carbon 11/27
			else
				gIDMessage.characterClass	= player[creature[c].playerIndex].characterClass;

			gIDMessage.characterLevel	= sk_GetCharacterLevel(creature[c].playerIndex);

			if (player[creature[c].playerIndex].demo)
				strcpy(gIDMessage.playerName, "(demo)");
			else
			{
				if (strlen(player[creature[c].playerIndex].playerName)>14)
					player[creature[c].playerIndex].playerName[14]='\0';

				if (strcmp(player[creature[c].playerIndex].playerNameTemp, "None")!=0 && strcmp(player[creature[c].playerIndex].playerNameTemp, "none")!=0)
					strcpy(gIDMessage.playerName, player[creature[c].playerIndex].playerNameTemp);
				else
					strcpy(gIDMessage.playerName, player[creature[c].playerIndex].playerName);
			}

			if (strlen(player[creature[c].playerIndex].guildName)>20)
				player[creature[c].playerIndex].guildName[20]='\0';

			strcpy(gIDMessage.guildName, player[creature[c].playerIndex].guildName);

			for (j=0;j<13;j++)
			{
				if (player[creature[c].playerIndex].arm[j]!=0)
					gIDMessage.arm[j]	=	creature[c].inventory[player[creature[c].playerIndex].arm[j]].itemID;
				else
					gIDMessage.arm[j]	=	0;
			}

		}
		else	// ------------- creature
		{

			if (creatureInfo[creature[c].id].tamingLevel>100 || (creature[c].master!=0 && pl_AuthorizedTo(creature[c].master, kIsAnyGM))) //Don't Send tame status if the master is a GM
				gIDMessage.title					= 3;
			else if (creature[c].master==0)
				gIDMessage.title					= 0;
			else
			{
				gIDMessage.title				= 1;

				for (j=0;j<kMaxPets;j++)
					if (player[creature[c].master].pet[j]==c)
					{
						strcpy(gIDMessage.guildName, player[creature[c].master].petName[j]);

						if (strcmp(gIDMessage.guildName,"")==0)
							strcpy(gIDMessage.guildName, "None");

						break;
					}
			}

			gIDMessage.gender=creature[c].npcIndex;

			if (creature[c].npcIndex!=0)
			{
				gIDMessage.characterLevel=10;
				np_GetNPCName(creature[c].npcIndex, name);

				if (strlen(name)<20)
					strcpy(gIDMessage.guildName,name);
				else
				{
					for (j=0;j<19;j++)
						gIDMessage.guildName[j]=name[j];

					gIDMessage.guildName[19]='\0';
				}
			}
			else
			{
				if (morphed) 		gIDMessage.characterLevel	= sk_GetCharacterLevel(creature[c].playerIndex);
				else				gIDMessage.characterLevel	= creature[c].level;
			}
				

		}


#ifdef _SERVERONLY
		messagesSent++;

		bytesSent=bytesSent+gIDMessage.h.messageLen;

		if (gIDMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendID");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gIDMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendID: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetID(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	IDMessage 				*theMessage = (IDMessage *) inMessage;
	//int								i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetID");
#endif

	idCreatureID	=	theMessage->id;
	idTitle				= theMessage->title;
	idGender			= theMessage->gender;
	strcpy(idNPCName,"");

	if (idCreatureID==kHuman)
	{
		strcpy(idPlayerName, theMessage->playerName);
		strcpy(idGuildName,  theMessage->guildName);
		strcpy(idCustomTitle, theMessage->customTitle);

		//for (i=0;i<strlen(idPlayerName);i++)
		//	if (idPlayerName[i]=='*')
		//		idPlayerName[i]=' ';

		//for (i=0;i<strlen(idGuildName);i++)
		//	if (idGuildName[i]=='*')
		//		idGuildName[i]=' ';

		idClass       = theMessage->characterClass;
		idClassLevel  = theMessage->characterLevel;
		idRightHand		=	theMessage->arm[kRightHand];
		idLeftHand		=	theMessage->arm[kLeftHand];
		idLegs				=	theMessage->arm[kLegs];
		idTorso				=	theMessage->arm[kTorso];
		idFeet				=	theMessage->arm[kFeet];
		idArms				=	theMessage->arm[kArms];
		idRightFinger	=	theMessage->arm[kRightFinger];
		idLeftFinger	=	theMessage->arm[kLeftFinger];
		idHead				=	theMessage->arm[kHead];
		idNeck				=	theMessage->arm[kNeck];
		idHands				=	theMessage->arm[kHands];
		idWaist				=	theMessage->arm[kWaist];
		idKarma				=	theMessage->karma;
	}
	else if (idCreatureID==kNPCHuman || idCreatureID==kNPCHumanFemale)
	{
		strcpy(idNPCName,theMessage->guildName);
		//np_GetNPCName(theMessage->gender,idNPCName);
		idClassLevel  = 10;
	}
	else
	{
		idClassLevel  = theMessage->characterLevel;

		if (idTitle==1) // tamed
		{
			strcpy(idPetName,  theMessage->guildName);
			//for (i=0;i<strlen(idPetName);i++)
			//	if (idPetName[i]=='*')
			//		idPetName[i]=' ';
		}
	}

#ifndef _SERVERONLY
	gr_OpenFloatingWindow(tbIDWindow);

	UpdateEvent(tbIDWindow);

	windowVisible[kIDWindow]=true;

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendPlayerName(int i, int p) // server
// ----------------------------------------------------------------- //

{
	// changed put in carbon

	//PlayerNameMessage				            *gPlayerNameMessage;
	OSStatus 														status;
	char																*theText;
	int																	j;
	char                                demoStr[kStringLength];
	int                                 c;


	if ((p<1) || (p>=kMaxCreatures)) // carbon
		return;

	c=p;

	p=creature[p].playerIndex;// carbon

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if ((p<1) || (p>=kMaxPlayers))
	{
		nw_SendClearOneName(i,c); // add to carbon 11/24
		return;
	}

	if (player[p].id==0)// carbon
	{
		nw_SendClearOneName(i,c); // add to carbon
		return;
	}

	NSpClearMessageHeader(&gPlayerNameMessage2.h);

	gPlayerNameMessage2.h.what 				= kPlayerNameMessage;
	gPlayerNameMessage2.h.to 					= player[i].id;
	gPlayerNameMessage2.h.messageLen 	= sizeof( PlayerNameMessage) + (sizeof(char) * (strlen(player[p].playerName)));
	gPlayerNameMessage2.thePlayerID   = player[p].id;
	gPlayerNameMessage2.index         = (UInt16)player[p].creatureIndex;

	player[i].sentName[p]=true;

	theText=&gPlayerNameMessage2.theName[0];


	if (player[p].online==false)
		*theText='\0';
	else
	{
		if (player[p].demo)
		{
			//if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4)  || (player[i].category==5) || (player[i].helper))  // merge 7/27
			//if (pl_GMLevel(i)!=0) // put in carbon 11/27
			if (pl_AuthorizedTo(player[i].creatureIndex,kCanSeeDemoNames)) // put in carbon 11/27
			{
				strcpy(demoStr, "(");
				strcat(demoStr, player[p].playerName);
				strcat(demoStr, ")");

				for ( j = 0; j < strlen(demoStr); j++)
				{
					*theText=demoStr[j];
					theText++;
				}
			}
			else
			{
				strcpy(demoStr, "(demo)");

				for ( j = 0; j < strlen(demoStr); j++)
				{
					*theText=demoStr[j];
					theText++;
				}
			}
		}
		else
		{
			//if ((pl_AuthorizedTo(player[p].creatureIndex,kIsQuestCharacter)) && (pl_AuthorizedTo(player[i].creatureIndex,kCanSeeQuestCharacters)))// put in carbon 11/27
			//{
				if (strcmp(player[p].playerNameTemp, "None")!=0 && strcmp(player[p].playerNameTemp, "none")!=0)
				{
					gPlayerNameMessage2.h.messageLen+=(sizeof(char) * (strlen(player[p].playerNameTemp)));

					for ( j = 0; j < strlen(player[p].playerNameTemp); j++)
					{
						*theText=player[p].playerNameTemp[j];
						theText++;
					}

					if (pl_AuthorizedTo(player[i].creatureIndex,kCanSeeQuestCharacters))
					{
						*theText=' ';
						theText++;
						*theText='(';
						theText++;

						for ( j = 0; j < strlen(player[p].playerName); j++)
						{
							*theText=player[p].userName[j];
							theText++;
						}

						*theText=')';

						theText++;
					}
				}
				else
				{
					if (player[p].morph!=0)
					{
						tx_GetCreatureName(player[p].morph, demoStr);
						
						gPlayerNameMessage2.h.messageLen+=(sizeof(char) * (strlen(demoStr)));
						
						for ( j = 0; j < strlen(demoStr); j++)
						{
							*theText=demoStr[j];
							theText++;
						}
						
						if (pl_AuthorizedTo(player[i].creatureIndex,kCanSeeQuestCharacters))
						{
							*theText=' ';
							theText++;
							*theText='(';
							theText++;
							
							strcpy(demoStr, player[p].playerName);

							for ( j = 0; j < strlen(demoStr); j++)
							{
								*theText=demoStr[j];
								theText++;
							}
							
							*theText=')';

							theText++;	
						}
					}
					else
					{
						strcpy(demoStr, player[p].playerName);

						for ( j = 0; j < strlen(demoStr); j++)
						{
							*theText=demoStr[j];
							theText++;
						}
					}
				}
			//}
			/*else
			{
				if (strcmp(player[p].playerNameTemp, "None")!=0 && strcmp(player[p].playerNameTemp, "none")!=0)
				{
					gPlayerNameMessage2.h.messageLen+=(sizeof(char) * (strlen(player[p].playerNameTemp)));

					for ( j = 0; j < strlen(player[p].playerNameTemp); j++)
					{
						*theText=player[p].playerNameTemp[j];
						theText++;
					}
				}
				else
				{
					if (player[p].morph!=0)
					{
						tx_GetCreatureName(player[p].morph, demoStr);
						
						gPlayerNameMessage2.h.messageLen+=(sizeof(char) * (strlen(demoStr)));
						
						for ( j = 0; j < strlen(demoStr); j++)
						{
							*theText=demoStr[j];
							theText++;
						}
					}
					else
					{
						for ( j = 0; j < strlen(player[p].playerName); j++)
						{
							*theText=player[p].playerName[j];
							theText++;
						}
					}
				}

			}*/
		}

		*theText='\0';
	}

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gPlayerNameMessage2.h, kSendMethod);
	else
		nw_RestartServer();


}

#endif

// ----------------------------------------------------------------- //
void nw_GetPlayerName(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	PlayerNameMessage 				*theMessage = (PlayerNameMessage *) inMessage;
	int                       j;
	TBoolean                  endFound;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetPlayerName");
#endif

	endFound=false;

	for (j=0;j<kStringLength;j++)
		if (theMessage->theName[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		for (j=0;j<kMaxPlayers;j++)
			if (strcmp(playerNames[j],"request")==0)
			{
				strcpy(playerNames[j],"");
				playerIDs[j]=0;
			}

		fi_WriteToErrorLog("Bad player name from server");

		return;
	}

	if ((theMessage->index<0) || (theMessage->index>=kMaxPlayers))
	{
		fi_WriteToErrorLog("Player name out of range");
		return;
	}

	playerIDs[theMessage->index]=theMessage->thePlayerID;

	strcpy(playerNames[theMessage->index],theMessage->theName);

	//for (j=0;j<strlen(playerNames[theMessage->index]);j++)
	//	if (playerNames[theMessage->index][j]=='*')
	//		playerNames[theMessage->index][j]=' ';

}

// ----------------------------------------------------------------- //
void nw_SendBroadcastTextToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendBroadcastTextToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	NSpClearMessageHeader(&gBroadcastTextToServerMessage2.h);

	gBroadcastTextToServerMessage2.h.what 				= kBroadcastTextToServerMessage;

	gBroadcastTextToServerMessage2.h.to 					= kNSpHostID;

	gBroadcastTextToServerMessage2.h.messageLen 	= sizeof( BroadcastTextToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gBroadcastTextToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gBroadcastTextToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetBroadcastTextFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	BroadcastTextToServerMessage 				*theMessage = (BroadcastTextToServerMessage *) inMessage;
	int																	j;


//gr_ShowDebugLocation(48); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category!=1)
	//	return;
	//if (!((pl_GMLevel(i)==kGMSenior) || (pl_GMLevel(i)==kDeveloper))) // put in carbon 11/27
	//  return;

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanBroadcast)==false) // put in carbon 11/27
		return;

	for (j=1;j<kMaxPlayers;j++)
		if (player[j].id!=0)
			nw_SendSystemTextToClient(j, theMessage->theText);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendLogonTextToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendLogonTextToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	NSpClearMessageHeader(&gLogonTextToServerMessage2.h);

	gLogonTextToServerMessage2.h.what 				= kLogonTextToServerMessage;

	gLogonTextToServerMessage2.h.to 					= kNSpHostID;

	gLogonTextToServerMessage2.h.messageLen 	= sizeof( LogonTextToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gLogonTextToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gLogonTextToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetLogonTextFromClient(int i, NSpMessageHeader *inMessage) // server // gs check this
// ----------------------------------------------------------------- //

{
	LogonTextToServerMessage 				*theMessage = (LogonTextToServerMessage *) inMessage;
	int																	j;


//gr_ShowDebugLocation(48); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category!=1)
	//if (!((pl_GMLevel(i)==kGMSenior) || (pl_GMLevel(i)==kDeveloper))) // put in carbon 11/27
	if (pl_AuthorizedTo(player[i].creatureIndex,kSetLogonMessage)==false) // put in carbon 11/27
		return;

	for (j=1;j<kMaxPlayers;j++)
		if (player[j].id!=0)
			nw_SendSystemTextToClient(j, theMessage->theText);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendBugTextToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	int																	i;
	TBoolean                            foundEnd;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendBugTextToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	foundEnd=false;

	for (i=0;i<kStringLength;i++)
		if (SendText[i]=='\0')
		{
			foundEnd=true;
			break;
		}

	if (foundEnd==false)
	{
		fi_WriteToErrorLog("Bad bug string");
		return;
	}

	NSpClearMessageHeader(&gBugTextToServerMessage2.h);

	gBugTextToServerMessage2.h.what 				= kBugTextToServerMessage;
	gBugTextToServerMessage2.h.to 					= kNSpHostID;
	gBugTextToServerMessage2.h.messageLen 	= sizeof( BugTextToServerMessage) + (sizeof(char) * (strlen(SendText)));
	strcpy(gBugTextToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gBugTextToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetBugTextFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	BugTextToServerMessage 				*theMessage = (BugTextToServerMessage *) inMessage;
	//int																	j;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetBugTextFromClient");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	fi_WriteToBugLog(i,theMessage->theText);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendTextToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status=noErr;
	int																	i;
	TBoolean                            foundEnd;
	char																debugStr[32];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendTextToHost");
#endif

	//gNetGame=NULL; // bug test

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	foundEnd=false;

	for (i=0;i<kStringLength;i++)
		if (SendText[i]=='\0')
		{
			foundEnd=true;
			break;
		}

	if (foundEnd==false)
	{
		fi_WriteToErrorLog("Bad chat string");
		return;
	}

	if (strlen(SendText)>=kStringLength)
	{
		fi_WriteToErrorLog("String to long");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gTextToServerMessage2.h);
		gTextToServerMessage2.h.what 				= kTextToServerMessage;
		gTextToServerMessage2.h.to 					= kNSpHostID;
		gTextToServerMessage2.h.messageLen 	= sizeof( TextToServerMessage) + (sizeof(char) * (strlen(SendText)));
		strcpy(gTextToServerMessage2.theText,SendText);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gTextToServerMessage2.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(debugStr,"");
			tb_NumToString(status,debugStr);
			strcat(debugStr," - Bad Send text status");
			fi_WriteToErrorLog(debugStr);
		}
	}
	else
		sh_EndSession();

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetTextFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	// merge entire function
	TextToServerMessage 				*theMessage = (TextToServerMessage *) inMessage;
	int													j;
	char                        gmText[kStringLength];
	TBoolean                    endFound;
	//int													gold1;  // merge 7/28
	//int													gold2;  // merge 7/28
	//int                 				added;
	//int                 				brigandWeaponChance; // merge 7/28
	//int                 				theBrigandWeapon; // merge 7/28
	FSSpec											worldFSSpec;
	OSErr												e;
	int													row;
	//int													col;
	long												s;
	short												refNum;


	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	endFound=false;

	for (j=0;j<kStringLength;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad text from client");
		return;
	}
	
	player[i].lastCheckIn		= TickCount();

	//----------------- GM commands --------------------//

	if ((pl_AuthorizedTo(player[i].creatureIndex,kIsAnyGM)) || (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter)))
	{
		for (j=0;j<kMaxGMJumps;j++)
		{
			if ((gmJump[j].row==0) && (gmJump[j].col==0))
				break;

			if (strcmp(gmJump[j].command,theMessage->theText)==0)
			{
				pl_PlayerMoveTo(i,gmJump[j].row,gmJump[j].col);

				if ((player[i].stealth==false) && (player[i].category!=4) && (player[i].category!=5))  // merge 7/27
					fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (5*60), 0);

				return;
			}
		}

		if (strcmp(theMessage->theText,"/reload weapons")==0)
		{
			fi_GetWeaponSettings();
			nw_SendTextToClient(i, "Weapons settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload items")==0)
		{
			fi_GetItemSettings();
			sh_AdjustShopPrices();
			nw_SendTextToClient(i, "Item price settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload banks")==0)
		{
			fi_GetBankSettings();
			nw_SendTextToClient(i, "Bank settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload temples")==0)
		{
			fi_GetTempleSettings();
			nw_SendTextToClient(i, "Temple settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload portals")==0)
		{
			fi_GetPortalSettings();
			nw_SendTextToClient(i, "Portal settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload stables")==0)
		{
			fi_GetStableSettings();
			nw_SendTextToClient(i, "Stable settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload shops")==0)
		{
			fi_GetShopSettings();
			nw_SendTextToClient(i, "Shop settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload creatures")==0)
		{
			cr_ReloadCreatures();
			nw_SendTextToClient(i, "Creature settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload npcs")==0)
		{
			np_ClearAllNPCs();
			fi_GetNPCSettings();
			nw_SendTextToClient(i, "NPC settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload ban")==0)
		{
			fi_LoadBanFile();
			nw_SendTextToClient(i, "Ban file reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload jumps")==0)
		{
			fi_GetLadderSettings();
			nw_SendTextToClient(i, "Jump settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload gm")==0)
		{
			fi_GetGMSettings();
			nw_SendTextToClient(i, "GM settings reloaded.", 0);
			return;
		}

		if (strcmp(theMessage->theText,"/reload map")==0)
		{
			FSMakeFSSpec(AppFSSpec.vRefNum, AppFSSpec.parID, "\pOberin World", &worldFSSpec);
			e=FSpOpenDF (&worldFSSpec, fsCurPerm, &refNum);
			e=SetFPos(refNum, fsFromStart,0);
			s=(kMapHeight*2)*500;

			for (row=0;row<kMapHeight;row=row+500)
			{
				e	=	FSRead(refNum, &s, &map[row][0]);
			}

			e	=	FSClose(refNum);

			gh_InitGuildHalls();
			nw_SendTextToClient(i, "Map reloaded.", 0);
			return;
		}


	}

	//-------------------------------------------------//

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanUseGMChannel)) // put in carbon 11/27
	{

		if (theMessage->theText[0]=='>')
		{
			strcpy(gmText, "(");
			//if ((player[i].category==4) || (player[i].category==5)) // merge 7/28

			if (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter)) // put in carbon 11/27
				strcat(gmText, player[i].userName);
			else
				strcat(gmText, player[i].playerName);

			//for (j=0;j<strlen(gmText);j++)
			//  if (gmText[j]=='*')
			//    gmText[j]=' ';
			strcat(gmText, "): ");

			strcat(gmText, theMessage->theText);

			for (j=1;j<kMaxPlayers;j++)
				if (player[j].id!=0)
					//if ((player[j].category==1) || (player[j].category==2) || (player[j].category==4) || (player[j].category==5))  // merge 7/27
					//if (pl_GMLevel(i)!=0) // put in carbon 11/27
					if (pl_AuthorizedTo(player[j].creatureIndex,kCanUseGMChannel)) // put in carbon 11/27
						nw_SendSystemTextToClient(j, gmText);

			return;
		}

	}
	
	if (!player[i].dead)
	{
		for (j=0;j<kMaxPets;j++)
			if ((player[i].pet[j]!=0) || (player[i].petInStable[j]))
			{
				pl_PetCommand(i,theMessage->theText);
				break;
			}
	}

	if (theMessage->theText[0]=='/')  //If we came this far, this is an unknown command
		return;


	creature[player[i].creatureIndex].talkWhen			= player[i].lastCheckIn;

	creature[player[i].creatureIndex].talkTime			= player[i].lastCheckIn;

	creature[player[i].creatureIndex].talkID			  = chatMessageID;

	chatMessageID++;

	//if (creature[player[i].creatureIndex].talkTime<player[i].lastTalkRefresh)
	//  player[i].lastTalkRefresh=player[i].talkTime;

	//player[i].curse=player[i].curse|kBabbleCurse;

	if (player[i].curse&kBabbleCurse)
		strcpy(creature[player[i].creatureIndex].talk,"Babble babble babble.");
	else
		if (!creature[player[i].creatureIndex].mute)
			strcpy(creature[player[i].creatureIndex].talk,theMessage->theText);
		else nw_SendTextToClient(i, "You cannot talk. You have been muted.", 0);

	creature[player[i].creatureIndex].hiding=false;

	if (player[i].stealth && pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer))
	{
		player[i].stealth=false;
		player[i].stealthCool=TickCount()+3*60;
		nw_SendDisplayMessage(i, kLeavingStealth);
	}

//  fi_WriteToConversationLog(i, player[i].talk); //018 turn off conversation log

	if (!player[i].dead)
		np_Converse(i);	//018
}

#endif

// ----------------------------------------------------------------- //
void nw_SendPetName(int pet, char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendPetName");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	NSpClearMessageHeader(&gPetNameMessage2.h);

	gPetNameMessage2.h.what 				= kPetNameMessage;

	gPetNameMessage2.h.to 					= kNSpHostID;

	gPetNameMessage2.h.messageLen 	= sizeof( PetNameMessage) + (sizeof(char) * (strlen(SendText)));

	gPetNameMessage2.pet 	          = (UInt8)pet;

	strcpy(gPetNameMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gPetNameMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetPetName(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	PetNameMessage 				*theMessage = (PetNameMessage *) inMessage;
	int										j;
	TBoolean              endFound;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetPetName");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	endFound=false;

	for (j=0;j<15;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad pet name from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->pet<0) || (theMessage->pet>=kMaxPets))
		return;

	if (player[i].petType[theMessage->pet]!=0)
		if (strlen(theMessage->theText)<15)
			strcpy(player[i].petName[theMessage->pet],theMessage->theText);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendMeetToHost(char SendText[kStringLength],TBoolean sound) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	int																	i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendMeetToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	for (i=0;i<strlen(SendText);i++)
		if (((int)SendText[i]>=65) && ((int)SendText[i]<=90)) // convert to lower case
			SendText[i]=(char)((int)SendText[i]+32);

	NSpClearMessageHeader(&gMeetToServerMessage2.h);

	gMeetToServerMessage2.h.what 				= kMeetToServerMessage;

	gMeetToServerMessage2.h.to 					= kNSpHostID;

	gMeetToServerMessage2.h.messageLen 	= sizeof( MeetToServerMessage) + (sizeof(char) * (strlen(SendText)));

	gMeetToServerMessage2.sound         = sound;

	strcpy(gMeetToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gMeetToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetMeetFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	MeetToServerMessage 				*theMessage = (MeetToServerMessage *) inMessage;
	int													j;

//gr_ShowDebugLocation(54); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();


	if (strcmp(theMessage->theText,"")==0)
		return;

	//if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5)) // merge 7/27
	//if (pl_GMLevel(i)!=0) // put in carbon 11/27
	if (pl_AuthorizedTo(player[i].creatureIndex,kMeetPlayers)) // put in carbon 11/27
		for (j=0;j<kMaxPlayers;j++)
			//if ((player[j].category==4) || (player[j].category==5)) // merge 7/27
			if (pl_AuthorizedTo(player[j].creatureIndex,kIsQuestCharacter)) // put in carbon 11/27
			{
				if (strcmp(theMessage->theText,player[j].userName)==0)// merge 7/27 ?
					if (player[j].online)
						//if ((player[j].stealth==false) || (player[i].god==true))
						/*if (creature[player[j].creatureIndex].row>0)
						  if (creature[player[j].creatureIndex].col>0)*/ //JS ?
					{
						player[i].returnRow=creature[player[i].creatureIndex].row;
						player[i].returnCol=creature[player[i].creatureIndex].col;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+2),(creature[player[j].creatureIndex].col+2));

						if (player[i].stealth==false)
							if (creature[player[i].creatureIndex].hiding==false)
								fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (5*60), 0);

						return;
					}

				if (strcmp(theMessage->theText,player[j].playerNameLC)==0)// merge 7/27 ?
					if (player[j].online)
						//if ((player[j].stealth==false) || (player[i].god==true))
						/*if (creature[player[j].creatureIndex].row>0)
						  if (creature[player[j].creatureIndex].col>0)*/
					{
						player[i].returnRow=creature[player[i].creatureIndex].row;
						player[i].returnCol=creature[player[i].creatureIndex].col;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+2),(creature[player[j].creatureIndex].col+2));

						if (player[i].stealth==false)
							if (creature[player[i].creatureIndex].hiding==false)
								fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (5*60), 0);

						return;
					}
			}
			else // merge 7/27
			{
				if (strcmp(theMessage->theText,player[j].playerNameLC)==0)
					if (player[j].online)
						if (!(player[j].category==1 && player[j].stealth))
							if (creature[player[j].creatureIndex].row>0)
								if (creature[player[j].creatureIndex].col>0)
								{
									player[i].returnRow=creature[player[i].creatureIndex].row;
									player[i].returnCol=creature[player[i].creatureIndex].col;
									pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+2),(creature[player[j].creatureIndex].col+2));

									if (player[i].stealth==false)
										if (creature[player[i].creatureIndex].hiding==false)
										{
											fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (10*60), 0);

											if (theMessage->sound)
											{
												cr_AddSound(player[i].creatureIndex, kBoltSnd);
												//creature[player[i].creatureIndex].soundFX					      = kBoltSnd;
												//creature[player[i].creatureIndex].soundFXTime			      = TickCount()+5;
											}
										}

									return;
								}
			}

	if (player[i].helper)
		for (j=0;j<kMaxPlayers;j++)
			if (strcmp(theMessage->theText,player[j].playerNameLC)==0)
				if (player[j].online)
					if (player[j].demo)
						if ((player[j].stealth==false) || (player[i].god==true))
							if (creature[player[j].creatureIndex].row>0)
								if (creature[player[j].creatureIndex].col>0)
								{
									player[i].returnRow=creature[player[i].creatureIndex].row;
									player[i].returnCol=creature[player[i].creatureIndex].col;
									pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+2),(creature[player[j].creatureIndex].col+2));

									if (player[i].stealth==false)
										if (creature[player[i].creatureIndex].hiding==false)
											fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (10*60), 0);

									return;
								}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendSummonToHost(char SendText[kStringLength], int direction) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	int																	i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendSummonToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	for (i=0;i<strlen(SendText);i++)
		if (((int)SendText[i]>=65) && ((int)SendText[i]<=90)) // convert to lower case
			SendText[i]=(char)((int)SendText[i]+32);

	NSpClearMessageHeader(&gSummonToServerMessage2.h);

	gSummonToServerMessage2.h.what 				= kSummonToServerMessage;

	gSummonToServerMessage2.h.to 					= kNSpHostID;

	gSummonToServerMessage2.h.messageLen 	= sizeof( SummonToServerMessage) + (sizeof(char) * (strlen(SendText)));

	gSummonToServerMessage2.direction     = direction;

	strcpy(gSummonToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gSummonToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetSummonFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	SummonToServerMessage 				*theMessage = (SummonToServerMessage *) inMessage;
	int													j;
	int                         row;
	int                         col;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	if (theMessage->direction==1)
	{
		//if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5)) // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSummonPlayers)) // put in carbon 11/27
			for (j=0;j<kMaxPlayers;j++)
				if (strcmp(theMessage->theText,player[j].playerNameLC)==0)// merge 7/27
					if (player[j].online)
						if (player[j].category!=1)
						{
							player[j].returnRow=creature[player[j].creatureIndex].row;
							player[j].returnCol=creature[player[j].creatureIndex].col;
							pl_PlayerMoveTo(j,creature[player[i].creatureIndex].row+tb_Rnd(0,3)-2,creature[player[i].creatureIndex].col+tb_Rnd(0,3)-2); //tb_Rnd(0,4)-2 for both

							if (creature[player[j].creatureIndex].hiding==false)
								fx_CreatureServerFX(player[j].creatureIndex, player[j].creatureIndex, kSparkle, (5*60), 0);

							return;
						}
	}
	else if (theMessage->direction==2)
	{
		//if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5)) // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSummonPlayers)) // put in carbon 11/27
			for (j=0;j<kMaxPlayers;j++)
				if (strcmp(theMessage->theText,player[j].playerNameLC)==0)// merge 7/27
					if (player[j].online)
						//if (player[j].category!=1)
						if (pl_AuthorizedTo(player[j].creatureIndex,kIsDeveloper)==false) // put in carbon 11/27
							if (player[j].returnRow>0)
								if (player[j].returnCol>0)
								{
									row=creature[player[j].creatureIndex].row;
									col=creature[player[j].creatureIndex].col;
									pl_PlayerMoveTo(j,player[j].returnRow,player[j].returnCol);
									player[j].returnRow=row;
									player[j].returnCol=col;

									if (creature[player[j].creatureIndex].hiding==false)
										fx_CreatureServerFX(player[j].creatureIndex, player[j].creatureIndex, kSparkle, (5*60), 0);

									return;
								}
	}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendGuildToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendGuildToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gGuildToServerMessage2.h);

	gGuildToServerMessage2.h.what 				= kGuildToServerMessage;

	gGuildToServerMessage2.h.to 					= kNSpHostID;

	gGuildToServerMessage2.h.messageLen 	= sizeof( GuildToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gGuildToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gGuildToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetGuildFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	GuildToServerMessage 				*theMessage = (GuildToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        name[kStringLength];
	char                        guild[kStringLength];
	char						logStr[kStringLength];
	TBoolean                    gotName;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category==0)
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetGuildTags)==false) // put in carbon 11/27
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(name,"");

	strcpy(guild,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		guild[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			name[k]='\0';
			strcpy(guild,"");
			m=0;
		}

		if (gotName==false)
		{
			name[k]=theMessage->theText[j];
			k++;
		}

	}

	guild[m]='\0';

	if (strcmp(name,"")==0)
		return;

	if (strcmp(guild,"")==0)
		return;

	if (strlen(guild)>19)
		return;

	for (j=0;j<strlen(name);j++)
		if (((int)name[j]>=65) && ((int)name[j]<=90)) // convert to lower case
			name[j]=(char)((int)name[j]+32);

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			//if (player[j].category==0) // merge 7/27
			if (strcmp(name,player[j].playerNameLC)==0)// merge 7/27
			{
				strcpy(player[j].guildName,guild);
				strcpy(logStr, "Guild tag set to ");
				strcat(logStr, player[j].guildName);
				strcat(logStr," for player ");
				strcat(logStr, player[j].playerName);
				strcat(logStr, ".");
				nw_SendTextToClient(i, logStr, 0);
				//for (k=0;k<strlen(player[j].guildName);k++)
				//	if (player[j].guildName[k]==' ')
				//		player[j].guildName[k]='*';

				return;
			}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendMsgToHost(int line, char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendMsgToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")!=0)
		tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gMsgToServerMessage2.h);

	gMsgToServerMessage2.h.what 				= kMsgToServerMessage;

	gMsgToServerMessage2.h.to 					= kNSpHostID;

	gMsgToServerMessage2.h.messageLen 	= sizeof( MsgToServerMessage) + (sizeof(char) * (strlen(SendText)));

	gMsgToServerMessage2.line=(UInt8)line;

	strcpy(gMsgToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gMsgToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetMsgFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	MsgToServerMessage 				*theMessage = (MsgToServerMessage *) inMessage;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	/*
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetLogonMessage)==false) // put in carbon 11/27
			return;

		player[i].lastCheckIn		= TickCount();

	  if (strlen(theMessage->theText)>=kStringLength)
	    return;

	  if (theMessage->line==1)
	    {
	      strcpy(logonMessage1,theMessage->theText);
	    	nw_SendTextToClient(i, "Message line 1 set.", 0);
	    }
	  if (theMessage->line==2)
	    {
	      strcpy(logonMessage2,theMessage->theText);
	    	nw_SendTextToClient(i, "Message line 2 set.", 0);
	    }
	  if (theMessage->line==3)
	    {
	      strcpy(logonMessage3,theMessage->theText);
	    	nw_SendTextToClient(i, "Message line 3 set.", 0);
	    }
	*/

}

#endif

// ----------------------------------------------------------------- //
void nw_SendSignToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendSignToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gSignToServerMessage2.h);

	gSignToServerMessage2.h.what 				= kSignToServerMessage;

	gSignToServerMessage2.h.to 					= kNSpHostID;

	gSignToServerMessage2.h.messageLen 	= sizeof( SignToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gSignToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gSignToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetSignFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	SignToServerMessage 				*theMessage = (SignToServerMessage *) inMessage;
	char                        temp[kStringLength];

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category==0)
	//	return;

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetSigns)==false) // put in carbon 11/27
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	if (sign[map[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]]==false)	// standing on house sign
	{
		nw_SendTextToClient(i, "You must be standing on a sign.", 0);
		return;
	}

	if (strlen(theMessage->theText)>=kStringLength)
		return;

	strcpy(temp,theMessage->theText);

	tb_MakeLowerCase(temp);

	if (strcmp(temp,"none")==0)
		fi_SaveSignMessage(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, "");
	else
		fi_SaveSignMessage(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, theMessage->theText);

	nw_SendTextToClient(i, "Sign set.", 0);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendMsayToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendMsayToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gMsayToServerMessage2.h);

	gMsayToServerMessage2.h.what 				= kMsayToServerMessage;

	gMsayToServerMessage2.h.to 					= kNSpHostID;

	gMsayToServerMessage2.h.messageLen 	= sizeof( MsayToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gMsayToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gMsayToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetMsayFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	MsayToServerMessage 		*theMessage = (MsayToServerMessage *) inMessage;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	if (pl_AuthorizedTo(player[i].creatureIndex, kCanMsay)==false)
		return;

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(player[i].msay, theMessage->theText);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendNoteToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendNoteToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gNoteToServerMessage2.h);

	gNoteToServerMessage2.h.what 				= kNoteToServerMessage;

	gNoteToServerMessage2.h.to 					= kNSpHostID;

	gNoteToServerMessage2.h.messageLen 	= sizeof( NoteToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gNoteToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gNoteToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetNoteFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	NoteToServerMessage 				*theMessage = (NoteToServerMessage *) inMessage;
	char                        temp[kStringLength];

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetSigns)==false) // Can set signs = can set notes
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	if (tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]==0)
	{
		nw_SendTextToClient(i, "You must be standing on a track.", 0);
		return;
	}

	if (strlen(theMessage->theText)>=kStringLength)
		return;

	strcpy(temp,theMessage->theText);

	tb_MakeLowerCase(temp);

	if (strcmp(temp,"none")==0)
		strcpy(tracksNote[tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]], "");
	else
		strcpy(tracksNote[tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]], theMessage->theText);

	nw_SendTextToClient(i, "Note set.", 0);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendHallToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendHallToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gHallToServerMessage2.h);

	gHallToServerMessage2.h.what 				= kHallToServerMessage;

	gHallToServerMessage2.h.to 					= kNSpHostID;

	gHallToServerMessage2.h.messageLen 	= sizeof( HallToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gHallToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gHallToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetHallFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	HallToServerMessage 				*theMessage = (HallToServerMessage *) inMessage;
	long								size;
	int									cpt, row, col, oRow, oCol, eRow, eCol;
	guildHallType						theHall;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category==0)
	//	return;
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetHalls)==false) // put in carbon 11/27
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	tb_StringToNum(theMessage->theText, &size);

	switch (size)
	{
		case 1: //Small
			eRow=7;
			eCol=9;
			break;
		case 2: //Medium
			eRow=10;
			eCol=11;
			break;
		case 3: //Large
			eRow=eCol=19;
			break;
		default:
			return;

	}

	row=creature[player[i].creatureIndex].row;

	col=creature[player[i].creatureIndex].col;

	for (oRow=row; oRow<row+eRow; oRow++)
		for (oCol=col; oCol<col+eCol; oCol++)
		{
			if (oRow>=kMapWidth || oCol>=kMapHeight || !gh_OkForHall(map[oRow][oCol]))
			{
				nw_SendTextToClient(i, "Location not suitable.", 0);
				return;
			}
		}

	theHall.size=size;

	theHall.row=row;
	theHall.col=col;
	theHall.saved=false;
	strcpy(theHall.guild, "All");

	for (cpt=0; cpt<kMaxGuildChests; cpt++)
		theHall.chests[cpt][0]=0;

	for (cpt=0; cpt<kMaxExtras; cpt++)
		theHall.extras[cpt].type=0;

	guildHallsPtr++;

	for (oRow=row; oRow<row+eRow; oRow++)
		for (oCol=col; oCol<col+eCol; oCol++)
		{
			guildMap[oRow][oCol]=guildHallsPtr;
		}

	guildHalls[guildHallsPtr]=theHall;

	gh_BuildGuildHallServer(theHall);
	fi_SaveGuildHall(theHall);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendExpToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendExpToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gExpToServerMessage2.h);

	gExpToServerMessage2.h.what 				= kExpToServerMessage;

	gExpToServerMessage2.h.to 					= kNSpHostID;

	gExpToServerMessage2.h.messageLen 	= sizeof( ExpToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gExpToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gExpToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetExpFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	ExpToServerMessage 				  *theMessage = (ExpToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        name[kStringLength];
	char                        Exp[kStringLength];
	char                        rewardStr[kStringLength];
	TBoolean                    gotName;
	long                        expNum=0;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category==0)
	//	return;
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanGrantExperience)==false) // put in carbon 11/27
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(name,"");

	strcpy(Exp,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		Exp[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			name[k]='\0';
			strcpy(Exp,"");
			m=0;
		}

		if (gotName==false)
		{
			name[k]=theMessage->theText[j];
			k++;
		}

	}

	Exp[m]='\0';

	if (strcmp(name,"")==0)
		return;

	if (strcmp(Exp,"")==0)
		return;

	if (strlen(Exp)>19)
		return;

	tb_StringToNum(Exp,&expNum);

	if ((expNum<-500000) || (expNum>500000) || expNum==0)
	{
		nw_SendTextToClient(i, "Experience points out of range.", 0);
		return;
	}

	tb_NumToString(abs(expNum), Exp);

	strcpy(rewardStr,name);
	strcat(rewardStr," not found.  No experience rewarded.");

	for (j=0;j<strlen(name);j++)
		if (((int)name[j]>=65) && ((int)name[j]<=90)) // convert to lower case
			name[j]=(char)((int)name[j]+32);

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			if (strcmp(name,player[j].playerNameLC)==0)// merge 7/27
			{
				player[j].title=player[j].title+expNum;

				if (player[j].title>500000)
					player[j].title=500000;
				
				if (player[j].title<0)
					player[j].title=0;
				
				strcpy(rewardStr, Exp);

				if (expNum<0)
					strcat(rewardStr, " xp removed from ");
				else
					strcat(rewardStr, " xp rewarded to ");

				strcat(rewardStr, player[j].playerName);

				nw_SendTextToClient(i, rewardStr, 0);

				if (expNum>0)
					strcpy(rewardStr,"You gain ");
				else
					strcpy(rewardStr, "You lose ");

				strcat(rewardStr,Exp);

				if (expNum!=1 && expNum!=-1)
					strcat(rewardStr," experience points.");
				else
					strcat(rewardStr," experience point.");

				nw_SendTextToClient(j, rewardStr, 0);

				return;
			}

	nw_SendTextToClient(i, rewardStr, 0);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendNameChangeToHost(char SendText[kStringLength], UInt8 perma) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendNameToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gNameChangeToServerMessage2.h);

	gNameChangeToServerMessage2.h.what 				= kNameChangeToServerMessage;

	gNameChangeToServerMessage2.h.to 					= kNSpHostID;

	gNameChangeToServerMessage2.h.messageLen 	= sizeof( NameChangeToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gNameChangeToServerMessage2.newName,SendText);

	gNameChangeToServerMessage2.perm=perma;


	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gNameChangeToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetNameChangeFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	NameChangeToServerMessage 				  *theMessage = (NameChangeToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	int							cpt;
	char                        name[kStringLength];
	char                        newname[kStringLength];
	char                        newnameLC[kStringLength];
	char                        rewardStr[kStringLength];
	char						fileName[kStringLength];
	char						fileName2[kStringLength];
	char						oldName[kStringLength];
	FILE						*dummy;
	TBoolean                    gotName;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif


	if (pl_AuthorizedTo(player[i].creatureIndex,kCanChangeNames)==false)
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->newName,"")==0)
		return;

	strcpy(name,"");

	strcpy(newname,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->newName);j++)
	{

		newname[m]=theMessage->newName[j];
		m++;

		if (theMessage->newName[j]=='/')
		{
			gotName=true;
			name[k]='\0';
			strcpy(newname,"");
			m=0;
		}

		if (gotName==false)
		{
			name[k]=theMessage->newName[j];
			k++;
		}

	}

	newname[m]='\0';

	if (strcmp(name,"")==0)
		return;

	if (strcmp(newname,"")==0)
		return;

	if (strlen(newname)>14)
	{
		nw_SendTextToClient(i, "New name too long.", 0);
		return;
	}

	strcpy(rewardStr,name);

	strcat(rewardStr," not found.  Name change aborted.");

	for (j=0;j<strlen(name);j++)
		if (((int)name[j]>=65) && ((int)name[j]<=90)) // convert to lower case
			name[j]=(char)((int)name[j]+32);

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			if (strcmp(name,player[j].playerNameLC)==0)// merge 7/27
			{
				if (theMessage->perm==1)
				{
					strcpy(oldName, player[j].userName);

					strcpy(fileName2, ":Accounts:");
					strcat(fileName2, player[j].userName);

					strcpy(fileName, ":Accounts:");
					strcat(fileName, newname);
					dummy=fopen(fileName, "r");

					if (dummy!=NULL)
					{
						nw_SendTextToClient(i, "Name already exists", 0);
						fclose(dummy);
						return;
					}

					fclose(dummy);

					strcpy(player[j].userName, newname);
					strcpy(player[j].playerName, newname);
					strcpy(newnameLC, newname);

					for (cpt=0;cpt<strlen(newnameLC);cpt++)
						if (((int)newnameLC[cpt]>=65) && ((int)newnameLC[cpt]<=90)) // convert to lower case
							newnameLC[cpt]=(char)((int)newnameLC[cpt]+32);

					strcpy(player[j].playerNameLC, newnameLC);


					dummy=fopen(fileName, "w");

					fprintf(dummy, "%s", "dummy");

					fclose(dummy);

					fi_UpdateCCData(j, oldName, newname);

					fi_SavePlayerData(j);

					remove(fileName2);
				}
				else
					strcpy(player[j].playerNameTemp, newname);

				nw_SendClearName(j);

				return;
			}

	nw_SendTextToClient(i, rewardStr, 0);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendCustomTitleToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendNameToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gCustomTitleToServerMessage2.h);

	gCustomTitleToServerMessage2.h.what 				= kCustomTitleToServerMessage;

	gCustomTitleToServerMessage2.h.to 					= kNSpHostID;

	gCustomTitleToServerMessage2.h.messageLen 	= sizeof( CustomTitleToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gCustomTitleToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gCustomTitleToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetCustomTitleFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	CustomTitleToServerMessage 				  *theMessage = (CustomTitleToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        name[kStringLength];
	char                        newname[kStringLength];
	char                        rewardStr[kStringLength];
	TBoolean                    gotName;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif


	if (pl_AuthorizedTo(player[i].creatureIndex,kCanChangeNames)==false)  //Can change name <=> can change title
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(name,"");

	strcpy(newname,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		newname[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			name[k]='\0';
			strcpy(newname,"");
			m=0;
		}

		if (gotName==false)
		{
			name[k]=theMessage->theText[j];
			k++;
		}

	}

	newname[m]='\0';

	if (strcmp(name,"")==0)
		return;

	if (strcmp(newname,"")==0)
		return;

	if (strlen(newname)>20)
	{
		nw_SendTextToClient(i, "New title too long.", 0);
		return;
	}

	strcpy(rewardStr,name);

	strcat(rewardStr," not found.  Title unchanged.");

	for (j=0;j<strlen(name);j++)
		if (((int)name[j]>=65) && ((int)name[j]<=90)) // convert to lower case
			name[j]=(char)((int)name[j]+32);

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			if (strcmp(name,player[j].playerNameLC)==0)// merge 7/27
			{
				strcpy(player[j].titleCustom, newname);
				strcpy(rewardStr, "Custom title set to ");
				strcat(rewardStr, player[j].titleCustom);
				strcat(rewardStr," for player ");
				strcat(rewardStr, player[j].playerName);
				strcat(rewardStr, ".");
				nw_SendTextToClient(i, rewardStr, 0);
				return;
			}

	nw_SendTextToClient(i, rewardStr, 0);

}

#endif


// ----------------------------------------------------------------- //
void nw_SendAnnounceToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendExpToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gAnnounceToServerMessage2.h);

	gAnnounceToServerMessage2.h.what 				= kAnnounceToServerMessage;

	gAnnounceToServerMessage2.h.to 					= kNSpHostID;

	gAnnounceToServerMessage2.h.messageLen 	= sizeof( AnnounceToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gAnnounceToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gAnnounceToServerMessage2.h, kSendMethod);

}


#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetAnnounceFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	AnnounceToServerMessage 				  *theMessage = (AnnounceToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        radius[kStringLength];
	char                        announceStr[kStringLength];
	TBoolean                    gotName;
	long                        radNum=0;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanBroadcast)==false)
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(radius,"");

	strcpy(announceStr,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		announceStr[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			radius[k]='\0';
			strcpy(announceStr,"");
			m=0;
		}

		if (gotName==false)
		{
			radius[k]=theMessage->theText[j];
			k++;
		}

	}

	announceStr[m]='\0';

	if (strcmp(radius,"")==0)
		return;

	if (strcmp(announceStr,"")==0)
		return;

	tb_StringToNum(radius,&radNum);

	if ((radNum<2) || (radNum>kMapWidth) || (radNum>kMapHeight))
	{
		nw_SendTextToClient(i, "Radius out of range.", 0);
		return;
	}

	for (j=1;j<kMaxPlayers;j++)
		if (player[j].id!=0)
			if (fabs(creature[player[j].creatureIndex].row-creature[player[i].creatureIndex].row)<=radNum && fabs(creature[player[j].creatureIndex].col-creature[player[i].creatureIndex].col)<=radNum)
			{
				nw_SendSystemTextToClient(j, announceStr);
			}
}

#endif

// ----------------------------------------------------------------- //
void nw_SendRExpToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendExpToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gRExpToServerMessage2.h);

	gRExpToServerMessage2.h.what 				= kRExpToServerMessage;

	gRExpToServerMessage2.h.to 					= kNSpHostID;

	gRExpToServerMessage2.h.messageLen 	= sizeof( RExpToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gRExpToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gRExpToServerMessage2.h, kSendMethod);

}


#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetRExpFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	RExpToServerMessage 				  *theMessage = (RExpToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        radius[kStringLength];
	char                        expStr[kStringLength];
	char						rewardStr[kStringLength];
	TBoolean                   	gotName;
	long                        	radNum=0;
	long							expNum=0;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanBroadcast)==false)
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(radius,"");

	strcpy(expStr,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		expStr[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			radius[k]='\0';
			strcpy(expStr,"");
			m=0;
		}

		if (gotName==false)
		{
			radius[k]=theMessage->theText[j];
			k++;
		}

	}

	expStr[m]='\0';

	if (strcmp(radius,"")==0)
		return;

	if (strcmp(expStr,"")==0)
		return;

	tb_StringToNum(radius,&radNum);

	tb_StringToNum(expStr,&expNum);

	tb_NumToString(abs(expNum), expStr);

	if ((radNum<2) || (radNum>kMapWidth) || (radNum>kMapHeight))
	{
		nw_SendTextToClient(i, "Radius out of range.", 0);
		return;
	}

	for (j=1;j<kMaxPlayers;j++)
		if (j!=i)
			if (player[j].id!=0)
				if (fabs(creature[player[j].creatureIndex].row-creature[player[i].creatureIndex].row)<=radNum && fabs(creature[player[j].creatureIndex].col-creature[player[i].creatureIndex].col)<=radNum)
				{

					player[j].title=player[j].title+expNum;

					if (player[j].title>500000)
						player[j].title=500000;

					if (player[j].title<0)
						player[j].title=0;

					if (expNum>0)
						strcpy(rewardStr,"You gain ");
					else
						strcpy(rewardStr, "You lose ");

					strcat(rewardStr,expStr);

					if (expNum!=1 && expNum!=-1)
						strcat(rewardStr," experience points.");
					else
						strcat(rewardStr," experience point.");

					nw_SendTextToClient(j, rewardStr, 0);
				}

	strcpy(rewardStr, "");

	strcat(rewardStr, expStr);
	strcat(rewardStr, " experience ");

	if (expNum>0)
		strcat(rewardStr, "given in a ");
	else
		strcat(rewardStr, "removed in a ");

	strcat(rewardStr, radius);

	strcat(rewardStr, " tiles radius.");

	nw_SendTextToClient(i, rewardStr, 0);
}

#endif

// ----------------------------------------------------------------- //
void nw_SendWhisperToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendWhisperToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gWhisperToServerMessage2.h);

	gWhisperToServerMessage2.h.what 				= kWhisperToServerMessage;

	gWhisperToServerMessage2.h.to 					= kNSpHostID;

	gWhisperToServerMessage2.h.messageLen 	= sizeof( WhisperToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gWhisperToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gWhisperToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetWhisperFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	ExpToServerMessage 				  *theMessage = (ExpToServerMessage *) inMessage;
	int													j;
	int                         k;
	int                         m;
	char                        name[kStringLength];
	char                        whispertxt[kStringLength];
	char                        rewardStr[kStringLength];
	TBoolean                    gotName;
	long                        expNum=0;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if (pl_AuthorizedTo(player[i].creatureIndex,kCanWhisper)==false)
		return;

	player[i].lastCheckIn		= TickCount();

	if (strcmp(theMessage->theText,"")==0)
		return;

	strcpy(name,"");

	strcpy(whispertxt,"");

	gotName=false;

	k=0;

	m=0;

	for (j=0;j<strlen(theMessage->theText);j++)
	{

		whispertxt[m]=theMessage->theText[j];
		m++;

		if (theMessage->theText[j]=='/')
		{
			gotName=true;
			name[k]='\0';
			strcpy(whispertxt,"");
			m=0;
		}

		if (gotName==false)
		{
			name[k]=theMessage->theText[j];
			k++;
		}

	}

	whispertxt[m]='\0';

	if (strcmp(name,"")==0)
		return;

	if (strcmp(whispertxt,"")==0)
		return;


	for (j=0;j<strlen(name);j++)
		if (((int)name[j]>=65) && ((int)name[j]<=90)) // convert to lower case
			name[j]=(char)((int)name[j]+32);

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			if (strcmp(name,player[j].playerNameLC)==0)// merge 7/27
				if (!(pl_AuthorizedTo(player[j].creatureIndex, kIsDeveloper) && player[j].stealth))
				{
					nw_SendTextToClient(j, whispertxt, 0);
					strcpy(rewardStr, "Whispered \"");
					strcat(rewardStr, whispertxt);
					strcat(rewardStr, "\" to ");
					strcat(rewardStr, name);
					strcat(rewardStr, ".");
					nw_SendTextToClient(i, rewardStr, 0);
					return;
				}

	strcpy(rewardStr,name);

	strcat(rewardStr," not found.  Whisper impossible.");

	nw_SendTextToClient(i, rewardStr, 0);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendGiftToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendGiftToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	NSpClearMessageHeader(&gGiftToServerMessage2.h);

	gGiftToServerMessage2.h.what 				= kGiftToServerMessage;

	gGiftToServerMessage2.h.to 					= kNSpHostID;

	gGiftToServerMessage2.h.messageLen 	= sizeof( GiftToServerMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gGiftToServerMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gGiftToServerMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetGiftFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	GiftToServerMessage 				*theMessage = (GiftToServerMessage *) inMessage;
	int                         c;

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	//if (player[i].category==0)
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetOwner)==false) // put in carbon 11/27
		return;

	player[i].lastCheckIn		= TickCount();

	c=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

	if (c!=0)
	{

		if ((strcmp(theMessage->theText,"All")==0) || (strcmp(theMessage->theText,"all")==0) || (strcmp(theMessage->theText,"ALL")==0))
		{
			strcpy(containerGift[c],"");
			nw_SendTextToClient(i, "Owner set.", 0);
			return;
		}

		if (strlen(theMessage->theText)<48)
		{
			strcpy(containerGift[c],theMessage->theText);
		}
		else
			strcpy(containerGift[c],"");

		nw_SendTextToClient(i, "Owner set.", 0);
	}
	else
	{
		c=tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

		if (c!=0)
		{
			if ((strcmp(theMessage->theText,"All")==0) || (strcmp(theMessage->theText,"all")==0) || (strcmp(theMessage->theText,"ALL")==0))
			{
				strcpy(tracksGift[c],"");
				nw_SendTextToClient(i, "Owner set.", 0);
				return;
			}

			if (strlen(theMessage->theText)<48)
			{
				strcpy(tracksGift[c],theMessage->theText);
			}
			else
				strcpy(tracksGift[c],"");

			nw_SendTextToClient(i, "Owner set.", 0);
		}
	}


}

#endif

// ----------------------------------------------------------------- //
void nw_SendAddToParty(int  index, char theSendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	int																	i;
	char 					                      SendText[kStringLength]; //[32];
	int                                 temp;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendAddToParty");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(theSendText,"")==0)
		return;

	if (strlen(theSendText)>14)
		return;

	temp=strlen(theSendText);

	for (i=0;i<strlen(theSendText);i++)
		SendText[i]=theSendText[i];

	SendText[strlen(theSendText)]='\0';

	tb_DeleteStringPadding(SendText);

	for (i=0;i<strlen(SendText);i++)
		if (((int)SendText[i]>=65) && ((int)SendText[i]<=90)) // convert to lower case
			SendText[i]=(char)((int)SendText[i]+32);

	NSpClearMessageHeader(&gAddToPartyMessage2.h);

	gAddToPartyMessage2.h.what 				= kAddToPartyMessage;

	gAddToPartyMessage2.h.to 					= kNSpHostID;

	gAddToPartyMessage2.h.messageLen 	= sizeof( AddToPartyMessage) + (sizeof(char) * (strlen(SendText)));

	gAddToPartyMessage2.index=(UInt8)index;;

	strcpy(gAddToPartyMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gAddToPartyMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetAddToParty(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	AddToPartyMessage 				  *theMessage = (AddToPartyMessage *) inMessage;
	int													j;
	TBoolean                    endFound;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetAddToParty");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	endFound=false;

	for (j=0;j<kStringLength;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad party name from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->index<0) || (theMessage->index>15))
		return;

	if (strlen(theMessage->theText)>14)
		return;

	strcpy(player[i].friendName[theMessage->index],theMessage->theText);

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
TBoolean nw_SendTrackingInfosToClient(int toID) // server
// ----------------------------------------------------------------- //

{
	int		clen=0, cpt=0, cpt2;
	int		i, j;//, l;
	int		level, range;
	int		row, col;

	int		temptab[200];
	int		tmp;

	level = sk_GetSkillLevel(player[toID].skill[kTracking].level);

	range=8+level*2;

	if (range>27)  //JS mini map bounds
		range=27;
	
	for (row=creature[player[toID].creatureIndex].row-range; row<creature[player[toID].creatureIndex].row+range; row++)
		for (col=creature[player[toID].creatureIndex].col-range; col<creature[player[toID].creatureIndex].col+range; col++)
			if (row>0)
				if (row<kMapWidth)
					if (col>0)
						if (col<kMapHeight)
							if (creatureMap[row][col]!=0)
							{
								i=creatureMap[row][col];
								
								if (cr_DistanceBetween(i, player[toID].creatureIndex) < 9)	continue; //on screen, don't track
								
								if (creature[i].playerIndex!=0) //tracking player or morphed QChar
								{
									if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer) || (pl_AuthorizedTo(player[i].creatureIndex, kIsQuestCharacter) && player[i].noTrack==false))
										if (player[i].stealth==false)
										{
											if (player[i].morph!=0)
											{
												if (level>=creatureInfo[player[i].morph].trackingLevel)
												{
													temptab[cpt]=i;
													cpt++;	
												}
											}
											else if (level>=creatureInfo[kHuman].trackingLevel)
											{
												temptab[cpt]=i;
												cpt++;
											}
											

											if (cpt>199)
												break;
										}
								}
								else //tracking creature
								{
									if (level>= creatureInfo[creature[i].id].trackingLevel)
									{
										temptab[cpt]=i;
										cpt++;

										if (cpt>199)
											break;
									}
								}
							}

	/*if (level>creatureInfo[kHuman].trackingLevel)
		for (i=0; i<kMaxPlayers; i++)
			if (player[i].online)
				if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer) || (pl_AuthorizedTo(player[i].creatureIndex, kIsQuestCharacter) && player[i].noTrack==false))
					if (player[i].stealth==false)
					{
						if (cr_DistanceBetween(player[i].creatureIndex, player[toID].creatureIndex) < range)
							if (cr_DistanceBetween(player[i].creatureIndex, player[toID].creatureIndex) > 8)
							{
								if (player[i].morph!=0)
								{
									if (level>creatureInfo[player[i].morph].trackingLevel)
									{
										temptab[cpt]=i;
										cpt++;	
									}
								}
								else if (level>creatureInfo[kHuman].trackingLevel)
								{
									temptab[cpt]=i;
									cpt++;
								}
								

								if (cpt>199)
									break;
							}

					}

	if (cpt<200)
		for (i=5000; i<kMaxCreatures; i++)
			if (creature[i].id!=0)
			{
				if (cr_DistanceBetween(i, player[toID].creatureIndex) < range)
					if (cr_DistanceBetween(i, player[toID].creatureIndex) > 8)
						if (creatureInfo[creature[i].id].trackingLevel <= level)
						{
							temptab[cpt]=i;
							cpt++;

							if (cpt>199)
								break;
						}
			}*/

	if (cpt>0)
		for (i=0; i<level; i++)
			if (tb_Rnd(1,10)<7)
			{

				cpt2=tb_Rnd(0,cpt-1);

				j=temptab[cpt2];

				if (j==0)
					continue;

				if (j<5000) //player
					j=player[j].creatureIndex;

				tmp=creature[player[toID].creatureIndex].row-creature[j].row;

				if (tmp<0)
					tmp=100+fabs(tmp);

				gTrackingInfosToClientMessage2.trackInfo[clen]=(UInt8)tmp;

				clen++;


				tmp=creature[player[toID].creatureIndex].col-creature[j].col;

				if (tmp<0)
					tmp=100+fabs(tmp);

				gTrackingInfosToClientMessage2.trackInfo[clen]=(UInt8)tmp;

				clen++;

				gTrackingInfosToClientMessage2.trackInfo[clen]=creatureInfo[creature[j].id].size;
				
				if (creature[j].playerIndex!=0 && player[creature[j].playerIndex].morph!=0)
				{
					gTrackingInfosToClientMessage2.trackInfo[clen]=creatureInfo[player[j].morph].size;
				}

				if ((creature[j].playerIndex!=0 && player[creature[j].playerIndex].morph==0) || creature[j].npcIndex!=0)
					gTrackingInfosToClientMessage2.trackInfo[clen]+=4;
				else
					gTrackingInfosToClientMessage2.trackInfo[clen]+=8;

				clen++;

				gTrackingInfosToClientMessage2.trackInfo[clen]=255; //} marker

				temptab[cpt2]=0;

				if (clen>3*kMaxTracked)
					break;
			}

	if (clen==0)
	{
		nw_SendDisplayMessage(toID, kTrackingFailed);
		return (false);
	}


	NSpClearMessageHeader(&gTrackingInfosToClientMessage2.h);

	gTrackingInfosToClientMessage2.h.what 				= kTrackingInfosToClientMessage;
	gTrackingInfosToClientMessage2.h.to 					= player[toID].id;
	gTrackingInfosToClientMessage2.h.messageLen 	= sizeof( TrackingInfoToClientMessage) + clen;

#ifdef _SERVERONLY
	messagesSent++;
	bytesSent=bytesSent+gTrackingInfosToClientMessage2.h.messageLen;
#endif

	if (gNetGame!=NULL)
	{
		NSpMessage_Send(gNetGame, &gTrackingInfosToClientMessage2.h, kSendMethod);
		return (true);
	}
	else
	{
		nw_RestartServer();
		return (false);
	}


}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_GetTrackingInfosFromServer(NSpMessageHeader *inMessage)
// ----------------------------------------------------------------- //

{
	TrackingInfoToClientMessage2				*theMessage=(TrackingInfoToClientMessage2 *) inMessage;
	int											cpt=0, cpt2=0;

	currentlyTracking=TickCount()+2*60;

	while (theMessage->trackInfo[cpt]!=255)
	{
		localTracking[cpt2][0]=theMessage->trackInfo[cpt++];
		localTracking[cpt2][1]=theMessage->trackInfo[cpt++];
		localTracking[cpt2][2]=theMessage->trackInfo[cpt++];
		cpt2++;
	}

	monstersTracked=cpt2;

	trackingRow=playerRow;
	trackingCol=playerCol;

	//tb_PlaySound(301); //JS Tracking sound
	gr_RenderMapWindow(true);

}

#endif

#ifdef _SERVER
/*
// ----------------------------------------------------------------- //
void nw_SendUpdateRequired(int toID) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char								errorMessage[kStringLength];
	char								errorNumber[kStringLength];
	FILE								*theFile;
	char								url[kStringLength];

	theFile=fopen("Updater URL", "r");

	if (theFile!=NULL)
	{
		fseek( theFile, 0L, SEEK_SET );

		fgets(url, kStringLength, theFile);

		fclose(theFile);
	}
	else strcpy(url, "http://oberin.com/update/main.obu");


	NSpClearMessageHeader(&gVersionToClientMessage.h);

	gVersionToClientMessage.h.what 				= kVersionToClientMessage;

	gVersionToClientMessage.h.to 					= toID;

	gVersionToClientMessage.h.messageLen 	= sizeof( gVersionToClientMessage);

	gVersionToClientMessage.version						= kServerVersion;

	strcpy(gVersionToClientMessage.url, url);

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gVersionToClientMessage.h.messageLen;

#endif

	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gVersionToClientMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendVersionToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}

		//else fi_WriteToErrorLog("Update sent");
	}
	else
		nw_RestartServer();

}

#endif

#ifndef _SERVER
// ----------------------------------------------------------------- //
void nw_GetUpdateRequired(NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	AlertStdAlertParamRec		paramRec;
	OSStatus					theError;
	VersionToClientMessage 		*theMessage = (VersionToClientMessage *) inMessage;
	char						errorMessage[kStringLength];
	char						errorNumber[kStringLength];
	short						itemHit=0;
	Str255						updateTitle="\pNew update available", updateString="\pThis version of Oberin needs to be updated. Click on 'OK' to begin the update process.";

	gr_HideAllWindows();

	if (inMessage->version<=kClientVersion)
	{
		strcpy(errorMessage, "nw_GetUpdateRequired : invalid server version : ");
		tb_NumToString(inMessage->version, errorNumber);
		strcat(errorMessage, errorNumber);
		fi_WriteToErrorLog(errorMessage);
		tb_Halt();
	}

	paramRec.movable			= false;

	paramRec.helpButton			= false;
	paramRec.filterProc			= NULL;
	paramRec.defaultText		= (StringPtr) kAlertDefaultOKText;
	paramRec.cancelText			= (StringPtr) kAlertDefaultCancelText;
	paramRec.otherText			= NULL;
	paramRec.defaultButton	= kAlertStdAlertOKButton;
	paramRec.cancelButton		= kAlertStdAlertCancelButton;
	paramRec.position				= kWindowDefaultPosition;

	StandardAlert(kAlertNoteAlert, updateTitle, updateString, &paramRec, &itemHit);

	if (itemHit==1)
	{
		gUpdating=true;
		sh_CloseResFiles();
		up_Update(theMessage->version, theMessage->url);
	}

	else Done=true;

}

#endif
*/
// ----------------------------------------------------------------- //
void nw_SendUpdateRequired(int toID) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char								errorMessage[kStringLength];
	char								errorNumber[kStringLength];
	FILE								*theFile;
	char								url[kStringLength];

	theFile=fopen("Updater URL", "r");

	if (theFile!=NULL)
	{
		fseek( theFile, 0L, SEEK_SET );

		fgets(url, kStringLength, theFile);

		fclose(theFile);
	}
	else
		strcpy(url, "http://oberin.com/update/mainx.obu");

	NSpClearMessageHeader(&gVersionToClientMessage.h);

	gVersionToClientMessage.h.what 				= kVersionToClientMessage;

	gVersionToClientMessage.h.to 					= toID;

	gVersionToClientMessage.h.messageLen 	= sizeof( gVersionToClientMessage);

	gVersionToClientMessage.version						= kServerVersion;

	strcpy(gVersionToClientMessage.url, url);

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gVersionToClientMessage.h.messageLen;

#endif

	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gVersionToClientMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendVersionToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}

		//else fi_WriteToErrorLog("Update sent");
	}
	else
		nw_RestartServer();

}

#endif

#ifndef _SERVER
// ----------------------------------------------------------------- //
void nw_GetUpdateRequired(NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	AlertStdAlertParamRec		paramRec;
	OSStatus					theError;
	VersionToClientMessage 		*theMessage = (VersionToClientMessage *) inMessage;
	char						errorMessage[kStringLength];
	char						errorNumber[kStringLength];
	short						itemHit=0;
	Str255						updateTitle="\pNew update available", updateString="\pThis version of Oberin needs to be updated. Click on 'OK' to begin the update process.";

	gr_HideAllWindows();

	if (inMessage->version<=kClientVersion)
	{
		strcpy(errorMessage, "nw_GetUpdateRequired : invalid server version : ");
		tb_NumToString(inMessage->version, errorNumber);
		strcat(errorMessage, errorNumber);
		fi_WriteToErrorLog(errorMessage);
		tb_Halt();
	}

	paramRec.movable			= false;

	paramRec.helpButton			= false;
	paramRec.filterProc			= NULL;
	paramRec.defaultText		= (StringPtr) kAlertDefaultOKText;
	paramRec.cancelText			= (StringPtr) kAlertDefaultCancelText;
	paramRec.otherText			= NULL;
	paramRec.defaultButton	= kAlertStdAlertOKButton;
	paramRec.cancelButton		= kAlertStdAlertCancelButton;
	paramRec.position				= kWindowDefaultPosition;

	StandardAlert(kAlertNoteAlert, updateTitle, updateString, &paramRec, &itemHit);

	if (itemHit==1)
	{
		gUpdating=true;
		sh_CloseResFiles();
		up_Update(theMessage->version, theMessage->url);
	}

	else Done=true;

}

#endif



// ----------------------------------------------------------------- //
void nw_SendLocateToHost(char SendText[kStringLength]) // client
// ----------------------------------------------------------------- //

{
	OSStatus 														status;
	//char																*theText;
	int																	i;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendLocateToHost");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (strcmp(SendText,"")==0)
		return;

	tb_DeleteStringPadding(SendText);

	for (i=0;i<strlen(SendText);i++)
		if (((int)SendText[i]>=65) && ((int)SendText[i]<=90)) // convert to lower case
			SendText[i]=(char)((int)SendText[i]+32);

	NSpClearMessageHeader(&gLocateMessage2.h);

	gLocateMessage2.h.what 				= kLocateMessage;

	gLocateMessage2.h.to 					= kNSpHostID;

	gLocateMessage2.h.messageLen 	= sizeof( LocateMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gLocateMessage2.theText,SendText);

	if (gNetGame!=NULL)
		status = NSpMessage_Send(gNetGame, &gLocateMessage2.h, kSendMethod);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetLocateFromClient(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	LocateMessage 				*theMessage = (LocateMessage *) inMessage;
	int										j;
	char                  textMessage[kStringLength];
	//int                   k;
	int                   row;
	int                   col;
	char                  temp[kStringLength];
	TBoolean              endFound;

//gr_ShowDebugLocation(56); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	endFound=false;

	for (j=0;j<kStringLength;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad locate text from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if ((creature[player[i].creatureIndex].row>1842) || (creature[player[i].creatureIndex].col>1842))
	{
		strcpy(textMessage,"Your orb doesn't work here.");
		nw_SendTextToClient(i, textMessage, 0);
		return;
	}

	for (j=0;j<kMaxPlayers;j++)
		if (strcmp(theMessage->theText,player[j].playerNameLC)==0)
			//if (player[j].category==0)  //018
			if (pl_AuthorizedTo(player[j].creatureIndex,kRegularPlayer)) // put in carbon 11/27
				//if (player[j].stealth==false)
			{
				strcpy(textMessage,player[j].playerName);
				//strcat(textMessage,": ");
				//for (k=0;k<strlen(textMessage);k++)
				//	if (textMessage[k]=='*')
				//		textMessage[k]=' ';


				if ((creature[player[j].creatureIndex].row>1842) || (creature[player[j].creatureIndex].col>1842) || (player[j].stealth))
				{
					strcat(textMessage,": Location unclear.");
					nw_SendTextToClient(i, textMessage, 0);
					return;
				}

				if (strcmp(player[j].playerNameTemp, "None")!=0 && strcmp(player[j].playerNameTemp, "none")!=0)
				{
					nw_SendDisplayMessage(i,kPlayerNotOnline);
					return;
				}
				
				strcat(textMessage," is roughly ");

				if (creature[player[j].creatureIndex].row>=creature[player[i].creatureIndex].row)
				{
					row=creature[player[j].creatureIndex].row-creature[player[i].creatureIndex].row;
					tb_IntegerToString(row,textMessage);
					strcat(textMessage," N and ");
				}
				else
				{
					row=creature[player[i].creatureIndex].row-creature[player[j].creatureIndex].row;
					tb_IntegerToString(row,textMessage);
					strcat(textMessage," S and ");
				}

				if (creature[player[j].creatureIndex].col>creature[player[i].creatureIndex].col)
				{
					col=creature[player[j].creatureIndex].col-creature[player[i].creatureIndex].col;
					tb_IntegerToString(col,temp);
					strcat(textMessage,temp);
					strcat(textMessage," E");
				}
				else
				{
					col=creature[player[i].creatureIndex].col-creature[player[j].creatureIndex].col;
					tb_IntegerToString(col,temp);
					strcat(textMessage,temp);
					strcat(textMessage," W");
				}
				
				strcat(textMessage," of your position.");

				pl_GetLocationName(temp,creature[player[j].creatureIndex].row,creature[player[j].creatureIndex].col);

				if (strcmp(temp,"")!=0)
				{
					strcat(textMessage," - ");	
					strcat(textMessage,temp);
				}

				nw_SendTextToClient(i, textMessage, 0);

				return;
			}

	// Position can't be determined
	nw_SendDisplayMessage(i,kPlayerNotOnline);

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendCompressedTextToClient(int toID, char textMessage[kStringLength], int location) // server
// ----------------------------------------------------------------- //

{
	OSStatus 														status=noErr;
	int																	i;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];
	//UInt16                              checksum;
	char																uncompressed[256];
	int																	ulen;
	int																	clen=0;

	if (strcmp(textMessage,"")==0)
		return;

	if (player[toID].id==0)
	{
		fi_WriteToErrorLog("Bad player id - nw_SendCompressedTextToClient");
		return;
	}

	if (player[toID].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (location!=0)
		uncompressed[0] = (UInt8)creature[location].id;
	else
		uncompressed[0] = 0;

	uncompressed[1]=location/256;

	uncompressed[2]=location-(uncompressed[1]*256);

	ulen=3;

	for ( i = 0; i < strlen(textMessage); i++)
	{
		uncompressed[i+3]=(UInt8)textMessage[i];
		ulen++;
	}

	//nw_CompressData((char*)&uncompressed[0], ulen, (char*)&gCompressedTextToClientMessage2.theStream[0], &clen, CMP_ASCII);//CMP_BINARY);//CMP_ASCII);

	if (clen>=256)
	{
		fi_WriteToErrorLog("Compressed message too long - nw_SendCompressedTextToClient");
		return;
	}

	NSpClearMessageHeader(&gCompressedTextToClientMessage2.h);

	gCompressedTextToClientMessage2.h.what 				= kCompressedTextToClientMessage;
	gCompressedTextToClientMessage2.h.to 					= player[toID].id;
	gCompressedTextToClientMessage2.h.messageLen 	= sizeof( CompressedTextToClientMessage) + clen;

#ifdef _SERVERONLY
	messagesSent++;
	bytesSent=bytesSent+gCompressedTextToClientMessage2.h.messageLen;

	if (gCompressedTextToClientMessage2.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendCompressedTextToClient");

#endif

	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gCompressedTextToClientMessage2.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendCompressedTextToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendTextToClient(int toID, char textMessage[kStringLength], int location) // server
// ----------------------------------------------------------------- //

{
//	TextToClientMessage								  *gTextToClientMessage;
	OSStatus 														status=noErr;
//	char																*theText;
	int																	i;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];
	UInt16                              checksum;
	//char																compressed[256];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendTextToClient");
#endif

	if (strcmp(textMessage,"")==0)
		return;

	if (player[toID].id==0)
	{
		fi_WriteToErrorLog("Bad player id - nw_SendTextToClient");
		return;
	}

// gs take out 2/22/03 //	nw_SendCompressedTextToClient(toID, textMessage, location); // test
	if (player[toID].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	NSpClearMessageHeader(&gTextToClientMessage2.h);

	gTextToClientMessage2.h.what 				= kTextToClientMessage;
	gTextToClientMessage2.h.to 					= player[toID].id;
	gTextToClientMessage2.h.messageLen 	= sizeof( TextToClientMessage) + (sizeof(char) * (strlen(textMessage)));
	gTextToClientMessage2.location 			= (UInt16)location;//location;

	if (location!=0)
		gTextToClientMessage2.id 			    = (UInt8)creature[location].id;//location;
	else
		gTextToClientMessage2.id 			    = 0;//location;

	strcpy(gTextToClientMessage2.theText,textMessage);

	checksum=location;

	for ( i = 0; i < strlen(textMessage); i++)
		checksum=checksum+(UInt8)textMessage[i];

	gTextToClientMessage2.checksum 			= checksum;//location;

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gTextToClientMessage2.h.messageLen;

	if (gTextToClientMessage2.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendTextToClient");

#endif

	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gTextToClientMessage2.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendTextToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetTextFromServer(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY
	TextToClientMessage 				*theMessage = (TextToClientMessage *) inMessage;
	char                        chat[kStringLength];
	TBoolean										endFound;
	int													i;
	UInt16                      checksum;
	unsigned long               currentTime=TickCount();
	char                        creatureName[kStringLength];
	int													slot;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetTextFromServer");
#endif

	endFound=false;

	for (i=0;i<kStringLength;i++)
		if (theMessage->theText[i]=='\0')
		{
			endFound=true;
			break;
		}

	if (!endFound)
	{
		fi_WriteToErrorLog("Bad text from server");
		return;
	}

	checksum=theMessage->location;

	for ( i = 0; i < strlen(theMessage->theText); i++)
		checksum=checksum+(UInt8)theMessage->theText[i];

	if (checksum!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad text checksum");
		fi_WriteToErrorLog(theMessage->theText);
		return;
	}

	if (theMessage->location==0) //255)
	{
		gr_AddText(theMessage->theText,false,true);
		return;
	}

	if (theMessage->location>=kMaxCreatures)
		//if (theMessage->location>=kMaxPlayers)
	{
		fi_WriteToErrorLog("Bad Text location from server");
		return;
	}

	//strcpy(chatText[theMessage->location],theMessage->theText);
	//chatDecay[theMessage->location]=TickCount()+(60*6);

	//if (chatLookUpTable[theMessage->location]!=0)
	//	{
	//		strcpy(chatString[chatLookUpTable[theMessage->location]],"");
	//		chatDecay[chatLookUpTable[theMessage->location]]=0;
	//		chatLookUpTable[theMessage->location]=0;
	//	}

	slot=0;

	for (i=1;i<kMaxChatSlots;i++)
		if ((strcmp(chatString[i],"")==0) || (currentTime>chatDecay[i]))
		{
			slot=i;
			break;
		}

	if (slot==0)	// no slots available ... skip
	{
		fi_WriteToErrorLog("No chat slot found, skipping.");
		return;
	}

	for (i=slot+1; i<kMaxChatSlots; i++)  //Clear all text belonging to the same PC (chat lag fix)
		if (chatCreatureID[i]==theMessage->location)
		{
			strcpy(chatString[i],"");
			chatDecay[i]=0;
			chatCreatureID[i]=0;
		}

	// {
	// 	if (chatReverseLookup[i]!=0)
	// 		chatLookUpTable[chatReverseLookup[i]]=0;
	// 	chatReverseLookup[i]=theMessage->location;
	//   chatLookUpTable[theMessage->location]=i;
	//   strcpy(chatString[i],theMessage->theText);
	//   chatDecay[i]=TickCount()+(60*8);
	//   break;
	// }

	strcpy(chatString[slot],theMessage->theText);

	chatDecay[slot]=TickCount()+(60*8);

	chatCreatureID[slot]=theMessage->location;

	if (theMessage->id==kHuman)
	{
		if (!((strcmp(playerNames[theMessage->location],"")==0) || (strcmp(playerNames[theMessage->location],"request")==0)))
		{
			strcpy(chat,"(");
			strcat(chat,playerNames[theMessage->location]);
			strcat(chat,"): ");
			strcat(chat,theMessage->theText);
		}
		else
			strcpy(chat,theMessage->theText);
	}
	else
	{
		tx_GetCreatureName(theMessage->id, creatureName);
		strcpy(chat,"(");
		strcat(chat,creatureName);
		strcat(chat,"): ");
		strcat(chat,theMessage->theText);
	}

	gr_AddToChatLog(chat, kChatMessage);

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendQuestDescription(int toID, int gold, int experience, int item, int count, char textMessage[kQuestStringLength]) // server
// ----------------------------------------------------------------- //

{
	//QuestDescriptionMessage						  *gQuestDescriptionMessage;
	OSStatus 														status=noErr;
	//char																*theText;
	//int																	i;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendQuestDescription");
#endif

	if (strcmp(textMessage,"")==0)
		return;

	if (player[toID].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	//gQuestDescriptionMessage = malloc( sizeof( QuestDescriptionMessage) + ( sizeof(char) * (strlen(textMessage))));

	//if (gQuestDescriptionMessage!=NULL)
	//	{
	NSpClearMessageHeader(&gQuestDescriptionMessage2.h);

	gQuestDescriptionMessage2.h.what 				= kQuestDescriptionMessage;

	gQuestDescriptionMessage2.h.to 					= player[toID].id;

	gQuestDescriptionMessage2.h.messageLen 	= sizeof( QuestDescriptionMessage) + (sizeof(char) * (strlen(textMessage)));

	gQuestDescriptionMessage2.gold 			    = (UInt16)gold;

	gQuestDescriptionMessage2.experience 	  = (UInt16)experience;

	gQuestDescriptionMessage2.item 	        = (UInt16)item;

	gQuestDescriptionMessage2.count 	      = (UInt16)count;

	strcpy(gQuestDescriptionMessage2.theText,textMessage);

	//theText=&gQuestDescriptionMessage->theText[0];

	//for ( i = 0; i < strlen(textMessage); i++)
	//	{
	//		*theText=textMessage[i];
	//  	theText++;
	//	}
	//*theText='\0';

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gQuestDescriptionMessage2.h.messageLen;

	if (gQuestDescriptionMessage2.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendQuestDescription");

#endif

	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gQuestDescriptionMessage2.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendQuestToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

	//if (gQuestDescriptionMessage!=NULL)
	//  free(gQuestDescriptionMessage);
	//	}

}

#endif

// ----------------------------------------------------------------- //
void nw_GetQuestDescription(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY
	QuestDescriptionMessage 				*theMessage = (QuestDescriptionMessage *) inMessage;
	TBoolean                        endFound;
	int                             j;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetQuestDescription");
#endif

	endFound=false;

	for (j=0;j<1024;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad quest text from server");
		return;
	}

	strcpy(questDescription,theMessage->theText);

	questGold         = theMessage->gold;
	questExperience   = theMessage->experience;
	questItem         = theMessage->item;
	questCount        = theMessage->count;

	fi_WriteToQuestFile(currentQuest, questGold, questExperience, questItem, questCount, questDescription);

	gr_RenderQuestWindow();

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendSystemTextToClient(int toID, char SendText[kStringLength]) // server
// ----------------------------------------------------------------- //

{
	//SystemTextToClientMessage						*gSystemTextToClientMessage;
	OSStatus 														status=noErr;
	//char																*theText;
	//int																	i;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendSystemTextToClient");
#endif

	if (player[toID].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	//gSystemTextToClientMessage = malloc( sizeof( SystemTextToClientMessage) + ( sizeof(char) * (strlen(SendText))));

	//if (gSystemTextToClientMessage!=NULL)
	//	{
	NSpClearMessageHeader(&gSystemTextToClientMessage2.h);

	gSystemTextToClientMessage2.h.what 				= kSystemTextToClientMessage;

	gSystemTextToClientMessage2.h.to 					= player[toID].id;

	gSystemTextToClientMessage2.h.messageLen 	= sizeof( SystemTextToClientMessage) + (sizeof(char) * (strlen(SendText)));

	strcpy(gSystemTextToClientMessage2.theText,SendText);

	//theText=&gSystemTextToClientMessage->theText[0];

	//for ( i = 0; i < strlen(SendText); i++)
	//	{
	//		*theText=SendText[i];
	//  	theText++;
	//	}
	//*theText='\0';

#ifdef _SERVERONLY
	messagesSent++;

	bytesSent=bytesSent+gSystemTextToClientMessage2.h.messageLen;

	if (gSystemTextToClientMessage2.h.messageLen>2000)
		fi_WriteToErrorLog("Large message in nw_SendSystemTextToClient");

#endif
	if (gNetGame!=NULL)
	{
		status = NSpMessage_Send(gNetGame, &gSystemTextToClientMessage2.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendSystemTextToClient: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

	//if (gSystemTextToClientMessage!=NULL)
	//  free(gSystemTextToClientMessage);
	//	}

}

#endif

// ----------------------------------------------------------------- //
void nw_GetSystemTextFromServer(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	SystemTextToClientMessage 				*theMessage = (SystemTextToClientMessage *) inMessage;
	TBoolean                          endFound;
	int                               j;
	char                              name[kStringLength];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetSystemTextFromServer");
#endif

	endFound=false;

	for (j=0;j<kStringLength;j++)
		if (theMessage->theText[j]=='\0')
		{
			endFound=true;
			break;
		}

	if (endFound==false)
	{
		fi_WriteToErrorLog("Bad system text");
		return;
	}

	gr_AddText(theMessage->theText,true,true);

	if (strlen(theMessage->theText)>10)
	{
		for (j=0;j<10;j++)
			name[j]=theMessage->theText[j];

		name[10]='\0';

		tb_MakeLowerCase(name);

		if (strcmp(name,"notice: ")==0)
			tb_Beep(9998);
		else
		{
			for (j=0;j<10;j++)
				name[j]=theMessage->theText[j];

			name[10]='\0';

			tb_MakeLowerCase(name);

			if (strcmp(name,"notice: ")==0)
				tb_Beep(9998);
		}
	}

	if (chimeOn)
		if (theMessage->theText[0]=='H')
			if (theMessage->theText[1]=='e')
				if (theMessage->theText[2]=='l')
					if (theMessage->theText[3]=='p')
						tb_Beep(200);
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendNPCTextToClient(int toID, int theTextID, int location) // server
// ----------------------------------------------------------------- //

{
	TextToClientMessage								  *gTextToClientMessage;
	OSStatus 														status=noErr;
	char																*theText;
	int																	i;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendNPCTextToClient");
#endif

	if (player[toID].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	gTextToClientMessage = malloc( sizeof( TextToClientMessage) + ( sizeof(char) * (strlen(npc[theTextID].talk))));

	if (gTextToClientMessage!=NULL)
	{
		NSpClearMessageHeader(&gTextToClientMessage->h);
		gTextToClientMessage->h.what 				= kTextToClientMessage;
		gTextToClientMessage->h.to 					= player[toID].id;
		gTextToClientMessage->h.messageLen 	= sizeof( TextToClientMessage) + (sizeof(char) * (strlen(npc[theTextID].talk)));
		gTextToClientMessage->location 			= location;

		theText=&gTextToClientMessage->theText[0];

		for ( i = 0; i < strlen(npc[theTextID].talk); i++)
		{
			*theText=npc[theTextID].talk[i];
			theText++;
		}

		*theText='\0';

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gTextToClientMessage->h.messageLen;

		if (gTextToClientMessage->h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendNPCTextToClient");

#endif
		if (gNetGame!=NULL)
		{
			status = NSpMessage_Send(gNetGame, &gTextToClientMessage->h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendNPCTextToClient: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();

		if (gTextToClientMessage!=NULL)
			free(gTextToClientMessage);
	}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendMake(UInt16 item) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendMake");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gMakeMessage.h);
		gMakeMessage.h.what 				= kMakeMessage;
		gMakeMessage.h.to 					= kNSpHostID;
		gMakeMessage.h.messageLen 	= sizeof(gMakeMessage);
		gMakeMessage.item						=	item;
		gMakeMessage.checksum			  =	item+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gMakeMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetMake(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	MakeMessage 					*theMessage = (MakeMessage *) inMessage;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetMake");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if ((theMessage->item+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad make from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->item<1) || (theMessage->item>kMaxItemsUsed))
		return;

	if (player[i].meditating) //Crafting an item breaks meditation
	{
		player[i].meditating	=false;
		nw_SendDisplayMessage(i,kConcentrationDisturbed);
	}

	if (player[i].stealth) // player interrupted stealth
	{
		if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer))
		{
			player[i].stealth		=	false;
			player[i].stealthCool	=	TickCount()+3*60;
			nw_SendDisplayMessage(i,kLeavingStealth);
		}
	}

	creature[player[i].creatureIndex].hiding						=	false;  // 018

	creature[player[i].creatureIndex].attackTarget			=	0;
	creature[player[i].creatureIndex].nextAttackTime		= 0;
	creature[player[i].creatureIndex].nextStrikeTime		=	TickCount();
	creature[player[i].creatureIndex].numberOfStrikes		=	100;

	player[i].gathering								=	20;//30;
	player[i].gatherType							= 255;	// constant used to indicate making an item
	player[i].gatherRow								=	theMessage->item;	// gatherRow used to indicate item ID
	player[i].gatherCol								=	0;

	if (itemList[theMessage->item].makeSkill==kWoodworking)
	{
		cr_AddSound(player[i].creatureIndex, kWoodworkingSnd);
		//creature[player[i].creatureIndex].soundFX				= kWoodworkingSnd;
		//creature[player[i].creatureIndex].soundFXTime		= TickCount();
	}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendSpecialAttack(int item) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendSpecialAttack");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gSpatMessage.h);
		gMakeMessage.h.what 				= kSpecialAttackMessage;
		gMakeMessage.h.to 					= kNSpHostID;
		gMakeMessage.h.messageLen 	= sizeof(gSpatMessage);
		gMakeMessage.item						=	item;
		gMakeMessage.checksum			  =	item+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gMakeMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetSpecialAttack(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	SpatMessage 					*theMessage = (SpatMessage *) inMessage;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetMake");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if ((theMessage->item+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad spat from client");
		return;
	}

	player[i].lastCheckIn		= TickCount();

	if (theMessage->item==0)
	{
		cr_ClearSpat(player[i].creatureIndex, false);	
	}
	
	if ((theMessage->item<1) || (theMessage->item>kMaxItemsUsed))
		return;

	pl_AddSpecialAttack(i, theMessage->item);
}

#endif

// ----------------------------------------------------------------- //
void nw_SendRepair(int slot) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendRepair");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gRepairMessage.h);
		gRepairMessage.h.what 				= kRepairMessage;
		gRepairMessage.h.to 					= kNSpHostID;
		gRepairMessage.h.messageLen 	= sizeof(gRepairMessage);
		gRepairMessage.slot						=	(UInt8)slot;

		gRepairMessage.checksum			  = slot+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gRepairMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetRepair(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	RepairMessage 					*theMessage = (RepairMessage *) inMessage;
	int                     theRepair;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetRepair");
#endif

	if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((theMessage->slot+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad repair message");
		return;
	}

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	if ((theMessage->slot<0) || (theMessage->slot>=kMaxInventorySize))  // paranoid error checking
		return;

	if (!(it_IsArmor(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, false) || (it_IsWeapon(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID))))

		pl_CheckForActionInterrupted(i, false);

	creature[player[i].creatureIndex].nextStrikeTime    = TickCount();

	creature[player[i].creatureIndex].numberOfStrikes   = 3;

	if (itemList[creature[player[i].creatureIndex].inventory[theMessage->slot].itemID].makeSkill==kWoodworking)
		creature[player[i].creatureIndex].finalStrikeSound  = kWoodworkingSnd;
	else creature[player[i].creatureIndex].finalStrikeSound  = kBlacksmithySnd;

//  if (tb_Rnd(1,100)==1) // bad luck ... repair failed ... item destroyed
//    {
//      if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount>127)
//        nw_SendToClientDropFromInventorySlot(i, kDamageItem, theMessage->slot, 127);
//      else
//        nw_SendToClientDropFromInventorySlot(i, kDamageItem, theMessage->slot, 0);
//      creature[player[i].creatureIndex].inventory[theMessage->slot].itemID=0;
//      creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount=0;
//      return;
//    }

	if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount>127)
	{
		theRepair=(int)creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount+tb_Rnd(10,50);

		if (theRepair>255)
			theRepair=255;
	}
	else
	{
		theRepair=(int)creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount+tb_Rnd(10,50);

		if (theRepair>127)
			theRepair=127;
	}

	creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount=theRepair;

	nw_SendItemRepaired(i, theMessage->slot, theRepair);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendGMCommand(UInt16 data1, UInt16 data2, UInt16 data3) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendGMCommand");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gGMMessage.h);
		gGMMessage.h.what 				= kGMMessage;
		gGMMessage.h.to 					= kNSpHostID;
		gGMMessage.h.messageLen 	= sizeof(gGMMessage);
		gGMMessage.data1					=	data1;
		gGMMessage.data2					=	data2;
		gGMMessage.data3					=	data3;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gGMMessage.h, kSendMethod);
			
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetGMCommand(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	GMMessage 	      *theMessage = (GMMessage *) inMessage;
	int               added;
	TBoolean          okToSpawn;
	char              logStr[kStringLength];
	char              tempStr[kStringLength];
	int               c; // merge 7/27
	int				t;
	int								gold1;
	int								gold2;
	int				row;
	int				col;
	
	//char tt[255];

//gr_ShowDebugLocation(67); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

// if (!((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5)))	// merge 7/28
	//	return;

	//if (player[i].category==1)
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanGoto)) // put in carbon 11/27
		if (theMessage->data1==1) // ------------------- go to
		{
			if ((theMessage->data2<25) || (theMessage->data2>3475))
				return;

			if ((theMessage->data3<25) || (theMessage->data3>3475))
				return;

			pl_PlayerMoveTo(i,theMessage->data2,theMessage->data3);

			if ((player[i].stealth==false) && (player[i].category!=4) && (player[i].category!=5))  // merge 7/27
				fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (5*60), 0);
		}

	// --------spawn creature ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSpawnCreatures)) // put in carbon 11/27
		if (theMessage->data1==2)
		{
			//if ((player[i].category!=1) && (theMessage->data2==kForestDragon))	return;	// gm's can't spawn forest dragons  // merge 7/27

			if ((theMessage->data2>2) && (theMessage->data2<kMaxCreatureTypes))
			{
				// merge 7/27
				c=cr_SpawnCreature(theMessage->data2, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col);

				if (c>0)
				{
					creature[c].startRow=creature[c].row;
					creature[c].startCol=creature[c].col;
					creature[c].range=5;
					creature[c].home=0;
				}

				// merge 7/27
				strcpy(logStr,tb_getTimeAndDate("date&time"));
				strcat(logStr, ": ");
				strcat(logStr,player[i].playerName);

				strcat(logStr," - ");

				tx_GetCreatureName(theMessage->data2, tempStr);

				strcat(logStr,tempStr);

				strcat(logStr," at ");

				tb_IntegerToString(creature[player[i].creatureIndex].row,tempStr);

				strcat(logStr,tempStr);

				strcat(logStr,"/");

				tb_IntegerToString(creature[player[i].creatureIndex].col,tempStr);

				strcat(logStr,tempStr);

				fi_WriteToGMLog(i, logStr);
			}

			return;
		}

	// ---------------------------------------------------------------

	// --------morph creature ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMorph)) // put in carbon 11/27
		if (theMessage->data1==3)
		{
			if (theMessage->data2==0)
			{
				player[i].morph=0;
				nw_SendClearName(i);
			}
			else
				if ((theMessage->data2>2) && (theMessage->data2<kMaxCreatureTypes))
					if (creatureInfo[theMessage->data2].canMorph)
					{
						player[i].morph=theMessage->data2;
						
						nw_SendClearName(i);
					}

			return;
		}

	// ---------------------------------------------------------------

	// -------- make container ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
		if (theMessage->data1==4)
		{
			if (theMessage->data2==0)
			{
				c=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

				if (c!=0)
					in_ClearContainer(c);
			}
			else
			{
				if ((theMessage->data2>0) && (theMessage->data2<kMaxContainerTypes))
					if (gmCanSpawn[theMessage->data2])
						in_MakeContainer(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, theMessage->data2, true); // carbon 11/12
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- decay container ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
		if (theMessage->data1==5)
		{
			c=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (c!=0)
			{
				nw_SendTextToClient(i, "Decay set.", 0);

				if (theMessage->data2==0)
				{
					containerDecayTime	[c]	=	0;
				}
				else
				{
					if (theMessage->data2>4320)
						theMessage->data2=4320;

					containerDecayTime	[c]	=	TickCount()+(theMessage->data2*3600);
				}
			}

			c=tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (c!=0)
			{
				nw_SendTextToClient(i, "Decay set.", 0);

				if (theMessage->data2==0)
				{
					tracksDecayTime	[c]	=	0;
				}
				else
				{
					if (theMessage->data2>4320)
						theMessage->data2=4320;

					tracksDecayTime	[c]	=	TickCount()+(theMessage->data2*3600);
				}
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- lock container ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
		if (theMessage->data1==6)
		{
			c=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (c!=0)
			{
				nw_SendTextToClient(i, "Lock set.", 0);

				if (theMessage->data2==0)
				{
					containerLock	[c]	=	0;
				}
				else
				{
					if (theMessage->data2>25)
						theMessage->data2=25;

					containerLock	[c]	=	theMessage->data2;
				}
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- trap container ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
		if (theMessage->data1==7)
		{
			c=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (c!=0)
			{
				nw_SendTextToClient(i, "Trap set.", 0);

				if (theMessage->data2==0)
				{
					containerTrap	[c]	=	0;
				}
				else
				{
					if (theMessage->data2>500)
						theMessage->data2=500;

					containerTrap	[c]	=	theMessage->data2;
				}
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- prop ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
		if (theMessage->data1==8)
		{
			if ((theMessage->data2>0) && (theMessage->data2<kMaxContainers))
				if (gmProp[theMessage->data2])
					in_DropProp(theMessage->data2, creature[player[i].creatureIndex].row,creature[player[i].creatureIndex].col);

			return;
		}

	// ---------------------------------------------------------------

	// -------- tracks ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer))
		if (theMessage->data1==9)
			if ((theMessage->data2>0) && (theMessage->data2<kMaxTracksTypes))
			{
				in_MakeTracks(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, theMessage->data2);
				return;
			}

	// ---------------------------------------------------------------

	// -------- spawn npcs ----------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSpawnCreatures))
		if (theMessage->data1==10)
			if ((theMessage->data2>0) && (theMessage->data2<kMaxNPCTypes))
			{
				for (c=1; c<kMaxNPCs; c++)
				{
					if (npc[c].type==0) //No npc
					{
						switch (theMessage->data2)
						{
							case kNPCRegular:
								if(theMessage->data3==1)
									np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kLightBlueRobe, 0, kNPCRegular,  1, 1, kBlack, kNPCHumanFemale, true);
								
								else
									np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kLightBlueRobe, 0, kNPCRegular,  0, 1, kBlack, kNPCHuman, true);
								return;
								break;
							
							case kNPCGuard:
								if(theMessage->data3==1)
									np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastPlate, kLegsPlate, kNPCGuard,  1, 1, kBlack, kNPCHumanFemale, true);
								
								else
									np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastPlate, kLegsPlate, kNPCGuard, 0, 1, kBlack, kNPCHuman, true);
								return;
								break;

							case kNPCBrigand:
								if(theMessage->data3==1)
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastChain, kBrownLeatherLegs, kNPCBrigand,  1, 1, kBlack, kNPCHumanFemale, true);
								
								else
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastChain, kBrownLeatherLegs, kNPCBrigand, 0, 1, kBlack, kNPCHuman, true);
								creature[t].alignment=kEvil;
								return;
								break;

							case kNPCMercenary:
								if(theMessage->data3==1)
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastChain, kChainLegs, kNPCMercenary,  1, 1, kBlack, kNPCHumanFemale, true);
								
								else
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBreastChain, kChainLegs, kNPCMercenary,  0, 1, kBlack, kNPCHuman, true);
								creature[t].alignment=kEvil;
								return;
								break;
								
							case kNPCDwarf1:
								if(theMessage->data3==1)
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBlackBreastPlate, kBlackLegsPlate, kNPCBrigand,  1, 1, kBlack, kNPCHumanFemale, true);
								
								else
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, kBlackBreastPlate, kBlackLegsPlate, kNPCBrigand,  0, 1, kRed, kNPCHuman, true);
								return;
								break;
								
								//Dave
							case kNPCPrivateer:
								if(theMessage->data3==1)
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, 0, kBrownLeatherLegs, kNPCPrivateer,  1, 1, kYellow, kNPCHumanFemale, true);
								
								else
									t=np_SpawnNPC(c, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, 5, 0, kBrownLeatherLegs, kNPCPrivateer,  0, 1, kYellow, kNPCHuman, true);
								creature[t].alignment=kEvil;
								return;
								break;
						}
					}
				}

				return;
			}

	// ---------------------------------------------------------------

	// -------- guild hall chest ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetHalls))
		if (theMessage->data1==11)
		{
			row=creature[player[i].creatureIndex].row;
			col=creature[player[i].creatureIndex].col;
			t=guildMap[row][col];

			if (t==0)
			{
				nw_SendTextToClient(i, "You must be standing in a Guild Hall.", 0);
				return;
			}

			/*c=0;

			for (gold1=0; gold1<kMaxGuildChests; gold1++)
				if (guildHalls[t].chests[gold1][0]!=0)
					c++;

			if (c>=(guildHalls[t].size*2)+1)
			{
				nw_SendTextToClient(i, "Maximum chests number reached for that Hall size.", 0);
				return;
			}*/

			c=-1;

			for (gold1=0; gold1<kMaxGuildChests; gold1++)
				if (guildHalls[t].chests[gold1][0]==0)
				{
					c=gold1;
					break;
				}

			if (c!=-1)
			{
				in_MakeContainer(row, col, 30, true);
				gold1=containerMap[row][col];
				guildHalls[t].chests[c][0]=gold1;
				guildHalls[t].chests[c][1]=row;
				guildHalls[t].chests[c][2]=col;

				if (strcmp(guildHalls[t].guild, "all")!=0)
					if (strcmp(guildHalls[t].guild, "All")!=0)
						strcpy(containerGift[gold1], guildHalls[t].guild);

				containerDecayTime[gold1]=0;

				fi_SaveGuildHall(guildHalls[t]);
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- guild hall extras ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetHalls))
		if (theMessage->data1==12)
		{
			row=creature[player[i].creatureIndex].row;
			col=creature[player[i].creatureIndex].col;
			t=guildMap[row][col];

			if (t==0)
			{
				nw_SendTextToClient(i, "You must be standing in a Guild Hall.", 0);
				return;
			}

			gold2=0;

			for (gold1=0; gold1<kMaxExtras; gold1++)
				if (guildHalls[t].extras[gold1].type!=0)
					gold2++;

			if (gold2>=guildHalls[t].size*3)
			{
				nw_SendTextToClient(i, "Maximum number of extras reached for that Hall size.", 0);
				return;
			}

			c=-1;

			for (gold1=0; gold1<kMaxExtras; gold1++)
				if (guildHalls[t].extras[gold1].type==0)
				{
					c=gold1;
					break;
				}

			if (c!=-1)
			{
				switch (theMessage->data2)
				{
					case 1: //Forge

						if (gold2>guildHalls[t].size*3-2)
						{
							nw_SendTextToClient(i, "Not enough extra slots left.", 0);
							break;
						}

						if (map[row][col]==613)
							if (map[row-1][col]==613)
								if (map[row][col+1]!=613)
									if (map[row-1][col+1]!=613)
									{
										guildHalls[t].extras[c].type=644;
										guildHalls[t].extras[c].row=row;
										guildHalls[t].extras[c].col=col;
										c=-1;

										for (gold1=0; gold1<kMaxExtras; gold1++)
											if (guildHalls[t].extras[gold1].type==0)
											{
												c=gold1;
												break;
											}

										guildHalls[t].extras[c].type=645;

										guildHalls[t].extras[c].row=row-1;
										guildHalls[t].extras[c].col=col;
										gh_BuildGuildHallServer(guildHalls[t]);
										fi_SaveGuildHall(guildHalls[t]);
										break;
									}

						nw_SendTextToClient(i, "Location not suitable.", 0);

						break;
					case 2: //Chess Table

						if (map[row][col]==608)
						{
							guildHalls[t].extras[c].type=974;
							guildHalls[t].extras[c].row=row;
							guildHalls[t].extras[c].col=col;
							gh_BuildGuildHallServer(guildHalls[t]);
							ac_AddChessTable(row, col);
							fi_SaveGuildHall(guildHalls[t]);
						}

						break;

					case 3: //Forge S

						if (gold2>guildHalls[t].size*3-2)
						{
							nw_SendTextToClient(i, "Not enough extra slots left.", 0);
							break;
						}

						if (map[row][col]==612)
							if (map[row][col+1]==612)
								if (map[row+1][col]!=612)
									if (map[row+1][col+1]!=612)
									{
										guildHalls[t].extras[c].type=1180;
										guildHalls[t].extras[c].row=row;
										guildHalls[t].extras[c].col=col;
										c=-1;

										for (gold1=0; gold1<kMaxExtras; gold1++)
											if (guildHalls[t].extras[gold1].type==0)
											{
												c=gold1;
												break;
											}

										guildHalls[t].extras[c].type=1181;

										guildHalls[t].extras[c].row=row;
										guildHalls[t].extras[c].col=col+1;
										gh_BuildGuildHallServer(guildHalls[t]);
										fi_SaveGuildHall(guildHalls[t]);
										break;
									}

						nw_SendTextToClient(i, "Location not suitable.", 0);

						break;
				}
			}

			return;
		}

	// ---------------------------------------------------------------

	// -------- clear hall extras ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetHalls))
		if (theMessage->data1==13)
		{
			row=creature[player[i].creatureIndex].row;
			col=creature[player[i].creatureIndex].col;
			t=guildMap[row][col];

			if (t==0)
			{
				nw_SendTextToClient(i, "You must be standing in a Guild Hall.", 0);
				return;
			}

			gold1=-1;

			for (c=0; c<kMaxExtras; c++)
				if (guildHalls[t].extras[c].type==map[row][col])
					if (guildHalls[t].extras[c].row==row)
						if (guildHalls[t].extras[c].col==col)
						{
							gold1=c;
							break;
						}

			if (gold1!=-1)
			{
				if (guildHalls[t].extras[gold1].type==645) //Forge... Must clear 2 extras
				{
					for (gold2=0; gold2<kMaxExtras; gold2++)
						if (guildHalls[t].extras[gold2].row==row+1)
						{
							guildHalls[t].extras[gold2].type=0;
							break;
						}
				}

				if (guildHalls[t].extras[gold1].type==644) //Forge... Must clear 2 extras
				{
					for (gold2=0; gold2<kMaxExtras; gold2++)
						if (guildHalls[t].extras[gold2].row==row-1)
						{
							guildHalls[t].extras[gold2].type=0;
							break;
						}
				}

				if (guildHalls[t].extras[gold1].type==1180) //Forge... Must clear 2 extras
				{
					for (gold2=0; gold2<kMaxExtras; gold2++)
						if (guildHalls[t].extras[gold2].col==col+1)
						{
							guildHalls[t].extras[gold2].type=0;
							break;
						}
				}

				if (guildHalls[t].extras[gold1].type==1181) //Forge... Must clear 2 extras
				{
					for (gold2=0; gold2<kMaxExtras; gold2++)
						if (guildHalls[t].extras[gold2].col==col-1)
						{
							guildHalls[t].extras[gold2].type=0;
							break;
						}
				}

				guildHalls[t].extras[gold1].type=0;

				gh_BuildGuildHallServer(guildHalls[t]);
				fi_SaveGuildHall(guildHalls[t]);
			}
			else
				nw_SendTextToClient(i, "You must be standing on an extra.", 0);

			return;
		}

	// ---------------------------------------------------------------

	// -------- speed test ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)
		if (theMessage->data1==15)
		{
			pl_SetSpeed(i, theMessage->data2);
			nw_SendTextToClient(i, "Speed set.",0);

			return;
		}

	// ---------------------------------------------------------------
	
	// -------- npc dressup ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)
		if (theMessage->data1==16)
		{
			player[i].npcDressup=theMessage->data2;

			return;
		}

	// ---------------------------------------------------------------
	
	// -------- damage reduc ------------------------------------
	if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)
		if (theMessage->data1==17)
		{
			if (theMessage->data2>=100)
			{
				creature[player[i].creatureIndex].damageReduc=0.0;
				nw_SendTextToClient(i, "Full Damage reduction.", 0);
			}
			else if (theMessage->data2<=0)
			{
				creature[player[i].creatureIndex].damageReduc=1.0;
				nw_SendTextToClient(i, "No Damage reduction.", 0);
			}
			else
			{
				creature[player[i].creatureIndex].damageReduc=1.0-((float)(theMessage->data2)/100.0f);
				sprintf(tempStr, "%d% Damage reduction.", theMessage->data2);
				nw_SendTextToClient(i, tempStr, 0);
			}
				
			

			return;
		}

	// ---------------------------------------------------------------

	okToSpawn=false;

	//if (player[i].category==1)
	//if (player[i].category!=0)
	if (pl_AuthorizedTo(player[i].creatureIndex,kCanSpawnItems)) // put in carbon 11/27
		okToSpawn=true;

	if ((theMessage->data2>=4000) && (theMessage->data2<5000))	// check for quest out of range
	{
		if ((theMessage->data2-4000)>kMaxQuests)
			return;

		if (quest[theMessage->data2-4000].active==false)
			return;

		okToSpawn=true;
	}

	/*if (theMessage->data1==0)
	  {
	    if (theMessage->data2==kMiningAxe)            okToSpawn=true;
	    if (theMessage->data2==kHandAxe)              okToSpawn=true;
	    if (theMessage->data2==kWoodworkingTools)     okToSpawn=true;
	    if (theMessage->data2==kTinkeringTools)       okToSpawn=true;
	    if (theMessage->data2==kFullHealPotion)       okToSpawn=true;
	    if (theMessage->data2==kFullDetoxifyPotion)   okToSpawn=true;
	    if (theMessage->data2==kNightVisionPotion)    okToSpawn=true;
	  }*/

	if (okToSpawn)
		if (theMessage->data1==0 || theMessage->data1==14) // ------------------- spawn item
		{
			if (theMessage->data2==998)
			{
				added=in_AddGoldtoInventory(player[i].creatureIndex,10000);

				if (added>0)
				{
					in_SetGoldTotal(added, &gold1, &gold2);
					nw_SendGoldAdd(i, kTakeItem, 0, gold1, gold2);	// version 0.1.2
				}

				return;
			}

			if (theMessage->data2==999)
			{
				strcpy(logStr,tb_getTimeAndDate("date&time"));
				strcat(logStr, ": ");
				strcat(logStr,player[i].playerName);
				strcat(logStr," - Reagents");
				fi_WriteToGMLog(i, logStr);

				added=in_AddtoInventory(player[i].creatureIndex, kDragonTooth, 100);

				if (added>0)	nw_SendInventoryAdd(i, kDragonTooth, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGinseng, 100);

				if (added>0)	nw_SendInventoryAdd(i, kGinseng, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kLotusFlower, 100);

				if (added>0)	nw_SendInventoryAdd(i, kLotusFlower, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kMandrakeRoot, 100);

				if (added>0)	nw_SendInventoryAdd(i, kMandrakeRoot, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kAmber, 100);

				if (added>0)	nw_SendInventoryAdd(i, kAmber, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGarlic, 100);

				if (added>0)	nw_SendInventoryAdd(i, kGarlic, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kVolcanicAsh, 100);

				if (added>0)	nw_SendInventoryAdd(i, kVolcanicAsh, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kPoisonGland, 100);

				if (added>0)	nw_SendInventoryAdd(i, kPoisonGland, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kCoal, 100);

				if (added>0)	nw_SendInventoryAdd(i, kCoal, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kElectricEel, 100);

				if (added>0)	nw_SendInventoryAdd(i, kElectricEel, added, kTakeItem,0);

				return;
			}

			if ((theMessage->data2<1) || (theMessage->data2>kMaxItemsUsed))
				return;

			if (in_CanGroup(theMessage->data2))
				added=in_AddtoInventory(player[i].creatureIndex, theMessage->data2, 1);
			else
			{
				if (theMessage->data1==0)
					added=in_AddtoInventory(player[i].creatureIndex, theMessage->data2, 127);
				else
					added=in_AddtoInventory(player[i].creatureIndex, theMessage->data2, 254);
			}

			if (added>0)
				nw_SendInventoryAdd(i, theMessage->data2, added, kTakeItem,0);

			strcpy(logStr,tb_getTimeAndDate("date&time"));
			strcat(logStr, ": ");
			strcat(logStr,player[i].playerName);

			strcat(logStr," - ");

			in_GetItemName(theMessage->data2,tempStr,-1);

			strcat(logStr,tempStr);

			fi_WriteToGMLog(i, logStr);
		}

}

#endif

// ----------------------------------------------------------------- //
void nw_SendGiveGold(UInt8 playerTarget, UInt32 amount) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendGiveGold");
#endif

	if (nw_IsGameOkay()==false)
		return;

	if (amount==0)  // can't drop zero gold 0.1.3
		return;

	if (gNetGame)
	{
		targetMode=kNoTarget;
		targetSlot=0;
		targetCount=0;
		nextAction=TickCount()+kNextAction;
		NSpClearMessageHeader(&gGiveGoldMessage.h);
		gGiveGoldMessage.h.what 				= kGiveGoldMessage;
		gGiveGoldMessage.h.to 					= kNSpHostID;
		gGiveGoldMessage.target					= playerTarget;
		gGiveGoldMessage.amount					= amount;

		gGiveGoldMessage.checksum		    = playerTarget + amount + validate;

		gGiveGoldMessage.h.messageLen 	= sizeof(gGiveGoldMessage);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gGiveGoldMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetGiveGold(int i,NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	GiveGoldMessage 							*theMessage = (GiveGoldMessage *) inMessage;
	//UInt32												playerID;
	//int														i;
	int														previousFrame=0;
	int														gold1;
	int														gold2;
	UInt32												added;
	UInt32												gold;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetGiveGold");
#endif

	if (targetFrame==0)
		previousFrame=1;

	if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((theMessage->target+theMessage->amount+validate)!=theMessage->checksum)
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad gold give message");
		return;
	}


#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	pl_CheckForActionInterrupted(i, false);

	if (theMessage->amount==0)  // can't drop zero gold 0.1.3
		return;

	if (theMessage->target==255)
	{

		if (sh_GetShopID(i)==kBank)
		{
			gold=in_GetGoldTotal(creature[player[i].creatureIndex].inventory[0].itemID, creature[player[i].creatureIndex].inventory[0].itemCount);

			if (theMessage->amount>gold)
				theMessage->amount=gold;

			added=in_BankGold(i,theMessage->amount);

			if (added>0)
			{
				nw_SendGoldRemove(i, kDepositItem, added, 0);
				gold=gold-added;
				in_SetGoldTotal(gold, &gold1, &gold2);
				creature[player[i].creatureIndex].inventory[0].itemID=gold1;
				creature[player[i].creatureIndex].inventory[0].itemCount=gold2;
			}
		}
		else
		{
			gold=in_GetGoldTotal(creature[player[i].creatureIndex].inventory[0].itemID, creature[player[i].creatureIndex].inventory[0].itemCount);

			if (theMessage->amount>gold)
				theMessage->amount=gold;

			added=in_DropGold(1,theMessage->amount, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, kTenMinuteDecay, false, "");

			if (added>0)
			{
				nw_SendGoldRemove(i, kDropItem, added, 0);
				gold=gold-added;
				in_SetGoldTotal(gold, &gold1, &gold2);
				creature[player[i].creatureIndex].inventory[0].itemID=gold1;
				creature[player[i].creatureIndex].inventory[0].itemCount=gold2;
			}
		}

	}
	else if ((playerState[i][targetFrame][theMessage->target] > 0) && (playerState[i][targetFrame][theMessage->target] < kMaxCreatures))
	{

		gold=in_GetGoldTotal(creature[player[i].creatureIndex].inventory[0].itemID, creature[player[i].creatureIndex].inventory[0].itemCount);

		if (theMessage->amount>gold)
			theMessage->amount=gold;

		added=in_AddGoldtoInventory(playerState[i][targetFrame][theMessage->target], theMessage->amount);

		if (added>0)
		{
			in_SetGoldTotal(added, &gold1, &gold2);

			if (creature[playerState[i][targetFrame][theMessage->target]].playerIndex!=0)	// give to another player
				nw_SendGoldAdd(creature[playerState[i][targetFrame][theMessage->target]].playerIndex, kTakeItem, i, gold1, gold2);

			nw_SendGoldRemove(i, kTransferItem, added, creature[playerState[i][targetFrame][theMessage->target]].playerIndex);

			gold=gold-theMessage->amount;

			in_SetGoldTotal(gold, &gold1, &gold2);

			creature[player[i].creatureIndex].inventory[0].itemID=gold1;

			creature[player[i].creatureIndex].inventory[0].itemCount=gold2;
		}

	}
	else if ((playerState[i][previousFrame][theMessage->target] > 0) && (playerState[i][previousFrame][theMessage->target] < kMaxCreatures))
	{

		gold=in_GetGoldTotal(creature[player[i].creatureIndex].inventory[0].itemID, creature[player[i].creatureIndex].inventory[0].itemCount);

		if (theMessage->amount>gold)
			theMessage->amount=gold;

		added=in_AddGoldtoInventory(playerState[i][previousFrame][theMessage->target], theMessage->amount);

		if (added>0)
		{
			in_SetGoldTotal(added, &gold1, &gold2);

			if (creature[playerState[i][previousFrame][theMessage->target]].playerIndex!=0)	// give to another player
				nw_SendGoldAdd(creature[playerState[i][previousFrame][theMessage->target]].playerIndex, kTakeItem, i, gold1, gold2);

			nw_SendGoldRemove(i, kTransferItem, added,creature[playerState[i][targetFrame][theMessage->target]].playerIndex);

			gold=gold-theMessage->amount;

			in_SetGoldTotal(gold, &gold1, &gold2);

			creature[player[i].creatureIndex].inventory[0].itemID=gold1;

			creature[player[i].creatureIndex].inventory[0].itemCount=gold2;
		}

	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendToClientDropFromInventorySlot(int i, int reason, int slot, int count, int to) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendToClientDropFromInventorySlot");
#endif

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendToClientDropFromInventorySlot");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gInventoryDropMessage.h);
		gInventoryDropMessage.h.what 				= kInventoryDropToClientMessage;
		gInventoryDropMessage.h.to 					= player[i].id;
		gInventoryDropMessage.to					= (UInt16)to;
		gInventoryDropMessage.checksum			= in_InventoryCheckSum_s(i);
		gInventoryDropMessage.reason				= (UInt8)reason;
		gInventoryDropMessage.slot					= (UInt8)slot;
		gInventoryDropMessage.count					= (UInt16)count;
		gInventoryDropMessage.h.messageLen 	= sizeof(gInventoryDropMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gInventoryDropMessage.h.messageLen;

		if (gInventoryDropMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendToClientDropFromInventorySlot");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gInventoryDropMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendToClientDropFromInventorySlot: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetToClientDropFromInventorySlot(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	InventoryDropMessage 				*theMessage = (InventoryDropMessage *) inMessage;
	int                         i;

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetToClientDropFromInventorySlot");
#endif

	if ((theMessage->slot<0) || (theMessage->slot>=kMaxInventorySize))  // paranoid error checking
	{
		fi_WriteToErrorLog("Drop from inventory out of range");
		return;
	}

	if (theMessage->slot==currentQuestSlot)
		gr_CloseFloatingWindow(tbQuestWindow);

	if (theMessage->reason==kTransferItem)
	{
		
		//in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
		//strcpy(message,"Given:  ");
		//strcat(message,itemName);
		//gr_AddText(message,false,true);
		
		//Ariki: Given to message
		if ((strcmp(playerNames[theMessage->to],"")==0) || (strcmp(playerNames[theMessage->to],"request")==0))
		{
			in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
			strcpy(message,"Given:  ");
			strcat(message,itemName);
		}
		else
		{
			in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
			strcpy(message,"Given to ");
			strcat(message,playerNames[theMessage->to]);
			strcat(message,":  ");
			strcat(message,itemName);
		}
		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kSoldItem)
	{
		in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
		strcpy(message,"Sold:  ");
		strcat(message,itemName);
		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kDropItem)
	{
		in_ClearPickUpPack();
		in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
		strcpy(message,"Dropped:  ");
		strcat(message,itemName);
		gr_AddText(message,false,true);

		if (windowVisible[kPackContentsWindow])
			nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
	}

	if (theMessage->reason==kBoughtItem)
	{
		in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
		strcpy(message,"Bought:  ");
		strcat(message,itemName);
		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kDepositItem)
	{
		in_ClearPickUpPack();
		in_ClearBank();
		in_GetItemName(localInventory[theMessage->slot].itemID, itemName, theMessage->count);
		strcpy(message,"Deposited:  ");
		strcat(message,itemName);
		gr_AddText(message,false,true);

		if (windowVisible[kBankWindow])
			nw_SendAction(kGetBankContents, kHighPriority, kSendNow,true);
	}

	if (theMessage->reason==kDamageItem)
		in_DamageInventorySlot(theMessage->slot,theMessage->count); // client mod 8/9
	else
		in_DropFromInventorySlot(theMessage->slot,theMessage->count); // client

	localWeight=0;

	for (i=1;i<kMaxInventorySize;i++)
		if (localInventory[i].itemID!=0)
			if (in_CanGroup(localInventory[i].itemID))
				localWeight=localWeight+itemList[localInventory[i].itemID].weight+(itemList[localInventory[i].itemID].weight*localInventory[i].itemCount)/10;
			else
				localWeight=localWeight+itemList[localInventory[i].itemID].weight;

	if (theMessage->checksum!=in_InventoryCheckSum_c())
	{
		inventoryCheckSumFailures++;
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
	}

#ifndef _SERVERONLY
	gr_RenderCharacterDisplayWindow();

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendItemRepaired(int i, int slot, int count) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_SendItemRepaired");
#endif

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendToClientDropFromInventorySlot");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gItemRepairedMessage.h);
		gItemRepairedMessage.h.what 			= kItemRepairedMessage;
		gItemRepairedMessage.h.to 				= player[i].id;
		gItemRepairedMessage.slot					= (UInt8)slot;
		gItemRepairedMessage.count				= (UInt16)count;
		gItemRepairedMessage.h.messageLen = sizeof(gItemRepairedMessage);

		gItemRepairedMessage.checksum			= slot+count+validate;

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gItemRepairedMessage.h.messageLen;

		if (gItemRepairedMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendItemRepaired");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gItemRepairedMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendItemRepaired: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetItemRepaired(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ItemRepairedMessage 				*theMessage = (ItemRepairedMessage *) inMessage;
	//int                         i;

	if ((theMessage->slot<0) || (theMessage->slot>=kMaxInventorySize))  // paranoid error checking
		return;

	if (!(it_IsArmor(localInventory[theMessage->slot].itemID, false) || (it_IsWeapon(localInventory[theMessage->slot].itemID))))
		return;

	if ((theMessage->slot+theMessage->count+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad item repaired message");
		return;
	}

	gr_AddText("You repair the item...",false,true);

	localInventory[theMessage->slot].itemCount=theMessage->count;
	gr_RenderPlayerDisplay(true);

}

// ----------------------------------------------------------------- //
void nw_SendDeleteFromInventorySlot(int slot) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


	if (nw_IsGameOkay()==false)
		return;

	if ((slot<0) || (slot>=kMaxInventorySize))  // paranoid error checking
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gInventoryDeleteMessage.h);
		gInventoryDeleteMessage.h.what 				= kInventoryDeleteMessage;
		gInventoryDeleteMessage.h.to 					= kNSpHostID;
		gInventoryDeleteMessage.slot					= (UInt8)slot;
		gInventoryDeleteMessage.checksum			= slot+validate;
		gInventoryDeleteMessage.h.messageLen 	= sizeof(gInventoryDeleteMessage);

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gInventoryDeleteMessage.h, kSendMethod);
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetDeleteFromInventorySlot(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	InventoryDeleteMessage 				*theMessage = (InventoryDeleteMessage *) inMessage;


//gr_ShowDebugLocation(73); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if ((theMessage->slot+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad delete from inventory slot");
		return;
	}


	if ((theMessage->slot<0) || (theMessage->slot>=kMaxInventorySize))  // paranoid error checking
		return;

	if (in_CanGroup(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID))
	{
		if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount>0)//018
			creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount--;

		if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount<=0)
			creature[player[i].creatureIndex].inventory[theMessage->slot].itemID		=	0;
	}
	else
	{
		creature[player[i].creatureIndex].inventory[theMessage->slot].itemID		=	0;
		creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount	= 0;
	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendInventoryAdd(int i, int item, int count, int reason, int from) // server
// ----------------------------------------------------------------- //

{
	// Sent from server when player picks up an item
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(74); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendInventoryAdd");
		return;
	}

	if ((item<1) || (item>kMaxItemsUsed))
		return;

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gInventoryAddMessage.h);
		gInventoryAddMessage.h.what 				= kInventoryAddMessage;
		gInventoryAddMessage.h.to 					= player[i].id;
		gInventoryAddMessage.checksum				= in_InventoryCheckSum_s(i);
		gInventoryAddMessage.item						= (UInt16)item;
		gInventoryAddMessage.count					= (UInt16)count;
		gInventoryAddMessage.reason					= (UInt8)reason;
		gInventoryAddMessage.from						= (UInt8)from;
		gInventoryAddMessage.h.messageLen 	= sizeof(gInventoryAddMessage);

		gInventoryAddMessage.check		      = item + count + reason + from + validate;

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gInventoryAddMessage.h.messageLen;

		if (gInventoryAddMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendInventoryAdd");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gInventoryAddMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendInventoryAdd: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetInventoryAdd(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY
	// Sent from server when items are added to player inventory

	InventoryAddMessage 				*theMessage = (InventoryAddMessage *) inMessage;
	int													i;
	TBoolean										outOfSynch=false;


	if ((theMessage->item+theMessage->count+theMessage->reason+theMessage->from+validate)!=theMessage->check)
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad inventory add message");
		return;
	}

	if ((theMessage->from<0) || (theMessage->from>=kMaxPlayers))
	{
		fi_WriteToErrorLog("Inventory add player out of range");
		return;
	}

	strcpy(message,"");

	if (in_CanGroup(theMessage->item))
		for (i=1;i<kMaxInventorySize;i++)
			if (localInventory[i].itemID==theMessage->item)
			{

				if (theMessage->reason==kRewardItem)
					strcpy(message,"Reward:  ");

				if (theMessage->reason==kStoleItem)
					strcpy(message,"Stolen:  ");

				if (theMessage->reason==kTakeItem)
				{
					if ((strcmp(playerNames[theMessage->from],"")==0) || (strcmp(playerNames[theMessage->from],"request")==0))
					{
						strcpy(message,"Taken:  ");

						if (windowVisible[kPackContentsWindow])
							nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
					}
					else
					{
						strcpy(message,"Taken from ");
						strcat(message,playerNames[theMessage->from]);
						strcat(message,":  ");
					}
				}

				if (theMessage->reason==kBoughtItem)
					strcpy(message,"Bought:  ");

				if (theMessage->reason==kCaughtItem)
					strcpy(message,"Caught:  ");

				if (theMessage->reason==kWithdrawItem)
				{
					strcpy(message,"Withdrawn:  ");

					if (windowVisible[kBankWindow])
						nw_SendAction(kGetBankContents, kHighPriority, kSendNow,true);
				}

				if (windowVisible[kSellListWindow])	//018
					nw_SendAction(kGetSellList, kHighPriority, kSendNow,true);

				in_GetItemName(theMessage->item, itemName, theMessage->count);

				strcat(message,itemName);

				if (theMessage->reason!=kNoMessage)
					gr_AddText(message,false,true);

				if (localInventory[i].itemCount+theMessage->count>kMaxInventoryItemCount)
				{
					outOfSynch=true;
					break;
				}

				localInventory[i].itemCount=localInventory[i].itemCount+theMessage->count;

				if (theMessage->checksum!=in_InventoryCheckSum_c())
				{
					inventoryCheckSumFailures++;
					nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
				}

				localWeight=0;

				for (i=1;i<kMaxInventorySize;i++)
					if (localInventory[i].itemID!=0)
						if (in_CanGroup(localInventory[i].itemID))
							localWeight=localWeight+itemList[localInventory[i].itemID].weight+(itemList[localInventory[i].itemID].weight*localInventory[i].itemCount)/10;
						else
							localWeight=localWeight+itemList[localInventory[i].itemID].weight;

				gr_RenderPlayerDisplay(true);

				return;
			}

	if (outOfSynch==false)
		for (i=1;i<kMaxInventorySize;i++)
			if (localInventory[i].itemID==0)
			{
				localInventory[i].itemID=theMessage->item;
				localInventory[i].itemCount=theMessage->count;

				localWeight=0;

				for (i=1;i<kMaxInventorySize;i++)
					if (localInventory[i].itemID!=0)
						if (in_CanGroup(localInventory[i].itemID))
							localWeight=localWeight+itemList[localInventory[i].itemID].weight+(itemList[localInventory[i].itemID].weight*localInventory[i].itemCount)/10;
						else
							localWeight=localWeight+itemList[localInventory[i].itemID].weight;

				if (theMessage->reason==kRewardItem)
					strcpy(message,"Reward:  ");

				if (theMessage->reason==kStoleItem)
					strcpy(message,"Stolen:  ");

				if (theMessage->reason==kTakeItem)
				{
					if ((strcmp(playerNames[theMessage->from],"")==0) || (strcmp(playerNames[theMessage->from],"request")==0))
					{
						strcpy(message,"Taken:  ");

						if (windowVisible[kPackContentsWindow])
							nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
					}
					else
					{
						strcpy(message,"Taken from ");
						strcat(message,playerNames[theMessage->from]);
						strcat(message,":  ");
					}
				}

				if (theMessage->reason==kBoughtItem)
					strcpy(message,"Bought:  ");

				if (theMessage->reason==kCaughtItem)
					strcpy(message,"Caught:  ");

				if (theMessage->reason==kWithdrawItem)
				{
					strcpy(message,"Withdrawn:  ");

					if (windowVisible[kBankWindow])
						nw_SendAction(kGetBankContents, kHighPriority, kSendNow,true);
				}

				if (windowVisible[kSellListWindow])	//018
					nw_SendAction(kGetSellList, kHighPriority, kSendNow,true);

				in_GetItemName(theMessage->item, itemName, theMessage->count);

				strcat(message,itemName);

				if (theMessage->reason!=kNoMessage)
					gr_AddText(message,false,true);

				gr_RenderPlayerDisplay(true);

				if (theMessage->checksum!=in_InventoryCheckSum_c())
				{
					inventoryCheckSumFailures++;
					nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
				}

				return;
			}

	// something is not right - request full inventory update
	nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendGoldRemove(int i, UInt8 reason, UInt32 amount, UInt8 to) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(76); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendGoldRemove");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gGoldRemoveMessage.h);
		gGoldRemoveMessage.h.what 				= kGoldRemoveMessage;
		gGoldRemoveMessage.h.to 					= player[i].id;
		gGoldRemoveMessage.reason					= reason;
		gGoldRemoveMessage.amount					= amount;
		gGoldRemoveMessage.to						= to;

		gGoldRemoveMessage.checksum		    = reason + amount + validate;

		gGoldRemoveMessage.h.messageLen 	= sizeof(gGoldRemoveMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gGoldRemoveMessage.h.messageLen;

		if (gGoldRemoveMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendGoldRemove");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gGoldRemoveMessage.h, kSendMethod);//kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendGoldRemove: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetGoldRemove(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY
	GoldRemoveMessage 					*theMessage = (GoldRemoveMessage *) inMessage;
	UInt32											gold;
	char												addedStr[kStringLength];
	int													gold1;
	int													gold2;

//gr_ShowDebugLocation(77); // server only

	if ((theMessage->reason+theMessage->amount+validate)!=theMessage->checksum)
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad gold remove message");
		return;
	}
	gold	=	in_GetGoldTotal(localInventory[0].itemID, localInventory[0].itemCount);

	tb_IntegerToString(theMessage->amount,	addedStr);

	if (theMessage->reason==kTransferItem)
	{
		if ((strcmp(playerNames[theMessage->to],"")==0) || (strcmp(playerNames[theMessage->to],"request")==0))
		{
			strcpy(message,"Given:  ");

		}
		else
		{
			strcpy(message,"Given to ");
			strcat(message,playerNames[theMessage->to]);
			strcat(message,":  ");
		}
		//strcpy(message,"Given:  ");
		strcat(message,addedStr);

		if (theMessage->amount>1)
			strcat(message," gold pieces");
		else
			strcat(message," gold piece");

		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kDropItem)
	{
		in_ClearPickUpPack();
		strcpy(message,"Dropped:  ");
		strcat(message,addedStr);

		if (theMessage->amount>1)
			strcat(message," gold pieces");
		else
			strcat(message," gold piece");

		gr_AddText(message,false,true);

		if (windowVisible[kPackContentsWindow])
			nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
	}

	if (theMessage->reason==kPaid)
	{
		//in_ClearPickUpPack();
		strcpy(message,"Paid:  ");
		strcat(message,addedStr);

		if (theMessage->amount>1)
			strcat(message," gold pieces");
		else
			strcat(message," gold piece");

		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kFinedItem)
	{
		strcpy(message,"Fined:  ");
		strcat(message,addedStr);

		if (theMessage->amount>1)
			strcat(message," gold pieces");
		else
			strcat(message," gold piece");

		gr_AddText(message,false,true);
	}

	if (theMessage->reason==kDepositItem)
	{
		in_ClearPickUpPack();
		in_ClearBank();
		strcpy(message,"Deposited:  ");
		strcat(message,addedStr);

		if (theMessage->amount>1)
			strcat(message," gold pieces");
		else
			strcat(message," gold piece");

		gr_AddText(message,false,true);

		if (windowVisible[kBankWindow])
			nw_SendAction(kGetBankContents, kHighPriority, kSendNow,true);
	}

	gold=gold-theMessage->amount;

	if (gold<0)	// something wrong ... gold out of synch
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		return;
	}

	in_SetGoldTotal(gold, &gold1, &gold2);

	localInventory[0].itemID		=	gold1;
	localInventory[0].itemCount	=	gold2;

	gr_RenderPlayerDisplay(true);

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendGoldAdd(int i, int reason, int from, int gold1, int gold2) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh player's gold total
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(78); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendGoldAdd");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gGoldAddMessage.h);
		gGoldAddMessage.h.what 				= kGoldAddMessage;
		gGoldAddMessage.h.to 					= player[i].id;
		gGoldAddMessage.reason 				= (UInt8)reason;
		gGoldAddMessage.from 					= (UInt8)from;
		gGoldAddMessage.gold1					= (UInt16)gold1;
		gGoldAddMessage.gold2					= (UInt8)gold2;

		gGoldAddMessage.checksum		  = reason + from + gold1 + gold2 + validate;

		gGoldAddMessage.h.messageLen 	= sizeof(gGoldAddMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gGoldAddMessage.h.messageLen;

		if (gGoldAddMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendGoldAdd");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gGoldAddMessage.h, kSendMethod);//kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendGoldAdd: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetGoldAdd(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY
	// Sent from server to refresh gold total

	GoldAddMessage 							*theMessage = (GoldAddMessage *) inMessage;
	UInt32											gold;
	UInt32											added;
	char												addedStr[kStringLength];
	int													gold1;
	int													gold2;

	if ((theMessage->reason+theMessage->from+theMessage->gold1+theMessage->gold2+validate)!=theMessage->checksum)
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad gold add message");
		return;
	}

	if ((theMessage->from<0) || (theMessage->from>=kMaxPlayers))
	{
		fi_WriteToErrorLog("Gold add player out of range");
		return;
	}

//gr_ShowDebugLocation(79); // server only
	gold	=	in_GetGoldTotal(localInventory[0].itemID, localInventory[0].itemCount);

	added	=	in_GetGoldTotal(theMessage->gold1, theMessage->gold2);

	tb_IntegerToString(added,	addedStr);

	if (theMessage->reason==kWithdrawItem)
	{
		strcpy(message,"Withdrawn:  ");

		if (windowVisible[kBankWindow])
			nw_SendAction(kGetBankContents, kHighPriority, kSendNow,true);
	}
	else if (theMessage->reason==kRewardItem)
		strcpy(message,"Reward:  ");
	else if (theMessage->reason==kGivenItem)
		strcpy(message,"Given:  ");
	else if (theMessage->reason==kStoleItem)
		strcpy(message,"Stolen:  ");
	else
	{
		if ((strcmp(playerNames[theMessage->from],"")==0) || (strcmp(playerNames[theMessage->from],"request")==0))
		{
			strcpy(message,"Taken:  ");

			if (windowVisible[kPackContentsWindow])
				nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
		}
		else
		{
			strcpy(message,"Taken from ");
			strcat(message,playerNames[theMessage->from]);
			strcat(message,":  ");
		}
	}

	strcat(message,addedStr);

	if (added>1)
		strcat(message," gold pieces");
	else
		strcat(message," gold piece");

	gr_AddText(message,false,true);

	gold=gold+added;

	in_SetGoldTotal(gold, &gold1, &gold2);

	localInventory[0].itemID		=	gold1;

	localInventory[0].itemCount	=	gold2;

	gr_RenderPlayerDisplay(true);

#endif
}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendDisplayMessage(int i, UInt8 message) // server
// ----------------------------------------------------------------- //

{
	// Sent from server.  Displays message on client
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(80); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendDisplayMessage");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gDisplayMessage.h);
		gDisplayMessage.h.what 				= kDisplayMessage;
		gDisplayMessage.h.to 					= player[i].id;
		gDisplayMessage.display				= message;

		gDisplayMessage.checksum			= message+validate;

		gDisplayMessage.h.messageLen 	= sizeof(gDisplayMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gDisplayMessage.h.messageLen;

		if (gDisplayMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendDisplayMessage");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gDisplayMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendDisplayMessage: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetDisplayMessage(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	DisplayMessage 				*theMessage = (DisplayMessage *) inMessage;

	if ((theMessage->display+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad display message");
		return;
	}

	if (theMessage->display==kNoOwner)
		gr_AddText("This house does not have an owner.",false,true);

	if (theMessage->display==kEnterPVP)
		gr_AddText("You have entered the player versus player area.",false,true);

	if (theMessage->display==kExitPVP)
		gr_AddText("You have left the player versus player area.",false,true);

	if (theMessage->display==kEnterBadlands)
		gr_AddText("You have entered the Badlands.",false,true);

	if (theMessage->display==kExitBadlands)
		gr_AddText("You have left the Badlands.",false,true);

	if (theMessage->display==kBeware)
		gr_AddText("Abandon all hope, ye who enter here.",false,true);
	
	if (theMessage->display==kNoManaForEnchant)
		gr_AddText("You don't have enough mana to perform that enchant.",false,true);

	// Not enough mana for potion
	if (theMessage->display==kNoManaForPotion)
		gr_AddText("You don't have enough mana to mix that potion.",false,true);

	// Not enough mana for spell
	if (theMessage->display==kNoManaForSpell)
		gr_AddText("You don't have enough mana to cast that spell.",false,true);

	// Not online
	if (theMessage->display==kPlayerNotOnline)
		gr_AddText("The orb sees nothing.",false,true);

	// Can't bank here
	if (theMessage->display==kCantBankHere)
		gr_AddText("Your bank box is full.",false,true);

	// GM Online
	else if (theMessage->display==kGMOnline)
		gr_AddText("Your request has been sent to a Game Master.",false,true);

	// No GM Online
	else if (theMessage->display==kNoGMOnline)
		//gr_AddText("There are no Game Masters online right now.",false,true);
		gr_AddText("There are no Game Masters available right now.  Please try again later.",false,true);

	// Detect Hidden
	else if (theMessage->display==kHiddenRevealed)
		gr_AddText("Hidden creatures are revealed.",false,true);

	// Can't attack player
	else if (theMessage->display==kCantAttackCreature)
		gr_AddText("You can't attack that creature.",false,true);

	// No ferrite left
	else if (theMessage->display==kNoMoreFerrite)
		gr_AddText("There is nothing left here to mine.",false,true);

	// No logs
	else if (theMessage->display==kNoMoreLogs)
		gr_AddText("There is nothing remaining here to be harvested.",false,true);

	// Hungry
	else if (theMessage->display==kHungry)
		gr_AddText("You are starving!",false,true);

	// Pet Starving
	else if (theMessage->display==kPetStarving)
		gr_AddText("Your pet looks very hungry.",true,true); //Dave false, true

	// Pet Dead
	else if (theMessage->display==kPetDies)
		gr_AddText("Your pet dies from starvation!",true,true); //Dave false, true

	// Can't see target
	else if (theMessage->display==kCantSeeTarget)
		gr_AddText("You can't see your target.",false,true);

	// Disoriented
	else if (theMessage->display==kYouAreDisoriented)
		gr_AddText("You are disoriented.",false,true);

	// Paralyzed
	else if (theMessage->display==kYouAreParalyzed)
		gr_AddText("You are paralyzed.",false,true);

	// Too far away ... healing and thievery
	else if (theMessage->display==kCreatureTooFarAway)
		gr_AddText("That creature is too far away.",false,true);

	// Poisoned
	else if (theMessage->display==kYouArePoisoned)
		gr_AddText("You are poisoned.",false,true);

	// Container Locked
	else if (theMessage->display==kContainerLocked)
		gr_AddText("The container is locked.",false,true);

	// Detect Failed
	else if (theMessage->display==kHiddenFailed)
		gr_AddText("No hidden creatures are detected.",false,true);

	// Already Tame
	else if (theMessage->display==kAlreadyTame)
		gr_AddText("That creature is already tame.",false,true);

	// Taming Failed
	else if (theMessage->display==kTamingFailed)
		gr_AddText("You fail to tame the creature.",false,true);

	// Taming Failed... too many pets
	else if (theMessage->display==kTooManyCreatures)
		gr_AddText("You can not control that many creatures.",false,true);

	// Taming OK
	else if (theMessage->display==kBeastHasBeenTamed)
		gr_AddText("The beast has been tamed.",false,true);

	// Spell Fizzles
	else if (theMessage->display==kSpellFizzles)
		gr_AddText("The spell fizzles.",false,true);

	// Not enough Reagents
	else if (theMessage->display==kNotEnoughReagents)
		gr_AddText("You don't have enough reagents.",false,true);

	// Not enough gold
	else if (theMessage->display==kNotEnoughGold)
		gr_AddText("You don't have enough gold.",false,true);

	// Can't drop here
	else if (theMessage->display==kCantDropHere)
		gr_AddText("You can't drop that here.",false,true);

	// can't give items to ghost
	else if (theMessage->display==kCantGiveToGhost)
		gr_AddText("You can't give items to a ghost.",false,true);

	// give fail ... target can't handle weight
	else if (theMessage->display==kPersonCantCarryThatMuch)
		gr_AddText("That person can't carry that much.",false,true);

	// pack full
	else if (theMessage->display==kHisPackFull)
		gr_AddText("Their pack is full.",false,true);

	// pack full
	else if (theMessage->display==kYourPackFull)
		gr_AddText("Your pack is full.",false,true);
	
	// pack full
	else if (theMessage->display==kRewardPackFull)
		gr_AddText("You don't have enough room in your pack for the reward.",false,true);

	// catch thief stealing
	else if (theMessage->display==kCatchThief)
		gr_AddText("Your pack suddenly seems a bit lighter.",false,true);

	// catch thief trying to steal
	else if (theMessage->display==kSomeoneTryingToSteal)
		gr_AddText("You feel a hand brush against your bag.",false,true);


	//Stealth
	else if (theMessage->display==kEnteringStealth)
		gr_AddText("You have successfully stealthed.",false,true);
	else if (theMessage->display==kStealthFailed)
		gr_AddText("You fail to stealth.",false,true);
	else if (theMessage->display==kLeavingStealth)
		gr_AddText("You are no longer stealthed.",false,true);
	
	// must be stealthed
	else if (theMessage->display==kMustBeStealthed)
		gr_AddText("You must be stealthy before you can do this action.",false,true);

	//Backstab
	else if (theMessage->display==kBackstabDone0)
		gr_AddText("Sting!",false,true);
	else if (theMessage->display==kBackstabDone1)
		gr_AddText("Backstab!",false,true);
	else if (theMessage->display==kBackstabDone2)
		gr_AddText("Decimate!",false,true);
	else if (theMessage->display==kBackstabbed)
		gr_AddText("You were backstabbed!",false,true);
	else if (theMessage->display==kBackstabFailed)
		gr_AddText("Backstab failed.",false,true);

	//Critical hit
	else if (theMessage->display==kCriticalDone)
		gr_AddText("Critical Hit!",false,true);

	//Missed
	else if (theMessage->display==kHitMissed)
		gr_AddText("Missed!",false,true);

	//Tracking
	else if (theMessage->display==kTrackingFailed)
		gr_AddText("You fail to track any creature.",false,true);

	// meditating
	else if (theMessage->display==kConcentrationDisturbed)
		gr_AddText("Your meditative trance is broken.",false,true);

	// lumberjacking
	else if (theMessage->display==kNoLumberHarvest)
		gr_AddText("You fail to harvest any logs.",false,true);

	// poisoned
	else if (theMessage->display==kYouHaveBeenPoisoned)
		gr_AddText("You have been poisoned.",false,true);

	// cursed
	else if (theMessage->display==kYouHaveBeenCursed)
		gr_AddText("You have been cursed.",false,true);

	// no longer cursed
	else if (theMessage->display==kCurseRemoved)
		gr_AddText("Your curse is removed.",false,true);

	// mining
	else if (theMessage->display==kNoOreHarvest)
		gr_AddText("You fail to mine any ore.",false,true);

	// fishing
	else if (theMessage->display==kNoFishCaught)
		gr_AddText("You fail to catch any fish.",false,true);

	// lockpicking
	else if (theMessage->display==kNoContainer)
		gr_AddText("You must be standing on a locked container.",false,true);
	else if (theMessage->display==kCantPickMagicLock)
		gr_AddText("You can't pick a magic lock.",false,true);
	else if (theMessage->display==kNotLocked)
		gr_AddText("That container is not locked.",false,true);
	else if (theMessage->display==kContainerUnlocked)
	{
		gr_AddText("You have successfully picked the lock.",false,true);
		in_ClearPickUpPack();

		if (windowVisible[kPackContentsWindow])
			nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
	}
	else if (theMessage->display==kLockpickingFailed)
		gr_AddText("You have failed to pick the lock.",false,true);

	// remove trap
	else if (theMessage->display==kNoContainerToDisarm)
		gr_AddText("You must be standing on a trapped container.",false,true);
	else if (theMessage->display==kCantDisarmMagicTrap)
		gr_AddText("You can't disarm a magic trap.",false,true);
	else if (theMessage->display==kNotTrappeed)
		gr_AddText("That container is not trapped.",false,true);
	else if (theMessage->display==kContainerDisarmed)
	{
		gr_AddText("You have successfully disarmed the trap.",false,true);
		in_ClearPickUpPack();

		if (windowVisible[kPackContentsWindow])
			nw_SendAction(kGetContainerContents, kHighPriority, kSendNow,true);
	}
	else if (theMessage->display==kRemoveTrapFailed)
		gr_AddText("You have failed to disarm the trap.",false,true);

	// failed make
	else if (theMessage->display==kFailedToMake)
		gr_AddText(makeFailedMessage,false,true);

	// successful make
	else if (theMessage->display==kSuccessfulMake)
		gr_AddText(makeSuccessMessage,false,true);

	// hiding
	else if (theMessage->display==kHidingFailed)
		gr_AddText("You fail to hide.",false,true);
	else if (theMessage->display==kYouAreHidden)
		gr_AddText("You are hidden.",false,true);

	// anatomy
	else if (theMessage->display==kVeryWeak)
		gr_AddText("That creature appears to be extremely weak.",false,true);
	else if (theMessage->display==kWeak)
		gr_AddText("That creature appears to be somewhat weak.",false,true);
	else if (theMessage->display==kAverageStrength)
		gr_AddText("That creature appears to be of average strength.",false,true);
	else if (theMessage->display==kStrong)
		gr_AddText("That creature appears to be quite strong.",false,true);
	else if (theMessage->display==kVeryStrong)
		gr_AddText("That creature appears to be very strong.",false,true);
	else if (theMessage->display==kExtremelyStrong)
		gr_AddText("That creature appears to be unbelievably strong.",false,true);
	else if (theMessage->display==kAnatomyFailed)
		gr_AddText("You are unable to determine anything useful.",false,true);

	// animal lore
	else if (theMessage->display==kAnimalLoreFailed)
		gr_AddText("You are unable to determine anything useful.",false,true);
	else if (theMessage->display==kLooksAggressive)
		gr_AddText("That creature looks quite aggressive.",false,true);
	else if (theMessage->display==kLooksNonAggressive)
		gr_AddText("That creature does not appear to be aggressive.",false,true);
	else if (theMessage->display==kVeryHappy)
		gr_AddText("That creature appears to be very happy.",false,true);
	else if (theMessage->display==kSomewhatHappy)
		gr_AddText("That creature appears to be somewhat happy.",false,true);
	else if (theMessage->display==kSomwhatUnhappy)
		gr_AddText("That creature appears to be rather unhappy.",false,true);
	else if (theMessage->display==kVeryUnhappy)
		gr_AddText("That creature appears to be very unhappy.",false,true);

	// evaluate intelligence
	else if (theMessage->display==kVeryStupid)
		gr_AddText("That creature appears to be extremely stupid.",false,true);
	else if (theMessage->display==kStupid)
		gr_AddText("That creature appears to be somewhat stupid.",false,true);
	else if (theMessage->display==kAverageIntelligence)
		gr_AddText("That creature appears to be of average intelligence.",false,true);
	else if (theMessage->display==kIntelligent)
		gr_AddText("That creature appears to be quite intelligent.",false,true);
	else if (theMessage->display==kVeryIntelligent)
		gr_AddText("That creature appears to be very intelligent.",false,true);
	else if (theMessage->display==kExtremelyIntelligent)
		gr_AddText("That creature appears to be unbelievably intelligent.",false,true);
	else if (theMessage->display==kEvaluatingIntelligenceFailed)
		gr_AddText("You are unable to determine anything useful.",false,true);

	// eating
	else if (theMessage->display==kStillHungry)
		gr_AddText("You eat the food, but are still very hungry.",false,true);
	else if (theMessage->display==kLessHungry)
		gr_AddText("You feel much less hungry.",false,true);
	else if (theMessage->display==kFeelingFull)
		gr_AddText("You are beginning to feel full.",false,true);
	else if (theMessage->display==kVeryFull)
		gr_AddText("You feel very full.",false,true);
	else if (theMessage->display==kCantEatMore)
		gr_AddText("You feel like you couldn't eat another bite.",false,true);

	// weapons lore (obsolete)
	else if (theMessage->display==kVeryPoorCondition)
		gr_AddText("That item is about to fall apart.",false,true);
	else if (theMessage->display==kPoorCondition)
		gr_AddText("That item is in poor condition.",false,true);
	else if (theMessage->display==kFairCondition)
		gr_AddText("That item is in fair condition.",false,true);
	else if (theMessage->display==kGoodCondition)
		gr_AddText("That item is in good condition.",false,true);
	else if (theMessage->display==kVeryGoodCondition)
		gr_AddText("That item is in very good condition.",false,true);
	else if (theMessage->display==kLikeNewCondition)
		gr_AddText("That item is like new.",false,true);

	// magic lore
	if (theMessage->display==kMagicLoreFailed)
		gr_AddText("You are unable to determine anything useful.",false,true);
	else if (theMessage->display==kOrdinaryItem)
		gr_AddText("It appears to be an ordinary item.",false,true);
	else if (theMessage->display==kMagicLoreSuccess)
		it_MagicLoreSuccess(magicLoreTarget);

	// healing
	else if (theMessage->display==kApplyingBandages)
		gr_AddText("You apply the bandages.",false,true);
	else if (theMessage->display==kBeingBandaged)
		gr_AddText("Your wounds are being bandaged.",false,true);
	else if (theMessage->display==kHealingFailed)
		gr_AddText("You fail to properly apply the bandages.",false,true);

//	else if (theMessage->display==kBandagesFellOff)
//		gr_AddText("Your bandages have fallen off.");

	// can't attack in town
	else if (theMessage->display==kCantAttackInTown)
		gr_AddText("You can't attack that creature within the town limits.",false,true);

	// can't cast in town
	else if (theMessage->display==kCantCastInTown)
		gr_AddText("You can't cast that spell within the town limits.",false,true);

	// quest complete
	else if (theMessage->display==kQuestFinished)
	{
		tb_PlaySound(201);
		gr_AddText("Quest completed!",false,true);
	}
	
	//not enough stamina
	else if (theMessage->display==kNotEnoughStaminaMsg)						
		gr_AddText("Not enough Stamina.",false,true);
	
	//defense stance
	else if (theMessage->display==kDefenseBegin)
		gr_AddText("Defensive stance assumed.",false,true);
	
	//defense stance ends
	else if (theMessage->display==kDefenseEnd)
		gr_AddText("You are exhausted. Leaving defensive stance.",false,true);
	
	//exhausted
	else if (theMessage->display==kExhausted)
		gr_AddText("You are exhausted!",false,true);
	
	//must be behind
	else if (theMessage->display==kMustBeBehindTarget)
		gr_AddText("You must be behind your target.", false, true);
	
	//vial of acid
	else if (theMessage->display==kVialOfAcidMsg)						
		gr_AddText("Something explodes on you!",false,true);
	
	//knockback
	else if (theMessage->display==kKnockback)						
		gr_AddText("Knockback!",false,true);
	
	//taunt
	else if (theMessage->display==kTaunt)						
		gr_AddText("Your target focuses on you!",false,true);
	
	//escape move
	else if (theMessage->display==kEscapeMove)						
		gr_AddText("Escape Move successful.",false,true);
	
	//aimed shot
	else if (theMessage->display==kAimedShot)						
		gr_AddText("Aimed Shot!",false,true);
	
	//grappling shot
	else if (theMessage->display==kGrapplingShot)						
		gr_AddText("Grappling Shot!",false,true);
	
	//booming blow
	else if (theMessage->display==kBoomingBlow)						
		gr_AddText("Booming Blow!",false,true);
	
	//ranger swiftness
	else if (theMessage->display==kRangerSwiftness)						
		gr_AddText("Ranger's Swiftness!",false,true);
	
	//battle ready
	else if (theMessage->display==kQuickDrawMsg)						
		gr_AddText("You are ready to fight again!",false,true);
	
	//ranger swiftness
	else if (theMessage->display==kNoQuickDrawMsg)						
		gr_AddText("You are already ready to fight.",false,true);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendActionUpdate(int i) // server
// ----------------------------------------------------------------- //

{

	int										j;
	OSStatus 							status=noErr;
	int										action;
	int										index;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(82); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendActionUpdate");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	ac_GetActionType(i,&action,&index);

	if (action==929)	// chess
	{
		if (gNetGame)
		{
			NSpClearMessageHeader(&gChessRefreshMessage.h);
			gChessRefreshMessage.h.what 				= kChessRefreshMessage;
			gChessRefreshMessage.h.to 					= player[i].id;

			for (j=0;j<96;j++)
				gChessRefreshMessage.square[j]	= chessBoard[index][j];

			gChessRefreshMessage.h.messageLen 	= sizeof(gChessRefreshMessage);

#ifdef _SERVERONLY
			messagesSent++;

			bytesSent=bytesSent+gChessRefreshMessage.h.messageLen;

			if (gChessRefreshMessage.h.messageLen>2000)
				fi_WriteToErrorLog("Large message in nw_SendActionUpdate");

#endif
			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gChessRefreshMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendSkillChange: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();
	}

}

#endif

// ----------------------------------------------------------------- //
void nw_GetChessUpdate(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ChessRefreshMessage 				*theMessage = (ChessRefreshMessage *) inMessage;
	int													i;


//gr_ShowDebugLocation(83); // server only
	nw_ClearQueueOf(kGetActionUpdate);

	for (i=0;i<96;i++)
		localChess[i]			=	theMessage->square[i];

#ifndef _SERVERONLY
	gr_RenderChessWindow();

	//gr_RenderPlayerDisplay(true);
#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendInventoryRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh the player's inventory
	int										j;
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(84); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendInventoryRefresh");
		return;
	}
	

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	for (j=1;j<kMaxInventorySize;j++) // paranoid error checking
	{
		if (creature[player[i].creatureIndex].inventory[j].itemID>kMaxItemsUsed)
		{
			fi_WriteToErrorLog("nw_SendInventoryRefresh - Over max item number");
			creature[player[i].creatureIndex].inventory[j].itemID     = 0;
			creature[player[i].creatureIndex].inventory[j].itemCount  = 0;
		}

		if ((creature[player[i].creatureIndex].inventory[j].itemCount==0) && (creature[player[i].creatureIndex].inventory[j].itemID!=0))
		{
			fi_WriteToErrorLog("nw_SendInventoryRefresh - Count zero but item not");
			creature[player[i].creatureIndex].inventory[j].itemID     = 0;
			creature[player[i].creatureIndex].inventory[j].itemCount  = 0;
		}

		if (creature[player[i].creatureIndex].inventory[j].itemCount>kMaxInventoryItemCount)
		{
			fi_WriteToErrorLog("nw_SendInventoryRefresh - Over max item count");
			creature[player[i].creatureIndex].inventory[j].itemID     = 0;
			creature[player[i].creatureIndex].inventory[j].itemCount  = 0;
		}

		if (in_CanGroup(creature[player[i].creatureIndex].inventory[j].itemID)==false)
			if (creature[player[i].creatureIndex].inventory[j].itemCount>255)
			{
				fi_WriteToErrorLog("nw_SendInventoryRefresh - Over max item condition");
				creature[player[i].creatureIndex].inventory[j].itemCount=255;
			}
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gInventoryRefreshMessage.h);
		gInventoryRefreshMessage.h.what 				= kInventoryRefreshMessage;
		gInventoryRefreshMessage.h.to 					= player[i].id;
		gInventoryRefreshMessage.checksum       = validate;

		for (j=0;j<kMaxInventorySize;j++)
		{
			gInventoryRefreshMessage.item[j]		= creature[player[i].creatureIndex].inventory[j].itemID;
			gInventoryRefreshMessage.count[j]		= creature[player[i].creatureIndex].inventory[j].itemCount;
			gInventoryRefreshMessage.checksum   = gInventoryRefreshMessage.checksum + creature[player[i].creatureIndex].inventory[j].itemID + creature[player[i].creatureIndex].inventory[j].itemCount;
		}

		gInventoryRefreshMessage.h.messageLen 	= sizeof(gInventoryRefreshMessage);

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gInventoryRefreshMessage.h.messageLen;

		if (gInventoryRefreshMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendInventoryRefresh");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gInventoryRefreshMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendInventoryRefresh: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendContainerContents(int i) // server
// ----------------------------------------------------------------- //

{
	int										j;
	OSStatus 							status=noErr;
	int										theContainer;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];
	int                   trapDamage;
	//UInt32                sum;

//gr_ShowDebugLocation(85); // server only

	if ((i<1) || (i>=kMaxPlayers))
		return;

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	theContainer=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

	if (theContainer==0)
		return;

	if (in_ContainerOwner(i)==false)
	{
		if (gNetGame)
		{
			NSpClearMessageHeader(&gContainerContentsMessage.h);
			gContainerContentsMessage.h.what 							= kContainerContentsMessage;
			gContainerContentsMessage.h.to 								= player[i].id;
			gContainerContentsMessage.checksum						= 0;
			strcpy(gContainerContentsMessage.corpseName, "");
			gContainerContentsMessage.item[0]							= 0;
			gContainerContentsMessage.count[0]						= 0;

			for (j=1;j<kMaxInventorySize;j++)
			{
				gContainerContentsMessage.item[j]					= 60001;
				gContainerContentsMessage.count[j]				= 0;
			}

			gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
			messagesSent++;
			bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

			if (gContainerContentsMessage.h.messageLen>2000)
				fi_WriteToErrorLog("Large message in nw_SendContainerContents");

#endif
			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendContainerContents: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();

		return;
	}

	if (in_ContainerLocked(player[i].creatureIndex))  // locked container
	{
		//	nw_SendDisplayMessage(i,kContainerLocked);
		if (gNetGame)
		{
			NSpClearMessageHeader(&gContainerContentsMessage.h);
			gContainerContentsMessage.h.what 							= kContainerContentsMessage;
			gContainerContentsMessage.h.to 								= player[i].id;
			gContainerContentsMessage.checksum						= 0;
			strcpy(gContainerContentsMessage.corpseName, "");
			gContainerContentsMessage.item[0]							= 0;
			gContainerContentsMessage.count[0]						= 0;

			for (j=1;j<kMaxInventorySize;j++)
			{
				gContainerContentsMessage.item[j]					= 60000;
				gContainerContentsMessage.count[j]				= 0;
			}

			gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
			messagesSent++;
			bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

			if (gContainerContentsMessage.h.messageLen>2000)
				fi_WriteToErrorLog("Large message in nw_SendContainerContents");

#endif
			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendContainerContents: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
	}
	else
	{
		theContainer=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

		if (theContainer==0)
			return;

		if ((containerTrap[theContainer]>0) && (strcmp(containerTrapName[theContainer],player[i].playerNameLC)!=0))
		{
			trapDamage=containerTrap[theContainer]*5;
			//trapDamage=0;
			cr_AddSound(player[i].creatureIndex, kExplosionSnd);
			//creature[player[i].creatureIndex].soundFX					= kExplosionSnd;
			//creature[player[i].creatureIndex].soundFXTime			= TickCount();
			fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kLightExplosion, (2*60), 0);

			if (creature[player[i].creatureIndex].hitPoints<=trapDamage)
			{
				creature[player[i].creatureIndex].hitPoints=0;
				cr_setDieTime(player[i].creatureIndex,(TickCount()+120));
				NSpClearMessageHeader(&gContainerContentsMessage.h);
				gContainerContentsMessage.h.what 							= kContainerContentsMessage;
				gContainerContentsMessage.h.to 								= player[i].id;
				gContainerContentsMessage.checksum						= 0;
				strcpy(gContainerContentsMessage.corpseName, "");
				gContainerContentsMessage.item[0]							= 0;
				gContainerContentsMessage.count[0]						= 0;

				for (j=1;j<kMaxInventorySize;j++)
				{
					gContainerContentsMessage.item[j]					= 60001;
					gContainerContentsMessage.count[j]				= 0;
				}

				gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
				messagesSent++;
				bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

				if (gContainerContentsMessage.h.messageLen>2000)
					fi_WriteToErrorLog("Large message in nw_SendContainerContents");

#endif
				if (gNetGame!=NULL)
					status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

				if (status!=noErr)
				{
					strcpy(errorMessage,"nw_SendContainerContents: ");
					tb_IntegerToString(status,errorNumber);
					strcat(errorMessage,errorNumber);
					fi_WriteToErrorLog(errorMessage);
				}

				containerTrap[theContainer]=0;

				player[i].sendStatsRefresh=true;
				return;
			}

			creature[player[i].creatureIndex].hitPoints=creature[player[i].creatureIndex].hitPoints-trapDamage;

			containerTrap[theContainer]=0;
			player[i].sendStatsRefresh=true;
		}

		if ((containerMagicTrap[theContainer]>0) && (strcmp(containerMagicTrapName[theContainer],player[i].playerNameLC)!=0))
		{
			trapDamage=containerMagicTrap[theContainer]*5;
			//trapDamage=0;
			cr_AddSound(player[i].creatureIndex, kExplosionSnd);
			//creature[player[i].creatureIndex].soundFX					= kExplosionSnd;
			//creature[player[i].creatureIndex].soundFXTime			= TickCount();
			fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kLightExplosion, (2*60), 0);

			if (creature[player[i].creatureIndex].hitPoints<=trapDamage)
			{
				creature[player[i].creatureIndex].hitPoints=0;
				cr_setDieTime(player[i].creatureIndex,(TickCount()+120));
				NSpClearMessageHeader(&gContainerContentsMessage.h);
				gContainerContentsMessage.h.what 							= kContainerContentsMessage;
				gContainerContentsMessage.h.to 								= player[i].id;
				gContainerContentsMessage.checksum						= 0;
				strcpy(gContainerContentsMessage.corpseName, "");
				gContainerContentsMessage.item[0]							= 0;
				gContainerContentsMessage.count[0]						= 0;

				for (j=1;j<kMaxInventorySize;j++)
				{
					gContainerContentsMessage.item[j]					= 60001;
					gContainerContentsMessage.count[j]				= 0;
				}

				gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
				messagesSent++;
				bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

				if (gContainerContentsMessage.h.messageLen>2000)
					fi_WriteToErrorLog("Large message in nw_SendContainerContents");

#endif
				if (gNetGame!=NULL)
					status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

				if (status!=noErr)
				{
					strcpy(errorMessage,"nw_SendContainerContents: ");
					tb_IntegerToString(status,errorNumber);
					strcat(errorMessage,errorNumber);
					fi_WriteToErrorLog(errorMessage);
				}

				containerMagicTrap[theContainer]=0;

				player[i].sendStatsRefresh=true;
				return;
			}

			creature[player[i].creatureIndex].hitPoints=creature[player[i].creatureIndex].hitPoints-trapDamage;

			containerMagicTrap[theContainer]=0;
			player[i].sendStatsRefresh=true;
			//sk_ResistingSpells(i);
		}

		if (gNetGame)
		{


			/*
							  // --------- check for special quest items ----------
								for (j=0;j<kMaxInventorySize;j++)
								  {

			    				  if ((container[theContainer][j].itemID==kQueensRuby) && (container[theContainer][j].itemCount>0))
			    				    if ((in_IsQuestInParty(i, kQueensRubyQuest)==false) || (in_IsItemInParty(i, kQueensRuby)==true))   // queens ruby quest
			    				      {
			    				        container[theContainer][j].itemID=0;
			    				        container[theContainer][j].itemCount=0;
			    				        break;
			    				      }

			    				  if ((container[theContainer][j].itemID==kHolyDagger) && (container[theContainer][j].itemCount>0))
			    				    if ((in_IsQuestInParty(i, kHolyDaggerQuest)==false) || (in_IsItemInParty(i, kHolyDagger)==true))   // holy dagger quest
			    				      {
			    				        container[theContainer][j].itemID=0;
			    				        container[theContainer][j].itemCount=0;
			    				        break;
			    				      }

			    				  if ((container[theContainer][j].itemID==kAncientTome) && (container[theContainer][j].itemCount>0))
			    				    if ((in_IsQuestInParty(i, kTomeQuest)==false) || (in_IsItemInParty(i, kAncientTome)==true))   // tome quest
			    				      {
			    				        container[theContainer][j].itemID=0;
			    				        container[theContainer][j].itemCount=0;
			    				        break;
			    				      }

			    				  if ((container[theContainer][j].itemID==kIceCrystal) && (container[theContainer][j].itemCount>0))
			    				    if ((in_IsQuestInParty(i, kIceCrystalQuest)==false) || (in_IsItemInParty(i, kIceCrystalQuest)==true))   // ice crystal quest
			    				      {
			    				        container[theContainer][j].itemID=0;
			    				        container[theContainer][j].itemCount=0;
			    				        break;
			    				      }

			  				  }
			*/

			// --------------------------------------------------

			NSpClearMessageHeader(&gContainerContentsMessage.h);
			gContainerContentsMessage.h.what 							= kContainerContentsMessage;
			gContainerContentsMessage.h.to 								= player[i].id;
			gContainerContentsMessage.checksum						= in_InventoryCheckSum_s(i);

			for (j=0;j<kMaxInventorySize;j++)
			{
				if (j!=0)
					if ((container[theContainer][j].itemID>kMaxItemsUsed) || (container[theContainer][j].itemCount==0))  // paranoid error checking
					{
						container[theContainer][j].itemID     = 0;
						container[theContainer][j].itemCount  = 0;
					}

				if (strlen(containerName[theContainer])>14)
					containerName[theContainer][14]='\0';

				strcpy(gContainerContentsMessage.corpseName, containerName[theContainer]);

				gContainerContentsMessage.item[j]					= container[theContainer][j].itemID;

				gContainerContentsMessage.count[j]				= container[theContainer][j].itemCount;
			}

			gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
			messagesSent++;
			bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

			if (gContainerContentsMessage.h.messageLen>2000)
				fi_WriteToErrorLog("Large message in nw_SendContainerContents");

#endif
			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendContainerContents: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
	}
}

#endif

// ----------------------------------------------------------------- //
void nw_GetContainerContents(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ContainerContentsMessage 				*theMessage = (ContainerContentsMessage *) inMessage;
	int															i;

	//gr_ShowDebugLocation(86); // server only
	nw_ClearQueueOf(kGetContainerContents);

	if (theMessage->checksum!=0)
		if (theMessage->checksum!=in_InventoryCheckSum_c())	// inventory mismatch - request refresh
		{
			inventoryCheckSumFailures++;
			nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		}

	for (i=1;i<kMaxInventorySize;i++)  // paranoid error checking
		if ((theMessage->item[i]>kMaxItemsUsed) && (theMessage->item[i]<60000))
		{
			theMessage->item[i]		=	0;
			theMessage->count[i]	=	0;
		}

	strcpy(localContainerName,theMessage->corpseName);

	for (i=0;i<kMaxInventorySize;i++)
	{
		pickUpInventory[i].itemID			=	theMessage->item[i];
		pickUpInventory[i].itemCount	=	theMessage->count[i];
	}

#ifndef _SERVERONLY
	gr_RenderPackContentsWindow();

	//gr_RenderPlayerDisplay(true);
#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendBankContents(int i) // server
// ----------------------------------------------------------------- //

{
	int										j;
	OSStatus 							status=noErr;
	//int										theContainer;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];


	if ((i<1) || (i>=kMaxPlayers))
		return;

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (sh_GetShopID(i)==kBank)
	{
		if (gNetGame)
		{
			NSpClearMessageHeader(&gContainerContentsMessage.h);
			gContainerContentsMessage.h.what 							= kBankContentsMessage;
			gContainerContentsMessage.h.to 								= player[i].id;
			gContainerContentsMessage.checksum						= in_InventoryCheckSum_s(i);
			strcpy(gContainerContentsMessage.corpseName, "");

			for (j=1;j<kMaxInventorySize;j++)  // paranoid error checking
				if ((player[i].bank[j].itemID>kMaxItemsUsed) || (player[i].bank[j].itemCount==0))
				{
					player[i].bank[j].itemID			=	0;
					player[i].bank[j].itemCount		=	0;
				}

			for (j=0;j<kMaxInventorySize;j++)
			{
				gContainerContentsMessage.item[j]					= player[i].bank[j].itemID;
				gContainerContentsMessage.count[j]				= player[i].bank[j].itemCount;
			}

			gContainerContentsMessage.h.messageLen 				= sizeof(gContainerContentsMessage);

#ifdef _SERVERONLY
			messagesSent++;
			bytesSent=bytesSent+gContainerContentsMessage.h.messageLen;

			if (gContainerContentsMessage.h.messageLen>2000)
				fi_WriteToErrorLog("Large message in nw_SendBankContents");

#endif
			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gContainerContentsMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendBankContents: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();
	}
}

#endif

// ----------------------------------------------------------------- //
void nw_GetBankContents(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ContainerContentsMessage 				*theMessage = (ContainerContentsMessage *) inMessage;
	int															i;

	//gr_ShowDebugLocation(86); // server only
	//nw_ClearQueueOf(kGetBankContents);

	if (theMessage->checksum!=0)
		if (theMessage->checksum!=in_InventoryCheckSum_c())	// inventory mismatch - request refresh
		{
			inventoryCheckSumFailures++;
			nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		}

	for (i=1;i<kMaxInventorySize;i++)  // paranoid error checking
		if ((theMessage->item[i]>kMaxItemsUsed) && (theMessage->item[i]<60000))
		{
			theMessage->item[i]		=	0;
			theMessage->count[i]	=	0;
		}

	for (i=0;i<kMaxInventorySize;i++)
	{
		bankInventory[i].itemID			=	theMessage->item[i];
		bankInventory[i].itemCount	=	theMessage->count[i];
	}

#ifndef _SERVERONLY
	gr_RenderBankWindow();

	//gr_RenderPackContentsWindow();
	//gr_RenderPlayerDisplay(true);
#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendShopContents(int i) // server
// ----------------------------------------------------------------- //

{
	int										j;
	OSStatus 							status=noErr;
	int										theShop;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];


//gr_ShowDebugLocation(87); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendShopContents");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	theShop=sh_GetShopID(i);

	if (theShop==0)
		return;

	for (j=0;j<kMaxInventorySize;j++)  // paranoid error checking
		if (shop[theShop][j].itemID>kMaxItemsUsed)
		{
			shop[theShop][j].itemID					=	0;
			shop[theShop][j].cost						=	0;
			shop[theShop][j].forSale				=	false;
			shop[theShop][j].willBuy				=	false;
			shop[theShop][j].transactions		=	0;
			shop[theShop][j].availibility   = 0;
		}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gShopMessage.h);
		gShopMessage.h.what 							= kShopContentsMessage;
		gShopMessage.h.to 								= player[i].id;

		for (j=0;j<kMaxInventorySize;j++)
		{
			if ((shop[theShop][j].forSale) && (shop[theShop][j].itemID>0))
			{
				gShopMessage.item[j]			= shop[theShop][j].itemID;
				gShopMessage.cost[j]			= shop[theShop][j].cost;
			}
			else
			{
				gShopMessage.item[j]			= 0;
				gShopMessage.cost[j]			= 0;
			}
		}

		gShopMessage.h.messageLen 				= sizeof(gShopMessage);

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gShopMessage.h.messageLen;

		if (gShopMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendShopContents");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gShopMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendShopContents: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetShopContents(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ShopMessage 						*theMessage = (ShopMessage *) inMessage;
	int															i;

//gr_ShowDebugLocation(88); // server only
	nw_ClearQueueOf(kGetShopContents);

	for (i=0;i<kMaxInventorySize;i++)  // paranoid error checking
		if (theMessage->item[i]>kMaxItemsUsed)
		{
			theMessage->item[i]	=	0;
			theMessage->cost[i]	=	0;
		}

	for (i=0;i<kMaxInventorySize;i++)
	{
		shopInventory[i].itemID		=	theMessage->item[i];
		shopInventory[i].cost			=	theMessage->cost[i];
	}

#ifndef _SERVERONLY
	gr_RenderPlayerDisplay(true);

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendSellList(int i) // server
// ----------------------------------------------------------------- //

{
	int										j;
	OSStatus 							status=noErr;
	int										theShop;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];


//gr_ShowDebugLocation(89); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendSellList");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	theShop=sh_GetShopID(i);

	if (theShop==0)
		return;

	if (gNetGame)
	{
		for (j=0;j<kMaxInventorySize;j++)  // paranoid error checking
			if (shop[theShop][j].itemID>kMaxItemsUsed)
			{
				shop[theShop][j].itemID					=	0;
				shop[theShop][j].cost						=	0;
				shop[theShop][j].forSale				=	false;
				shop[theShop][j].willBuy				=	false;
				shop[theShop][j].transactions		=	0;
				shop[theShop][j].availibility   = 0;
			}

		NSpClearMessageHeader(&gShopMessage.h);

		gShopMessage.h.what 							= kSellListMessage;
		gShopMessage.h.to 								= player[i].id;

		for (j=0;j<kMaxInventorySize;j++)
		{
			if ((shop[theShop][j].willBuy) && (shop[theShop][j].itemID!=0))
			{
				gShopMessage.item[j]			= shop[theShop][j].itemID;
				
			/**/	//Dave add 04-25-2012 shops buy and sell store tokens for 50,000
				if (shop[theShop][j].itemID==kStoreToken)
					
						gShopMessage.cost[j]			= (shop[theShop][j].cost);
					
				
				else if (in_CanGroup(shop[theShop][j].itemID))
					gShopMessage.cost[j]			= (shop[theShop][j].cost/2);
				else
					gShopMessage.cost[j]			= (shop[theShop][j].cost/3);

				if (gShopMessage.cost[j]<=0)
					gShopMessage.cost[j]=1;
			}
			else
			{
				gShopMessage.item[j]			= 0;
				gShopMessage.cost[j]			= 0;
			}
		}

		gShopMessage.h.messageLen 				= sizeof(gShopMessage);

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gShopMessage.h.messageLen;

		if (gShopMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendSellList");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gShopMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendSellList: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetSellList(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ShopMessage 						*theMessage = (ShopMessage *) inMessage;
	int											i;

//gr_ShowDebugLocation(90); // server only
	nw_ClearQueueOf(kGetSellList);

	for (i=0;i<kMaxInventorySize;i++)  // paranoid error checking
		if (theMessage->item[i]>kMaxItemsUsed)
		{
			theMessage->item[i]	=	0;
			theMessage->cost[i]	=	0;
		}

	for (i=0;i<kMaxInventorySize;i++)
	{
		sellList[i].itemID		=	theMessage->item[i];
		sellList[i].cost			=	theMessage->cost[i];
	}

#ifndef _SERVERONLY
	gr_RenderPlayerDisplay(true);

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendSkillChange(int i, int skill, int level) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh a single player skill
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(91); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendSkillChange");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gSkillChangeMessage.h);
		gSkillChangeMessage.h.what 					= kSkillChangeMessage;
		gSkillChangeMessage.h.to 						= player[i].id;
		gSkillChangeMessage.skill						= (UInt8)skill;
		gSkillChangeMessage.level						= (UInt8)level;

		gSkillChangeMessage.checksum			  = skill+level+validate;

		gSkillChangeMessage.h.messageLen 	= sizeof(gSkillChangeMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gSkillChangeMessage.h.messageLen;

		if (gSkillChangeMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendSkillChange");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gSkillChangeMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendSkillChange: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
		else
			cr_AddSound(player[i].creatureIndex, kSkillLevelSnd);
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetSkillChange(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	SkillChangeMessage 				*theMessage = (SkillChangeMessage *) inMessage;
	char											skillName[kStringLength];
	char											message[kStringLength];
	int												i;

	if ((theMessage->skill+theMessage->level+validate)!=theMessage->checksum)
	{
		nw_SendAction(kRefreshSkills, kHighPriority, kSendNow,false);
		fi_WriteToErrorLog("Bad skill change");
		return;
	}

	tx_GetSkillName(theMessage->skill, skillName);

	for (i=0;i<strlen(skillName);i++)
		if (((int)skillName[i]>=65) && ((int)skillName[i]<=90)) // make skill name lower case
			skillName[i]=(char)((int)skillName[i]+32);

	strcpy(message,"Your ");

	strcat(message,skillName);

	strcat(message," skill level has increased.");

	gr_AddText(message,true,true);

#ifndef _SERVERONLY
	gr_RenderPlayerDisplay(true);

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendClientCommand(int i, int command) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendClientCommand");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gClientCommandMessage.h);
		gClientCommandMessage.h.what 					= kClientCommandMessage;
		gClientCommandMessage.h.to 						= player[i].id;
		gClientCommandMessage.command					= (UInt8)command;

		gClientCommandMessage.checksum			  = command+validate;

		gClientCommandMessage.h.messageLen 		= sizeof(gClientCommandMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gClientCommandMessage.h.messageLen;

		if (gClientCommandMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendClientCommand");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gClientCommandMessage.h, kSendMethod);

		if (status!=noErr)
		{
			strcpy(errorMessage,"nw_SendClientCommand: ");
			tb_IntegerToString(status,errorNumber);
			strcat(errorMessage,errorNumber);
			fi_WriteToErrorLog(errorMessage);
		}
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetClientCommand(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	ClientCommandMessage 			*theMessage = (ClientCommandMessage *) inMessage;
	int												i;

	if ((theMessage->command+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad client command");
		return;
	}

	if (theMessage->command==kClearNamesCommand) // -------------- clear names
	{
		for (i=0;i<kMaxPlayers;i++)
		{
			strcpy(playerNames[i],"");
			playerIDs[i]=0;
		}
	}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendSkillsRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh the player's skill list

	int										j;
	OSStatus 							status;

	//gr_ShowDebugLocation(93); // server only

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendSkillsRefresh");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gSkillsRefreshMessage.h);
		gSkillsRefreshMessage.h.what 					= kSkillsRefreshMessage;
		gSkillsRefreshMessage.h.to 						= player[i].id;
		gSkillsRefreshMessage.checksum        = validate;

		for (j=0;j<kMaxSkills;j++)
		{
			gSkillsRefreshMessage.skill[j]			= sk_GetSkillLevel(player[i].skill[j].level);
			gSkillsRefreshMessage.checksum			= gSkillsRefreshMessage.checksum + gSkillsRefreshMessage.skill[j];
		}

		gSkillsRefreshMessage.h.messageLen 	= sizeof(gSkillsRefreshMessage);

#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gSkillsRefreshMessage.h.messageLen;

		if (gSkillsRefreshMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendSkillsRefresh");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gSkillsRefreshMessage.h, kSendMethod);
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetRefreshSkills(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	SkillsRefreshMessage 				*theMessage = (SkillsRefreshMessage *) inMessage;
	int													i;
	UInt32                      sum=0;


	for (i=0;i<kMaxSkills;i++)
		sum	=	sum + theMessage->skill[i];

	if ((sum+validate)!=theMessage->checksum)
	{
		nw_SendAction(kRefreshSkills, kHighPriority, kSendNow,false);
		fi_WriteToErrorLog("Bad skills refresh");
		return;
	}

	gotSkillsRefresh=true;

	for (i=0;i<kMaxSkills;i++)
		localSkills[i].level			=	theMessage->skill[i];

#ifndef _SERVERONLY
	gr_RenderSkillsWindow();
#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendPetNameRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh the player's pet names
	int										j;
	OSStatus 							status;

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
	{
		fi_WriteToErrorLog("Out of Range - nw_SendSkillsRefresh");
		return;
	}

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	for (j=0;j<kMaxPets;j++)
		if (strcmp(player[i].petName[j],"")==0)
			strcpy(player[i].petName[j],"None");

	if (gNetGame)
	{
		NSpClearMessageHeader(&gPetNameRefreshMessage.h);
		gPetNameRefreshMessage.h.what 			  = kPetNamesRefreshMessage;
		gPetNameRefreshMessage.h.to 					= player[i].id;

		for (j=0;j<kMaxPets;j++)
			if (strlen(player[i].petName[j])<15)
				strcpy(gPetNameRefreshMessage.petName[j],player[i].petName[j]);

		gPetNameRefreshMessage.h.messageLen 	= sizeof(gPetNameRefreshMessage);

#ifdef _SERVERONLY
		messagesSent++;

		bytesSent=bytesSent+gPetNameRefreshMessage.h.messageLen;

		if (gPetNameRefreshMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendPetNameRefresh");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gPetNameRefreshMessage.h, kSendMethod);
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_GetRefreshPetNames(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	PetNameRefreshMessage 				*theMessage = (PetNameRefreshMessage *) inMessage;
	int													i;

	for (i=0;i<kMaxPets;i++)
	{
		if (strlen(theMessage->petName[i])>=15) // 11/6
		{
			fi_WriteToErrorLog("Corrupt pet name");
			strcpy(localPetName[i],"");
		}
		else
			strcpy(localPetName[i],theMessage->petName[i]);
	}

#ifndef _SERVERONLY
	gr_RenderPetsWindow();

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendRefreshToAllPlayers(void) // server
// ----------------------------------------------------------------- //

{
	// Sent from server to refresh the displays of all players

	int										i;
	unsigned long					currentTime=TickCount();
	//char debug[kStringLength];
	//char debugg[kStringLength];


	if (gNetGame)
		if (currentTime>nextTextRefresh)
		{
			for (i=1;i<kMaxPlayers;i++)
				if (player[i].id!=0)
				{
					nw_SendTextRefresh(i);
					// gs questionable 2/26/03 //  nw_GetMessages();
				}

			nextTextRefresh=currentTime+1;
		}

	if (gNetGame)
		if (currentTime>nextRefresh)
		{

			nbRefresh++;

			if (numberOfPlayersOnline<50)
				refreshFrequency=1;
			else if (numberOfPlayersOnline<65)
				refreshFrequency=1;
			else if (numberOfPlayersOnline<80)
				refreshFrequency=1;
			else if (numberOfPlayersOnline<95)
				refreshFrequency=1;
			else
				refreshFrequency=1;

			nextRefresh=currentTime+refreshFrequency;


			for (i=1;i<kMaxPlayers;i++)
				if (player[i].id!=0)
				{
					if (player[i].category!=0 && player[i].category!=8)
					{
						if ((player[i].skipNextRefresh==false))  // 11/4
						{
							if ((player[i].playerRefreshTime==0) || (currentTime>player[i].playerRefreshTime))
							{
								player[i].playerRefreshTime=0;
								nw_SendRefresh(i,false);
								// gs questionable 2/26/03 //  nw_GetMessages();
							}

							if (player[i].slow)
								player[i].skipNextRefresh=true;
						}
						else
							player[i].skipNextRefresh=false;
					}
					else
					{
						if ((player[i].skipNextRefresh==false) && (bytesSent<24000))  // 11/4
						{
							if ((player[i].playerRefreshTime==0) || (currentTime>player[i].playerRefreshTime))
							{
								player[i].playerRefreshTime=0;
								nw_SendRefresh(i,false);
								// gs questionable 2/26/03 //  nw_GetMessages();
							}

							if (player[i].slow)
								player[i].skipNextRefresh=true;
						}
						else
							player[i].skipNextRefresh=false;
					}
				}

			//if (bytesSent>20000)	nextRefresh=currentTime+30;
			//if (bytesSent>25000)	nextRefresh=currentTime+60;

			/*if (nbRefresh%1000==0)
			{
				tb_IntegerToString(currentTime - TickCount(), debug);
				strcpy(debugg, "Refresh : ");
				strcat(debugg, debug);
				fi_WriteToErrorLog(debugg);
			}*/

		}


}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_BuildTrail(int Ax, int Ay, int Bx, int By, int *length)
// ----------------------------------------------------------------- //

{
	int 		dX = abs(Bx-Ax);	// store the change in X and Y of the line EndPoints
	int 		dY = abs(By-Ay);
	int 		Xincr, Yincr;
	int			trailPtr=0;

//gr_ShowDebugLocation(96); // server only

	if ((Bx==0) && (By==0))
	{
		*length=-1;
		return;
	}

	if (Ax > Bx) { Xincr=-1; }
	else { Xincr=1; }	// which direction in X?

	if (Ay > By) { Yincr=-1; }
	else { Yincr=1; }	// which direction in Y?

	if (dX >= dY)	// if X is the indep}ent variable
	{
		int dPr 	= dY<<1;           // amount to increment decision if right is chosen (always)
		int dPru 	= dPr - (dX<<1);   // amount to increment decision if up is chosen
		int P 		= dPr - dX;  // decision variable start value

		for (; dX>=0; dX--)            // process each point in the line one at a time (just use dX)
		{
			fxTrailRow[trailPtr]=Ax;
			fxTrailCol[trailPtr]=Ay;
			trailPtr++;

			if (trailPtr>=100)
			{
				trailPtr=99;
				*length=trailPtr;
				return;
			}

			if (P > 0)               // is the pixel going right AND up?
			{
				Ax+=Xincr;	       // increment indep}ent variable
				Ay+=Yincr;         // increment dep}ent variable
				P+=dPru;           // increment decision (for up)
			}
			else                     // is the pixel just going right?
			{
				Ax+=Xincr;         // increment indep}ent variable
				P+=dPr;            // increment decision (for right)
			}
		}
	}
	else              // if Y is the indep}ent variable
	{
		int dPr 	= dX<<1;           // amount to increment decision if right is chosen (always)
		int dPru 	= dPr - (dY<<1);   // amount to increment decision if up is chosen
		int P 		= dPr - dY;  // decision variable start value

		for (; dY>=0; dY--)            // process each point in the line one at a time (just use dY)
		{
			fxTrailRow[trailPtr]=Ax;
			fxTrailCol[trailPtr]=Ay;
			trailPtr++;

			if (trailPtr>=100)
			{
				trailPtr=99;
				*length=trailPtr;
				return;
			}

			if (P > 0)               // is the pixel going up AND right?
			{
				Ax+=Xincr;         // increment dep}ent variable
				Ay+=Yincr;         // increment indep}ent variable
				P+=dPru;           // increment decision (for up)
			}
			else                     // is the pixel just going up?
			{
				Ay+=Yincr;         // increment indep}ent variable
				P+=dPr;            // increment decision (for right)
			}
		}
	}

	trailPtr--;

	*length=trailPtr;

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendTextRefresh(int i) // server
// ----------------------------------------------------------------- //

{
	int							j;
	TBoolean				stealth;
	TBoolean				SendTheText;
	int							row;
	int							col;
	int							offsetRow=creature[player[i].creatureIndex].row-13;
	int							offsetCol=creature[player[i].creatureIndex].col-13;
	unsigned long		currentTime=TickCount();
	char						stink[kStringLength];
	//char						roller[kStringLength]; //Dave
	int							rand;
	//int							rollChance; //Dave
	for (j=0;j<kMaxScreenElements;j++)	// build creature list
	{

		if (creatureMap[mapToScreenRow[j]+offsetRow][mapToScreenCol[j]+offsetCol]==0)
			continue;

		row=mapToScreenRow[j]+offsetRow;

		col=mapToScreenCol[j]+offsetCol;

		stealth=false;

		if (pl_AuthorizedTo(player[i].creatureIndex, kIsDeveloper)==false)
			if (creatureMap[row][col]!=0)
				if (creature[creatureMap[row][col]].playerIndex!=0)
					if (creature[creatureMap[row][col]].playerIndex!=i)
					{
						if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer))
							stealth=player[creature[creatureMap[row][col]].playerIndex].stealth;
						else
							if (pl_AuthorizedTo(creatureMap[row][col], kIsDeveloper))
								stealth=player[creature[creatureMap[row][col]].playerIndex].stealth;
					}


		//if (stealth==false)
		if (creature[creatureMap[row][col]].playerIndex>0)
			if (player[creature[creatureMap[row][col]].playerIndex].curse&kStenchCurse)
				if (currentTime>player[i].previousStenchMessage)
					if (i!=creature[creatureMap[row][col]].playerIndex)
					{
						if (stealth)
							rand=tb_Rnd(1,5);
						else
							rand=tb_Rnd(1,8);

						if (rand==1) 			nw_SendTextToClient(i, "There is an awful stench nearby.", 0);
						else if (rand==2) 		nw_SendTextToClient(i, "Something smells terrible.", 0);
						else if (rand==3) 		nw_SendTextToClient(i, "An overwhelming stench begins to make you feel dizzy.", 0);
						else if (rand==4) 		nw_SendTextToClient(i, "There is an awful stench nearby.", 0);
						else if (rand==5) 		nw_SendTextToClient(i, "A foul odor is in the air.", 0);
						else if (rand==6)
						{
							strcpy(stink,"A foul odor is coming from the direction of ");

							if (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "none")!=0 && (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "None")!=0))
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerNameTemp);
							else
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerName);

							strcat(stink,".");

							nw_SendTextToClient(i, stink, 0);
						}
						else if (rand==7)
						{
							strcpy(stink,"An overwhelming odor from ");

							if (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "none")!=0 && (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "None")!=0))
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerNameTemp);
							else
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerName);

							strcat(stink," begins to make you feel dizzy.");

							nw_SendTextToClient(i, stink, 0);
						}
						else
						{
							strcpy(stink,"A horrid smell from ");

							if (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "none")!=0 && (strcmp(player[creature[creatureMap[row][col]].playerIndex].playerNameTemp, "None")!=0))
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerNameTemp);
							else
								strcat(stink,player[creature[creatureMap[row][col]].playerIndex].playerName);

							strcat(stink," begins to make you feel woozy.");

							nw_SendTextToClient(i, stink, 0);
						}

						player[i].previousStenchMessage=currentTime+(60*tb_Rnd(40,150));
					}
		//Dave
		if (creature[creatureMap[row][col]].playerIndex>0)
		
				if (i!=creature[creatureMap[row][col]].playerIndex)
					{
						
						
					}
		
		//Dave^


		if ((creature[creatureMap[row][col]].hiding==false) && (stealth==false))	// if hiding then no need to include in refresh
		{
			if (creature[creatureMap[row][col]].talkWhen<TickCount())
				if (creature[creatureMap[row][col]].talkTime>player[i].logon)
					if (!pl_PlayerHeard(i,creature[creatureMap[row][col]].talkID))
					{
						SendTheText=true; // 017

						if ((player[creature[creatureMap[row][col]].playerIndex].dead==true) && (player[i].dead==false))    SendTheText=false;

						if (i==creature[creatureMap[row][col]].playerIndex)                                                 SendTheText=true;

						if (player[creature[creatureMap[row][col]].playerIndex].category!=0 && player[creature[creatureMap[row][col]].playerIndex].category!=8)                                SendTheText=true;

						if (player[i].category!=0 && player[i].category!=8)                                                                          SendTheText=true; // 0.1.4

						if (player[i].characterClass==1)
							if (sk_GetCharacterLevel(i)>=6)                                                                   SendTheText=true; // 0.1.7

						if (SendTheText)
							nw_SendTextToClient(i, creature[creatureMap[row][col]].talk, creatureMap[row][col]);
						else
						{
							if (creature[creatureMap[row][col]].playerIndex!=0)
								if ((player[creature[creatureMap[row][col]].playerIndex].dead==true))
									nw_SendTextToClient(i, "oooOOOoooOOOooo", creatureMap[row][col]);
						}
					}
		}
	}

}

#endif

#ifdef _SERVER

// ----------------------------------------------------------------- //
static pascal unsigned int nw_ReadProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	//if ( gReadBytesLeft ) {
	//   if ( size > gReadBytesLeft ) size = gReadBytesLeft;
	//   BlockMove(globalRead, buf, size);
	//   gReadBytesLeft -= size;
	//  return size;
	//} else {
	//   return 0;
	//}
}

// ----------------------------------------------------------------- //
static pascal void nw_WriteProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	//BlockMove(buf, globalWrite, size);
	//gWriteBytes=size;

}

// ----------------------------------------------------------------- //
void nw_CompressData(char */*inBuffer*/, int /*inSize*/, char */*outBuffer*/, int */*outSize*/, unsigned int /*type*/)
// ----------------------------------------------------------------- //

{
	//unsigned int					e;

	//gReadBytesLeft	=	inSize;
	//gWriteBytes			=	0;

	//globalRead			=inBuffer;
	//globalWrite			=outBuffer;

	//e=implode(nw_ReadProc, nw_WriteProc, WorkBuff, nil, type, 1024);

	//*outSize				=	gWriteBytes;

}

// ----------------------------------------------------------------- //
void nw_DecompressData(char */*inBuffer*/, int /*inSize*/, char */*outBuffer*/, int */*outSize*/)
// ----------------------------------------------------------------- //

{
	//unsigned int					e;

	//gReadBytesLeft	=	inSize;
	//gWriteBytes			=	0;

	//globalRead			=inBuffer;
	//globalWrite			=outBuffer;

	//e=explode(nw_ReadProc, nw_WriteProc, WorkBuff, nil);

	//*outSize				=	gWriteBytes;


}

// ----------------------------------------------------------------- //
static pascal unsigned int nw_imReadProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	// if ( gReadBytesLeft ) {
	//    if ( size > gReadBytesLeft ) size = gReadBytesLeft;
	//    BlockMove(&original[0], buf, size);
	//    gReadBytesLeft -= size;
	//    return size;
	// } else {
	//    return 0;
	// }
}

// ----------------------------------------------------------------- //
static pascal void nw_imWriteProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	//BlockMove(buf, &packed[0], size);
	//gWriteBytes=size;

}

// ----------------------------------------------------------------- //
static pascal unsigned int nw_exReadProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	// if ( gReadBytesLeft ) {
	//    if ( size > gReadBytesLeft ) size = gReadBytesLeft;
	//    BlockMove(&packed[0], buf, size);
	//    gReadBytesLeft -= size;
	//    return size;
	// } else {
	//    return 0;
	// }
}

// ----------------------------------------------------------------- //
static pascal void nw_exWriteProc(char */*buf*/, unsigned int /*size*/, void */*Param*/)
// ----------------------------------------------------------------- //
{

	//BlockMove(buf, &unpacked[0], size);
	//gWriteBytes=size;

}

// ----------------------------------------------------------------- //
void nw_CompressDataTest(int p, int length) // server
// ----------------------------------------------------------------- //

{
	//UInt8                 symbol[256];
	int                   i;
	int                   unique=0;
	FILE                  *errorStream;
	//char                  *src;
	//char                  *dst;
	//int                   start;
	//int                   destLen;
	//unsigned int					e;
	float									pc;
	int										newSize;
	int										unpackedSize;

	return;

	for (i=0;i<5000;i++)
	{
		original[i]=playerData[p][i];
		//original[i]=0;
		packed[i]=0;
		unpacked[i]=0;
	}

	//nw_CompressData((char*)&original[0], length, (char*)&packed[0], &newSize, CMP_BINARY);

	nw_DecompressData((char*)&packed[0], newSize, (char*)&unpacked[0], &unpackedSize);

	pc=(((float)newSize/(float)length)*100);


	//strcpy((char*)original,"This is a test of text compression.");
	//length=strlen((char*)original);

//  gReadBytesLeft=length;
//  start=length;
//  gWriteBytes=0;

//  e=implode(nw_imReadProc, nw_imWriteProc, WorkBuff, nil, CMP_BINARY, 1024);

//	gReadBytesLeft=gWriteBytes;

//  pc=(((float)gWriteBytes/(float)length)*100);

	errorStream = fopen("Analyze Data", "a" );

	if (errorStream != NULL)
	{
		fprintf( errorStream, "%s", "---------------------------------");
		fprintf( errorStream, "%s", "\n");
		fprintf( errorStream, "%i", length);
		fprintf( errorStream, "%s", "\n");
		fprintf( errorStream, "%i", newSize);
		fprintf( errorStream, "%s", "\n");
		fprintf( errorStream, "%i", (int)pc);
		fprintf( errorStream, "%s", "\n");
		fclose( errorStream );
	}

//  e=explode(nw_exReadProc, nw_exWriteProc, WorkBuff, nil);

// src=(char*)&playerData[p];
// start=(int)&packed;
// dst=(char*)&packed;

// PackBits(&src,&dst,length+1);

// destLen=((int)dst-start)+2;

// src=(char*)&packed;
// dst=(char*)&unpacked;
// UnpackBits(&src,&dst,length+1);

// errorStream = fopen("Analyze Data", "a" );
// if (errorStream != NULL)
//   {
//     fprintf( errorStream, "%s", "---------------------------------");
//     fprintf( errorStream, "%s", "\n");
//     fprintf( errorStream, "%i", length);
//     fprintf( errorStream, "%s", "\n");
//     fprintf( errorStream, "%i", destLen);
//     fprintf( errorStream, "%s", "\n");
// 	  fclose( errorStream );
// 	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendRefresh(int i, TBoolean forceRefresh) // server
// ----------------------------------------------------------------- //

{
	// Sent from the server to refresh the display of a single player
	//
	// forceRefresh - Send data even if there was no change from previous frame.  This if for when
	// the player moves with no objects or creatures in view.

	int																	j;
	int																	k;
	int																	p;
	int																	q;
	int																	w;
	int                                 								s;
	int																	t;
	int																	cpt;
	int																	row;
	int																	col;
	int																	row2;
	int																	col2;
	//int																	row3;
	//int																	col3;
	//int																	stepRow;
	//int																	stepCol;
	OSStatus 														status=noErr;
	int																	dataPtr;
	TBoolean														SendData;
	int																	hits;
	float																fhits;
	float																fmhits;
	unsigned long												currentTime=TickCount();
	//PlayerRefreshMessage								*gPlayerRefreshMessage;
	//int																	m;
	//UInt8																*theData;
	//TBoolean														fxFound;
	int																	target;
	//int																	lastKnowPosition;
	int																	monsterPtr;
	int																	containerPtr;
	int																	tracksPtr;
	int																	bloodStainPtr;
	int																	fxPtr;
	int																	soundFX;
	int																	detected=0;
	int																	len;		// fx trail length
	int																	edge;
	int																	b1;
	int																	b2;
	UInt8																tempData;
	int                                 soundsSent[6];
	TBoolean                            soundAlreadySent;
	char                                errorMessage[kStringLength];
	char                                errorNumber[kStringLength];
	//TBoolean                            SendTheText;
	TBoolean														stealth;
	int																	playerTarget=255;
	int                                 d1;
	int                                 d2;
	int                                 d3;
	int                                 d4;
	//float																damage;
	//float																resist;
	char                                debugStr[kStringLength];
	TBoolean                            found;
	TBoolean                            macroing;
	TBoolean                            gmCharacter;
	TBoolean                            doAttack;
	//int                                 debugtemp;
	TBoolean                            okToAttack;
	TBoolean                            doRandomSound;
	int																	desiredAttackCount;
	int																	drop;
	int																	maxTarget;
	int																	maxTargetIndex;
	int																	possibleAttackers;
	//FILE									              *errorStream; // gs test
	//int									oldhunger;
	int									toGuard=0;
	int									hallsPtr=0;
	int									halls[kMaxGuildHalls];
	int									theClass;
//	int									wmLevel;
//	int									range;
	
//gr_ShowDebugLocation(97); // server only

	if ((i<1) || (i>=kMaxPlayers))
		return;

	if (player[i].id==0)
		return;

	//if (player[i].online==false)
	//return;


	if (targetFrame==0)
		targetFrame=1;
	else
		targetFrame=0;

	if (player[i].gathering>0)	// check status of player resource harvest
	{
		if (player[i].meditating) //Harvesting breaks meditation
		{
			player[i].meditating	=false;
			nw_SendDisplayMessage(i,kConcentrationDisturbed);
		}

		player[i].gathering--;

		if (player[i].gathering==0)
		{
			creature[player[i].creatureIndex].strikePose=0;
			creature[player[i].creatureIndex].numberOfStrikes=0;
			creature[player[i].creatureIndex].nextStrikeTime=0;

			if (player[i].gatherType==kLumberjacking)
				sk_Lumberjacking(i);
			else if (player[i].gatherType==kMining)
				sk_Mining(i);
			else if (player[i].gatherType==kFishing)
				sk_Fishing(i);
			else if (player[i].gatherType==255)	// making item
				it_MakeItem(i,player[i].gatherRow);
		}
	}

	if (creature[player[i].creatureIndex].hitPoints>=creature[player[i].creatureIndex].strength)
		player[i].healing=false;

	if (gNetGame)
		if ((player[i].active) && (player[i].online==false))	// ------ player logged off ... check for remove time
		{
			if (((player[i].lastCheckIn+(2700*kStayInWorldTime))<currentTime))
			{
				cr_ClearAttackTarget(player[i].creatureIndex);
				nw_SendClearName(i);
				fi_Logout(i);
				creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
				cr_ClearCreatureRecord(player[i].creatureIndex);
				pl_ClearPlayerRecord(i);
#ifdef _SERVERONLY
				gr_DrawServerNameAt(i);
#endif
			}
		}
		else if (((player[i].lastCheckIn+(3600*kInactivetyLogoff))<currentTime) && (player[i].category==0 || player[i].category==8))	// ------ check for player inactivity
		{
			cr_ClearAttackTarget(player[i].creatureIndex);
			nw_SendPlayerLeft(player[i].id);
			nw_RemovePlayer(player[i].id);
			//fi_WriteToErrorLog("Inactivity - Remove Player:");
			//fi_WriteToErrorLog(player[i].playerName);
			nw_SendClearName(i);
			fi_Logout(i);
			creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
			cr_ClearCreatureRecord(player[i].creatureIndex);
			pl_ClearPlayerRecord(i);
#ifdef _SERVERONLY
			gr_DrawServerNameAt(i);
#endif
		}
		else if (player[i].active)	// ------ everything ok ... Send refresh
		{
#ifdef _REFRESHTEST
			fi_WriteToErrorLog("1");
#endif

			if (creature[player[i].creatureIndex].hunger>0)	// ------ decrement hunger counter
				if (tb_Rnd(1,3)==1)
				{
					creature[player[i].creatureIndex].hunger--;

					if (creature[player[i].creatureIndex].hunger==0)
						if (player[i].dead==false)
							nw_SendDisplayMessage(i,kHungry);

					if ((creature[player[i].creatureIndex].hunger==0) || (creature[player[i].creatureIndex].hunger==1749) || (creature[player[i].creatureIndex].hunger==3499) || (creature[player[i].creatureIndex].hunger==5249) || (creature[player[i].creatureIndex].hunger==6999)) //Hunger state change
						player[i].sendStatsRefresh=true;
				}
			//Dave 12-05-2012 qChar and dev don't go  AFK
			if (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper)) 
				{player[i].afk=false;}
				
			
			else if (player[i].lastCheckIn+(3600*kAfkTime)<currentTime)
				player[i].afk=true;
			else
				player[i].afk=false;

			//if (currentTime>(player[i].talkTime+(60*8)))	// ------ check for talk erase time
			//	{
			//		strcpy(player[i].talk, "");
			//		player[i].talkTime=0;
			//		player[i].talkID=0;
			//	}

			player[i].totalRefresh++;

			if (player[i].totalRefresh>25)
			{
				player[i].sendStatsRefresh=true;
				player[i].sendPetsRefresh=true;
				player[i].totalRefresh=0;
			}

			SendData=false;

			dataPtr=0;

			monsterPtr=0;
			containerPtr=0;
			tracksPtr=0;
			bloodStainPtr=0;
			fxPtr=0;

			tempData=dayNight;

			if (player[i].category!=0 && player[i].category!=8)  // merge 7/27
				tempData=0;

			if (player[i].dead)
				tempData=tempData+127;

			gPlayerRefreshMessage2.data[dataPtr]	=	tempData;

			if (playerData[i][dataPtr]!=tempData) SendData=true;

			playerData[i][dataPtr]	=	tempData;

			dataPtr++;

			// check for standing in bank/shop ------------------//
			tempData=0;

			for (j=0;j<kMaxBanks;j++)
				if (bankRect[j].right!=0)
				{
					if ((creature[player[i].creatureIndex].row>=bankRect[j].left) && (creature[player[i].creatureIndex].row<=bankRect[j].right))
						if ((creature[player[i].creatureIndex].col>=bankRect[j].top) && (creature[player[i].creatureIndex].col<=bankRect[j].bottom))
						{
							tempData=1;
							break;
						}
				}
				else
					break;

			for (j=1;j<kMaxShops;j++)
				if (shopRect[j].right!=0)
				{
					if ((creature[player[i].creatureIndex].row>=shopRect[j].left) && (creature[player[i].creatureIndex].row<=shopRect[j].right))
						if ((creature[player[i].creatureIndex].col>=shopRect[j].top) && (creature[player[i].creatureIndex].col<=shopRect[j].bottom))
						{
							tempData=2;
							break;
						}
				}
				else
					break;

			for (j=0;j<kMaxChessBoards;j++)
				if (chessBoardRow1[j]!=0)
				{
					if ((chessBoardRow1[j]==creature[player[i].creatureIndex].row) && (chessBoardCol1[j]==creature[player[i].creatureIndex].col))
					{
						tempData=tempData+4;
						break;
					}
					else if ((chessBoardRow2[j]==creature[player[i].creatureIndex].row) && (chessBoardCol2[j]==creature[player[i].creatureIndex].col))
					{
						tempData=tempData+4;
						break;
					}
				}
				else
					break;

			if ((tempData==0) || (tempData==4))
				if (creature[player[i].creatureIndex].attackTarget==0)
					if (sh_StandingNextToTrader(i))
						tempData=tempData+8;

			gPlayerRefreshMessage2.data[dataPtr]	=	tempData;

			if (playerData[i][dataPtr]!=tempData) SendData=true;

			playerData[i][dataPtr]	=	tempData;

			dataPtr++;

			//---------------------------------------------------//

			nw_CheckForFriends(i, &d1, &d2, &d3, &d4);

			if (d1>255)
				fi_WriteToErrorLog("d1 error");	// beta debug

			gPlayerRefreshMessage2.data[dataPtr]	=	d1;

			if (playerData[i][dataPtr]!=d1) SendData=true;

			playerData[i][dataPtr]	=	d1;

			dataPtr++;

			if (d2>255)
				fi_WriteToErrorLog("d2 error");	// beta debug

			gPlayerRefreshMessage2.data[dataPtr]	=	d2;

			if (playerData[i][dataPtr]!=d2) SendData=true;

			playerData[i][dataPtr]	=	d2;

			dataPtr++;

			if (d3>255)
				fi_WriteToErrorLog("d3 error");	// beta debug

			gPlayerRefreshMessage2.data[dataPtr]	=	d3;

			if (playerData[i][dataPtr]!=d3) SendData=true;

			playerData[i][dataPtr]	=	d3;

			dataPtr++;

			if (d4>255)
				fi_WriteToErrorLog("d4 error");	// beta debug

			gPlayerRefreshMessage2.data[dataPtr]	=	d4;

			if (playerData[i][dataPtr]!=d4) SendData=true;

			playerData[i][dataPtr]	=	d4;

			dataPtr++;

			for (cpt=0; cpt<kMaxGuildHalls; cpt++)
				halls[cpt]=0;

			for (j=0;j<kMaxScreenElements;j++)	// build creature list
			{
				row=mapToScreenRow[j]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[j]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (containerMap[row][col]!=0)	// build container list
				{
					containerList[containerPtr]=j;
					containerPtr++;
				}

				if (tracksMap[row][col]!=0)	// build tracks list
				{
					tracksList[tracksPtr]=j;
					tracksPtr++;
				}

				if (bloodStainMap[row][col]!=0)	// build blood stain list
				{
					bloodStainList[bloodStainPtr]=j;
					bloodStainPtr++;
				}

				if (fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)]!=0)	// build terrain fx list
				{
					//check for magic plant
					//if (serverFXRow[fxMap[row][col]]==0 || serverFXCol[fxMap[row][col]]==0)
					//	serverFX[fxMap[row][col]]=0;
					//else
					{
						fxList[fxPtr]=j;
						fxPtr++;
					}
				}


				if (guildMap[row][col]!=0) //Build Guild Halls list
				{
					t=guildMap[row][col];
					cpt=0;
					found=false;

					while (cpt<hallsPtr)
					{
						if (halls[cpt]==guildMap[row][col])
						{
							found=true;
							break;
						}

						cpt++;
					}


					if (!found)
						halls[hallsPtr++]=guildMap[row][col];

				}

				if (creatureMap[row][col]!=0) // build monster list
				{
					monsterList[monsterPtr]=j;
					monsterPtr++;
					playerState[i][targetFrame][j]=creatureMap[row][col];
					creature[creatureMap[row][col]].respawn=false;
				}
				else
				{
					playerState[i][targetFrame][j]=0;
					continue;
				}

				stealth=false;

				if (pl_AuthorizedTo(player[i].creatureIndex, kIsDeveloper)==false)
					if (creatureMap[row][col]!=0)
						if (creature[creatureMap[row][col]].playerIndex!=0)
							if (creature[creatureMap[row][col]].playerIndex!=i)
							{
								if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer))
									stealth=player[creature[creatureMap[row][col]].playerIndex].stealth;
								else
									if (pl_AuthorizedTo(creatureMap[row][col], kIsDeveloper))
										stealth=player[creature[creatureMap[row][col]].playerIndex].stealth;
							}
				
				if (stealth)
					continue;

				if (creatureMap[row][col]!=0)
					if ((creature[creatureMap[row][col]].hiding==false))	// if hiding then no need to include in refresh
					{
						if (creature[player[i].creatureIndex].attackTarget!=0)
							if (creature[player[i].creatureIndex].attackTarget==creatureMap[row][col])  // ?? going to target position 0 when no target & no creature at row,col
								playerTarget=j;

						if (j>255)
							fi_WriteToErrorLog("j 1 error");	// beta debug

						gPlayerRefreshMessage2.data[dataPtr]	= j;

						if (playerData[i][dataPtr]!=j) SendData=true;

						playerData[i][dataPtr]	=	j;

						dataPtr++;

						if (creature[creatureMap[row][col]].playerIndex!=0)
						{
							if (player[creature[creatureMap[row][col]].playerIndex].afk)
								gPlayerRefreshMessage2.data[dataPtr]=1;
							else
								gPlayerRefreshMessage2.data[dataPtr]=0;
						}
						else
							gPlayerRefreshMessage2.data[dataPtr]=0;

						if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

						playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

						dataPtr++;

						gPlayerRefreshMessage2.data[dataPtr]=0;

						if (creature[creatureMap[row][col]].attackTarget==player[i].creatureIndex)
							gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 1;

						if (creature[creatureMap[row][col]].poison>0)
							gPlayerRefreshMessage2.data[dataPtr]  = gPlayerRefreshMessage2.data[dataPtr] + 2;

						//-------- new stuff ----------//

						if (creature[creatureMap[row][col]].regeneratingTime!=0)
							gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 64;

						if (creature[creatureMap[row][col]].master!=0)
						{
							/*if (creature[creature[creatureMap[row][col]].master].playerIndex == i)
								gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 128;
							  else*/
							if (pl_AuthorizedTo(creature[creatureMap[row][col]].master, kIsAnyGM)==false && creature[creatureMap[row][col]].npcIndex==0)
								gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 4;
							else if (player[i].category!=0 && player[i].category!=8)
								gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 16;
						}

						if (creature[creatureMap[row][col]].playerIndex!=0)
						{

							if (pl_AuthorizedTo(creatureMap[row][col], kIsAnyGM))
								gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 8;

							if (player[i].category!=0 && player[i].category!=8)
								if ((player[creature[creatureMap[row][col]].playerIndex].category==4) || (player[creature[creatureMap[row][col]].playerIndex].category==5))
									gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 16;

							if (player[creature[creatureMap[row][col]].playerIndex].stealth)
								gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 32;

							//if (player[creature[creatureMap[row][col]].playerIndex].online)
							//    gPlayerRefreshMessage2.data[dataPtr]	= gPlayerRefreshMessage2.data[dataPtr] + 64;

						}

						//-----------------------------//

						if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

						playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

						dataPtr++;

						//-----------------------------//									begin test

						b1=creatureMap[row][col]/256;

						b2=creatureMap[row][col]-(b1*256);

						// if (creature[creatureMap[row][col]].playerIndex!=0)
						//   if (player[creature[creatureMap[row][col]].playerIndex].online==false)
						//     {
						//       b1=0;
						//       b2=0;
						//     }

						gPlayerRefreshMessage2.data[dataPtr]	=	b1;

						if (playerData[i][dataPtr]!=b1) SendData=true;

						playerData[i][dataPtr]	=	b1;

						dataPtr++;

						gPlayerRefreshMessage2.data[dataPtr]	=	b2;

						if (playerData[i][dataPtr]!=b2) SendData=true;

						playerData[i][dataPtr]	=	b2;

						dataPtr++;

						//-----------------------------//									end test


						if (creature[creatureMap[row][col]].id>255)
							fi_WriteToErrorLog("creature[creatureMap[row][col]].id error");	// beta debug

						if (creature[creatureMap[row][col]].playerIndex!=0) // mod 8/25
						{
							if (player[creature[creatureMap[row][col]].playerIndex].morph==0) // mod 8/25
							{
								gPlayerRefreshMessage2.data[dataPtr]	=	creature[creatureMap[row][col]].id;

								if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].id) SendData=true;

								playerData[i][dataPtr]	=	creature[creatureMap[row][col]].id;
								
							}
							else
							{
								gPlayerRefreshMessage2.data[dataPtr]	=	player[creature[creatureMap[row][col]].playerIndex].morph+127;

								if (playerData[i][dataPtr]!=player[creature[creatureMap[row][col]].playerIndex].morph+127) SendData=true;

								playerData[i][dataPtr]	=	player[creature[creatureMap[row][col]].playerIndex].morph+127;
								

								//debugtemp=creature[creatureMap[row][col]].strikePose;
								//if (debugtemp!=0)
								if (creature[creatureMap[row][col]].strikePose==2)  // 8/30
								{
									if (creature[creatureMap[row][col]].facing<10)
										creature[creatureMap[row][col]].facing=creature[creatureMap[row][col]].facing+10;
								}
								else
								{
									if (creature[creatureMap[row][col]].facing>9)
										creature[creatureMap[row][col]].facing=creature[creatureMap[row][col]].facing-10;
								}
							}
						}
						else
						{
							gPlayerRefreshMessage2.data[dataPtr]	=	creature[creatureMap[row][col]].id;

							if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].id) SendData=true;

							playerData[i][dataPtr]	=	creature[creatureMap[row][col]].id;
							
						}

						dataPtr++;

						if (creature[creatureMap[row][col]].facing>255)
							fi_WriteToErrorLog("creature[creatureMap[row][col]].facing error");	// beta debug

						gPlayerRefreshMessage2.data[dataPtr]	=	creature[creatureMap[row][col]].facing;

						if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].facing) SendData=true;

						playerData[i][dataPtr]	=	creature[creatureMap[row][col]].facing;

						dataPtr++;

						if (creature[creatureMap[row][col]].id==kNPCHuman || creature[creatureMap[row][col]].id==kNPCHumanFemale)
						{

							// ------- check for chat text
							//if ((creature[creatureMap[row][col]].npcIndex<0) || (creature[creatureMap[row][col]].npcIndex>=kMaxNPCs))			// beta debug
							//	fi_WriteToErrorLog("Out of Range - npc - nw_SendRefresh");

							//if ((npc[creature[creatureMap[row][col]].npcIndex].talkTime>=player[i].lastTalkRefresh) && (player[i].lastTalkRefresh!=0))
							//	if (player[i].dead==false)
							//	  {
							// nw_SendNPCTextToClient(i, creature[creatureMap[row][col]].npcIndex, j); // chat change in 0.1.2 breaks NPC talking
							//		}
							// ----------------------------

							if (creature[creatureMap[row][col]].strikePose>255)
								fi_WriteToErrorLog("creature[creatureMap[row][col]].strikePose error");	// beta debug

							gPlayerRefreshMessage2.data[dataPtr]=creature[creatureMap[row][col]].strikePose;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].legs>255)
								fi_WriteToErrorLog("npc[creature[creatureMap[row][col]].npcIndex].legs error");	// beta debug

							if (npc[creature[creatureMap[row][col]].npcIndex].legs==0)
								gPlayerRefreshMessage2.data[dataPtr]	=	0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].legs;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].torso>255)
								fi_WriteToErrorLog("npc[creature[creatureMap[row][col]].npcIndex].torso error");	// beta debug

							if (npc[creature[creatureMap[row][col]].npcIndex].torso==0)
								gPlayerRefreshMessage2.data[dataPtr] = 0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].torso;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].type==0)
								gPlayerRefreshMessage2.data[dataPtr] = 0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].type;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].NPCSex==0)
								gPlayerRefreshMessage2.data[dataPtr] = 0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].NPCSex;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].NPCHairStyle==0)
								gPlayerRefreshMessage2.data[dataPtr] = 0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].NPCHairStyle;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

							if (npc[creature[creatureMap[row][col]].npcIndex].NPCHairColor==0)
								gPlayerRefreshMessage2.data[dataPtr] = 0;
							else
								gPlayerRefreshMessage2.data[dataPtr]	=	npc[creature[creatureMap[row][col]].npcIndex].NPCHairColor;

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;

						}

						if (creature[creatureMap[row][col]].id==kHuman)
							if (player[creature[creatureMap[row][col]].playerIndex].morph==0) // mod 8/25
							{

								/*
								  if (player[creature[creatureMap[row][col]].playerIndex].talkTime>player[i].logon)
								if (!pl_PlayerHeard(i,player[creature[creatureMap[row][col]].playerIndex].talkID))
								  	  {
								  	    SendTheText=true; // 017
								  		  if ((player[creature[creatureMap[row][col]].playerIndex].dead==true) && (player[i].dead==false))    SendTheText=false;
								  		  if (i==creature[creatureMap[row][col]].playerIndex)                                                 SendTheText=true;
								  		  if (player[creature[creatureMap[row][col]].playerIndex].category==1)                                SendTheText=true;
								  		  if (player[creature[creatureMap[row][col]].playerIndex].category==2)                                SendTheText=true;
								  		  if (player[creature[creatureMap[row][col]].playerIndex].category==4)                                SendTheText=true; // merge 7/27
								  		  if (player[creature[creatureMap[row][col]].playerIndex].category==5)                                SendTheText=true; // merge 7/27
								  		  if (player[i].category==1)                                                                          SendTheText=true; // 0.1.4
								  		  if (player[i].category==2)                                                                          SendTheText=true; // 0.1.4
								  		  if (player[i].category==4)                                                                          SendTheText=true; // merge 7/27
								  		  if (player[i].category==5)                                                                          SendTheText=true; // merge 7/27
								  		  if (player[i].characterClass==1)
								  		    if (sk_GetCharacterLevel(i)>=6)                                                                   SendTheText=true; // 0.1.7
								  		  if (SendTheText)
								  			  nw_SendTextToClient(i, player[creature[creatureMap[row][col]].playerIndex].talk, creature[creatureMap[row][col]].playerIndex);
								  		  else if ((player[creature[creatureMap[row][col]].playerIndex].dead==true))
								  			  nw_SendTextToClient(i, "oooOOOoooOOOooo", creature[creatureMap[row][col]].playerIndex);
								  	  }
								  */


								// ----------------------------

								/*
								   										if (player[creature[creatureMap[row][col]].playerIndex].online)
								   										  {
								 if (creature[creatureMap[row][col]].playerIndex>255)
								 fi_WriteToErrorLog("creature[creatureMap[row][col]].playerIndex error");	// beta debug

								       										gPlayerRefreshMessage2.data[dataPtr]=creature[creatureMap[row][col]].playerIndex;
								       										if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;
								       										playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];
								       										dataPtr++;
								                         }
								                       else
								   										  {
								       										gPlayerRefreshMessage2.data[dataPtr]=0;
								       										if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;
								       										playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];
								       										dataPtr++;
								                         }
								*/

								if (creature[creatureMap[row][col]].strikePose>255)
									fi_WriteToErrorLog("creature[creatureMap[row][col]].strikePose error");	// beta debug

								gPlayerRefreshMessage2.data[dataPtr]=player[creature[creatureMap[row][col]].playerIndex].gender;

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								gPlayerRefreshMessage2.data[dataPtr]=player[creature[creatureMap[row][col]].playerIndex].hairStyle;

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								gPlayerRefreshMessage2.data[dataPtr]=player[creature[creatureMap[row][col]].playerIndex].hairColor;

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								gPlayerRefreshMessage2.data[dataPtr]=creature[creatureMap[row][col]].strikePose;

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kHead]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kHead]].itemID>255)
									{
										fi_WriteToErrorLog("head error -----------");	// beta debug
										tb_IntegerToString(creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kHead]].itemID,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										tb_IntegerToString(kHead,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										fi_WriteToErrorLog(player[creature[creatureMap[row][col]].playerIndex].playerName);	// beta debug
									}

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kHead]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]].itemID>255)
									{
										fi_WriteToErrorLog(player[creature[creatureMap[row][col]].playerIndex].playerName);	// beta debug
										fi_WriteToErrorLog("right hand error");	// beta debug
										tb_IntegerToString(creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]].itemID,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										tb_IntegerToString(kRightHand,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
									}

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kLeftHand]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLeftHand]].itemID>255)
										fi_WriteToErrorLog("left hand error");	// beta debug

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLeftHand]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kLegs]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLegs]].itemID>255)
										fi_WriteToErrorLog("legs hand error");	// beta debug

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLegs]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]].itemID>255)
									{
										fi_WriteToErrorLog("torso error -----------");	// beta debug
										tb_IntegerToString(creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]].itemID,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										tb_IntegerToString(kTorso,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										fi_WriteToErrorLog(player[creature[creatureMap[row][col]].playerIndex].playerName);	// beta debug
									}

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]].itemID>255)
									{
										fi_WriteToErrorLog("feet error -----------");	// beta debug
										tb_IntegerToString(creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]].itemID,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										tb_IntegerToString(kFeet,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										fi_WriteToErrorLog(player[creature[creatureMap[row][col]].playerIndex].playerName);	// beta debug
									}

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

								if (player[creature[creatureMap[row][col]].playerIndex].arm[kArms]==0)
									gPlayerRefreshMessage2.data[dataPtr]=0;
								else
								{
									if (creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kArms]].itemID>255)
									{
										fi_WriteToErrorLog("arms error -----------");	// beta debug
										tb_IntegerToString(creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kArms]].itemID,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										tb_IntegerToString(kArms,debugStr);
										fi_WriteToErrorLog(debugStr);	// beta debug
										fi_WriteToErrorLog(player[creature[creatureMap[row][col]].playerIndex].playerName);	// beta debug
									}

									gPlayerRefreshMessage2.data[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kArms]].itemID;
								}

								if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

								playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

								dataPtr++;

							}

						if ((creature[creatureMap[row][col]].timeStamp>=player[i].lastRefresh) && (player[i].lastRefresh!=0))
						{
							// creature moved since last player refresh

							if (j==90)
								gPlayerRefreshMessage2.data[dataPtr]=4;
							else
							{
								gPlayerRefreshMessage2.data[dataPtr]	=	creature[creatureMap[row][col]].moveDirection;

								if (creature[creatureMap[row][col]].moveDirection>15)
									fi_WriteToErrorLog("move direction 1 error");	// beta debug
							}


							if (creature[creatureMap[row][col]].hitPoints==0)
							{
								hits=0;
							}
							else
							{
								fhits=creature[creatureMap[row][col]].hitPoints;

								if (creature[creatureMap[row][col]].id==kHuman)
								{
									fmhits=creature[creatureMap[row][col]].strength;

									if (fhits>0)
										if (creature[creatureMap[row][col]].playerIndex>0)
											if (player[creature[creatureMap[row][col]].playerIndex].dead)
											{
												fhits=0;
												creature[creatureMap[row][col]].poison																			=	0;
												creature[creatureMap[row][col]].paralyzed																		= 0;
												creature[creatureMap[row][col]].disorientation															= 0;
												creature[creatureMap[row][col]].lifespan															      = 0;
												creature[creatureMap[row][col]].calmUntil															      = 0;
												creature[creatureMap[row][col]].attackTarget																=	0;
												creature[creatureMap[row][col]].reflectDamage																=	0;
												creature[creatureMap[row][col]].hitPoints																		=	0;
												creature[creatureMap[row][col]].magicPoints																	=	0;
												creature[creatureMap[row][col]].magicReflection															= false;
												creature[creatureMap[row][col]].strengthOffset															=	0;
												creature[creatureMap[row][col]].intelligenceOffset													=	0;
												creature[creatureMap[row][col]].dexterityOffset															= 0;
												creature[creatureMap[row][col]].strengthTime																=	0;
												creature[creatureMap[row][col]].intelligenceTime														=	0;
												creature[creatureMap[row][col]].dexterityTime																= 0;
												creature[creatureMap[row][col]].dieTime																      = 0;
												creature[creatureMap[row][col]].beingAttacked																= 0;
												creature[creatureMap[row][col]].finalStrikeSound                            = 0;
											}
								}
								else
									fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxHitPoints;

								hits=(((fhits/fmhits)*100)/7)+1;
							}


							if (gPlayerRefreshMessage2.data[dataPtr]>15)
							{
								//tb_Beep();
								fi_WriteToErrorLog("Out of Range 1");
							}

							if (hits>15) hits=15;

							if (hits>7) 	{ hits=hits-8; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+128; 	}

							if (hits>3) 	{ hits=hits-4; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+64; 		}

							if (hits>1) 	{ hits=hits-2; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+32; 		}

							if (hits>0) 	{ hits=hits-1; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+16; 		}

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;
							
							/*
							
							gPlayerRefreshMessage2.data[dataPtr]=1;
							
							if (creature[creatureMap[row][col]].playerIndex!=0)
								if (player[creature[creatureMap[row][col]].playerIndex].curse!=0)
									gPlayerRefreshMessage2.data[dataPtr]=4;
								
							if (creature[creatureMap[row][col]].magicPoints==0)
							{
								hits=0;
							}
							else
							{
								fhits=creature[creatureMap[row][col]].magicPoints;

								if (creature[creatureMap[row][col]].id==kHuman)
								{
									fmhits=creature[creatureMap[row][col]].intelligence;
								}
								else
									fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxMagicPoints;

								hits=(((fhits/fmhits)*100)/7)+1;
							}

							if (gPlayerRefreshMessage2.data[dataPtr]>15)
							{
								fi_WriteToErrorLog("Out of Range 2");
							}

							if (hits>15) hits=15;

							if (hits>7) 	{ hits=hits-8; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+128; 	}

							if (hits>3) 	{ hits=hits-4; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+64; 		}

							if (hits>1) 	{ hits=hits-2; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+32; 		}

							if (hits>0) 	{ hits=hits-1; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+16; 		}

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;
							
							*/
						}
						else
						{

							if (creature[creatureMap[row][col]].hitPoints==0)
								if (creature[creatureMap[row][col]].playerIndex!=0)
									if (creature[creatureMap[row][col]].godMode || creature[creatureMap[row][col]].noDeath)
									{
										creature[creatureMap[row][col]].hitPoints=1;
										creature[creatureMap[row][col]].dieTime=0;
									}

							gPlayerRefreshMessage2.data[dataPtr]=4;

							if (creature[creatureMap[row][col]].hitPoints==0)
							{
								hits=0;
							}
							else
							{
								fhits=creature[creatureMap[row][col]].hitPoints;

								if (creature[creatureMap[row][col]].id==kHuman)
								{
									fmhits=creature[creatureMap[row][col]].strength;

									if (fhits>0)
										if (creature[creatureMap[row][col]].playerIndex>0)
											if (player[creature[creatureMap[row][col]].playerIndex].dead)
											{
												fhits=0;
												creature[creatureMap[row][col]].poison																			=	0;
												creature[creatureMap[row][col]].paralyzed																		= 0;
												creature[creatureMap[row][col]].disorientation															= 0;
												creature[creatureMap[row][col]].lifespan															      = 0;
												creature[creatureMap[row][col]].calmUntil															      = 0;
												creature[creatureMap[row][col]].attackTarget																=	0;
												creature[creatureMap[row][col]].reflectDamage																=	0;
												creature[creatureMap[row][col]].hitPoints																		=	0;
												creature[creatureMap[row][col]].magicPoints																	=	0;
												creature[creatureMap[row][col]].magicReflection															= false;
												creature[creatureMap[row][col]].strengthOffset															=	0;
												creature[creatureMap[row][col]].intelligenceOffset													=	0;
												creature[creatureMap[row][col]].dexterityOffset															= 0;
												creature[creatureMap[row][col]].strengthTime																=	0;
												creature[creatureMap[row][col]].intelligenceTime														=	0;
												creature[creatureMap[row][col]].dexterityTime																= 0;
												creature[creatureMap[row][col]].dieTime																      = 0;
												creature[creatureMap[row][col]].beingAttacked																= 0;
												creature[creatureMap[row][col]].finalStrikeSound                            = 0;
											}
								}
								else
									fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxHitPoints;

								hits=(((fhits/fmhits)*100)/7)+1;
							}

							if (gPlayerRefreshMessage2.data[dataPtr]>15)
							{
								fi_WriteToErrorLog("Out of Range 2");
							}

							if (hits>15) hits=15;

							if (hits>7) 	{ hits=hits-8; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+128; 	}

							if (hits>3) 	{ hits=hits-4; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+64; 		}

							if (hits>1) 	{ hits=hits-2; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+32; 		}

							if (hits>0) 	{ hits=hits-1; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+16; 		}

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;
							
							/*
							
							gPlayerRefreshMessage2.data[dataPtr]=1;
							
							if (creature[creatureMap[row][col]].playerIndex!=0)
								if (player[creature[creatureMap[row][col]].playerIndex].curse!=0)
									gPlayerRefreshMessage2.data[dataPtr]=4;
								
							if (creature[creatureMap[row][col]].magicPoints==0)
							{
								hits=0;
							}
							else
							{
								fhits=creature[creatureMap[row][col]].magicPoints;

								if (creature[creatureMap[row][col]].id==kHuman)
								{
									fmhits=creature[creatureMap[row][col]].intelligence;
								}
								else
									fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxMagicPoints;

								hits=(((fhits/fmhits)*100)/7)+1;
							}

							if (gPlayerRefreshMessage2.data[dataPtr]>15)
							{
								fi_WriteToErrorLog("Out of Range 2");
							}

							if (hits>15) hits=15;

							if (hits>7) 	{ hits=hits-8; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+128; 	}

							if (hits>3) 	{ hits=hits-4; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+64; 		}

							if (hits>1) 	{ hits=hits-2; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+32; 		}

							if (hits>0) 	{ hits=hits-1; gPlayerRefreshMessage2.data[dataPtr]=gPlayerRefreshMessage2.data[dataPtr]+16; 		}

							if (playerData[i][dataPtr]!=gPlayerRefreshMessage2.data[dataPtr]) SendData=true;

							playerData[i][dataPtr]	=	gPlayerRefreshMessage2.data[dataPtr];

							dataPtr++;
							
							*/
						}

					}
					else
					{
						gmCharacter=false;  // mod 8/12

						if (creature[creatureMap[row][col]].playerIndex!=0)
							if (player[creature[creatureMap[row][col]].playerIndex].category!=0 && player[creature[creatureMap[row][col]].playerIndex].category!=8)
								gmCharacter=true;

						if ((gmCharacter==false) || (player[i].category==1))  // mod 8/12, 11/4
							if (player[i].revealHidden>0)
								if (tb_Rnd(0,100)<player[i].revealHidden)
									if (creatureMap[row][col]!=0)
										if (creature[creatureMap[row][col]].hiding)
										{
											detected++;
											creature[creatureMap[row][col]].hiding=false;

											// -------------------- anti macroing code -----------------
											macroing=false;

											for (s=0;s<kMaxMacroTrace;s++)
												if (player[i].detectingHiddenTarget[s]==creatureMap[row][col])
													macroing=true;

											if (macroing==false)
											{
												found=false;

												for (s=0;s<kMaxMacroTrace;s++)
													if (player[i].detectingHiddenTarget[s]==0)
													{
														player[i].detectingHiddenTarget[s]=creatureMap[row][col];
														found=true;
														break;
													}

												if (found==false)
												{
													for (s=1;s<kMaxMacroTrace;s++)
														player[i].detectingHiddenTarget[s-1]=player[i].detectingHiddenTarget[s];

													player[i].detectingHiddenTarget[kMaxMacroTrace-1]=creatureMap[row][col];
												}
											}

											// ----------------------------------------------------------
											if (macroing==false)
												sk_DetectHidden(i);
										}
					}
			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]=255;

			dataPtr++;



//------- new start ---------------------------------------------------------------------------------//

			for (j=0;j<bloodStainPtr;j++)	// build blood stain list
			{
				row=mapToScreenRow[bloodStainList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[bloodStainList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (bloodStainMap[row][col]!=0)
				{

					gPlayerRefreshMessage2.data[dataPtr]	=	bloodStainList[j];

					if (playerData[i][dataPtr]!=bloodStainList[j]) SendData=true;

					playerData[i][dataPtr]	= bloodStainList[j];

					dataPtr++;

					gPlayerRefreshMessage2.data[dataPtr]	=	bloodStain[bloodStainMap[row][col]].id;

					if (playerData[i][dataPtr]!=bloodStain[bloodStainMap[row][col]].id) SendData=true;

					playerData[i][dataPtr]	= bloodStain[bloodStainMap[row][col]].id;

					dataPtr++;

				}
			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

//------- new End ---------------------------------------------------------------------------------//





			if (playerTarget>255)
				fi_WriteToErrorLog("playerTarget error");	// beta debug

			gPlayerRefreshMessage2.data[dataPtr]=(UInt8)playerTarget;

			if (playerData[i][dataPtr]!=(UInt8)playerTarget) SendData=true;

			playerData[i][dataPtr]=(UInt8)playerTarget;

			dataPtr++;

//--------------------------------------------------------------------------------------------//

			for (j=0;j<containerPtr;j++)	// build container list
			{
				row=mapToScreenRow[containerList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[containerList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (containerMap[row][col]!=0)
				{

#ifdef _LEVEL2DEBUGGING

					if (containerList[j]==255)
						fi_WriteToErrorLog("containerList[j]==255 - nw_SendRefresh");	// beta debug

#endif

					if (containerList[j]>=255)
						fi_WriteToErrorLog("containerList[j] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	containerList[j];

					if (playerData[i][dataPtr]!=containerList[j]) SendData=true;

					playerData[i][dataPtr]	= containerList[j];

					dataPtr++;

#ifdef _LEVEL2DEBUGGING
					if (containerID[containerMap[row][col]]==255)
						fi_WriteToErrorLog("containerID[containerMap[row][col]]==255 - nw_SendRefresh");	// beta debug

#endif

					if (containerID[containerMap[row][col]]>=255)
						fi_WriteToErrorLog("containerID[containerMap[row][col]] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	containerID[containerMap[row][col]];

					if (playerData[i][dataPtr]!=containerID[containerMap[row][col]]) SendData=true;

					playerData[i][dataPtr]	=	containerID[containerMap[row][col]];

					dataPtr++;

				}
			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

//--------------------------------------------------------------------------------------------//

			for (j=0;j<tracksPtr;j++)	// build tracks list
			{
				row=mapToScreenRow[tracksList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[tracksList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (tracksMap[row][col]!=0)
				{

#ifdef _LEVEL2DEBUGGING

					if (tracksList[j]==255)
						fi_WriteToErrorLog("tracksList[j]==255 - nw_SendRefresh");	// beta debug

#endif

					if (tracksList[j]>=255)
						fi_WriteToErrorLog("tracksList[j] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	tracksList[j];

					if (playerData[i][dataPtr]!=tracksList[j]) SendData=true;

					playerData[i][dataPtr]	= tracksList[j];

					dataPtr++;

#ifdef _LEVEL2DEBUGGING
					if (tracksID[tracksMap[row][col]]==255)
						fi_WriteToErrorLog("tracksID[tracksMap[row][col]]==255 - nw_SendRefresh");	// beta debug

#endif

					if (tracksID[tracksMap[row][col]]>=255)
						fi_WriteToErrorLog("tracksID[tracksMap[row][col]] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	tracksID[tracksMap[row][col]];

					if (playerData[i][dataPtr]!=tracksID[tracksMap[row][col]]) SendData=true;

					playerData[i][dataPtr]	=	tracksID[tracksMap[row][col]];

					dataPtr++;

				}
			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

//---------------------------------------------------------------------------------//

			for (j=0;j<hallsPtr;j++)	// build halls list
			{

				gPlayerRefreshMessage2.data[dataPtr]	=	halls[j];

				if (playerData[i][dataPtr]!=halls[j]) SendData=true;

				playerData[i][dataPtr]	= halls[j];

				dataPtr++;


				s=guildHalls[halls[j]].size * 10;

				for (cpt=0; cpt<kMaxExtras; cpt++)
				{
					if (guildHalls[halls[j]].extras[cpt].type!=0)
						s++;
				}

				gPlayerRefreshMessage2.data[dataPtr]	=	s;

				if (playerData[i][dataPtr]!=s) SendData=true;

				playerData[i][dataPtr]	= s;

				dataPtr++;



			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

//---------------------------------------------------------------------------------//

			player[i].currentlyAttacking=0;

			for (j=0;j<monsterPtr;j++)	// count number of creatures attacking player
			{
				row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (creatureMap[row][col]!=0)
					if (creature[creatureMap[row][col]].attackTarget==player[i].creatureIndex)
						if (creatureInfo[creature[creatureMap[row][col]].id].alignment==kEvil)
						{
							player[i].currentlyAttacking++;

							if (toGuard!=0)
							{
								if (tb_Rnd(1,10)<5)
									toGuard=creatureMap[row][col];
							}
							else
								toGuard=creatureMap[row][col];
						}

			}

			possibleAttackers=0;

			for (j=0;j<monsterPtr;j++)	// build fx list & target players
			{
				row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (creatureMap[row][col]!=0)
				{

					if (player[i].creatureIndex!=creatureMap[row][col])
					{

						okToAttack=false;

						if ((player[i].poisonField>0) || (player[i].paralyzeField>0) || (player[i].disorientfield>0))
						{
							if (cr_BothInPVPZone(player[i].creatureIndex,creatureMap[row][col]))
								okToAttack=true;

							if (creature[creatureMap[row][col]].master!=0)
								if (creature[creatureMap[row][col]].master==player[i].creatureIndex)
									okToAttack=false;
						}

						if ((okToAttack) || (!((creature[creatureMap[row][col]].playerIndex==0 && creature[creatureMap[row][col]].master==0 && (player[i].category==5 || player[i].category==8)) ||
											   (creature[creatureMap[row][col]].playerIndex!=0 && (player[creature[creatureMap[row][col]].playerIndex].category!=5 && player[creature[creatureMap[row][col]].playerIndex].category!=8) && (player[i].category!=5 && player[i].category!=8)) ||
											   (creature[creatureMap[row][col]].playerIndex!=0 && (player[creature[creatureMap[row][col]].playerIndex].category==5 || player[creature[creatureMap[row][col]].playerIndex].category==8) && (player[i].category==5 || player[i].category==8)) ||//Dave edit below - && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCPrivateer))
											   (creature[creatureMap[row][col]].npcIndex!=0 && (npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCBrigand && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCMercenary && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCPrivateer && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCDwarf1)) ||
											   (creature[creatureMap[row][col]].id==kCentaur)||
											   (creature[creatureMap[row][col]].id==kElderCentaur) ||
											   (creature[creatureMap[row][col]].master!=0 && (player[creature[creature[creatureMap[row][col]].master].playerIndex].category!=5 && player[creature[creature[creatureMap[row][col]].master].playerIndex].category!=8) && (player[i].category!=5 && player[i].category!=8)) ||
											   (creature[creatureMap[row][col]].master!=0 && (player[creature[creature[creatureMap[row][col]].master].playerIndex].category==5 || player[creature[creature[creatureMap[row][col]].master].playerIndex].category==8) && (player[i].category==5 || player[i].category==8)))))
						{
							if (player[i].poisonField>0)	// --------------------- poison field spell
								{
									if (cr_ViewBlocked(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, creature[creatureMap[row][col]].row, creature[creatureMap[row][col]].col)==false)
										sk_CastSpellOnCreature(player[i].creatureIndex, creatureMap[row][col], kPoisonSpell, true);
								}

							if (player[i].paralyzeField>0)	// --------------------- paralyze field spell
							{
								if (cr_ViewBlocked(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, creature[creatureMap[row][col]].row, creature[creatureMap[row][col]].col)==false)
									sk_CastSpellOnCreature(player[i].creatureIndex, creatureMap[row][col], kParalyzeSpell, true);
							}

							if (player[i].disorientfield>0)	// --------------------- disorient field spell
							{
								if (cr_ViewBlocked(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, creature[creatureMap[row][col]].row, creature[creatureMap[row][col]].col)==false)
									
									sk_CastSpellOnCreature(player[i].creatureIndex, creatureMap[row][col], kDisorientationSpell, true);
							}
						}

						if (!((creature[creatureMap[row][col]].playerIndex==0 && creature[creatureMap[row][col]].master==0 && player[i].category!=5 && player[i].category!=8) ||
								(creature[creatureMap[row][col]].playerIndex!=0 && (player[creature[creatureMap[row][col]].playerIndex].category==5 || player[creature[creatureMap[row][col]].playerIndex].category==8) && player[i].category!=5 && player[i].category!=8) ||
								(creature[creatureMap[row][col]].playerIndex!=0 && (player[creature[creatureMap[row][col]].playerIndex].category!=5 && player[creature[creatureMap[row][col]].playerIndex].category!=8) && (player[i].category==5 || player[i].category==8)) || //Dave edit below && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCPrivateer
								(creature[creatureMap[row][col]].npcIndex!=0 && (npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCBrigand && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCMercenary && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCPrivateer && npc[creature[creatureMap[row][col]].npcIndex].type!=kNPCDwarf1)) ||
								(creature[creatureMap[row][col]].id==kCentaur) ||
								(creature[creatureMap[row][col]].id==kElderCentaur) ||
								(creature[creatureMap[row][col]].master!=0 && (player[creature[creature[creatureMap[row][col]].master].playerIndex].category!=5 && player[creature[creature[creatureMap[row][col]].master].playerIndex].category!=8) && (player[i].category==5 || player[i].category==8)) ||
								(creature[creatureMap[row][col]].master!=0 && (player[creature[creature[creatureMap[row][col]].master].playerIndex].category==5 || player[creature[creature[creatureMap[row][col]].master].playerIndex].category==8) && (player[i].category!=5 && player[i].category!=8)))) //Support field spells
						{
							if (player[i].massheal>0)	// --------------------- mass heal spell
							{
								//tb_Beep(0);
								if (cr_ViewBlocked(creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, creature[creatureMap[row][col]].row, creature[creatureMap[row][col]].col)==false)
									sk_CastSpellOnCreature(player[i].creatureIndex, creatureMap[row][col], kLesserHealSpell, true);
							}
							
							if (player[i].creatureField>0)
							{
								//ignore los (will be handled in the spell itself)
								sk_CastSpellOnCreature(player[i].fieldFrom, creatureMap[row][col], player[i].creatureField, true);
							}
						}

#ifdef _REFRESHTEST
						fi_WriteToErrorLog("4");

#endif

#ifdef _DONTATTACKGMS
						if (player[i].category==0 || player[i].category==8) // no gm attack line
#endif
						{
							if (creature[creatureMap[row][col]].alignment==kEvil)	// target player for attack
							{
								if (player[i].currentlyAttacking<player[i].maxAttackers)
									if (creature[creatureMap[row][col]].calmUntil==0)
										if (creature[creatureMap[row][col]].lifespan==0)
											if (creature[creatureMap[row][col]].attackTarget==0)
												if (creature[creatureMap[row][col]].possibleTarget==0)
													if (creature[creatureMap[row][col]].master==0)
														if (creature[player[i].creatureIndex].hiding==false && player[i].stealth==false)
														{
															if (player[i].dead==false)
																if (((creature[creatureMap[row][col]].hitPoints*3)>creatureInfo[creature[creatureMap[row][col]].id].maxHitPoints) && (!creature[creatureMap[row][col]].wounded))
																{
																	if (creatureInfo[creature[creatureMap[row][col]].id].mobile)
																		doAttack=cr_BuildAttackPath(creatureMap[row][col],player[i].creatureIndex,&k,&k);

																	else
																		doAttack=true;

																	/*if ((creature[creatureMap[row][col]].id==kRedDragon) || (creature[creatureMap[row][col]].id==kIceDragon) || (creature[creatureMap[row][col]].id==kForestDragon))
																	  	if (in_ItemEquipedServer(i,kDragonStoneRing))
																				doAttack=false;
																	  if ((creatureInfo[creatureMap[row][col]].domain==kWater))
																	  	if (in_ItemEquipedServer(i,kNeptunesRing))
																	  		doAttack=false;
																	  if ((creature[creatureMap[row][col]].id==kAirElemental) || (creature[creatureMap[row][col]].id==kWaterElemental) || (creature[creatureMap[row][col]].id==kPoisonElemental) || (creature[creatureMap[row][col]].id==kFireElemental))
																	  	if (in_ItemEquipedServer(i,kElementalRing))
																	  		doAttack=false;
																	  if (creatureInfo[creatureMap[row][col]].fire)
																	  	if (in_ItemEquipedServer(i,kHadesRing))
																	  		doAttack=false;*/


																	if (doAttack)
																	{
																		possibleAttackers++;
																		creature[creatureMap[row][col]].possibleTarget=player[i].creatureIndex;
																		creature[creatureMap[row][col]].goodTarget=cr_DistanceBetween(player[i].creatureIndex,creatureMap[row][col])-creatureInfo[creature[creatureMap[row][col]].id].attackRange;
																	}
																}


														}
														else
														{
															if (player[i].dead==false && player[i].stealth==false)
																if (!cr_ViewBlocked(creature[creatureMap[row][col]].row, creature[creatureMap[row][col]].col, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col))
																	if (tb_Rnd(1,3)==1)
																		tx_CreatureTalkText(creatureMap[row][col], player[i].creatureIndex, kHidingTalk);
														}
							}
							else
								if (creature[creatureMap[row][col]].npcIndex!=0)	// "protectors" will attack thieves
									if (npc[creature[creatureMap[row][col]].npcIndex].type==kNPCGuard)
									{
										//if (player[i].thiefUntil>currentTime) no more thievery, /evil pcs instead
										if	(creature[creatureMap[row][col]].attackTarget==0) //If not already attacking
											if (cr_DistanceFrom(creatureMap[row][col], npc[creature[creatureMap[row][col]].npcIndex].startRow, npc[creature[creatureMap[row][col]].npcIndex].startCol)<=npc[creature[creatureMap[row][col]].npcIndex].range) //Only attack if we are in range
											{
												if (player[i].category==8) //Priority given to /evil pcs
												{
													if (creature[player[i].creatureIndex].hiding==false && player[i].stealth==false)
														if (player[i].dead==false)
															creature[creatureMap[row][col]].attackTarget=player[i].creatureIndex;
												}
												else
													creature[creatureMap[row][col]].possibleTarget=toGuard;
											}
											else if (creature[creatureMap[row][col]].master==0)
												cr_CreatureMoveTo(creatureMap[row][col], npc[creature[creatureMap[row][col]].npcIndex].startRow, npc[creature[creatureMap[row][col]].npcIndex].startCol); //Jump back to start position


									}
						}
					}


					if (creature[creatureMap[row][col]].activeFX)		//(fxFound)
					{
						gPlayerRefreshMessage2.data[dataPtr]	=	monsterList[j];			// position

						if (gPlayerRefreshMessage2.data[dataPtr]==255)
							fi_WriteToErrorLog("monsterList[j] error");	// beta debug

						if (playerData[i][dataPtr]!=monsterList[j]) SendData=true;

						playerData[i][dataPtr]	= monsterList[j];

						dataPtr++;

						for (k=0;k<kMaxCreatureEffects;k++)
						{
							b1=(creature[creatureMap[row][col]].fx[k]/256);
							b2=creature[creatureMap[row][col]].fx[k]-(b1*256);

							if (b1>255)
								fi_WriteToErrorLog("b1 fx error");	// beta debug

							gPlayerRefreshMessage2.data[dataPtr]	=	b1;	// id

							if (playerData[i][dataPtr]!=b1) SendData=true;

							playerData[i][dataPtr]	=	b1;

							dataPtr++;

							if (b2>255)
								fi_WriteToErrorLog("b2 fx error");	// beta debug

							gPlayerRefreshMessage2.data[dataPtr]	=	b2;	// id

							if (playerData[i][dataPtr]!=b2) SendData=true;

							playerData[i][dataPtr]	=	b2;

							dataPtr++;

							if (serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)]>255)
								fi_WriteToErrorLog("serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)] error");	// beta debug

							gPlayerRefreshMessage2.data[dataPtr]	=	serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)]; // type

							if (playerData[i][dataPtr]!=serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)]) SendData=true;

							playerData[i][dataPtr]	=	serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)];

							dataPtr++;

							target=255;

							for (p=0;p<monsterPtr;p++)	// build fx list
							{
								row2=mapToScreenRow[monsterList[p]]+creature[player[i].creatureIndex].row-13;
								col2=mapToScreenCol[monsterList[p]]+creature[player[i].creatureIndex].col-13;

								if (creatureMap[row2][col2]==creature[creatureMap[row][col]].fxTarget[k])
								{
									target=(UInt8)monsterList[p];
									break;
								}
							}

							if (target==255)	// probably dead ... target last know position or edge of viewable area
							{
								nw_BuildTrail(row, col, creature[creature[creatureMap[row][col]].fxTarget[k]].row, creature[creature[creatureMap[row][col]].fxTarget[k]].col, &len);
								w=0;
								edge=-1;

								for (p=0;p<kMaxScreenElements;p++)	// build creature list
								{
									row2=mapToScreenRow[p]+creature[player[i].creatureIndex].row-13;
									col2=mapToScreenCol[p]+creature[player[i].creatureIndex].col-13;

									for (q=len;q>=w;q--)
										if ((row2==fxTrailRow[q]) && (col2==fxTrailCol[q]))
										{
											w=q;
											edge=p;
										}

									if ((row2==creature[creatureMap[row][col]].fxTargetRow[k]) && (col2==creature[creatureMap[row][col]].fxTargetCol[k]))
									{
										target=p;
										break;
									}
								}

								if ((target==255) && (edge>=0))
									target=edge;
							}

							if (target>255)
								fi_WriteToErrorLog("target position error");	// beta debug

							gPlayerRefreshMessage2.data[dataPtr]	=	target;				// targt position

							if (playerData[i][dataPtr]!=target) SendData=true;

							playerData[i][dataPtr]	=	target;

							dataPtr++;
						}
					}

				}
			}

#ifdef _REFRESHTEST
			fi_WriteToErrorLog("6");

#endif
			if (player[i].currentlyAttacking>player[i].maxAttackers)
			{
				if (monsterPtr>0)
				{
					j=tb_Rnd(0,(monsterPtr-1));
					row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
					col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

					if (row>0 && row<kMapWidth && col>0 && col<kMapHeight)
						if (creatureMap[row][col]!=0)
							if (creature[creatureMap[row][col]].attackTarget==player[i].creatureIndex)
							{
								creature[creatureMap[row][col]].attackTarget			=	0;
								creature[creatureMap[row][col]].possibleTarget		=	0;
								creature[creatureMap[row][col]].nextStrikeTime		=	0;
								creature[creatureMap[row][col]].numberOfStrikes		=	0;
								creature[creatureMap[row][col]].trackRow					= 0;
								creature[creatureMap[row][col]].trackCol					= 0;
								creature[creatureMap[row][col]].tracking					= false;
							}
				}

				for (j=0;j<monsterPtr;j++)	// count number of creatures attacking player // creature[creatureMap[row][col]].goodTarget
				{
					row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
					col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

					if (row<0) continue;

					if (row>kMapWidth) continue;

					if (col<0) continue;

					if (col>kMapHeight) continue;

					if (creatureMap[row][col]!=0)
						if (creature[creatureMap[row][col]].possibleTarget==player[i].creatureIndex)
						{
							creature[creatureMap[row][col]].goodTarget=0;
							creature[creatureMap[row][col]].possibleTarget=0;
						}
				}
			}
			else
			{
				desiredAttackCount=player[i].maxAttackers-player[i].currentlyAttacking;

				if ((desiredAttackCount>0) && (desiredAttackCount<possibleAttackers) && (possibleAttackers>0))
				{
					drop=possibleAttackers-desiredAttackCount;

					for (k=0;k<drop;k++)
					{
						maxTarget=0;
						maxTargetIndex=-1;

						for (j=0;j<monsterPtr;j++)	// count number of creatures attacking player // creature[creatureMap[row][col]].goodTarget
						{
							row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
							col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

							if (row<0) continue;

							if (row>kMapWidth) continue;

							if (col<0) continue;

							if (col>kMapHeight) continue;

							if (creatureMap[row][col]!=0)
								if (creature[creatureMap[row][col]].possibleTarget==player[i].creatureIndex)
									if (creature[creatureMap[row][col]].goodTarget>maxTarget)
									{
										maxTarget=creature[creatureMap[row][col]].goodTarget;
										maxTargetIndex=j;
									}

						}

						if (maxTargetIndex>0)
						{
							row=mapToScreenRow[monsterList[maxTargetIndex]]+creature[player[i].creatureIndex].row-13;
							col=mapToScreenCol[monsterList[maxTargetIndex]]+creature[player[i].creatureIndex].col-13;
							creature[creatureMap[row][col]].possibleTarget=0;
							creature[creatureMap[row][col]].goodTarget=0;
						}
					}
				}
			}


			player[i].poisonField=0;
			player[i].creatureField=0;
			player[i].paralyzeField=0;
			player[i].massheal=0;
			player[i].disorientfield=0;

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

			for (j=0;j<fxPtr;j++)	// build fx list
			{

				row=mapToScreenRow[fxList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[fxList[j]]+creature[player[i].creatureIndex].col-13;

				if (fxList[j]>=255)
					fi_WriteToErrorLog("fxList[j] error");	// beta debug

				gPlayerRefreshMessage2.data[dataPtr]	=	fxList[j];

				if (fxList[j]==255)
					fi_WriteToErrorLog("fxList[j]==255 - nw_SendRefresh");	// beta debug

				if (playerData[i][dataPtr]!=fxList[j]) SendData=true;

				playerData[i][dataPtr]	= fxList[j];

				dataPtr++;

				b1=(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)]/256);

				b2=fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)]-(b1*256);

				if (b1>=255)
					fi_WriteToErrorLog("b1 x error");	// beta debug

				gPlayerRefreshMessage2.data[dataPtr]	=	b1;

				if (playerData[i][dataPtr]!=b1) SendData=true;

				playerData[i][dataPtr]	=	b1;

				dataPtr++;

				if (b2>=255)
					fi_WriteToErrorLog("b2 x error");	// beta debug

				gPlayerRefreshMessage2.data[dataPtr]	=	b2;

				if (playerData[i][dataPtr]!=b2) SendData=true;

				playerData[i][dataPtr]	=	b2;

				dataPtr++;

				if (serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)]>=255)
					fi_WriteToErrorLog("serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)] error");	// beta debug

				gPlayerRefreshMessage2.data[dataPtr]	=	serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)];

				if (playerData[i][dataPtr]!=serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)]) SendData=true;

				playerData[i][dataPtr]	=	serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)];

				dataPtr++;

			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

			if (player[i].sendStatsRefresh)
			{
				/*
				//------------------ debugging ------------------
				if (creature[player[i].creatureIndex].strengthOffset>creature[player[i].creatureIndex].strength)
				{
					fi_WriteToErrorLog("str 1 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].strength,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].strengthOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].strengthOffset=0;
				}

				if ((creature[player[i].creatureIndex].strength+creature[player[i].creatureIndex].strengthOffset)>=255)
				{
					fi_WriteToErrorLog("str 2 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].strength,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].strengthOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].strengthOffset=0;
				}

				//------------------ debugging ------------------
				if (creature[player[i].creatureIndex].intelligenceOffset>creature[player[i].creatureIndex].intelligence)
				{
					fi_WriteToErrorLog("int 1 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].intelligence,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].intelligenceOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].intelligenceOffset=0;
				}

				if ((creature[player[i].creatureIndex].intelligence+creature[player[i].creatureIndex].intelligenceOffset)>=255)
				{
					fi_WriteToErrorLog("int 2 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].intelligence,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].intelligenceOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].intelligenceOffset=0;
				}

				//------------------ debugging ------------------
				if (creature[player[i].creatureIndex].dexterityOffset>creature[player[i].creatureIndex].dexterity)
				{
					fi_WriteToErrorLog("dex 1 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].dexterity,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].dexterityOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].dexterityOffset=0;
				}

				if ((creature[player[i].creatureIndex].dexterity+creature[player[i].creatureIndex].dexterityOffset)>=255)
				{
					fi_WriteToErrorLog("dex 2 error");	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].dexterity,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					tb_IntegerToString(creature[player[i].creatureIndex].dexterityOffset,debugStr);
					fi_WriteToErrorLog(debugStr);	// beta debug
					creature[player[i].creatureIndex].dexterityOffset=0;
				}

				//-------------------------------------------------
				
				*/

				gPlayerRefreshMessage2.data[dataPtr]	=	(creature[player[i].creatureIndex].strength+creature[player[i].creatureIndex].strengthOffset);

				if (playerData[i][dataPtr]!=(creature[player[i].creatureIndex].strength+creature[player[i].creatureIndex].strengthOffset)) SendData=true;

				playerData[i][dataPtr]	=	(creature[player[i].creatureIndex].strength+creature[player[i].creatureIndex].strengthOffset);

				dataPtr++;

				gPlayerRefreshMessage2.data[dataPtr]	=	(creature[player[i].creatureIndex].intelligence+creature[player[i].creatureIndex].intelligenceOffset);

				if (playerData[i][dataPtr]!=(creature[player[i].creatureIndex].intelligence+creature[player[i].creatureIndex].intelligenceOffset)) SendData=true;

				playerData[i][dataPtr]	=	(creature[player[i].creatureIndex].intelligence+creature[player[i].creatureIndex].intelligenceOffset);

				dataPtr++;

				gPlayerRefreshMessage2.data[dataPtr]	=	(creature[player[i].creatureIndex].dexterity + creature[player[i].creatureIndex].dexterityOffset);

				if (playerData[i][dataPtr]!=(creature[player[i].creatureIndex].dexterity + creature[player[i].creatureIndex].dexterityOffset)) SendData=true;

				playerData[i][dataPtr]	=	(creature[player[i].creatureIndex].dexterity + creature[player[i].creatureIndex].dexterityOffset);

				dataPtr++;

				if (creature[player[i].creatureIndex].hitPoints>=255)
				{
					fi_WriteToErrorLog("hp error");	// beta debug
					fi_WriteToErrorLog(player[i].playerName);	// beta debug
					creature[player[i].creatureIndex].hitPoints=creature[player[i].creatureIndex].strength;
				}

				gPlayerRefreshMessage2.data[dataPtr]	=	(UInt8)creature[player[i].creatureIndex].hitPoints;

				if (playerData[i][dataPtr]!=(UInt8)creature[player[i].creatureIndex].hitPoints) SendData=true;

				playerData[i][dataPtr]	=	(UInt8)creature[player[i].creatureIndex].hitPoints;

				dataPtr++;

				if (creature[player[i].creatureIndex].magicPoints>=255)
				{
					fi_WriteToErrorLog("mp error");	// beta debug
					fi_WriteToErrorLog(player[i].playerName);	// beta debug
					creature[player[i].creatureIndex].magicPoints=creature[player[i].creatureIndex].intelligence;
				}

				gPlayerRefreshMessage2.data[dataPtr]	=	creature[player[i].creatureIndex].magicPoints;

				if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].magicPoints) SendData=true;

				playerData[i][dataPtr]	=	creature[player[i].creatureIndex].magicPoints;

				dataPtr++;

				if (creature[player[i].creatureIndex].armorRating>=255)
				{
					fi_WriteToErrorLog("ar error");	// beta debug
					fi_WriteToErrorLog(player[i].playerName);	// beta debug
				}

				gPlayerRefreshMessage2.data[dataPtr]	=	creature[player[i].creatureIndex].armorRating;

				if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].armorRating) SendData=true;

				playerData[i][dataPtr]	=	creature[player[i].creatureIndex].armorRating;

				dataPtr++;

				gPlayerRefreshMessage2.data[dataPtr]	=	sk_GetCharacterLevel(i);

				if (playerData[i][dataPtr]!= sk_GetCharacterLevel(i)) SendData=true;

				playerData[i][dataPtr]	=	sk_GetCharacterLevel(i);

				dataPtr++;

				if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer))
					theClass=player[i].characterClass;
				else if (pl_AuthorizedTo(player[i].creatureIndex, kIsApprenticeGM))
					theClass=8;
				else if (pl_AuthorizedTo(player[i].creatureIndex, kIsRegularGM) || pl_AuthorizedTo(player[i].creatureIndex, kIsSeniorGM))
					theClass=9;
				else if (pl_AuthorizedTo(player[i].creatureIndex, kIsDeveloper))
					theClass=10;
				else if (pl_AuthorizedTo(player[i].creatureIndex, kIsQuestCharacter))
					theClass=10+player[i].characterClass;
				else
					theClass=7;

				gPlayerRefreshMessage2.data[dataPtr]	=	theClass;

				if (playerData[i][dataPtr]!=theClass) SendData=true;

				playerData[i][dataPtr]	=	theClass;

				dataPtr++;

				t=(int)creature[player[i].creatureIndex].hunger/100;

				gPlayerRefreshMessage2.data[dataPtr]	=	t;

				if (playerData[i][dataPtr]!=t) SendData=true;

				playerData[i][dataPtr]	=	t;

				dataPtr++;

				t=creature[player[i].creatureIndex].hunger-(t*100);

				gPlayerRefreshMessage2.data[dataPtr]	=	t;

				if (playerData[i][dataPtr]!=t) SendData=true;

				playerData[i][dataPtr]	=	t;

				dataPtr++;
				
				t=creature[player[i].creatureIndex].stamina;
				
				gPlayerRefreshMessage2.data[dataPtr]	=	t;
				
				if (playerData[i][dataPtr]!=t) SendData=true;

				playerData[i][dataPtr]	=	t;

				dataPtr++;

				player[i].sendStatsRefresh=false;

				forceRefresh=true;
			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;

			if (player[i].sendPetsRefresh)
				for (j=0;j<kMaxPets;j++)
				{

					if (player[i].petType[j]>=255)
						fi_WriteToErrorLog("player[i].petType[j] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	player[i].petType[j];

					if (playerData[i][dataPtr]!=player[i].petType[j]) SendData=true;

					playerData[i][dataPtr]	=	player[i].petType[j];

					dataPtr++;

					if (player[i].petInStable[j])
					{
						gPlayerRefreshMessage2.data[dataPtr]	=	99;

						if (playerData[i][dataPtr]!=99) SendData=true;

						playerData[i][dataPtr]	=	99;

						dataPtr++;
					}
					else
					{

						if (creature[player[i].pet[j]].command>=255)
							fi_WriteToErrorLog("creature[player[i].pet[j]].command error");	// beta debug

						gPlayerRefreshMessage2.data[dataPtr]	=	creature[player[i].pet[j]].command;

						if (playerData[i][dataPtr]!=creature[player[i].pet[j]].command) SendData=true;

						playerData[i][dataPtr]	=	creature[player[i].pet[j]].command;

						dataPtr++;
					}

					if (player[i].petLoyalty[j]>=255)
						fi_WriteToErrorLog("player[i].petLoyalty[j] error");	// beta debug

					gPlayerRefreshMessage2.data[dataPtr]	=	player[i].petLoyalty[j];

					if (playerData[i][dataPtr]!=100) SendData=true;

					playerData[i][dataPtr]	=	100;

					dataPtr++;

					player[i].sendPetsRefresh=false;

					forceRefresh=true;
				}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			dataPtr++;



			soundFX=0;

			for (j=0;j<6;j++)
				soundsSent[j]=-1;

			for (j=0;j<monsterPtr;j++)	// build sound list
			{
				row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
				col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;

				if (row<0) continue;

				if (row>kMapWidth) continue;

				if (col<0) continue;

				if (col>kMapHeight) continue;

				if (creatureMap[row][col]!=0)
				{

					doRandomSound=true;
					//if ((creature[creatureMap[row][col]].soundFX!=0) && ((creature[creatureMap[row][col]].soundFXTime)>=player[i].lastRefresh) && (player[i].lastRefresh!=0))

					if (creature[creatureMap[row][col]].soundFX!=0)
						if (creature[creatureMap[row][col]].soundFXTime>player[i].logon)
							if (!pl_PlayerHeardSound(i,creature[creatureMap[row][col]].soundID,creature[creatureMap[row][col]].soundFXTime))
							{
								soundAlreadySent=false;

								for (k=0;k<6;k++)
								{
									if (soundsSent[k]==creature[creatureMap[row][col]].soundFX)
									{
										soundAlreadySent=true;
										break;
									}

									if (soundsSent[k]==-1)
										break;
								}

								if (soundAlreadySent==false)
								{
									doRandomSound=false;

									if (creature[creatureMap[row][col]].soundFX>=255)
										fi_WriteToErrorLog("creature[creatureMap[row][col]].soundFX error");	// beta debug

									gPlayerRefreshMessage2.data[dataPtr]	=	creature[creatureMap[row][col]].soundFX;

									if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].soundFX) SendData=true;

									playerData[i][dataPtr]	=	creature[creatureMap[row][col]].soundFX;

									soundsSent[soundFX]=playerData[i][dataPtr];

									dataPtr++;

									soundFX++;

									if (soundFX>5)
										break;
								}
							}

					if ((creatureInfo[creature[creatureMap[row][col]].id].randomSound!=0) && (doRandomSound))
					{
						if (tb_Rnd(1,200)==1)
						{
							soundAlreadySent=false;

							for (k=0;k<6;k++)
							{
								if (soundsSent[k]==creatureInfo[creature[creatureMap[row][col]].id].randomSound)
								{
									soundAlreadySent=true;
									break;
								}

								if (soundsSent[k]==-1)
									break;
							}

							if (soundAlreadySent==false)
							{

								if (creatureInfo[creature[creatureMap[row][col]].id].randomSound>=255)
									fi_WriteToErrorLog("randomSound error");	// beta debug

								gPlayerRefreshMessage2.data[dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].randomSound;

								if (playerData[i][dataPtr]!=creatureInfo[creature[creatureMap[row][col]].id].randomSound) SendData=true;

								playerData[i][dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].randomSound;

								soundsSent[soundFX]=playerData[i][dataPtr];

								dataPtr++;

								soundFX++;

								if (soundFX>5)
									break;
							}
						}
					}
				}

			}

			gPlayerRefreshMessage2.data[dataPtr]=255;

			if (playerData[i][dataPtr]!=255) SendData=true;

			playerData[i][dataPtr]	=	255;

			if (dataPtr>kDataSize)		// beta debug
				fi_WriteToErrorLog("Out of Range - nw_SendRefresh");

			if (dataPtr>maxDataSize)
				maxDataSize=dataPtr;

			lastMessageSize=dataPtr;


			//nw_CompressDataTest(i,dataPtr+1);

			//if (dataPtr>60)
			//  {
			//  	errorStream = fopen("Send Data", "a" );
			//  	if (errorStream != NULL)
			//  	  {
			//        fprintf( errorStream, "%s", "---------------------------------");
			//        fprintf( errorStream, "%s", "\n");
			//        for (j=0;j<=dataPtr;j++)
			//          {
			//            fprintf( errorStream, "%i", playerData[i][j]);
			//            fprintf( errorStream, "%s", "\n");
			//          }
			//  		  fclose( errorStream );
			//  		}
			//  }




			if (player[i].revealHidden>0)
			{
				if (detected>0)
					nw_SendDisplayMessage(i,kHiddenRevealed);
				else
					nw_SendDisplayMessage(i,kHiddenFailed);

				player[i].revealHidden=0;
			}

			player[i].lastTalkRefresh=TickCount();

			if ((SendData) || (forceRefresh))	// Send the data
			{
				player[i].lastRefresh=TickCount();  //test

				NSpClearMessageHeader(&gPlayerRefreshMessage2.h);
				gPlayerRefreshMessage2.h.what 				= kPlayerRefreshMessage;
				gPlayerRefreshMessage2.h.to 					= player[i].id;
				gPlayerRefreshMessage2.h.messageLen 	= sizeof( PlayerRefreshMessage) + (sizeof(UInt8) * (dataPtr));
				gPlayerRefreshMessage2.row						=	creature[player[i].creatureIndex].row-13;
				gPlayerRefreshMessage2.col						=	creature[player[i].creatureIndex].col-13;
				gPlayerRefreshMessage2.dataLength		  =	dataPtr+1;

				gPlayerRefreshMessage2.checksum		    =	gPlayerRefreshMessage2.row + gPlayerRefreshMessage2.col;

				for (k=0;k<gPlayerRefreshMessage2.dataLength;k++)
					gPlayerRefreshMessage2.checksum     = gPlayerRefreshMessage2.checksum + gPlayerRefreshMessage2.data[k];


#ifdef _SERVERONLY
				messagesSent++;

				bytesSent=bytesSent+gPlayerRefreshMessage2.h.messageLen;

				if (gPlayerRefreshMessage2.h.messageLen>2000)
					fi_WriteToErrorLog("Large message in nw_SendRefresh");

				if (gPlayerRefreshMessage2.h.messageLen>maxMessageSize)
					maxMessageSize=gPlayerRefreshMessage2.h.messageLen;

#endif

				//tb_IntegerToString(gPlayerRefreshMessage2.h.messageLen,errorMessage);
				//fi_WriteToErrorLog(errorMessage);
				if ((gPlayerRefreshMessage2.h.messageLen>800) && (player[i].category==0 || player[i].category==8))
					player[i].playerRefreshTime=TickCount()+tb_Rnd(80,180);

				else if ((gPlayerRefreshMessage2.h.messageLen>600) && (player[i].category==0 || player[i].category==8))
					player[i].playerRefreshTime=TickCount()+tb_Rnd(75,120);

				else if (gPlayerRefreshMessage2.h.messageLen>400)
					player[i].playerRefreshTime=TickCount()+tb_Rnd(45,75);

				else if ((gPlayerRefreshMessage2.h.messageLen>200) && (player[i].category==0 || player[i].category==8))
					player[i].skipNextRefresh=true;

				if (gNetGame!=NULL)
				{
					status = NSpMessage_Send(gNetGame, &gPlayerRefreshMessage2.h, kSendMethod);

					if (status!=noErr)
					{
						strcpy(errorMessage,"nw_SendRefresh: ");
						tb_IntegerToString(status,errorNumber);
						strcat(errorMessage,errorNumber);
						fi_WriteToErrorLog(errorMessage);
					}
				}
				else
					nw_RestartServer();
			}

		}


}

/*
// ----------------------------------------------------------------- //
void nw_SendRefresh(int i, TBoolean forceRefresh) // server
// ----------------------------------------------------------------- //

{
	// Sent from the server to refresh the display of a single player
	//
	// forceRefresh - Send data even if there was no change from previous frame.  This if for when
	// the player moves with no objects or creatures in view.

	int																	j;
	int																	k;
	int																	p;
	int																	row;
	int																	col;
	int																	row2;
	int																	col2;
	OSStatus 														status;
	int																	dataPtr;
	TBoolean														SendData;
	int																	hits;
	float																fhits;
	float																fmhits;
	unsigned long												currentTime=TickCount();
	int																	raiseChance;
	PlayerRefreshMessage								*gPlayerRefreshMessage;
	int																	m;
	UInt8																*theData;
	TBoolean														fxFound;
	int																	target;
	int																	lastKnowPosition;
	int																	monsterPtr;
	int																	containerPtr;
	int																	fxPtr;
	int																	soundFX;

	if ((i<1) || (i>=kMaxPlayers))			// beta debug
		fi_WriteToErrorLog("Out of Range - nw_SendRefresh");

	if (targetFrame==0)
		targetFrame=1;
	else
		targetFrame=0;

	if (player[i].gathering>0)	// check status of player resource harvest
		{
			player[i].gathering--;
			if (player[i].gathering==0)
				{
					creature[player[i].creatureIndex].strikePose=0;
					creature[player[i].creatureIndex].numberOfStrikes=0;
					creature[player[i].creatureIndex].nextStrikeTime=0;
					if (player[i].gatherType==kLumberjacking)
						sk_Lumberjacking(i);
					else if (player[i].gatherType==kMining)
						sk_Mining(i);
					else if (player[i].gatherType==kFishing)
						sk_Fishing(i);
					else if (player[i].gatherType==255)	// making item
						(i,player[i].gatherRow);
				}
		}

	// ------- raise magic points
	if ((creature[player[i].creatureIndex].magicPoints<creature[player[i].creatureIndex].intelligence) && (tb_Rnd(1,4)==1))
		{
			if (player[i].skill[kMeditation].level<10) 		  raiseChance=8;
			else if (player[i].skill[kMeditation].level<20) raiseChance=7;
			else if (player[i].skill[kMeditation].level<35) raiseChance=6;
			else if (player[i].skill[kMeditation].level<50) raiseChance=5;
			else if (player[i].skill[kMeditation].level<65) raiseChance=4;
			else if (player[i].skill[kMeditation].level<80) raiseChance=3;
			else if (player[i].skill[kMeditation].level<95) raiseChance=2;
			else raiseChance=1;
			if (tb_Rnd(1,raiseChance)==1)
				{
					creature[player[i].creatureIndex].magicPoints++;
					if (player[i].meditating)
						creature[player[i].creatureIndex].magicPoints++;
					if (creature[player[i].creatureIndex].magicPoints>creature[player[i].creatureIndex].intelligence)
						creature[player[i].creatureIndex].magicPoints=creature[player[i].creatureIndex].intelligence;
					player[i].sendStatsRefresh=true;
				}
		}

	// ------- raise health points
	if ((creature[player[i].creatureIndex].hitPoints<creature[player[i].creatureIndex].strength) && (tb_Rnd(1,4)==1))
		{
			if (player[i].skill[kHealing].level<10) 		 raiseChance=8;
			else if (player[i].skill[kHealing].level<20) raiseChance=7;
			else if (player[i].skill[kHealing].level<35) raiseChance=6;
			else if (player[i].skill[kHealing].level<50) raiseChance=5;
			else if (player[i].skill[kHealing].level<65) raiseChance=4;
			else if (player[i].skill[kHealing].level<80) raiseChance=3;
			else if (player[i].skill[kHealing].level<95) raiseChance=2;
			else raiseChance=1;
			if (tb_Rnd(1,raiseChance)==1)
				{
					creature[player[i].creatureIndex].hitPoints++;
					if (player[i].healing)
						creature[player[i].creatureIndex].hitPoints++;
					if (creature[player[i].creatureIndex].hitPoints>creature[player[i].creatureIndex].strength)
						creature[player[i].creatureIndex].hitPoints=creature[player[i].creatureIndex].strength;
					player[i].sendStatsRefresh=true;
				}
		}

	if (creature[player[i].creatureIndex].hitPoints>=creature[player[i].creatureIndex].strength)
		player[i].healing=false;

	if (gNetGame)
		if ((player[i].active) && (!player[i].online))	// ------ player logged off ... check for remove time
			{
				if ((player[i].lastCheckIn+(60*10))<currentTime)
					{
						fi_Logout(i);
						creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
						cr_ClearCreatureRecord(player[i].creatureIndex);
						pl_ClearPlayerRecord(i);
					}
			}
		else if ((player[i].lastCheckIn+(3600*kInactivetyLogoff))<currentTime)	// ------ check for player inactivity
			{
				nw_RemovePlayer(player[i].id);
				fi_Logout(i);
				creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
				cr_ClearCreatureRecord(player[i].creatureIndex);
				pl_ClearPlayerRecord(i);
			}
		else if (player[i].active)	// ------ everything ok ... Send refresh
			{

				if (player[i].hunger>0)	// ------ decrement hunger counter
					player[i].hunger--;

				if (currentTime>(player[i].talkTime+(60*8)))	// ------ check for talk erase time
					{
						strcpy(player[i].talk, "");
						player[i].talkTime=0;
					}

				SendData=false;
				dataPtr=0;

				monsterPtr=0;
				containerPtr=0;
				fxPtr=0;

				for (j=0;j<kMaxScreenElements;j++)	// build creature list
					{
						row=mapToScreenRow[j]+creature[player[i].creatureIndex].row-13;
						col=mapToScreenCol[j]+creature[player[i].creatureIndex].col-13;

						if (containerMap[row][col]!=0)	// build container list
							{
								containerList[containerPtr]=j;
								containerPtr++;
							}

						if (fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)]!=0)	// build terrain fx list
							{
								fxList[fxPtr]=j;
								fxPtr++;
							}

						if (creatureMap[row][col]!=0) // build monster list
							{
								monsterList[monsterPtr]=j;
								monsterPtr++;
								playerState[i][targetFrame][j]=creatureMap[row][col];
							}
						else
							continue;

						if (creature[creatureMap[row][col]].hiding==false)	// if hiding then no need to include in refresh
							{
								outData[dataPtr]				=	j;
								if (playerData[i][dataPtr]!=j) SendData=true;
								playerData[i][dataPtr]	=	j;

								dataPtr++;

								outData[dataPtr]				=	creature[creatureMap[row][col]].id; // creatureMap[row][col];
								if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].id) SendData=true;
								playerData[i][dataPtr]	=	creature[creatureMap[row][col]].id;

								dataPtr++;

								outData[dataPtr]				=	creature[creatureMap[row][col]].facing; // creatureMap[row][col];
								if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].facing) SendData=true;
								playerData[i][dataPtr]	=	creature[creatureMap[row][col]].facing;

								dataPtr++;

								if (creature[creatureMap[row][col]].id==kNPCHuman)
									{

										// ------- check for chat text
										if ((creature[creatureMap[row][col]].playerIndex<0) || (creature[creatureMap[row][col]].playerIndex>=kMaxNPCs))			// beta debug
											fi_WriteToErrorLog("Out of Range - npc - nw_SendRefresh");

										if ((npc[creature[creatureMap[row][col]].playerIndex].talkTime>=player[i].lastRefresh) && (player[i].lastRefresh!=0))
											(i, creature[creatureMap[row][col]].playerIndex, j);
										// ----------------------------

										if (npc[creature[creatureMap[row][col]].playerIndex].legs==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	npc[creature[creatureMap[row][col]].playerIndex].legs;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (npc[creature[creatureMap[row][col]].playerIndex].torso==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	npc[creature[creatureMap[row][col]].playerIndex].torso;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;
									}

								if (creature[creatureMap[row][col]].id==kHuman)
									{

										// ------- check for chat text
										if ((player[creature[creatureMap[row][col]].playerIndex].talkTime>=player[i].lastRefresh) && (player[i].lastRefresh!=0))
											nw_SendTextToClient(i, creature[creatureMap[row][col]].playerIndex, j);
										// ----------------------------

										outData[dataPtr]=creature[creatureMap[row][col]].strikePose;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kRightHand]].itemID;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (player[creature[creatureMap[row][col]].playerIndex].arm[kLeftHand]==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLeftHand]].itemID;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (player[creature[creatureMap[row][col]].playerIndex].arm[kLegs]==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kLegs]].itemID;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kTorso]].itemID;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

										if (player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]==0)
											outData[dataPtr]=0;
										else
											outData[dataPtr]				=	creature[creatureMap[row][col]].inventory[player[creature[creatureMap[row][col]].playerIndex].arm[kFeet]].itemID;
										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;

									}


								if ((creature[creatureMap[row][col]].timeStamp>=player[i].lastRefresh) && (player[i].lastRefresh!=0))
									{ // creature moved since last player refresh
										if (j==90)
											outData[dataPtr]=4;
										else
											outData[dataPtr]	=	creature[creatureMap[row][col]].moveDirection;
										fhits=creature[creatureMap[row][col]].hitPoints;
										fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxHitPoints;
										hits=(((fhits/fmhits)*100)/7)+1;
										if (hits>15) hits=15;
										if (hits>7) 	{ hits=hits-8; outData[dataPtr]=outData[dataPtr]+128; 	}
										if (hits>3) 	{ hits=hits-4; outData[dataPtr]=outData[dataPtr]+64; 		}
										if (hits>1) 	{ hits=hits-2; outData[dataPtr]=outData[dataPtr]+32; 		}
										if (hits>0) 	{ hits=hits-1; outData[dataPtr]=outData[dataPtr]+16; 		}

										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];

										dataPtr++;
									}
								else
									{
										outData[dataPtr]=4;

										fhits=creature[creatureMap[row][col]].hitPoints;
										fmhits=creatureInfo[creature[creatureMap[row][col]].id].maxHitPoints;
										hits=(((fhits/fmhits)*100)/7)+1;
										if (hits>15) hits=15;
										if (hits>7) 	{ hits=hits-8; outData[dataPtr]=outData[dataPtr]+128; 	}
										if (hits>3) 	{ hits=hits-4; outData[dataPtr]=outData[dataPtr]+64; 		}
										if (hits>1) 	{ hits=hits-2; outData[dataPtr]=outData[dataPtr]+32; 		}
										if (hits>0) 	{ hits=hits-1; outData[dataPtr]=outData[dataPtr]+16; 		}

										if (playerData[i][dataPtr]!=outData[dataPtr]) SendData=true;
										playerData[i][dataPtr]	=	outData[dataPtr];
										dataPtr++;
									}

							}
						//playerState[i][targetFrame][j]=creatureMap[row][col];
					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]=255;
				dataPtr++;

				for (j=0;j<containerPtr;j++)	// build container list
					{
						row=mapToScreenRow[containerList[j]]+creature[player[i].creatureIndex].row-13;
						col=mapToScreenCol[containerList[j]]+creature[player[i].creatureIndex].col-13;
						if (containerMap[row][col]!=0)
							{

								outData[dataPtr]	=	containerList[j];
								if (playerData[i][dataPtr]!=containerList[j]) SendData=true;
								playerData[i][dataPtr]	= containerList[j];
								dataPtr++;

								outData[dataPtr]	=	containerID[containerMap[row][col]]; //1; // satchel;
								if (playerData[i][dataPtr]!=containerID[containerMap[row][col]]) SendData=true;
								playerData[i][dataPtr]	=	containerID[containerMap[row][col]];
								dataPtr++;

							}
					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]	=	255;
				dataPtr++;

				for (j=0;j<monsterPtr;j++)	// build fx list
					{
						row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
						col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;
						if (creatureMap[row][col]!=0)
							{

								//fxFound=false;
								//for (k=0;k<kMaxCreatureEffects;k++)
								//	if (creature[creatureMap[row][col]].fx[k]!=0)
								//		{
								//			fxFound=true;
								//			break;
								//		}

								if (creature[creatureMap[row][col]].activeFX)		//(fxFound)
									{
										outData[dataPtr]	=	monsterList[j];
										if (playerData[i][dataPtr]!=monsterList[j]) SendData=true;
										playerData[i][dataPtr]	= monsterList[j];
										dataPtr++;

										for (k=0;k<kMaxCreatureEffects;k++)
											{
												outData[dataPtr]	=	creature[creatureMap[row][col]].fx[k];
												if (playerData[i][dataPtr]!=creature[creatureMap[row][col]].fx[k]) SendData=true;
												playerData[i][dataPtr]	=	creature[creatureMap[row][col]].fx[k];
												dataPtr++;

												outData[dataPtr]	=	serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)];
												if (playerData[i][dataPtr]!=serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)]) SendData=true;
												playerData[i][dataPtr]	=	serverFX[RC(creature[creatureMap[row][col]].fx[k],kMaxFX)];
												dataPtr++;

												target=255;
												for (p=0;p<monsterPtr;p++)	// build fx list
													{
														row2=mapToScreenRow[monsterList[p]]+creature[player[i].creatureIndex].row-13;
														col2=mapToScreenCol[monsterList[p]]+creature[player[i].creatureIndex].col-13;
														if (creatureMap[row2][col2]==creature[creatureMap[row][col]].fxTarget[k])
															{
																target=(UInt8)monsterList[p];
																break;
															}
													}

												if (target==255)	// probably dead ... target last know position
													for (p=0;p<kMaxScreenElements;p++)	// build creature list
														{
															row2=mapToScreenRow[p]+creature[player[i].creatureIndex].row-13;
															col2=mapToScreenCol[p]+creature[player[i].creatureIndex].col-13;
															if ((row2==creature[creatureMap[row][col]].fxTargetRow[k]) && (col2==creature[creatureMap[row][col]].fxTargetCol[k]))
																{
																	target=p;
																	break;
																}
														}

												outData[dataPtr]	=	target;
												if (playerData[i][dataPtr]!=target) SendData=true;
												playerData[i][dataPtr]	=	target;
												dataPtr++;
											}
									}

							}
					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]	=	255;
				dataPtr++;

				for (j=0;j<fxPtr;j++)	// build fx list
					{
						row=mapToScreenRow[fxList[j]]+creature[player[i].creatureIndex].row-13;
						col=mapToScreenCol[fxList[j]]+creature[player[i].creatureIndex].col-13;

						outData[dataPtr]	=	fxList[j];
						if (playerData[i][dataPtr]!=fxList[j]) SendData=true;
						playerData[i][dataPtr]	= fxList[j];
						dataPtr++;

						outData[dataPtr]	=	fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)];
						if (playerData[i][dataPtr]!=fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)]) SendData=true;
						playerData[i][dataPtr]	=	fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)];
						dataPtr++;

						outData[dataPtr]	=	serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)];
						if (playerData[i][dataPtr]!=serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)]) SendData=true;
						playerData[i][dataPtr]	=	serverFX[RC(fxMap[RC(row,kMapWidth)][RC(col,kMapHeight)],kMaxFX)];
						dataPtr++;

					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]	=	255;
				dataPtr++;

				if (player[i].sendStatsRefresh)
					{
						outData[dataPtr]	=	creature[player[i].creatureIndex].strength;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].strength) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].strength;
						dataPtr++;

						outData[dataPtr]	=	creature[player[i].creatureIndex].intelligence;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].intelligence) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].intelligence;
						dataPtr++;

						outData[dataPtr]	=	creature[player[i].creatureIndex].dexterity;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].dexterity) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].dexterity;
						dataPtr++;

						outData[dataPtr]	=	creature[player[i].creatureIndex].hitPoints;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].hitPoints) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].hitPoints;
						dataPtr++;

						outData[dataPtr]	=	creature[player[i].creatureIndex].magicPoints;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].magicPoints) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].magicPoints;
						dataPtr++;

						outData[dataPtr]	=	creature[player[i].creatureIndex].armorRating;
						if (playerData[i][dataPtr]!=creature[player[i].creatureIndex].armorRating) SendData=true;
						playerData[i][dataPtr]	=	creature[player[i].creatureIndex].armorRating;
						dataPtr++;

						player[i].sendStatsRefresh=false;
						forceRefresh=true;
					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]	=	255;
				dataPtr++;

				soundFX=0;
				for (j=0;j<monsterPtr;j++)	// build sound list
					{
						row=mapToScreenRow[monsterList[j]]+creature[player[i].creatureIndex].row-13;
						col=mapToScreenCol[monsterList[j]]+creature[player[i].creatureIndex].col-13;
						if (creatureMap[row][col]!=0)
							{
								if ((creature[creatureMap[row][col]].soundFX!=0) && (creature[creatureMap[row][col]].soundFXTime>=player[i].lastRefresh) && (player[i].lastRefresh!=0))
									{
										outData[dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].attackSound;
										if (playerData[i][dataPtr]!=creatureInfo[creature[creatureMap[row][col]].id].attackSound) SendData=true;
										playerData[i][dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].attackSound;
										dataPtr++;
										soundFX++;
										if (soundFX>5)
											break;
									}
								else if (creatureInfo[creature[creatureMap[row][col]].id].randomSound!=0)
									{
										if (tb_Rnd(1,250)==1)
											{
												outData[dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].randomSound;
												if (playerData[i][dataPtr]!=creatureInfo[creature[creatureMap[row][col]].id].randomSound) SendData=true;
												playerData[i][dataPtr]	=	creatureInfo[creature[creatureMap[row][col]].id].randomSound;
												dataPtr++;
												soundFX++;
												if (soundFX>5)
													break;
											}
									}
							}

					}

				outData[dataPtr]=255;
				if (playerData[i][dataPtr]!=255) SendData=true;
				playerData[i][dataPtr]	=	255;

				if (dataPtr>kDataSize)		// beta debug
					fi_WriteToErrorLog("Out of Range - nw_SendRefresh");

				if (dataPtr>maxDataSize)
					maxDataSize=dataPtr;

				lastMessageSize=dataPtr;

				if ((SendData) || (forceRefresh))	// Send the data
					{
						player[i].lastRefresh=TickCount();
						gPlayerRefreshMessage = malloc( sizeof( PlayerRefreshMessage) + ( sizeof(UInt8) * (dataPtr)));
						if (gPlayerRefreshMessage!=NULL)
							{
								NSpClearMessageHeader(&gPlayerRefreshMessage->h);
								gPlayerRefreshMessage->h.what 				= kPlayerRefreshMessage;
								gPlayerRefreshMessage->h.to 					= player[i].id;
								gPlayerRefreshMessage->h.messageLen 	= sizeof( PlayerRefreshMessage) + (sizeof(UInt8) * (dataPtr));
								gPlayerRefreshMessage->row						=	creature[player[i].creatureIndex].row-13;
								gPlayerRefreshMessage->col						=	creature[player[i].creatureIndex].col-13;
								gPlayerRefreshMessage->dataLength			=	dataPtr+1;
								theData=&gPlayerRefreshMessage->data[0];
								for ( m = 0; m <= dataPtr; m++)
									{
										*theData=outData[m];
								  	theData++;
									}
								status = NSpMessage_Send(gNetGame, &gPlayerRefreshMessage->h, kSendMethod);
								free(gPlayerRefreshMessage);
							}
					}

			}

}
*/
#endif

// ----------------------------------------------------------------- //
void nw_ShutdownNetworking(void) // server/client
// ----------------------------------------------------------------- //

{
	OSStatus	status;

//gr_ShowDebugLocation(98); // server only
	gotTermination=true;

	if (gNetGame)
	{
		status = NSpGame_Dispose( gNetGame, kNSpGameFlag_ForceTerminateGame );
		gNetGame = NULL;
		fi_WriteToErrorLog("Server connection terminated.");	// logging
	}

}

// ----------------------------------------------------------------- //
void nw_GetMessages(void)	// server/client
// ----------------------------------------------------------------- //
{
	NSpMessageHeader					*theMessage;
	int									i;
	unsigned long						timeExceeded;
	unsigned long					  	currentTime=TickCount();
	int									messageType;
	//TBoolean                  			skipMessage;


#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetMessages Entry");
#endif

	if (gNetGame==NULL)
		return;

	fi_WriteToTraceLog("1");

#ifdef _SERVER
	for (i=1;i<kMaxPlayers;i++)
		if (waitingForAuthorization[i]!=0)
			if (currentTime>authorizationTimeOut[i])
			{
				fi_WriteToTraceLog("2");
				nw_RemovePlayer(waitingForAuthorization[i]);
				fi_WriteToTraceLog("3");
				waitingForAuthorization[i]=0;
				authorizationTimeOut[i]=0;
			}

	for (i=0;i<5000;i++)
	{
		if (ToRemove[i]!=0)
			if (TickCount()>ToRemove[i])
			{
				ToRemove[i]=0;
				nw_RemovePlayer(i);
			}
	}

#endif

	timeExceeded=currentTime+30;

	/*
	  while (queueReadPtr!=queueWritePtr)
			{
			  if (gInCallback)  return;
				theMessage = (NSpMessageHeader*)(myQueue[queueReadPtr]);
				nw_DoHandleMessage(theMessage);
				queueReadPtr++;
				if (queueReadPtr>=kMaxHandlerMessageCount)
				  queueReadPtr=0;
			}

	  #ifdef _FUNCTIONTRACEDEBUGGING
		fi_WriteToErrorLog("Leaving nw_GetMessages");
	  #endif
	*/

#ifndef _SERVERONLY
	if (currentlyTracking!=0)
		gr_RenderMapWindow(true);

#endif

	do
	{
		theMessage = NULL;
		fi_WriteToTraceLog("4");
		theMessage = NSpMessage_Get(gNetGame);

		if (theMessage!=NULL)
		{
			messageType=theMessage->what;
			fi_WriteToTraceLog("5");
			nw_DoHandleMessage(theMessage);

			if (gNetGame!=NULL)
			{
				fi_WriteToTraceLog("6");
				NSpMessage_Release(gNetGame, theMessage);
				theMessage=NULL;
			}
		}
	}
	while ((gNetGame && theMessage != NULL) && (TickCount()<timeExceeded));

	fi_WriteToTraceLog("7");

#ifdef _FUNCTIONTRACEDEBUGGING
	fi_WriteToErrorLog("nw_GetMessages Exit");

#endif

	/*
	#ifdef _SERVERONLY
		if (theMessage!=NULL)
			exceededNT++;
	#endif

	#ifndef _SERVERONLY
		for (i=0;i<10;i++)
			if (waitingFor[i]!=0)
				if (TickCount()>reSendAt[i])
					{
					  waitingFor[i]	=	0;
					  reSendAt[i]		=	0;
						nw_SendAction(waitingFor[i], kHighPriority, kSendNow,false);
						dataReSendError++;
					}
	#endif
	*/

}

// ----------------------------------------------------------------- //
void nw_GetRefresh(NSpMessageHeader *inMessage) // client variable bytes
// ----------------------------------------------------------------- //

{
	PlayerRefreshMessage2 				*theMessage = (PlayerRefreshMessage2 *) inMessage;  // rev 017
	int														i;
	//int														length;
	UInt32                        sum=0;

//	if (lastRefreshMessageTime>inMessage->when)
//		{
	//		nw_SendAction(kRefreshStats, kHighPriority, kSendNow,false);
//			fi_WriteToErrorLog("Refresh Order Error");	// beta debug
//			return;
//		}
	//if (NSpGetCurrentTimeStamp(gNetGame)>(inMessage->when+5000))
	//	{
	//		nw_SendAction(kRefreshStats, kHighPriority, kSendNow,false);
	//		fi_WriteToErrorLog("Old Message ... Skip");	// beta debug
	//		return;
	//	}
//	lastRefreshMessageTime=inMessage->when; // 11/7

	for (i=0;i<theMessage->dataLength;i++)
	{
		inData[i]=theMessage->data[i];
		sum=sum+inData[i];
	}

	for (i=theMessage->dataLength;i<kDataSize;i++)
		inData[i]=0;

	sum=sum+theMessage->row+theMessage->col;

	if (sum!=theMessage->checksum)
	{
		//nw_SendAction(kRefreshStats, kHighPriority, kSendNow,false);
		fi_WriteToErrorLog("Refresh checksum failure");	// beta debug
		//if (thisIsReconnect)
		//  if (gNetGame!=NULL)
		//    myID=NSpPlayer_GetMyID(gNetGame);
		return;
	}

	if (theMessage->dataLength>kDataSize)
		return;

	if (nw_RefreshDataOkay(theMessage->dataLength))
	{
		if ((theMessage->row>=kMapWidth) || (theMessage->col>=kMapHeight))
		{
			//nw_SendAction(kRefreshStats, kHighPriority, kSendNow,false);
			fi_WriteToErrorLog("Bad Player Location");	// beta debug
			return;
		}

		inRow	=	theMessage->row;

		inCol	=	theMessage->col;
		nw_ParseRefresh();
#ifndef _SERVERONLY
		gr_SwapBitmaps();
#endif
	}
	else
	{
		//nw_SendAction(kRefreshStats, kHighPriority, kSendNow,false);
		fi_WriteToErrorLog("Bad Refresh");	// beta debug
	}

}

// ----------------------------------------------------------------- //
TBoolean nw_RefreshDataOkay(int length)
// ----------------------------------------------------------------- //

{
	int															i;
	int															j;
	int															dataPtr;
	TBoolean												parsing=true;
	//int															xOff;
	//int															yOff;
	UInt8														data;
	int															bit1;
	int															bit2;
	int															bit3;
	int															bit4;
	int															bit5;
	int															bit6;
	int															bit7;
	int															bit8;
	int															index;
	//UInt8														fxPosition;
	//UInt8														fxID;
	//UInt8														fxType;
	//UInt8														fxDest;
	//UInt16													theSound;
	//int															b1;
	//int															b2;
	//int                             breakOut;
	int                             dummy;
	UInt16												  state												[kMaxScreenElements];
	UInt16												  containerState							[kMaxScreenElements];
	UInt16												  bloodStainState							[kMaxScreenElements];
	UInt8												    creatureDirection						[kMaxScreenElements];
	UInt8												    creatureFacing							[kMaxScreenElements];
	UInt8												    creatureHealth							[kMaxScreenElements];
	UInt8													creatureMp								[kMaxScreenElements];	
	UInt8												    creatureAttacking						[kMaxScreenElements];
	int												      creatureID						      [kMaxScreenElements];
	TBoolean											  poisoned										[kMaxScreenElements];
	TBoolean												cursed									[kMaxScreenElements];
	UInt8												    human												[kMaxScreenElements][16];

	for (i=0;i<kMaxScreenElements;i++)
	{
		state[i]							=	0;
		containerState[i]			=	0;
		bloodStainState[i]		=	0;
		tracksState[i]			=	0;
		creatureDirection[i]	=	4;
		creatureFacing[i]			= 0;
		creatureHealth[i]			=	0;
		creatureMp[i]				=0;
		creatureAttacking[i]	=	0;
		creatureID[i]	        =	0;
		poisoned[i]						= false;
		cursed[i]						= false;

		for (j=0;j<19;j++)
			human[i][j]					= 0;
	}

	dataPtr=0;

	dummy=inData[dataPtr];
	dataPtr++;

	dummy=inData[dataPtr];
	dataPtr++;

	dummy=inData[dataPtr];
	dataPtr++;
	dummy=inData[dataPtr];
	dataPtr++;
	dummy=inData[dataPtr];
	dataPtr++;
	dummy=inData[dataPtr];
	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		index=inData[dataPtr];

		if (index>=kMaxScreenElements) // paranoid error checking .. something wrong ... skip frame
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 1");	// beta debug
			return (false);
		}

		dataPtr++;

		dummy=inData[dataPtr];
		dataPtr++;

		data=inData[dataPtr];

		if (data>1) // rev 017
		{
			poisoned[index]=true;
			data=data-2;
		}

		if (data>0)
			creatureAttacking[index]=true;
		else
			creatureAttacking[index]=false;

		dataPtr++;

		dataPtr++;

		state[index]=inData[dataPtr+1];
		
		if (state[index]>127)
		{
			state[index]-=127;
		}

		if (state[index]>=kMaxCreatureTypes)
		{
			fi_WriteToErrorLog("Parse Error Creature type out of range.");
			return (false);
		}

		creatureFacing[index]=inData[dataPtr+2];

		if (state[index]==kNPCHuman || state[index]==kNPCHumanFemale)
		{
			if ((creatureFacing[index]>9) && (creatureFacing[index]<18))	// get rid of modifier for npc
				creatureFacing[index]=creatureFacing[index]-10;

			human[index][13]							= inData[dataPtr+3];	// pose

			human[index][kLegs]						= inData[dataPtr+4];

			human[index][kTorso]					= inData[dataPtr+5];

			human[index][18]						= inData[dataPtr+6];

			human[index][15]						= inData[dataPtr+7];

			human[index][16]						= inData[dataPtr+8];

			human[index][17]						= inData[dataPtr+9];

			human[index][kFeet]						= kBlackBoots;

			if (inData[dataPtr+6]==kNPCGuard) // guard ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kMedalShield;
				human[index][kHead]				    = kClosedPlateHelmet;
			}

			if (inData[dataPtr+6]==kNPCBrigand) // brigand ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kWoodenShield;
				human[index][kHead]				    = kVikingHelmet;
			}

			if (inData[dataPtr+6]==kNPCMercenary) // merc ... add glaive
			{
				human[index][kRightHand]			= kGlaive;
				human[index][kHead]				    = kChainMailHelmet;
			}
			//Dave
			if (inData[dataPtr+6]==kNPCPrivateer) // Privateer ...
			{
				human[index][kRightHand]			= kKatanaSpeed;
				//human[index][kRightFinger]				   = k15ARRing;
				//human[index][kNeck]				   = kCharmOfSwiftness;
			}
			if (inData[dataPtr+6]==kNPCDwarf1) // Legionnaire ...
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kWoodenShield;
				human[index][kHead]				    = kVikingHelmet;
				
				//human[index][kRightHand]			= kLongSword;
				//human[index][kHead]				    = kBurningPlateHelmet;
				//human[index][kLeftHand]				= kBurningMetalShield;
				//human[index][kFeet]					= kRedBoots;
				//human[index][kArms]					= kBurningPlateArms;			
			}
			dataPtr=dataPtr+7;
		}

		/*
					if (state[index]==kHuman)
						{
						  human[index][14]              = inData[dataPtr+3];
						  human[index][15]              = inData[dataPtr+4];
							human[index][13]							= inData[dataPtr+5];	// pose
							human[index][kHead]			      = inData[dataPtr+6];
							human[index][kRightHand]			= inData[dataPtr+7];
							human[index][kLeftHand]				= inData[dataPtr+8];
							human[index][kLegs]						= inData[dataPtr+9];
							human[index][kTorso]					= inData[dataPtr+10];
							human[index][kFeet]						= inData[dataPtr+11];
							human[index][kArms]						= inData[dataPtr+12];
							dataPtr=dataPtr+10;
						}
		*/
		if (state[index]==kHuman)
		{
			//human[index][14]              = inData[dataPtr+3];
			human[index][15]              = inData[dataPtr+3];
			human[index][16]              = inData[dataPtr+4];
			human[index][17]              = inData[dataPtr+5];
			human[index][13]							= inData[dataPtr+6];	// pose
			human[index][kHead]			      = inData[dataPtr+7];
			human[index][kRightHand]			= inData[dataPtr+8];
			human[index][kLeftHand]				= inData[dataPtr+9];
			human[index][kLegs]						= inData[dataPtr+10];
			human[index][kTorso]					= inData[dataPtr+11];
			human[index][kFeet]						= inData[dataPtr+12];
			human[index][kArms]						= inData[dataPtr+13];
			dataPtr=dataPtr+11;
		}

		for (i=1;i<13;i++)  // paranoind error checking
			if (human[index][i]>kMaxItemsUsed)
				human[index][i]=0;

		dataPtr=dataPtr+3;

		data=inData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		creatureDirection[index]		=bit1+bit2+bit3+bit4;

		creatureHealth[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureHealth[index]>100)
			creatureHealth[index]=100;

		dataPtr=dataPtr+1;
		
		/*data=inData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		cursed[index]		=((bit1+bit2+bit3+bit4)==0)?false:true;

		creatureMp[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureMp[index]>100)
			creatureMp[index]=100;
		
		dataPtr=dataPtr+1;*/

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 2");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 2");	// beta debug
			return (false);
		}

	}

	dataPtr++;


	while (inData[dataPtr]!=255)
	{

		bloodStainState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>length)
			return (false);

	}


	dataPtr++;

	dummy=inData[dataPtr];
	dataPtr++;

	while (inData[dataPtr]!=255)
	{

		containerState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 3");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 3");	// beta debug
			return (false);
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)
	{

		tracksState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 3");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 3");	// beta debug
			return (false);
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		dummy=inData[dataPtr];
		dummy=inData[dataPtr+1];

		dataPtr=dataPtr+2;
	}

	dataPtr++;

	while (inData[dataPtr]!=255)	// parse creature FX
	{

		dummy		    =	inData[dataPtr];

		if (dummy>=kMaxScreenElements)
		{
			fi_WriteToErrorLog("Parse Error Bad Creature FX position.");
			return (false);
		}

		//--------------------------------

		dummy				= inData[dataPtr+1];

		dummy				= inData[dataPtr+2];

		dummy				=	inData[dataPtr+3];

		dummy				=	inData[dataPtr+4];

		//--------------------------------

		dummy				= inData[dataPtr+5];

		dummy				= inData[dataPtr+6];

		dummy				=	inData[dataPtr+7];

		dummy				=	inData[dataPtr+8];

		//--------------------------------

		dummy				= inData[dataPtr+9];

		dummy				= inData[dataPtr+10];

		dummy				=	inData[dataPtr+11];

		dummy				=	inData[dataPtr+12];

		dataPtr=dataPtr+13;

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 4");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 4");	// beta debug
			return (false);
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)	// parse terrain FX
	{

		dummy		=	inData[dataPtr];

		if (dummy>=kMaxScreenElements)
		{
			fi_WriteToErrorLog("Parse Error Bad terrain FX position.");
			return (false);
		}

		dummy		= inData[dataPtr+1];

		dummy		= inData[dataPtr+2];

		dummy		=	inData[dataPtr+3];

		dataPtr=dataPtr+4;

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 5");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 5");	// beta debug
			return (false);
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		dummy		= inData[dataPtr];
		dummy		= inData[dataPtr+1];
		dummy		= inData[dataPtr+2];
		dummy		= inData[dataPtr+3];
		dummy		= inData[dataPtr+4];
		dummy		= inData[dataPtr+5];
		dummy		= inData[dataPtr+6];
		dummy		= inData[dataPtr+7];
		dummy		= inData[dataPtr+8];
		dummy		= inData[dataPtr+9];
		dummy		= inData[dataPtr+10];
		dataPtr=dataPtr+11;

		if (dataPtr>length)
		{
			// fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 6");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 6");	// beta debug
			return (false);
		}
	}

	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		for (i=0;i<kMaxPets;i++)
		{
			dummy  = inData[dataPtr];
			dummy  = inData[dataPtr+1];
			dummy  = inData[dataPtr+2];
			dataPtr=dataPtr+3;
		}

		if (dataPtr>length)
		{
			//fi_DumpTestData(length);
			//fi_WriteToErrorLog("Test out of range 7");	// beta debug
			//fi_WriteToErrorLog("Skip Frame/Corrupt Data 7");	// beta debug
			return (false);
		}
	}

	dataPtr++;

	while (inData[dataPtr]!=255)			// sound effects
	{
		if ((inData[dataPtr]>0) && (inData[dataPtr]<kMaxSoundFX))
		{
			if (dataPtr>length)
			{
				//fi_DumpTestData(length);
				//fi_WriteToErrorLog("Test out of range 8");	// beta debug
				//fi_WriteToErrorLog("Skip Frame/Corrupt Data 8");	// beta debug
				return (false);
			}
		}

		dataPtr++;
	}

	return (true);

}

// ----------------------------------------------------------------- //
void nw_ParseRefresh(void) // client
// ----------------------------------------------------------------- //

{
#ifndef _SERVERONLY

	int															i;
	int															cpt;
	int															j;
	int															dataPtr;
	TBoolean												parsing=true;
	int															xOff;
	int															yOff;
	UInt8														data;
	int															bit1;
	int															bit2;
	int															bit3;
	int															bit4;
	int															bit5;
	int															bit6;
	int															bit7;
	int															bit8;
	int															index;
	UInt8														fxPosition;
	UInt8														fxID;
	UInt8														fxType;
	UInt8														fxDest;
	int													theSound;
	int															b1;
	int															b2;
	TBoolean												wasDead=playerDead;
	int                             breakOut;
	TBoolean                        fOnline[16];
	TBoolean                        fSharing[16];
	TBoolean                        fChange;
	int															cID;

	frame=4;
	
	


//gr_ShowDebugLocation(101); // server only

	for (i=0;i<kMaxScreenElements;i++)
	{
		state[i]							=	0;
		containerState[i]			=	0;
		bloodStainState[i]		=	0;
		tracksState[i]			=	0;
		creatureDirection[i]	=	4;
		creatureFacing[i]			= 0;
		creatureHealth[i]			=	0;
		creatureMp[i]				=0;
		creatureAttacking[i]	=	0;
		creatureID[i]	        =	0;
		cursed[i]					=false;
		poisoned[i]						= false;
		afk[i]							=false;
		questChar[i]				  = false;
		stealthChar[i]				= false;
		regenerating[i]				=false;
		gmChar[i]						  = false;
		isPet[i]						  = false;
		playerOnline[i]       = false;
		morphed[i]					=false;

		for (j=0;j<19;j++)
			human[i][j]					= 0;
	}

	dataPtr=0;

	gameTime=inData[dataPtr];

	if (gameTime>=127)
	{
		playerDead=true;
		gameTime=gameTime-127;
	}
	else
		playerDead=false;

	dataPtr++;

	if (gameTime==0)
		nightVision=false;

	if (previousLocationValue==0)
	{
		if (inData[dataPtr]&1)
			tx_AddText(kEnterBankMsg);

		if (inData[dataPtr]&2)
			tx_AddText(kEnterShopMsg);

		if (inData[dataPtr]&8)
			tx_AddText(kMeetTraderMsg);
	}

	if (inData[dataPtr]&1)
		standingInBank=true;
	else
		standingInBank=false;

	if (inData[dataPtr]&4)
		standingOnActionSpot=true;
	else
		standingOnActionSpot=false;

	if ((inData[dataPtr]&2) || (inData[dataPtr]&8))
		standingInShop=true;
	else
		standingInShop=false;

	//if (inData[dataPtr]==3)
	//	standingInShop=true;
	//else
	//	standingInShop=false;

	previousLocationValue=inData[dataPtr];

	dataPtr++;

	friend1=inData[dataPtr];

	dataPtr++;

	friend2=inData[dataPtr];

	dataPtr++;

	friend3=inData[dataPtr];

	dataPtr++;

	friend4=inData[dataPtr];

	dataPtr++;

	for (i=0;i<16;i++)
	{
		fOnline[i]=false;
		fSharing[i]=false;
	}

	if (friend1>127)   { fOnline[0]=true;  friend1=friend1-128; }

	if (friend1>63)    { fOnline[1]=true;  friend1=friend1-64;  }

	if (friend1>31)    { fOnline[2]=true;  friend1=friend1-32;  }

	if (friend1>15)    { fOnline[3]=true;  friend1=friend1-16;  }

	if (friend1>7)     { fOnline[4]=true;  friend1=friend1-8;   }

	if (friend1>3)     { fOnline[5]=true;  friend1=friend1-4;   }

	if (friend1>1)     { fOnline[6]=true;  friend1=friend1-2;   }

	if (friend1>0)     { fOnline[7]=true;  friend1=friend1-1;   }

	if (friend2>127)   { fSharing[0]=true;  friend2=friend2-128; }

	if (friend2>63)    { fSharing[1]=true;  friend2=friend2-64;  }

	if (friend2>31)    { fSharing[2]=true;  friend2=friend2-32;  }

	if (friend2>15)    { fSharing[3]=true;  friend2=friend2-16;  }

	if (friend2>7)     { fSharing[4]=true;  friend2=friend2-8;   }

	if (friend2>3)     { fSharing[5]=true;  friend2=friend2-4;   }

	if (friend2>1)     { fSharing[6]=true;  friend2=friend2-2;   }

	if (friend2>0)     { fSharing[7]=true;  friend2=friend2-1;   }

	if (friend3>127)   { fOnline[8]=true;  friend3=friend3-128; }

	if (friend3>63)    { fOnline[9]=true;  friend3=friend3-64;  }

	if (friend3>31)    { fOnline[10]=true; friend3=friend3-32;  }

	if (friend3>15)    { fOnline[11]=true; friend3=friend3-16;  }

	if (friend3>7)     { fOnline[12]=true; friend3=friend3-8;   }

	if (friend3>3)     { fOnline[13]=true; friend3=friend3-4;   }

	if (friend3>1)     { fOnline[14]=true; friend3=friend3-2;   }

	if (friend3>0)     { fOnline[15]=true; friend3=friend3-1;   }

	if (friend4>127)   { fSharing[8]=true;  friend4=friend4-128; }

	if (friend4>63)    { fSharing[9]=true;   friend4=friend4-64;  }

	if (friend4>31)    { fSharing[10]=true;  friend4=friend4-32;  }

	if (friend4>15)    { fSharing[11]=true;  friend4=friend4-16;  }

	if (friend4>7)     { fSharing[12]=true;  friend4=friend4-8;   }

	if (friend4>3)     { fSharing[13]=true;  friend4=friend4-4;   }

	if (friend4>1)     { fSharing[14]=true;  friend4=friend4-2;   }

	if (friend4>0)     { fSharing[15]=true;  friend4=friend4-1;   }

	fChange=false;

	for (i=0;i<16;i++)
		if (fOnline[i]!=friendOnline[i])
		{
			friendOnline[i]=fOnline[i];
			fChange=true;
		}

	for (i=0;i<16;i++)
		if (fSharing[i]!=friendSharing[i])
		{
			friendSharing[i]=fSharing[i];
			fChange=true;
		}

	if (fChange)
		gr_RenderPartyWindow();

//extern TBoolean             friendOnline[16];
//extern TBoolean             friendInParty[16];
//  TBoolean                        fOnline[16];
//  TBoolean                        fChange;




	if ((!playerDead) && (wasDead))
	{
		//gr_DrawButtons(true);
		gr_DrawAllWindows();
	}

	//if ((playerDead) && ((!wasDead) || (gotFirstRefresh==false)))
	if ((playerDead) && (!wasDead))
	{
		tb_SetCursor(0);
		playerAction				=	0;
		targetSlot					=	0;
		targetCount					=	0;
		targetMode					= kNoTarget;
		currentInfoDisplay	=	kCharacterDisplay;

		gr_DrawAllWindows();

		/*	 // gs (11/18)
					gr_CloseFloatingWindow(tbInventoryWindow);
					gr_CloseFloatingWindow(tbPackContentsWindow);
					gr_CloseFloatingWindow(tbShopContentsWindow);
					gr_CloseFloatingWindow(tbSellListWindow);
					gr_CloseFloatingWindow(tbIDWindow);
					gr_CloseFloatingWindow(tbMakeWindow);
					gr_CloseFloatingWindow(tbSkillsWindow);
					gr_CloseFloatingWindow(tbStatsWindow);
					gr_CloseFloatingWindow(tbCharacterWindow);
					gr_CloseFloatingWindow(tbChessWindow);
					gr_CloseFloatingWindow(tbBankWindow);
					gr_CloseFloatingWindow(tbQuestWindow);
					gr_CloseFloatingWindow(tbPetsWindow);
		*/

		//gr_RenderCharacterDisplayWindow();
		// gr_RenderStatsWindow();

		//gr_RenderCharacterDisplay(true);
		//gr_DrawButtons(true);
	}
	while (inData[dataPtr]!=255)
	{
		index=inData[dataPtr];

		if (index>=kMaxScreenElements) // paranoid error checking .. something wrong ... skip frame
		{
			//tb_Beep();
			fi_DumpInData();
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh No GM Robe");	// beta debug
			return;
		}

		dataPtr++;

		if (inData[dataPtr]==1)
			afk[index]=true;


		dataPtr++;

		data=inData[dataPtr];

		if (data>=64) // rev 018
		{
			regenerating[index]=true;
			data=data-64;
		}

		playerOnline[index]=true;

		if (data>=32) // rev 017
		{
			stealthChar[index]=true;
			data=data-32;
		}

		if (data>=16) // rev 017
		{
			questChar[index]=true;
			data=data-16;
		}

		if (data>=8) // rev 017
		{
			gmChar[index]=true;
			data=data-8;
		}

		if (data>=4) // rev 017
		{
			isPet[index]=true;
			data=data-4;
		}

		if (data>=2) // rev 017
		{
			poisoned[index]=true;
			data=data-2;
		}

		if (data>0)
			creatureAttacking[index]=true;
		else
			creatureAttacking[index]=false;

		// start test stuff
		dataPtr++;

		b1						= inData[dataPtr];

		dataPtr++;

		b2						= inData[dataPtr];

		cID		= b1*256+b2;

		creatureID[index] = cID;

		// end test stuff

		state[index]=inData[dataPtr+1];
		
		if (state[index]>127) //morphed player
		{
			state[index]-=127;
			morphed[index]=true;
			human[index][14]=cID;
			
			//don't send gm or afk status if morphed
			gmChar[index]=false;
			afk[index]=false;
		}
		
		creatureFacing[index]=inData[dataPtr+2];

		if (state[index]==kNPCHuman || state[index]==kNPCHumanFemale)
		{
			if ((creatureFacing[index]>9) && (creatureFacing[index]<18))	// get rid of modifier for npc
				creatureFacing[index]=creatureFacing[index]-10;

			human[index][13]							= inData[dataPtr+3];	// pose

			human[index][kLegs]						= inData[dataPtr+4];

			human[index][kTorso]					= inData[dataPtr+5];

			human[index][18]						= inData[dataPtr+6];

			human[index][15]						= inData[dataPtr+7];

			human[index][16]						= inData[dataPtr+8];

			human[index][17]						= inData[dataPtr+9];

			human[index][kFeet]						= kBlackBoots;

			if (inData[dataPtr+6]==kNPCGuard) // guard ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kMedalShield;
				human[index][kHead]				    = kClosedPlateHelmet;
			}

			if (inData[dataPtr+6]==kNPCBrigand) // brigand ... add sword & shield
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kWoodenShield;
				human[index][kHead]				    = kVikingHelmet;
			}

			if (inData[dataPtr+6]==kNPCMercenary) // merc ... add glaive
			{
				human[index][kRightHand]			= kGlaive;
				human[index][kHead]				    = kChainMailHelmet;
			}
			//Dave
			if (inData[dataPtr+6]==kNPCPrivateer) // Privateer ...
			{
				human[index][kRightHand]			= kKatanaSpeed;
				//human[index][kHead]				    = kChainMailHelmet;
			}
			if (inData[dataPtr+6]==kNPCDwarf1) // Legionnaire ...
			{
				human[index][kRightHand]			= kLongSword;
				human[index][kLeftHand]				= kWoodenShield;
				human[index][kHead]				    = kVikingHelmet;
				
				//human[index][kRightHand]			= kFireSword;
				//human[index][kHead]				    = kBurningPlateHelmet;
				//human[index][kLeftHand]				= kBurningMetalShield;
				//human[index][kFeet]					= kRedBoots;			
			}

			dataPtr=dataPtr+7;
		}

		if (state[index]==kHuman)
		{
			//human[index][14]              = inData[dataPtr+3];
			human[index][14]              = cID;
			human[index][15]              = inData[dataPtr+3];
			human[index][16]              = inData[dataPtr+4];
			human[index][17]              = inData[dataPtr+5];
			human[index][13]							= inData[dataPtr+6];	// pose
			human[index][kHead]			      = inData[dataPtr+7];
			human[index][kRightHand]			= inData[dataPtr+8];
			human[index][kLeftHand]				= inData[dataPtr+9];
			human[index][kLegs]						= inData[dataPtr+10];
			human[index][kTorso]					= inData[dataPtr+11];
			human[index][kFeet]						= inData[dataPtr+12];
			human[index][kArms]						= inData[dataPtr+13];
			dataPtr=dataPtr+11;
		}

		for (i=1;i<13;i++)  // paranoind error checking
			if (human[index][i]>kMaxItemsUsed)
				human[index][i]=0;

		dataPtr=dataPtr+3;

		//if (inData[dataPtr]>0)
		//	poisoned[index]=true;

		//dataPtr=dataPtr+1;

		data=inData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		creatureDirection[index]		=bit1+bit2+bit3+bit4;

		creatureHealth[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureHealth[index]>100)
			creatureHealth[index]=100;

		dataPtr=dataPtr+1;
		
		/*data=inData[dataPtr];

		bit1=0;

		bit2=0;

		bit3=0;

		bit4=0;

		bit5=0;

		bit6=0;

		bit7=0;

		bit8=0;

		if (data>127) 	{ data=data-128; 	bit8=8; 	}

		if (data>63) 		{ data=data-64; 	bit7=4; 	}

		if (data>31) 		{ data=data-32; 	bit6=2; 	}

		if (data>15) 		{ data=data-16; 	bit5=1; 	}

		if (data>7) 		{ data=data-8; 		bit4=8; 	}

		if (data>3) 		{ data=data-4; 		bit3=4; 	}

		if (data>1) 		{ data=data-2; 		bit2=2; 	}

		if (data>0) 		{ data=data-1; 		bit1=1; 	}

		cursed[index]		=((bit1+bit2+bit3+bit4)==1)?false:true;

		creatureMp[index]				=(bit5+bit6+bit7+bit8)*2;

		if (creatureMp[index]>100)
			creatureMp[index]=100;
				
		dataPtr=dataPtr+1;*/

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 1");	// beta debug
			return;
		}

	}

	dataPtr++;
	while (inData[dataPtr]!=255)
	{

		bloodStainState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh blood state");	// beta debug
			return;
		}

	}

	dataPtr++;

	localTarget=inData[dataPtr];
	dataPtr++;

	while (inData[dataPtr]!=255)
	{

		containerState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 2");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)  //Tracks
	{
		tracksState[inData[dataPtr]]=inData[dataPtr+1];
		dataPtr=dataPtr+2;

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 3");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)  //Guild Halls
	{
		i=inData[dataPtr];
		j=inData[dataPtr+1];

		if (localHallsIDs[i]!=j)
		{
			localHallsIDs[i]=j;
			nw_SendHallRequest(i);
		}

		dataPtr=dataPtr+2;
	}

	dataPtr++;

	for (i=0;i<kMaxLocalFX;i++)
		localEffectsServiced	[i]=false;

	while (inData[dataPtr]!=255)	// parse creature FX
	{

		fxPosition		=	inData[dataPtr];

		b1						= inData[dataPtr+1];
		b2						= inData[dataPtr+2];
		fxID					= b1*256+b2;

		fxType				=	inData[dataPtr+3];
		fxDest				=	inData[dataPtr+4];

		gr_AddLocalFX(fxPosition, fxID, fxType, fxDest);

		b1						= inData[dataPtr+5];
		b2						= inData[dataPtr+6];
		fxID					= b1*256+b2;

		fxType				=	inData[dataPtr+7];
		fxDest				=	inData[dataPtr+8];

		gr_AddLocalFX(fxPosition, fxID, fxType, fxDest);

		b1						= inData[dataPtr+9];
		b2						= inData[dataPtr+10];
		fxID					= b1*256+b2;

		fxType				=	inData[dataPtr+11];
		fxDest				=	inData[dataPtr+12];

		gr_AddLocalFX(fxPosition, fxID, fxType, fxDest);

		dataPtr=dataPtr+13;

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 3");	// beta debug
			return;
		}

	}

	dataPtr++;

	while (inData[dataPtr]!=255)	// parse terrain FX
	{

		fxPosition		=	inData[dataPtr];

		b1						= inData[dataPtr+1];
		b2						= inData[dataPtr+2];
		fxID					= b1*256+b2;

		fxType				=	inData[dataPtr+3];

		gr_AddLocalFX(fxPosition, fxID, fxType, fxPosition);


		dataPtr=dataPtr+4;

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 4");	// beta debug
			return;
		}

	}

	for (i=0;i<kMaxScreenElements;i++)
	{

		/*if ((mapToScreenRow[i]+inRow==2128) && (mapToScreenCol[i]+inCol==1901))	// fountain in castle
			gr_AddLocalFX(i, kCastleFountainFX, kWaterFountain, i);

		if ((mapToScreenRow[i]+inRow==600) && (mapToScreenCol[i]+inCol==89))	// fountain in gm home
			gr_AddLocalFX(i, kCastleFountainFX, kWaterFountain, i);

		if ((mapToScreenRow[i]+inRow==2153) && (mapToScreenCol[i]+inCol==1986))	// temple fire
			gr_AddLocalFX(i, kTempleFire1FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2150) && (mapToScreenCol[i]+inCol==1989))	// temple fire
			gr_AddLocalFX(i, kTempleFire2FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2147) && (mapToScreenCol[i]+inCol==1986))	// temple fire
			gr_AddLocalFX(i, kTempleFire3FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2150) && (mapToScreenCol[i]+inCol==1983))	// temple fire
			gr_AddLocalFX(i, kTempleFire4FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2302) && (mapToScreenCol[i]+inCol==1773))	// temple fire
			gr_AddLocalFX(i, kTempleFire5FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2298) && (mapToScreenCol[i]+inCol==1773))	// temple fire
			gr_AddLocalFX(i, kTempleFire6FX, kTempleFire, i);

		if ((mapToScreenRow[i]+inRow==2213) && (mapToScreenCol[i]+inCol==1910))	// temple fire
			gr_AddLocalFX(i, kTempleFire7FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2218) && (mapToScreenCol[i]+inCol==1905))	// temple fire
			gr_AddLocalFX(i, kTempleFire8FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2208) && (mapToScreenCol[i]+inCol==1905))	// temple fire
			gr_AddLocalFX(i, kTempleFire9FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==2213) && (mapToScreenCol[i]+inCol==1900))	// temple fire
			gr_AddLocalFX(i, kTempleFire10FX, kTempleFire, i);

		if ((mapToScreenRow[i]+inRow==549) && (mapToScreenCol[i]+inCol==2929))	// temple fire
			gr_AddLocalFX(i, kTempleFire11FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==550) && (mapToScreenCol[i]+inCol==2930))	// temple fire
			gr_AddLocalFX(i, kTempleFire12FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==1311) && (mapToScreenCol[i]+inCol==3175))	// temple fire
			gr_AddLocalFX(i, kTempleFire13FX, kTempleFire, i);
		if ((mapToScreenRow[i]+inRow==1315) && (mapToScreenCol[i]+inCol==3175))	// temple fire
			gr_AddLocalFX(i, kTempleFire14FX, kTempleFire, i);*/

		if (clientMap[mapToScreenRow[i]+inRow][mapToScreenCol[i]+inCol]==666)
		{
			cpt=3000;

			for (j=0;j<kMaxLocalFX;j++)
				if (localEffectsID[j]==cpt && (localEffectsType[j]!=kWaterFountain || localEffectsLocation[j]!=i || localEffectsDestination[j]!=i))
					cpt++;

			gr_AddLocalFX(i, cpt, kWaterFountain, i);
		}

		if (clientMap[mapToScreenRow[i]+inRow][mapToScreenCol[i]+inCol]==971 || clientMap[mapToScreenRow[i]+inRow][mapToScreenCol[i]+inCol]==933)
		{
			cpt=5000;

			for (j=0;j<kMaxLocalFX;j++)
				if (localEffectsID[j]==cpt && (localEffectsType[j]!=kTempleFire || localEffectsLocation[j]!=i || localEffectsDestination[j]!=i))
					cpt++;

			gr_AddLocalFX(i, cpt, kTempleFire, i);
		}


	}

	gr_KillLocalFX();
	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		localStrength				= inData[dataPtr];
		localIntelligence		= inData[dataPtr+1];
		localDexterity			= inData[dataPtr+2];
		localHitPoints			= inData[dataPtr+3];
		localMagicPoints		= inData[dataPtr+4];
		localArmorRating		= inData[dataPtr+5];
		localLevel				= inData[dataPtr+6];
		localClass				= inData[dataPtr+7];
		localHunger				= (inData[dataPtr+8]*100)+inData[dataPtr+9];
		localStamina			= inData[dataPtr+10];
		dataPtr=dataPtr+11;
		gr_RenderStatsWindow();

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 5");	// beta debug
			return;
		}
	}
	//First refresh? resize skills window
	cpt=0;

	if (thisIsFirstRefresh)
	{
		for (j=0; j<kMaxSkills; j++)
		{	
			if (localClass>6 || maxSkillLevel[localClass][j]>0 || questChar[90])
				cpt++;
			else if (localRightFinger !=0 && itemList[itemList[localInventory[localRightFinger].itemID].spell].makeSkill==j)
				cpt++;
			else if (localLeftFinger != 0 && itemList[itemList[localInventory[localLeftFinger].itemID].spell].makeSkill==j)
				cpt++;
		}
			

		tb_SetRect(&skillsWindowRect,0,0,152,42+(13.2*cpt));

		SetWindowBounds(skillsWindow, kWindowContentRgn, &skillsWindowRect);

		MoveWindowStructure(skillsWindow,windowPosition[kSkillsWindow].h,windowPosition[kSkillsWindow].v);

		//gr_RenderSkillsWindow();
	}

	dataPtr++;

	while (inData[dataPtr]!=255)
	{
		for (i=0;i<kMaxPets;i++)
		{
			localPetType[i]     = inData[dataPtr];
			localPetStatus[i]   = inData[dataPtr+1];
			localPetLoyalty[i]  = inData[dataPtr+2];
			dataPtr=dataPtr+3;
		}

		gr_RenderPetsWindow();

		if (dataPtr>kDataSize)
		{
			//tb_Beep();
#ifdef _LEVEL2DEBUGGING
			fi_DumpInData();
#endif
			fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 5");	// beta debug
			return;
		}
	}
	dataPtr++;

	while (inData[dataPtr]!=255)			// sound effects
	{
		if ((inData[dataPtr]>0) && (inData[dataPtr]<kMaxSoundFX))
		{
			theSound=0;
			breakOut=0;

			do
			{
				theSound=soundList[inData[dataPtr]][tb_Rnd(0,(kMaxSoundTypes-1))];
				breakOut++;

				if (breakOut>20)
					break;
			}
			while (theSound==0);

			if (theSound==999)
				tb_Beep(0);
			else
				tb_PlaySound(theSound);

			if (dataPtr>kDataSize)
			{
				//tb_Beep();
#ifdef _LEVEL2DEBUGGING
				fi_DumpInData();
#endif
				fi_WriteToErrorLog("Out of Range - nw_ParseRefresh 6");	// beta debug
				return;
			}
		}

		dataPtr++;
	}
	
	xOff=0;

	yOff=0;
	animateStep=false;

	if (((playerCol-1)==inCol) && ((playerRow+1)==inRow))
	{
		xOff=24;//24;
		yOff=0;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=1;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerCol+1)==inCol) && ((playerRow-1)==inRow))
	{
		xOff=-24;//-24;
		yOff=0;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=2;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerCol+1)==inCol) && ((playerRow+1)==inRow))
	{
		xOff=0;
		yOff=24;//24;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=3;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerCol-1)==inCol) && ((playerRow-1)==inRow))
	{
		xOff=0;
		yOff=-24;//-24;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=4;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerRow+1)==inRow) && (playerCol==inCol))
	{
		xOff=16;
		yOff=16;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=5;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerRow-1)==inRow) && (playerCol==inCol))
	{
		xOff=-16;
		yOff=-16;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=6;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerCol+1)==inCol) && (playerRow==inRow))
	{
		xOff=-16;
		yOff=16;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=7;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}
	else if (((playerCol-1)==inCol) && (playerRow==inRow))
	{
		xOff=16;
		yOff=-16;
		animateStep=true;

		//		stepFrame=1;
		//		stepDirection=8;
		//    nextRenderStep1=TickCount()-1; // small step test
		//    nextRenderStep2=TickCount()+10; // small step test
		//    nextRenderStep3=TickCount()+20; // small step test
		//    nextRenderStep4=TickCount()+30; // small step test

	}

	if (((playerRow==inRow) && (playerCol==inCol)) || ((xOff==0) && (yOff==0)))
	{
		canStep=true;
	}

	//canStep=false;

	if ((fabs(playerRow-inRow)>1) || (fabs(playerCol-inCol)>1))
		updateMapView=true;

	/*    0.1.3
		if (gotFirstRefresh)
		  {
		    if ((playerRow>2075) && (playerRow<2310) && (playerCol>1749) && (playerCol<2001))
		      {
		        if (!inTown)
		          {
		            gr_AddText("You have entered the town limits.",false);
		            inTown=true;
		          }
		      }
		    else
		      {
		        if (inTown)
		          {
			          gr_AddText("You have left the town limits.",false);
			          inTown=false;
			        }
		      }
		  }
	*/
	playerRow=inRow;

	playerCol=inCol;

	// gs map change // fi_LoadClientMap(playerRow-13-125,playerCol-13-125);

	xOffset=xOff;

	yOffset=yOff;

	//gr_RenderPlayArea(xOffset, yOffset);

	nextStep=TickCount()+15;//20;
	if (flip==0) flip=1; else flip=0;

	//if (gotFirstRefresh==false)  Why is this here?
	//	gr_RenderPlayerDisplay(true);

//	if (gotFirstRefresh==false) 0.1.3
//	  {
//	    if ((playerRow>2075) && (playerRow<2310) && (playerCol>1749) && (playerCol<2001))
//	      inTown=true;
//	    else
//	      inTown=false;
//	  }
	if (gotFirstRefresh==false)
	{

		if (gotInventoryRefresh==false)
		{
			nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
			fi_WriteToErrorLog("No Inventory at Initialization");
		}

		if (gotSkillsRefresh==false)
		{
			nw_SendAction(kRefreshSkills, kHighPriority, kSendNow,false);
			fi_WriteToErrorLog("No Skills at Initialization");
		}

		if (gotArmsRefresh==false)
		{
			nw_SendAction(kRefreshArm, kHighPriority, kSendNow,true);
			fi_WriteToErrorLog("No Arms at Initialization");
		}

		nw_SendMAC();

		nw_SendCalendarRequest();
		//cm_ReadParty();

		for (i=0;i<16;i++)
			if (strcmp(localParty[i],"")!=0)
				nw_SendAddToParty(i, localParty[i]);

		for (i=0;i<kMaxWindows;i++)
			if (windowVisible[i])
			{
				gr_OpenFloatingWindowID(i);
				gr_RenderWindow(i);
			}
	}

	if (thisIsReconnect)
	{
		gr_AddText("Reconnected.",true,true);
		fi_WriteToErrorLog("Reconnected.");

		for (i=0;i<kMaxPlayers;i++)
		{
			strcpy(playerNames[i],"");
			playerIDs[i]=0;
		}

		thisIsReconnect=false;
	}

	gotFirstRefresh = true;

	lastRefreshTick=TickCount();

	if (currentSkillDisplay==kCooking)
		if (windowVisible[kMakeWindow])
			if (!sk_StandingNearFire())
				gr_CloseFloatingWindow(tbMakeWindow);

	if (currentSkillDisplay==kBlacksmithing)
		if (windowVisible[kMakeWindow])
			if (!sk_StandingNearForge())
				gr_CloseFloatingWindow(tbMakeWindow);

	//gr_RenderPlayArea(xOffset, yOffset);
	//if (fl_FloaterIsOpen(kMainFloater)==false)
	//  fl_OpenFloater(kMainFloater,currentMainWindowsRect);

#endif

}

// ----------------------------------------------------------------- //
void nw_SendPathMessage(int row, int col) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

	if (nw_IsGameOkay()==false)
		return;

	if ((row<0) || (row>=kMapWidth) || (col<0) || (col>=kMapHeight))			// beta debug
		fi_WriteToErrorLog("Out of Range - nw_SendTargetResourceMessage");

	if (gNetGame)
		if (TickCount()>nextAction)
		{
#ifndef _DEBUGGING

			if (playerDead==false)
				if (localWeight>(localStrength*kWeightLimit))
				{
					gr_AddText("You can't move.  Your bag is too heavy.",false,true);
					nextAction=TickCount()+kNextAction;
					return;
				}

#endif
			nextAction=TickCount()+kNextAction;//40;

			NSpClearMessageHeader(&gPathMessage.h);

			gPathMessage.h.what 				= kPathMessage;

			gPathMessage.h.to 					= kNSpHostID;

			gPathMessage.h.messageLen 	= sizeof(gPathMessage);

			gPathMessage.row						=	row;

			gPathMessage.col						=	col;

			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gPathMessage.h, kSendMethod);
		}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetPathMessage(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	PathMessage 			*theMessage = (PathMessage *) inMessage;
	//int               c;


//gr_ShowDebugLocation(103); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	pl_CheckForActionInterrupted(i, true);

	cr_CreatePath(player[i].creatureIndex, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, theMessage->row, theMessage->col);

}

#endif

// ----------------------------------------------------------------- //
void nw_SendTargetResourceMessage(int row, int col, int gatherType) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

	if (nw_IsGameOkay()==false)
		return;

	if ((row<0) || (row>=kMapWidth) || (col<0) || (col>=kMapHeight))			// beta debug
		fi_WriteToErrorLog("Out of Range - nw_SendTargetResourceMessage");

	if (gNetGame)
		if (TickCount()>nextAction)
		{
			targetMode=kNoTarget;
			targetSlot=0;
			targetCount=0;
			nextAction=TickCount()+kNextAction;//40;
			NSpClearMessageHeader(&gPlayerTargetResourceMessage.h);
			gPlayerTargetResourceMessage.h.what 				= kPlayerTargetResourceMessage;
			gPlayerTargetResourceMessage.h.to 					= kNSpHostID;
			gPlayerTargetResourceMessage.h.messageLen 	= sizeof(gPlayerTargetResourceMessage);
			gPlayerTargetResourceMessage.gatherType			=	gatherType;
			gPlayerTargetResourceMessage.row						=	row;
			gPlayerTargetResourceMessage.col						=	col;

			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gPlayerTargetResourceMessage.h, kSendMethod);
		}
		else
			gr_AddText("You must wait to perform another action.",false,false);

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetTargetResourceMessage(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	PlayerTargetResourceMessage 			*theMessage = (PlayerTargetResourceMessage *) inMessage;
	int										            rowDelta;
	int										            colDelta;

//gr_ShowDebugLocation(105); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	pl_CheckForActionInterrupted(i, false);

	player[i].lastCheckIn=TickCount();

	if ((theMessage->gatherType==kFireFieldTarget) || (theMessage->gatherType==kTeleportTarget))
	{
		sk_MageryOnTerrain(i, theMessage->row, theMessage->col, theMessage->gatherType);
	}
	else
	{
		creature[player[i].creatureIndex].attackTarget			=	0;
		creature[player[i].creatureIndex].nextAttackTime		= 0;
		creature[player[i].creatureIndex].nextStrikeTime		=	TickCount();
		creature[player[i].creatureIndex].numberOfStrikes		=	100;
		player[i].gathering								=	20;//30;//30; beta
		player[i].gatherType							= theMessage->gatherType;
		player[i].gatherRow								=	theMessage->row;
		player[i].gatherCol								=	theMessage->col;

		// --------------- face direction ---------------

		rowDelta=theMessage->row-creature[player[i].creatureIndex].row;
		colDelta=theMessage->col-creature[player[i].creatureIndex].col;

		if ((rowDelta<=-1) && (colDelta<=-1)) 				creature[player[i].creatureIndex].facing=0;
		else if ((rowDelta<=-1) && (colDelta==0)) 		creature[player[i].creatureIndex].facing=1;
		else if ((rowDelta<=-1) && (colDelta>=1)) 		creature[player[i].creatureIndex].facing=2;
		else if ((rowDelta==0) && (colDelta<=-1)) 		creature[player[i].creatureIndex].facing=7;
		else if ((rowDelta==0) && (colDelta>=1)) 			creature[player[i].creatureIndex].facing=3;
		else if ((rowDelta>=1) && (colDelta<=-1)) 		creature[player[i].creatureIndex].facing=6;
		else if ((rowDelta>=1) && (colDelta==0)) 			creature[player[i].creatureIndex].facing=5;
		else if ((rowDelta>=1) && (colDelta>=1)) 			creature[player[i].creatureIndex].facing=4;
	}

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
//void nw_SendItemTransferMessage(UInt8 playerTarget, int slot, int count) // client
void nw_SendItemTransferMessage(int playerTarget, int slot, int count) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

	if (nw_IsGameOkay()==false)
		return;

	if ((slot<0) || (slot>=kMaxInventorySize))  // paranoid error checking
		return;

	if (count==0)
		return;

	if (gNetGame)
	{
		targetMode=kNoTarget;
		targetSlot=0;
		targetCount=0;
		nextAction=TickCount()+kNextAction;
		NSpClearMessageHeader(&gItemTransferMessage.h);
		gItemTransferMessage.h.what 				= kItemTransferMessage;
		gItemTransferMessage.h.to 					= kNSpHostID;
		gItemTransferMessage.h.messageLen 	= sizeof(gItemTransferMessage);
		gItemTransferMessage.target					=	(UInt16)playerTarget;
		gItemTransferMessage.slot						=	(UInt8)slot;
		gItemTransferMessage.count					=	(UInt16)count;
		gItemTransferMessage.checksum			  =	playerTarget+slot+count+validate;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gItemTransferMessage.h, kSendMethod);
	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetItemTransferMessage(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	ItemTransferMessage 					*theMessage = (ItemTransferMessage *) inMessage;
	int														previousFrame=0;
	int														added=0;
	UInt32												gold;
	UInt32												goldAdded;
	int														gold1;
	int														gold2;
	TBoolean											canGroup;
	TBoolean											eating;
	int														j;
	TBoolean                      itemsDropped;
	int                      		questComplete=2;
	TBoolean					 	quest=false;
	char                          logStr[kStringLength];
	char                          tempStr[kStringLength];

//gr_ShowDebugLocation(107); // server only
	//if (targetFrame==0)
	//previousFrame=1;

	if ((i<=0) || (i>=kMaxPlayers)) return;

	if ((theMessage->target+theMessage->slot+theMessage->count+validate)!=theMessage->checksum)
	{
		fi_WriteToErrorLog("Bad transfer item message.");
		return;
	}

	if (targetFrame==0)
		previousFrame=1;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	if (theMessage->count==0)
		return;

	if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==0)
		return;

	if (in_CanGroup(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID))
	{
		if (theMessage->count>creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount)
			theMessage->count=creature[player[i].creatureIndex].inventory[theMessage->slot].itemCount;;
	}

	pl_CheckForActionInterrupted(i, false);

	if (theMessage->target==254) // sold item
	{
		canGroup=in_CanGroup(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID);
		gold=sh_ServerSellPriceOf(i,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID);

		if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kGinseng)
			ginsengSpawn        = ginsengSpawn       + theMessage->count;
		else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kLotusFlower)
			lotusFlowerSpawn    = lotusFlowerSpawn   + theMessage->count;
		else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kMandrakeRoot)
			mandrakeRootSpawn   = mandrakeRootSpawn  + theMessage->count;
		else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kGarlic)
			garlicSpawn         = garlicSpawn        + theMessage->count;
		else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kVolcanicAsh)
			volcanicAshSpawn    = volcanicAshSpawn   + theMessage->count;
		//Dave
		else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kMushroom)
			mushroomSpawn    = mushroomSpawn   + theMessage->count;
		
	//	else if (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==(tb_Rnd(kRedEgg,kBlueEgg)))
		//	eggSpawn    = eggSpawn   + theMessage->count;

		//in_RemoveFromInventorySlot(i, theMessage->slot, theMessage->count);
		in_RemoveFromInventorySlot(player[i].creatureIndex, theMessage->slot, theMessage->count);	//018

		nw_SendToClientDropFromInventorySlot(i, kSoldItem, theMessage->slot, theMessage->count, 0);

		if (canGroup)
			gold=gold*theMessage->count;

		goldAdded=in_AddGoldtoInventory(player[i].creatureIndex, gold);

		if (goldAdded>0)
		{
			in_SetGoldTotal(goldAdded, &gold1, &gold2);
			nw_SendGoldAdd(i, kTakeItem, 0, gold1, gold2);
		}

	}
	else if (theMessage->target==255)	// dropped item
	{

		if (sh_GetShopID(i)==kBank)
		{
			added=in_BankItem(i, creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count);

			if (added>0)
			{
				//in_RemoveFromInventorySlot(i, theMessage->slot, added);
				in_RemoveFromInventorySlot(player[i].creatureIndex, theMessage->slot, added);	//018
				nw_SendToClientDropFromInventorySlot(i, kDepositItem, theMessage->slot, added, 0);
			}
			else
				nw_SendDisplayMessage(i, kCantBankHere);
		}
		else
		{
			if (pl_Teleporting(i,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID))  // mod 8/4 teleporting
				added=theMessage->count;
			else if ((creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kLock) || (creature[player[i].creatureIndex].inventory[theMessage->slot].itemID==kTrap))
				added=in_DropItem(player[i].creatureIndex,1,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, sk_GetSkillLevel(player[i].skill[kTinkering].level), false, "");
			else
				added=in_DropItem(player[i].creatureIndex,1,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count, creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col, kTenMinuteDecay, false, "");

			if (added>0)
			{
				if (player[i].category!=0 && player[i].category!=8)  // gm ... log action  ------------------------
				{
					strcpy(logStr,tb_getTimeAndDate("date&time"));
					strcat(logStr, ": ");
					strcat(logStr,player[i].playerName);
					strcat(logStr," dropped ");
					tb_IntegerToString(added,tempStr);
					strcat(logStr,tempStr);
					strcat(logStr," ");
					in_GetItemName(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID,tempStr,-1);
					strcat(logStr,tempStr);
					strcat(logStr," at ");
					tb_IntegerToString(creature[player[i].creatureIndex].row,tempStr);
					strcat(logStr,tempStr);
					strcat(logStr,"/");
					tb_IntegerToString(creature[player[i].creatureIndex].col,tempStr);
					strcat(logStr,tempStr);
					fi_WriteToGMLog(i, logStr);
				} // -----------------------------------------------------------------

				//in_RemoveFromInventorySlot(i, theMessage->slot, added);
				in_RemoveFromInventorySlot(player[i].creatureIndex, theMessage->slot, added);	//018

				nw_SendToClientDropFromInventorySlot(i, kDropItem, theMessage->slot, added, 0);
			}
			else
				nw_SendDisplayMessage(i, kCantDropHere);
		}

	}
	else if ((theMessage->target > 0) && (theMessage->target< kMaxCreatures))
		if (creature[theMessage->target].id!=0)
		{
			if (creature[theMessage->target].playerIndex!=0) // to another player
			{
				if (in_PlayerCanCarryWeight(creature[theMessage->target].playerIndex, in_WeightOf(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count))==false)
				{
					nw_SendDisplayMessage(i, kPersonCantCarryThatMuch);
					return;
				}

				if (player[creature[theMessage->target].playerIndex].dead)
				{
					nw_SendDisplayMessage(i, kCantGiveToGhost);
					return;
				}
			}
			else if (creature[theMessage->target].npcIndex!=0) // to npc ----- check for quest complete
			{
				questComplete=np_QuestComplete(i,creature[theMessage->target].npcIndex,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID,theMessage->count);
				if (questComplete==1) //abort item transfer
				{
					nw_SendDisplayMessage(i, kRewardPackFull);
					return;
				}
			}
			else
			{
				eating=false;

				if (itemList[creature[player[i].creatureIndex].inventory[theMessage->slot].itemID].itemType==kRawMeatType)
					if (creatureInfo[creature[theMessage->target].id].meatEater)
						eating=true;

				if (itemList[creature[player[i].creatureIndex].inventory[theMessage->slot].itemID].itemType==kCookedMeatType)
					if (creatureInfo[creature[theMessage->target].id].meatEater)
						eating=true;

				if (itemList[creature[player[i].creatureIndex].inventory[theMessage->slot].itemID].itemType==kRawVegType)
					if (creatureInfo[creature[theMessage->target].id].vegEater)
						eating=true;

				if (itemList[creature[player[i].creatureIndex].inventory[theMessage->slot].itemID].itemType==kCookedVegType)
					if (creatureInfo[creature[theMessage->target].id].vegEater)
						eating=true;

				if (eating)
				{
					if (creatureInfo[creature[theMessage->target].id].attackSound!=0)
					{
						cr_AddSound(theMessage->target, creatureInfo[creature[theMessage->target].id].attackSound);
						//creature[theMessage->target].soundFX			= creatureInfo[creature[theMessage->target].id].attackSound;
						//creature[theMessage->target].soundFXTime	= TickCount();
					}

					for (j=0;j<theMessage->count;j++)
					{
						creature[theMessage->target].hunger = creature[theMessage->target].hunger + 500;

						if (creature[theMessage->target].hunger>5000)
						{
							creature[theMessage->target].hunger=5000;
							break;
						}

						if (creature[theMessage->target].hunger>1749)
							creature[theMessage->target].hungerfeedback=false;
					}

					in_RemoveFromInventorySlot(player[i].creatureIndex, theMessage->slot, theMessage->count);	//018

					nw_SendToClientDropFromInventorySlot(i, kTransferItem, theMessage->slot, theMessage->count, 0);
					return;
				}
			}

			itemsDropped=false;

			if (player[i].category==0 || player[i].category==8)  // not GM ... GMs can give anything to NPC
				if (questComplete==2)
					if (creature[theMessage->target].playerIndex==0) // to creature
						if (!cr_CreatureWantsItem(theMessage->target,creature[player[i].creatureIndex].inventory[theMessage->slot].itemID))
						{
							added=in_DropItem(player[i].creatureIndex,1, creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count, creature[theMessage->target].row, creature[theMessage->target].col, kThreeMinuteDecay, false, "");
							itemsDropped=true;
						}

			if (questComplete==0)
				added=theMessage->count;
			else if (!itemsDropped)
				added=in_AddtoInventory(theMessage->target, creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, theMessage->count); // server

			if (added>0)
			{
				if (creature[theMessage->target].playerIndex>0)
				{

					if (player[i].category!=0 && player[i].category!=8)  // gm ... log action  ------------------------
					{
						strcpy(logStr,tb_getTimeAndDate("date&time"));
						strcat(logStr, ": ");
						strcat(logStr,player[i].playerName);
						strcat(logStr," gave ");
						tb_IntegerToString(added,tempStr);
						strcat(logStr,tempStr);
						strcat(logStr," ");
						in_GetItemName(creature[player[i].creatureIndex].inventory[theMessage->slot].itemID,tempStr,-1);
						strcat(logStr,tempStr);
						strcat(logStr," to ");
						strcat(logStr,player[creature[theMessage->target].playerIndex].playerName);
						fi_WriteToGMLog(i, logStr);
					} // -----------------------------------------------------------------

					nw_SendInventoryAdd(creature[theMessage->target].playerIndex, creature[player[i].creatureIndex].inventory[theMessage->slot].itemID, added,kTakeItem, i); // server
				}


				in_RemoveFromInventorySlot(player[i].creatureIndex, theMessage->slot, added);	//018

				nw_SendToClientDropFromInventorySlot(i, kTransferItem, theMessage->slot, added, creature[theMessage->target].playerIndex);


			}
			else
				nw_SendDisplayMessage(i, kHisPackFull);

		}

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_SendSortMessageToServer(WindowRef theWindow, int start, int stop) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;

	if (nw_IsGameOkay()==false)
		return;

	if (gNetGame)
	{
		NSpClearMessageHeader(&gSortMessage.h);
		gSortMessage.h.what 				= kSortMessage;
		gSortMessage.h.to 					= kNSpHostID;
		gSortMessage.h.messageLen 			= sizeof(gSortMessage);

		if (theWindow==inventoryWindow)
			gSortMessage.what				= kSortInventory;
		else if (theWindow==bankWindow)
			gSortMessage.what				= kSortBank;

		gSortMessage.start					= (UInt8)start;

		gSortMessage.stop					= (UInt8)stop;

		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gSortMessage.h, kSendMethod);
	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetSortMessage(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	SortMessage 					*theMessage = (SortMessage *) inMessage;
	int								j;
	int								futureEquip[kMaxInventorySize];
	inventoryType					moved;
	//char							debug[kStringLength];
	TBoolean						refresh=false;
	int								location;

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;
	messagesReceived++;
#endif

	if (theMessage->start==theMessage->stop)
		return;

	if (theMessage->start<=0 || theMessage->start>=kMaxInventorySize)
		return;

	if (theMessage->stop<=0 || theMessage->stop>=kMaxInventorySize)
		return;

	if (theMessage->what==kSortInventory)
	{
		moved=creature[player[i].creatureIndex].inventory[theMessage->start];
		futureEquip[theMessage->stop]=in_GetEquipSlotServer(i, theMessage->start);

		if (theMessage->start>theMessage->stop)
		{
			for (j=theMessage->start; j>theMessage->stop; j--)
			{
				futureEquip[j]=in_GetEquipSlotServer(i, j-1);
				creature[player[i].creatureIndex].inventory[j].itemID=creature[player[i].creatureIndex].inventory[j-1].itemID;
				creature[player[i].creatureIndex].inventory[j].itemCount=creature[player[i].creatureIndex].inventory[j-1].itemCount;
			}
		}
		else
		{
			for (j=theMessage->start; j<theMessage->stop; j++)
			{
				futureEquip[j]=in_GetEquipSlotServer(i, j+1);
				creature[player[i].creatureIndex].inventory[j].itemID=creature[player[i].creatureIndex].inventory[j+1].itemID;
				creature[player[i].creatureIndex].inventory[j].itemCount=creature[player[i].creatureIndex].inventory[j+1].itemCount;
			}
		}

		creature[player[i].creatureIndex].inventory[theMessage->stop].itemID=moved.itemID;

		creature[player[i].creatureIndex].inventory[theMessage->stop].itemCount=moved.itemCount;

		if (theMessage->start>theMessage->stop)
		{
			for (j=theMessage->start; j>=theMessage->stop; j--)
				in_EquipServer(i, futureEquip[j], j);
		}
		else
		{
			for (j=theMessage->start; j<=theMessage->stop; j++)
				in_EquipServer(i, futureEquip[j], j);
		}

		for (j=1;j<13;j++)
			if (player[i].arm[j]>0)
			{
				location=j;

				if (location==kLeftFinger)
					location=kRightFinger;

				if (itemList[creature[player[i].creatureIndex].inventory[player[i].arm[j]].itemID].useLocation!=location)	// check for armable in given location
				{
					refresh=true;
					player[i].arm[j]=0;
				}
			}

		if (refresh)
		{
			nw_SendInventoryRefresh(i); //018
			nw_SendArmRefresh(i);
		}

		it_CalculateArmorRating(i);
	}
	else if (theMessage->what==kSortBank)
	{
		moved=player[i].bank[theMessage->start];

		if (theMessage->start>theMessage->stop)
		{
			for (j=theMessage->start; j>theMessage->stop; j--)
			{
				player[i].bank[j].itemID=player[i].bank[j-1].itemID;
				player[i].bank[j].itemCount=player[i].bank[j-1].itemCount;
			}
		}
		else
		{
			for (j=theMessage->start; j<theMessage->stop; j++)
			{
				player[i].bank[j].itemID=player[i].bank[j+1].itemID;
				player[i].bank[j].itemCount=player[i].bank[j+1].itemCount;
			}
		}

		player[i].bank[theMessage->stop].itemID=moved.itemID;

		player[i].bank[theMessage->stop].itemCount=moved.itemCount;

	}
}

#endif

// ----------------------------------------------------------------- //
//void nw_SendTargetMessage(UInt8 playerTarget, int playerAction) // client
void nw_SendTargetMessage(int playerTarget, int playerAction) // client
// ----------------------------------------------------------------- //

{
	OSStatus 							status;


	if (nw_IsGameOkay()==false)
		return;

	if (playerAction>kMaxActions)
	{
		fi_WriteToErrorLog("Target out of range.");
		return;
	}

	if (gNetGame)
		if (TickCount()>nextAction)
		{
			targetMode=kNoTarget;
			targetSlot=0;
			targetCount=0;
			nextAction=TickCount()+kNextAction;//40;
			NSpClearMessageHeader(&gPlayerTargetMessage.h);
			gPlayerTargetMessage.h.what 				= kPlayerTargetMessage;
			gPlayerTargetMessage.h.to 					= kNSpHostID;
			gPlayerTargetMessage.h.messageLen 	= sizeof(gPlayerTargetMessage);
			gPlayerTargetMessage.action					=	playerAction;
			gPlayerTargetMessage.target					=	(UInt16)playerTarget;
			//gPlayerTargetMessage.target					=	playerTarget;

			if (gNetGame!=NULL)
				status = NSpMessage_Send(gNetGame, &gPlayerTargetMessage.h, kSendMethod);//kSendMethod);//kSendMethod);
		}
		else
		{
			gr_AddText("You must wait to perform another action.",false,false);
			tb_FlushMouseEvents();
		}

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetTargetMessage(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	PlayerTargetMessage 					*theMessage = (PlayerTargetMessage *) inMessage;
	int														previousFrame=0;
	char													*myIP;//[64];
	char                          textMessage[kStringLength];

//gr_ShowDebugLocation(109); // server only
//	if (targetFrame==0)
//		previousFrame=1;

	if ((i<=0) || (i>=kMaxPlayers)) return;

	player[i].lastCheckIn=TickCount();

#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

//	if (theMessage->action==kMagicLore)
//		{
//			pl_CheckForActionInterrupted(i, false);
//			sk_MagicLore(i,theMessage->target);
//		}

	if ((theMessage->target<0) || (theMessage->target>=kMaxCreatures))
	{
		return;
	}

	if (creature[theMessage->target].id==0)
	{
		return;
	}

	if (theMessage->action==kTameTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Taming(i,theMessage->target);
		//if ((playerState[i][targetFrame][theMessage->target] > 0) && (playerState[i][targetFrame][theMessage->target] < kMaxCreatures))
		//	sk_Taming(i,playerState[i][targetFrame][theMessage->target]);
		//else if ((playerState[i][previousFrame][theMessage->target] > 0) && (playerState[i][previousFrame][theMessage->target] < kMaxCreatures))
		//	sk_Taming(i,playerState[i][previousFrame][theMessage->target]);
		//else
		//	nw_SendDisplayMessage(i,kTamingFailed);
	}

	else if (theMessage->action==kIDTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendID(i,theMessage->target);
		//if ((playerState[i][targetFrame][theMessage->target] > 0) && (playerState[i][targetFrame][theMessage->target] < kMaxCreatures))
		//	nw_SendID(i,playerState[i][targetFrame][theMessage->target]);
		//else if ((playerState[i][previousFrame][theMessage->target] > 0) && (playerState[i][previousFrame][theMessage->target] < kMaxCreatures))
		//	nw_SendID(i,playerState[i][previousFrame][theMessage->target]);
	}

	else if (theMessage->action==kIPTarget)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanGetIP))
		{
			pl_CheckForActionInterrupted(i, false);

			if (creature[theMessage->target].playerIndex!=0)	// ----------- human
			{
				if (NSpPlayer_GetIPAddress(gNetGame,player[creature[theMessage->target].playerIndex].id,&myIP)==kNMNoError)
				{
					strcpy(textMessage,"IP:  ");
					strcat(textMessage,myIP);
					nw_SendSystemTextToClient(i, textMessage);
					NSpPlayer_FreeAddress( gNetGame, (void **)&myIP );
				}
			}
			else
				nw_SendSystemTextToClient(i, "Invalid Target.");
		}
	}

	else if (theMessage->action==kAKATarget)
	{
		pl_CheckForActionInterrupted(i, false);
		fi_GetAKA(i,theMessage->target);
	}

	else if (theMessage->action==kGainTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_GainPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kNoGainTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_NoGainPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kKillTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_KillPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kResTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_ResPlayer(i,theMessage->target);
	}
	
	else if (theMessage->action==kDressupTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		np_DressupNpc(i,theMessage->target);
	}

	else if (theMessage->action==kResetTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_ResetPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kPitTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_PitPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kJailTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_JailPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kMuteTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_MutePlayer(i,theMessage->target);
	}

	else if (theMessage->action==kNPCITarget)
	{
		pl_CheckForActionInterrupted(i, false);
		np_DumpNPCInfo(i,theMessage->target);
	}

	else if (theMessage->action==kSkillsTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_DumpPlayerSkills(i,theMessage->target);
	}

	else if (theMessage->action==kMsayTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_MakeSay(i,theMessage->target);
	}

	else if (theMessage->action==kMakeEvilTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_MakeEvil(i, theMessage->target);
	}

	else if (theMessage->action==kMakeGoodTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_MakeGood(i, theMessage->target);
	}

	else if (theMessage->action==kClassTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_DumpPlayerClass(i,theMessage->target);
	}

	else if (theMessage->action==kUnmuteTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_UnmutePlayer(i,theMessage->target);
	}

	else if (theMessage->action==kKickTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_KickPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kBanTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_BanPlayer(i,theMessage->target);
	}

	else if (theMessage->action==kBanishTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_BanishCreature(i,theMessage->target);
	}

	else if (theMessage->action==kHomeTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_SendPlayerHome(i,theMessage->target);
	}

	else if (theMessage->action==kActivateTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_ActivatePlayer(i,theMessage->target);
	}
	
	else if (theMessage->action==kDumpInvTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_DumpPlayerInv(i,theMessage->target);
	}
	
	else if (theMessage->action==kDumpBankTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		pl_DumpPlayerBank(i,theMessage->target);
	}

	else if (theMessage->action==kLesserHealTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kGreaterHealTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kRemoveCurseTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kFullHealTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kClumsySpellTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kAgilitySpellTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kWeakenSpellTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kStrengthSpellTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kSimpleMindSpellTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kElevateMindTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kFireballTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kFireblastTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kMagicMissileTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kFireblast3Target)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kFireblast4Target)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kFireblast5Target)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}
	else if (theMessage->action==kLesserDetoxifyTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kGreaterDetoxifyTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kFullDetoxifyTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kExplosionTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kNightVisionTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kInvisibilityTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kOpportunityTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kPoisonTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kReflectiveArmorTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kDrainManaTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kDisorientationTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kMagicReflectionTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kParalyzeTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kLightningBoltTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kResurrectTarget)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_MageryOnCreature(i,theMessage->target,theMessage->action);
	}

	else if (theMessage->action==kHealing)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Healing(i,theMessage->target);
	}

	else if (theMessage->action==kAnatomy)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Anatomy(i,theMessage->target,true);
	}

	else if (theMessage->action==kAttackTarget)
	{
		//pl_CheckForActionInterrupted(i, false); JS Stealth test
		pl_Attack(i,theMessage->target);
	}

	else if (theMessage->action==kStealth)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Thievery(i,theMessage->target);
	}
	
	//else if (theMessage->action==kGMCurse)
	//{
	//	pl_CheckForActionInterrupted(i, false);
	//	pl_CursePlayer(player[creature[i].playerIndex].curse=1);
	//}
}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_GetPlayerAction(int i, NSpMessageHeader *inMessage) // server
// ----------------------------------------------------------------- //

{
	PlayerActionMessage 		*theMessage = (PlayerActionMessage *) inMessage;
	char                        textMessage[kStringLength];
	char                        numMessage[kStringLength];
	int							twentyChance;//Dave
	int							hundredChance;//Dave
	int							wisdomRoll; //Dave
	int							strengthRoll; //Dave
	int							dexRoll; //Dave
	int							sixChance;  //Dave
	char                        rollMessage[kStringLength]; //Dave
	int                         j;
	int                         k;
	TBoolean                    gmFound;
	//int							row;
	//int							col;
	char                        debugStr[kStringLength];
	//TBoolean					canStealth;	// merge 7/28
	TBoolean                    developer;	// merge 7/29
	int							gold1;  // merge 7/28
	int							gold2;  // merge 7/28
	int                 		added;
	int                 		brigandWeaponChance; // merge 7/28
	int                 		theBrigandWeapon; // merge 7/28
	unsigned long               decayTime;
	int							dist;
	
	//char						pcRoll[kStringLength];
	//char						*myIP;//[64];

	//char t[255];


//gr_ShowDebugLocation(110); // server only

	if ((i<=0) || (i>=kMaxPlayers)) return;


#ifdef _SERVERONLY
	bytesReceived=bytesReceived+theMessage->h.messageLen;

	messagesReceived++;

#endif

	player[i].lastCheckIn		= TickCount();

	developer=false;

	if (pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper) || pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter)) // put in carbon 11/27
		developer=true;

	if (theMessage->data<9)
	{
		if (pl_PlayerMove(i, theMessage->data))
		{
			
			
				
		if (pl_AuthorizedTo(i, kRegularPlayer) && player[creature[i].playerIndex].stealth)
			{
				//all declaration top of block, variable unset when block exists, for both
				int total_num_moves = sk_GetSkillLevel(player[creature[i].playerIndex].skill[kStealth].level);
				
				int player_stealth_moves = player[creature[i].playerIndex].stealthMoves;
				
				dist=0;
				
				/* Above unedited ^
				int total_num_moves = sk_GetSkillLevel(player[creature[i].playerIndex].skill[kStealth].level);
				
				int player_stealth_moves = player[creature[i].playerIndex].stealthMoves;
				
				dist=0;
				//Dave class level restrictions
				if (sk_GetSkillLevel(player[i].skill[kStealth].level)<3)			dist=dist+1;
					else if (sk_GetSkillLevel(player[i].skill[kStealth].level)<6)	dist=dist-tb_Rnd(1,3);
					else if (sk_GetSkillLevel(player[i].skill[kStealth].level)<10)	dist=dist-tb_Rnd(2,5);
					else 
						dist=0;	
							
							//Dave^
					//temp		
					if 	(in_ItemEquipedServer(creature[i].playerIndex, kBrownBreastLeather)) || 
						(in_ItemEquipedServer(creature[i].playerIndex, kBrownLeatherLegs)) || 
						(in_ItemEquipedServer(creature[i].playerIndex, kBrownLeatherArms)) || 
						(in_ItemEquipedServer(creature[i].playerIndex, kWoodenShield))
						dist=-1;
					
					placeholder*/
					
				
						
					//Dave armor restriction for rogues
					if 	   (in_ItemEquipedServer(creature[i].playerIndex, (kGrayRobeProtection || kLightBlueRobeProtection || kRedRobeProtection || kBlueRobeProtection ||
							kBrownRobeProtection || kBlackRobeProtection || kGreenRobeProtection || kPurpleRobeProtection || kYellowRobeProtection || kGrayRobeResistance ||
							kLightBlueRobeResistance || kRedRobeResistance || kBlueRobeResistance || kBrownRobeResistance || KBlackRobeResistance || kGreenRobeResistance ||
							kPurpleRobeResistance || kYellowRobeResistance || kRoyalRobe)))
							dist=dist-2;
							
							
							
					
					if 	    (in_ItemEquipedServer(creature[i].playerIndex, (kBrownBreastLeather || kBrownLeatherLegs || kBrownLeatherArms)))
							//(in_ItemEquipedServer(creature[i].playerIndex, kWoodenShield)) ||
							//(in_ItemEquipedServer(creature[i].playerIndex, kMedalShield)) ||
							//(in_ItemEquipedServer(creature[i].playerIndex, kBlackMetalShield))
							dist=dist-1;
					/*
					
					if 	   (in_ItemEquipedServer(creature[i].playerIndex, (kChainGauntlets || kChainArms || kChainLegs || kBreastChain || kChainMailHelmet)))
							dist=dist-2;
					
					if 	   (in_ItemEquipedServer(creature[i].playerIndex, (kClosedPlateHelmet || kSlottedPlateHelmet || kPlateGauntlets || kPlateArms ||
								 kLegsPlate || kBreastPlate || kOpenPlateHelmet || kVikingHelmet)))
							dist=dist-10;
							
					if 	   (in_ItemEquipedServer(creature[i].playerIndex, (kBlackBreastPlate || kBlackLegsPlate || kBlackVikingHelmet || kBlackClosedPlateHelmet || 
								kBlackSlottedPlateHelmet || kBlackOpenPlateHelmet || kBlackPlateGauntlets || kBlackPlateArms || kVikingHelmet)))
							dist=dist-7;
					
					
					*/
					
					
					if 	   ((in_ItemEquipedServer(creature[i].playerIndex, kChainGauntlets)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kChainArms)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kChainLegs)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBreastChain)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kChainMailHelmet)))
							dist=-2;
					
					if 	   ((in_ItemEquipedServer(creature[i].playerIndex, kBlackBreastPlate)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackLegsPlate)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackVikingHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackClosedPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackSlottedPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackOpenPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackPlateGauntlets)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBlackPlateArms)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kVikingHelmet)))
							dist=-7;
					
					if 	   ((in_ItemEquipedServer(creature[i].playerIndex, kClosedPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kSlottedPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kPlateGauntlets)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kPlateArms)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kLegsPlate)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kBreastPlate)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kOpenPlateHelmet)) || 
							(in_ItemEquipedServer(creature[i].playerIndex, kVikingHelmet)))
							dist=-10;
							
					
							
					if (in_ItemEquipedServer(creature[i].playerIndex, kRingOfShadows))					dist=dist+3;
						else if (in_ItemEquipedServer(creature[i].playerIndex, kRingOfStealth))			dist=dist+2;
						 	else if (in_ItemEquipedServer(creature[i].playerIndex, kDiscretionRing))	dist=dist+1;
						
			
						
						//Dave^
						//Dave
						total_num_moves+=dist; //add dist
						if (player_stealth_moves+3 >= total_num_moves)   
					//if (player[creature[i].playerIndex].stealthMoves+1>=sk_GetSkillLevel(player[creature[i].playerIndex].skill[kStealth].level)+dist)  
						{
							int tmp_shog_rnd = tb_Rnd(1,10);
							//char tmp_thing [150];
							//sprintf (tmp_thing,"orig random was: %d and player stealth is: %d and total_num_moves is %d",tmp_shog_rnd,player_stealth_moves+5,total_num_moves);
							//fi_WriteToErrorLog(tmp_thing);

							
							//check if this is the move we _should_ be unstealthed regardless
							if (player_stealth_moves+1>=total_num_moves)
								{
								//we are unstealthed
								player[creature[i].playerIndex].stealth=false;
								player[creature[i].playerIndex].stealthCool=TickCount()+3*60;
								nw_SendDisplayMessage(i, kLeavingStealth);
								player[creature[i].playerIndex].stealthMoves=0;
								}
							//half chance last X moves
							else if (tmp_shog_rnd>=4){ //1,2 = 50% chance
								//we are ok
								player[creature[i].playerIndex].stealthMoves++;
								//fi_WriteToErrorLog("Stealted ok");
							}
							else{
								//sprintf (tmp_thing,"Mean random was: %d",tmp_shog_rnd);
								//fi_WriteToErrorLog(tmp_thing);
								//we are unstealthed
								player[creature[i].playerIndex].stealth=false;
								player[creature[i].playerIndex].stealthCool=TickCount()+3*60;
								nw_SendDisplayMessage(i, kLeavingStealth);
								player[creature[i].playerIndex].stealthMoves=0;
							}
						}
						else
							player[creature[i].playerIndex].stealthMoves++;
					}
			
			
			//Dave^
			creature[player[i].creatureIndex].nextMoveTime	=	TickCount()+30;

			if (player[i].speed>0)
				creature[player[i].creatureIndex].nextMoveTime -= player[i].speed;
			
			if (in_ItemEquipedServer(i, kCharmOfSwiftness))
				creature[player[i].creatureIndex].nextMoveTime -= 5;
//-----------------------ARMOR RESTRICTIONS--------------------------------------------------
	/*
					if (player[creature[i].playerIndex].characterClass!=1)
						if (in_ItemEquipedServer(creature[i].playerIndex, (kBrownBreastLeather || kBrownLeatherLegs)))
							//creature[i].nextMoveTime -= 5;
						creature[i].nextMoveTime += 10;
					
					if (player[creature[i].playerIndex].characterClass!=1)
						if (in_ItemEquipedServer(creature[i].playerIndex, (kChainGauntlets || kChainArms || kChainLegs || kBreastChain || kChainMailHelmet)))
							//creature[i].nextMoveTime	=	currentTime+2*60;
							creature[i].nextMoveTime += 15;
						
					if (player[creature[i].playerIndex].characterClass!=1)
						if (in_ItemEquipedServer(creature[i].playerIndex, (kClosedPlateHelmet || kSlottedPlateHelmet || kPlateGauntlets || kPlateArms ||
								 kLegsPlate || kBreastPlate || kOpenPlateHelmet || kVikingHelmet)))
							//creature[i].nextMoveTime	=	currentTime+3*60;
							creature[i].nextMoveTime += 5*60;
	
	*/					 		
//-------------------------------------------------------------------------------------------		
					
		
		}

		return;
	}

	else if (theMessage->data==kFace1)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace2)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace3)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace4)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace5)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace6)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace7)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}
	else if (theMessage->data==kFace8)
	{
		pl_CheckForActionInterrupted(i, true);
		pl_PlayerFaceDirection(i, theMessage->data);
	}

	else if (theMessage->data==kPickUp)
	{
		pl_CheckForActionInterrupted(i, false);

		if (player[i].dead==false)  // 11/12
		{
			if (in_ContainerLocked(player[i].creatureIndex))
			{
				nw_SendDisplayMessage(i,kContainerLocked);
				return;
			}

			if (in_ContainerOwner(i))
				in_PickUp(i);
		}
	}

	else if (theMessage->data==kInsult)
	{
		pl_Insult(i);
		//if (sh_StandingInInn(i))  // version 0.1.1 - Instant log out in inn
		//  tb_Idle();
	}

	//--------------------------------------
	else if (theMessage->data==kLogOff)
	{
		gracefulLogoffs++;
		player[i].online=false;

		if (sh_StandingInInn(i))  // version 0.1.1 - Instant log out in inn
		{
			cr_ClearAttackTarget(player[i].creatureIndex);
			nw_SendClearName(i);
			fi_Logout(i);
			creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
			cr_ClearCreatureRecord(player[i].creatureIndex);
			pl_ClearPlayerRecord(i);
			//nw_SendClearName(i); // carbon moved
		}
		else
			nw_SendClearName(i);

#ifdef _SERVERONLY
		gr_DrawServerNameAt(i);

#endif
	}

//	else if (theMessage->data==kReturn)
//  	{
//  	  if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5)) // merge 7/27
//        if (player[i].returnRow>0)
//          if (player[i].returnCol>0)
//          	{
//          		row=creature[player[i].creatureIndex].row;
//          		col=creature[player[i].creatureIndex].col;
//            	pl_PlayerMoveTo(i,player[i].returnRow,player[i].returnCol);
//		      	  player[i].returnRow=row;
//		      	  player[i].returnCol=col;
//           	}
//  	}

	else if (theMessage->data==kPosition)
	{
		//if ((player[i].category==1) || (player[i].category==2) || (player[i].category==4) || (player[i].category==5))
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanGetPosition)) // put in carbon 11/27
		{
			strcpy(textMessage,"Pos:  ");
			tb_NumToString(creature[player[i].creatureIndex].row,numMessage);
			strcat(textMessage,numMessage);
			strcat(textMessage," ");
			tb_NumToString(creature[player[i].creatureIndex].col,numMessage);
			strcat(textMessage,numMessage);
			nw_SendSystemTextToClient(i, textMessage); 
		}
	}

	else if (theMessage->data==kSlow)
	{
		player[i].slow=true;
		//nw_SendSystemTextToClient(i, "Slow mode on.");
	}

	else if (theMessage->data==kFast)
	{
		player[i].slow=false;
		//nw_SendSystemTextToClient(i, "Fast mode on.");
	}

	else if (theMessage->data==kStatus)
	{
		if (player[i].category!=0 && player[i].category!=8) // merge 7/27
			//if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false) // put in carbon 11/27
		{
			strcpy(numMessage,"Server build:  ");
			strcat(numMessage,buildString);
			nw_SendSystemTextToClient(i, numMessage);
			nw_SendSystemTextToClient(i, startDateAndTime);
			//if (fi_IsCharacterCreatorUp())
			//  nw_SendSystemTextToClient(i, "CC is on.");
			//else
			//  nw_SendSystemTextToClient(i, "CC is off.");
		}
	}

	else if (theMessage->data==kGMInfo)
	{
		k=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

		if (k!=0)
			//if (player[i].category!=0) // merge 7/27
			if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer)) // put in carbon 11/27
			{
				if (containerGM[k])
					nw_SendSystemTextToClient(i, "GM Spawned");
				else
					nw_SendSystemTextToClient(i, "Not GM Spawned");

				strcpy(textMessage,"Owner:  ");

				if (strcmp(containerGift[k],"")==0)
					strcat(textMessage,"All");
				else
					strcat(textMessage,containerGift[k]);

				nw_SendSystemTextToClient(i, textMessage);

				strcpy(textMessage,"Lock:  ");

				tb_NumToString(containerLock[k],numMessage);

				strcat(textMessage,numMessage);

				nw_SendSystemTextToClient(i, textMessage);

				strcpy(textMessage,"Trap:  ");

				tb_NumToString(containerTrap[k],numMessage);

				strcat(textMessage,numMessage);

				nw_SendSystemTextToClient(i, textMessage);

				strcpy(textMessage,"Magic Lock:  ");

				tb_NumToString(containerMagicLock[k],numMessage);

				strcat(textMessage,numMessage);

				nw_SendSystemTextToClient(i, textMessage);

				strcpy(textMessage,"Magic Trap:  ");

				tb_NumToString(containerMagicTrap[k],numMessage);

				strcat(textMessage,numMessage);

				nw_SendSystemTextToClient(i, textMessage);

				if (containerDecayTime[k]==0)
				{
					nw_SendSystemTextToClient(i, "Decay:  Never");
				}
				else
				{
					decayTime=(containerDecayTime[k]-TickCount())/3600;
					strcpy(textMessage,"Decay:  ");
					tb_NumToString(decayTime,numMessage);
					strcat(textMessage,numMessage);
					nw_SendSystemTextToClient(i, textMessage);
				}
			}

		k=tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

		if (k!=0)
		{
			if (pl_AuthorizedTo(player[i].creatureIndex,kCanMakeContainer))
				if (tracksDecayTime[k]==0)
				{
					nw_SendSystemTextToClient(i, "Decay:  Never");
				}
				else
				{
					decayTime=(tracksDecayTime[k]-TickCount())/3600;
					strcpy(textMessage,"Decay:  ");
					tb_NumToString(decayTime,numMessage);
					strcat(textMessage,numMessage);
					nw_SendSystemTextToClient(i, textMessage);
				}

			strcpy(textMessage,"Owner:  ");

			if (strcmp(tracksGift[k],"")==0)
				strcat(textMessage,"All");
			else
				strcat(textMessage,tracksGift[k]);

			nw_SendSystemTextToClient(i, textMessage);

			if (strcmp(tracksNote[k], "")==0)
				nw_SendSystemTextToClient(i, "No note");
			else
			{
				strcpy(textMessage, "Note: ");
				strcat(textMessage, tracksNote[k]);
				nw_SendSystemTextToClient(i, textMessage);
			}
		}
	}

	else if (theMessage->data==kUsers)
	{
		if (developer)  // merge 7/29
		{
			for (j=1;j<kMaxPlayers;j++)
				if (player[j].id!=0)
					nw_SendSystemTextToClient(i, player[j].playerName);

			strcpy(textMessage,"Users:  ");

			tb_IntegerToString(numberOfPlayersOnline,numMessage);

			strcat(textMessage,numMessage);

			nw_SendSystemTextToClient(i, textMessage);
		}
	}

	else if (theMessage->data==kHelp)
	{
		strcpy(textMessage,"Help Request: ");
		strcat(textMessage,player[i].playerName);
		gmFound=false;

		for (j=1;j<kMaxPlayers;j++)
		{
			if (pl_AuthorizedTo(player[j].creatureIndex,kCanSeeHelpRequestsWithFeedback)) // put in carbon 11/27
				//if ((((player[j].category==1) || (player[j].category==2)) && (player[j].stealth==false)) || ((player[j].helper) && (player[i].demo)))  // merge 7/27
			{
				//for (k=0;k<strlen(textMessage);k++)
				//	if (textMessage[k]=='*')
				//		textMessage[k]=' ';
				nw_SendSystemTextToClient(j, textMessage);
				gmFound=true;
			}

			// if ((player[j].category==4) || (player[j].category==5) || (player[j].stealth))  // merge 7/27
			else if (pl_AuthorizedTo(player[j].creatureIndex,kCanSeeHelpRequestsWithNoFeedback))  // merge 7/27
			{
				//for (k=0;k<strlen(textMessage);k++)
				//	if (textMessage[k]=='*')
				//		textMessage[k]=' ';
				nw_SendSystemTextToClient(j, textMessage);
			}
		}

		if (gmFound)
			nw_SendDisplayMessage(i,kGMOnline);
		else
			nw_SendDisplayMessage(i,kNoGMOnline);
	}

	else if (theMessage->data==kMeetNext)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer)==false)  // merge 7/29
		{
			player[i].meet++;

			if (player[i].meet>=kMaxPlayers)
				player[i].meet=1;

			for (j=player[i].meet;j<kMaxPlayers;j++)
				if (strcmp(player[j].playerName,"")!=0 && i!=j)
					if (!(pl_AuthorizedTo(player[j].creatureIndex, kIsDeveloper) && player[j].stealth))
					{
						player[i].meet=j;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+3),(creature[player[j].creatureIndex].col+3));
						return;
					}

			for (j=1;j<player[i].meet;j++)
				if (strcmp(player[j].playerName,"")!=0 && i!=j)
					if (!(pl_AuthorizedTo(player[j].creatureIndex, kIsDeveloper) && player[j].stealth))
					{
						player[i].meet=j;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+3),(creature[player[j].creatureIndex].col+3));
						return;
					}
		}
	}

	else if (theMessage->data==kMeetLast)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex, kRegularPlayer)==false)  // merge 7/29
		{
			player[i].meet--;

			if (player[i].meet==0)
				player[i].meet=kMaxPlayers;

			for (j=player[i].meet;j>0;j--)
				if (strcmp(player[j].playerName,"")!=0 && i!=j)
					if (!(pl_AuthorizedTo(player[j].creatureIndex, kIsDeveloper) && player[j].stealth))
					{
						player[i].meet=j;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+3),(creature[player[j].creatureIndex].col+3));
						return;
					}

			for (j=kMaxPlayers;j>player[i].meet;j--)
				if (strcmp(player[j].playerName,"")!=0 && i!=j)
					if (!(pl_AuthorizedTo(player[j].creatureIndex, kIsDeveloper) && player[j].stealth))
					{
						player[i].meet=j;
						pl_PlayerMoveTo(i,(creature[player[j].creatureIndex].row+3),(creature[player[j].creatureIndex].col+3));
						return;
					}
		}
	}

	else if (theMessage->data==kGetFX)
	{
		if (developer)  // merge 7/29
		{
			strcpy(textMessage,"FX:  ");
			tb_IntegerToString(fxMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col],numMessage);
			strcat(textMessage,numMessage);
			nw_SendSystemTextToClient(i, textMessage);
		}
	}

	else if (theMessage->data==kGMInit)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			pl_ResPlayer(i,player[i].creatureIndex);

			added=in_AddGoldtoInventory(player[i].creatureIndex, tb_Rnd(1,100));

			if (added>0)
			{
				in_SetGoldTotal(added, &gold1, &gold2);
				nw_SendGoldAdd(i, kTakeItem, 0, gold1, gold2);
			}

			if (player[i].characterClass==3)
			{
				added=in_AddtoInventory(player[i].creatureIndex, kLongSword, 127);

				if (added>0)	nw_SendInventoryAdd(i, kLongSword, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kLegsPlate, 127);

				if (added>0)	nw_SendInventoryAdd(i, kLegsPlate, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kBreastPlate, 127);

				if (added>0)	nw_SendInventoryAdd(i, kBreastPlate, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kClosedPlateHelmet, 127);

				if (added>0)	nw_SendInventoryAdd(i, kClosedPlateHelmet, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kMedalShield, 127);

				if (added>0)	nw_SendInventoryAdd(i, kMedalShield, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kBlackBoots, 1);

				if (added>0)	nw_SendInventoryAdd(i, kBlackBoots, added, kTakeItem,0);
			}

			if (player[i].characterClass==5)
			{
				theBrigandWeapon=kBroadSword;
				brigandWeaponChance=tb_Rnd(1,6); // merge 7/28

				if (brigandWeaponChance==1) theBrigandWeapon=kBroadSword; // merge 7/28

				if (brigandWeaponChance==2) theBrigandWeapon=kKatana; // merge 7/28

				if (brigandWeaponChance==3) theBrigandWeapon=kLongSword; // merge 7/28

				if (brigandWeaponChance==4) theBrigandWeapon=kScimitar; // merge 7/28

				if (brigandWeaponChance==5) theBrigandWeapon=kKatanaPoison; // merge 7/28

				if (brigandWeaponChance==6) theBrigandWeapon=kScimitarPoison; // merge 7/28

				added=in_AddtoInventory(player[i].creatureIndex, theBrigandWeapon, 127);

				if (added>0)	nw_SendInventoryAdd(i, theBrigandWeapon, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kLegsPlate, 127);

				if (added>0)	nw_SendInventoryAdd(i, kLegsPlate, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kBrownBreastLeather, 127);

				if (added>0)	nw_SendInventoryAdd(i, kBrownBreastLeather, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kOpenPlateHelmet, 127);

				if (added>0)	nw_SendInventoryAdd(i, kOpenPlateHelmet, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kBrownBoots, 1);

				if (added>0)	nw_SendInventoryAdd(i, kBrownBoots, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kWoodenShield, 127);

				if (added>0)	nw_SendInventoryAdd(i, kWoodenShield, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGreaterHealPotion, tb_Rnd(2,6));

				if (added>0)	nw_SendInventoryAdd(i, kGreaterHealPotion, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGreaterDetoxifyPotion, tb_Rnd(2,6));

				if (added>0)	nw_SendInventoryAdd(i, kGreaterDetoxifyPotion, added, kTakeItem,0);
			}

			if (player[i].characterClass==6)
			{
				added=in_AddtoInventory(player[i].creatureIndex, kRedRobeProtection, 127);

				if (added>0)	nw_SendInventoryAdd(i, kRedRobeProtection, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kRedWizardHat, 1);

				if (added>0)	nw_SendInventoryAdd(i, kRedWizardHat, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kDragonTooth, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kDragonTooth, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGinseng, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kGinseng, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kLotusFlower, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kLotusFlower, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kMandrakeRoot, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kMandrakeRoot, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kAmber, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kAmber, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kGarlic, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kGarlic, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kVolcanicAsh, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kVolcanicAsh, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kPoisonGland, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kPoisonGland, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kCoal, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kCoal, added, kTakeItem,0);

				added=in_AddtoInventory(player[i].creatureIndex, kElectricEel, tb_Rnd(20,40));

				if (added>0)	nw_SendInventoryAdd(i, kElectricEel, added, kTakeItem,0);
			}
		}
	}

	else if (theMessage->data==kKillFX)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			fxMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
			k=containerMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (k>0)
				in_ClearContainer(k);

			k=tracksMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col];

			if (k>0)
				in_ClearTracks(k);
		}
	}

	else if (theMessage->data==kSharingOn)
	{
		player[i].sharing=true;
	}

	/*else if (theMessage->data==kShutdown) // beta test take out
	  {
	  	Done=true;
	  }*/

	else if (theMessage->data==kSharingOff)
	{
		player[i].sharing=false;
	}

	else if (theMessage->data==kStealthOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		
		{
			player[i].stealth=true;
			nw_SendSystemTextToClient(i, "Stealth On");
		}
	}

	else if (theMessage->data==kStealthOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			player[i].stealth=false;
			nw_SendSystemTextToClient(i, "Stealth Off");
		}
	}

	else if (theMessage->data==kNoTrackOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			player[i].noTrack=true;
			nw_SendSystemTextToClient(i, "NoTrack On");
		}
	}

	else if (theMessage->data==kNoTrackOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			player[i].noTrack=false;
			nw_SendSystemTextToClient(i, "NoTrack Off");
		}
	}

	else if (theMessage->data==kAutoLockOn)
	{

		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].autoLock=true;
			nw_SendSystemTextToClient(i, "AutoLock On");
		}
	}

	else if (theMessage->data==kAutoLockOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].autoLock=false;
			nw_SendSystemTextToClient(i, "AutoLock Off");
		}
	}

	else if (theMessage->data==kGodModeOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].godMode=true;
			nw_SendSystemTextToClient(i, "God Mode On");
		}
	}

	else if (theMessage->data==kGodModeOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].godMode=false;
			nw_SendSystemTextToClient(i, "God Mode Off");
		}
	}
	
	else if (theMessage->data==kNoDeathOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].noDeath=true;
			nw_SendSystemTextToClient(i, "No Death On");
		}
	}

	else if (theMessage->data==kNoDeathOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			creature[player[i].creatureIndex].noDeath=false;
			nw_SendSystemTextToClient(i, "No Death Off");
		}
	}

	else if (theMessage->data==kGMHome)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			player[i].returnRow=creature[player[i].creatureIndex].row;
			player[i].returnCol=creature[player[i].creatureIndex].col;
			pl_PlayerMoveTo(i,609,87);

			if (player[i].category!=4)  // merge 7/27
				if (player[i].category!=5)  // merge 7/27
					if (player[i].stealth==false)
						if (creature[player[i].creatureIndex].hiding==false)
							fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kSparkle, (5*60), 0);
		}
	}
	else if (theMessage->data==kGoodChar)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))    // merge 7/27
		{
			player[i].category=4;
			nw_SendTextToClient(i, "Alignment set to good.", 0);
		}
	}
	else if (theMessage->data==kEvilChar)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))   // merge 7/27
		{
			player[i].category=5;
			nw_SendTextToClient(i, "Alignment set to evil.", 0);
		}
	}
	/**///Dave 11-25-2012 back to dev
	else if (theMessage->data==kDev)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter))
		// (player[i].playerName=="Host" || "Charnath" || "Liaraen" || "Jinker")    // merge 7/27
		{
			player[i].category=1;
			nw_SendTextToClient(i, "Set to Developer.", 0);
		}
	}
	
	else if (theMessage->data==kNews)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			nw_SendTextToClient(i, "No news.", 0);
		}
	}
	else if (theMessage->data==kMsgOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetLogonMessage))  // merge 7/27
		{
			showLogonMessage=true;
			nw_SendTextToClient(i, "Message on.", 0);
		}
	}
	else if (theMessage->data==kMsgOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetLogonMessage))  // merge 7/27
		{
			showLogonMessage=false;
			nw_SendTextToClient(i, "Message off.", 0);
		}
	}
	else if (theMessage->data==kMsgShow)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetLogonMessage))  // merge 7/27
			fi_SendLoginMessage(i);

		//if (pl_AuthorizedTo(player[i].creatureIndex,kCanSetLogonMessage))  // merge 7/27
		//  {
		//    if ((strcmp(logonMessage1,"")==0) && (strcmp(logonMessage2,"")==0) && (strcmp(logonMessage3,"")==0))
		//      {
		//		    nw_SendSystemTextToClient(i,"No log on message.");
		//      }
		//    else
		//      {
		//		    if (strcmp(logonMessage1,"")!=0)
		//		      nw_SendSystemTextToClient(i,logonMessage1);
		//		    if (strcmp(logonMessage2,"")!=0)
		//		      nw_SendSystemTextToClient(i,logonMessage2);
		//		    if (strcmp(logonMessage3,"")!=0)
		//		      nw_SendSystemTextToClient(i,logonMessage3);
		//      }
		//  }
	}
	else if (theMessage->data==kBeep)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		{
			cr_AddSound(player[i].creatureIndex, kBeepSnd);
		}
	}
	else if (theMessage->data==kThunder)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kThunderSnd);
		}
	}
	else if (theMessage->data==kHorn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kMirithHornSnd);
		}
	}
	else if (theMessage->data==kDragon)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kDragonFlightSnd);
		}
	}
	else if (theMessage->data==kFlute)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kFluteSnd);
		}
	}
	else if (theMessage->data==kScream)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kScreamSnd);
		}
	}
	
	//Dave
	else if (theMessage->data==kLichLaugh)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kLichLaughSnd);
		}
	}
	
	else if (theMessage->data==kSplash)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kSharkAttackSnd);
		}
	}
	
	else if (theMessage->data==kJaws)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kSharkSnd);
		}
	}
	//Dave^
	else if (theMessage->data==kGasp)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kGaspSnd);
		}
	}
	
	else if (theMessage->data==kFemaleGasp)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kFemaleGaspSnd);
		}
	}
	/*
	else if (theMessage->data==kAFK)
	{
		player[i].afk=true;
		nw_SendSystemTextToClient(i, "Away from keyboard.");
	}
	*/
	//Dave^
	else if (theMessage->data==kFanfare)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kFanfareSnd);
		}
	}
	else if (theMessage->data==kReload)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper) || pl_AuthorizedTo(player[i].creatureIndex,kIsSeniorGM))  // merge 7/27
		{
			cm_ReloadSettings();
		}
	}
	else if (theMessage->data==kBoom)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsAnyGM) || pl_AuthorizedTo(player[i].creatureIndex,kIsQuestCharacter))  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			cr_AddSound(player[i].creatureIndex, kExplosionSnd);
			fx_CreatureServerFX(player[i].creatureIndex, player[i].creatureIndex, kLightExplosion, (2*60), 0);
		}
	}
	else if (theMessage->data==kCCOn)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanControlCharacterCreator))  // merge 7/27
		{
			if (fi_TurnCharacterCreatorOn()==0)
				nw_SendTextToClient(i, "Character Creator is now on.", 0);
			else
				nw_SendTextToClient(i, "CCOn failed.", 0);
		}
	}
	else if (theMessage->data==kCCOff)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kCanControlCharacterCreator))  // merge 7/27
		{
			if (fi_TurnCharacterCreatorOff()==0)
				nw_SendTextToClient(i, "Character Creator is now off.", 0);
			else
				nw_SendTextToClient(i, "CCOff failed.  Please try again.", 0);
		}
	}
	
	
	//Dave Dice rolls
	
	else if (theMessage->data==k1d6)
	{
			//make the roll
			sixChance=tb_Rnd(1,6);
			//copy string...
			strcpy(rollMessage,"*Rolls a 1d6 for a roll of ");
			//add the roll
			tb_NumToString(sixChance,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			//send the text
			//strcat(creature[player[i].creatureIndex].talk,textMessage);
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	  = chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				nw_SendDisplayMessage(i,kYouRolled);
				}
	}
	
	
	else if (theMessage->data==k1d20)
	{
			//make the roll
			twentyChance=tb_Rnd(1,20);
			//copy string...
			strcpy(rollMessage,"*Rolls a 1d20 for a roll of ");
			//add the roll
			tb_NumToString(twentyChance,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			//send the text
			//strcat(creature[player[i].creatureIndex].talk,textMessage);
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	  = chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				nw_SendDisplayMessage(i,kYouRolled);
				}
	}

	else if (theMessage->data==k1d100)
	{
			//make the roll
			hundredChance=tb_Rnd(1,100);
			//copy string...
			strcpy(rollMessage,"*Rolls a 1d100 for a roll of ");
			//add the roll
			tb_NumToString(hundredChance,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			//send the text
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	  = chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				
				}
			
	}
	
	else if (theMessage->data==kWisdomRoll)
	{
			
			if ((player[i].characterClass==1) || (player[i].characterClass==2) ||(player[i].characterClass==6))
				{
					
				//make the roll
				if 		(sk_GetCharacterLevel(i)<=4)  	{ wisdomRoll=tb_Rnd(1,20)+2;	}
		  		else if (sk_GetCharacterLevel(i)<=6)	{ wisdomRoll=tb_Rnd(1,20)+4;	}
				else if (sk_GetCharacterLevel(i)<=8) 	{ wisdomRoll=tb_Rnd(1,20)+6; 	}
				else if (sk_GetCharacterLevel(i)<=10) 	{ wisdomRoll=tb_Rnd(1,20)+8;	}
				else if (sk_GetCharacterLevel(i)>=10)	{ wisdomRoll=tb_Rnd(1,20)+10;	}
				
				}
				
				else
					{
					wisdomRoll=tb_Rnd(1,20)-5;
					}
				
				if (wisdomRoll<=1) 	{ wisdomRoll=1;		}	
				if (wisdomRoll>=30) { wisdomRoll=30;	}	
				
			//copy string...
			strcpy(rollMessage,"*Rolls intelligence check of ");
			//add the roll
			tb_NumToString(wisdomRoll,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			
			//send the text
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	= chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				nw_SendDisplayMessage(i,kYouRolled);
				}
	}
	
	
		else if (theMessage->data==kStrengthRoll)
			{
			if ((player[i].characterClass==3) || (player[i].characterClass==4) ||(player[i].characterClass==5))
				
				{
				//make the roll
				if (sk_GetCharacterLevel(i)<=4)			{ strengthRoll=tb_Rnd(1,20)+2;	}
		  		else if (sk_GetCharacterLevel(i)<=6)	{ strengthRoll=tb_Rnd(1,20)+4;	}
				else if (sk_GetCharacterLevel(i)<=8) 	{ strengthRoll=tb_Rnd(1,20)+6;	}
				else if (sk_GetCharacterLevel(i)<=10) 	{ strengthRoll=tb_Rnd(1,20)+8;	}
				else if (sk_GetCharacterLevel(i)>=10)  	{ strengthRoll=tb_Rnd(1,20)+10;	}
				}
				else 
					{
					strengthRoll=tb_Rnd(1,20)-5;
					}
					
					if (player[i].characterClass==3)	{ strengthRoll=strengthRoll+5;	}
				 	if (strengthRoll<=1) 				{ strengthRoll=1;	}	
					if (strengthRoll>=30) 				{ strengthRoll=30;	}
						
			//copy string...
			strcpy(rollMessage,"*Rolls strength check of ");
			//add the roll
			tb_NumToString(strengthRoll,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			//send the text
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	= chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				nw_SendDisplayMessage(i,kYouRolled);
				}
	}
			
			else if (theMessage->data==kDexRoll)
			{
				//make the roll
				if ((player[i].characterClass==3) || (player[i].characterClass==4) || (player[i].characterClass==5))
					{
				if (sk_GetCharacterLevel(i)<=4)  		{ dexRoll=tb_Rnd(1,20)+2;	}
		  		else if (sk_GetCharacterLevel(i)<=6)  	{ dexRoll=tb_Rnd(1,20)+4;	}
				else if (sk_GetCharacterLevel(i)<=8) 	{ dexRoll=tb_Rnd(1,20)+6;	}
				else if (sk_GetCharacterLevel(i)<=10) 	{ dexRoll=tb_Rnd(1,20)+8;	}
				else if (sk_GetCharacterLevel(i)>=10)  	{ dexRoll=tb_Rnd(1,20)+10;	}
				}
				else
						{
						dexRoll=tb_Rnd(1,20)-5;
						}
				
			if (player[i].characterClass==5)
				{
					dexRoll=dexRoll+3;
				}
			//if ((player[i].characterClass==1) || (player[i].characterClass==2) || (player[i].characterClass==6))	
			//	{
			//		dexRoll=dexRoll-5;
			//	}
				
			if (dexRoll<=1) 	{ dexRoll=1;	}	
			if (dexRoll>=30) 	{ dexRoll=30;	}	
			
			//copy string...
			strcpy(rollMessage,"*Rolls dexterity check of ");
			//add the roll
			tb_NumToString(dexRoll,numMessage);
			strcat(rollMessage,numMessage);
			strcat(rollMessage,"*");
			//send the text
			
			if (player[i].curse&kBabbleCurse)
				strcpy(rollMessage,"Babble babble babble.");

			if (creature[player[i].creatureIndex].mute)
				strcpy(rollMessage, "");
			
			creature[player[i].creatureIndex].talkWhen  = TickCount();

			creature[player[i].creatureIndex].talkTime  = TickCount();

			creature[player[i].creatureIndex].talkID	= chatMessageID;

			chatMessageID++;

			strcpy(creature[player[i].creatureIndex].talk,rollMessage);
			
			nw_SendTextToClient(i, rollMessage, 0);
			
			creature[player[i].creatureIndex].hiding=false;
			
			if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer) || pl_AuthorizedTo(player[i].creatureIndex,kIsDeveloper))
				{
				cr_AddSound(player[i].creatureIndex, kDiceRollSnd);
				nw_SendDisplayMessage(i,kYouRolled);
				}
	}
	/*
	else if (theMessage->data==kGMCurse)
	{
		if (pl_AuthorizedTo(player[i].creatureIndex,kRegularPlayer)==false)  // merge 7/27
		if (pl_AuthorizedTo(player[i].creatureIndex,kIsApprenticeGM)==false)
		{
			//if (creatureInfo[creature[i].id].curse>0) // curse
			//	if (creature[i].playerIndex!=0)
					pl_CursePlayer(player[i].creatureIndex);
		}
	}
	*/
	//--------------------------------------

	else if (theMessage->data==kCastParalyzeFieldSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastParalyzeFieldSpell);

	else if (theMessage->data==kCastPoisonFieldSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastPoisonFieldSpell);

	else if (theMessage->data==kCastRevealSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastRevealSpell);

	else if (theMessage->data==kCastProvocationSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastProvocationSpell);

	else if (theMessage->data==kCastCalmSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastCalmSpell);

	else if (theMessage->data==kCastCreateFoodSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastCreateFoodSpell);

	else if (theMessage->data==kCastTurnUndeadSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastTurnUndeadSpell);

	else if (theMessage->data==kCastSummonCreatureSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastSummonCreatureSpell);
	
	//Dave 
	else if (theMessage->data==kCastGreaterSummonCreatureSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastGreaterSummonCreatureSpell);


	else if (theMessage->data==kCastSummonPetSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastSummonPetSpell);

	else if (theMessage->data==kCastHealPetSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastHealPetSpell);

	else if (theMessage->data==kCastMagicTrapSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastMagicTrapSpell);

	else if (theMessage->data==kCastMagicDisarmSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastMagicDisarmSpell);

	else if (theMessage->data==kCastMassHealSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastMassHealSpell);

	else if (theMessage->data==kCastDisorientFieldSpell)
		sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastDisorientFieldSpell);
	
	else if (theMessage->data==kCastShapeshiftLesserSpell)
			sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastShapeshiftLesserSpell);
	
	else if (theMessage->data==kCastShapeshiftSpell)
			sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastShapeshiftSpell);
	
	else if (theMessage->data==kCastShapeshiftGreaterSpell)
			sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastShapeshiftGreaterSpell);
	
	
	else if (theMessage->data==kCastShapeshiftAllSpell)
			sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastShapeshiftAllSpell);
	
	else if (theMessage->data==kCastShapeshiftDragonSpell)
			sk_MageryOnTerrain(i,creature[player[i].creatureIndex].row, creature[player[i].creatureIndex].col,kCastShapeshiftDragonSpell);
	
	else if (theMessage->data==kRefreshInventory)
		nw_SendInventoryRefresh(i);

	else if (theMessage->data==kRefreshStats)
	{
		it_CalculateArmorRating(i);
		(i);
		player[i].sendStatsRefresh=true;
		player[i].sendPetsRefresh=true;
		strcpy(debugStr,"Bad Refresh:  ");
		strcat(debugStr,player[i].playerName);
		fi_WriteToErrorLog(debugStr);	// beta debug
	}

	else if (theMessage->data==kRefreshSkills)
		nw_SendSkillsRefresh(i);

	else if (theMessage->data==kRefreshArm)
		nw_SendArmRefresh(i);

	else if (theMessage->data==kHide)
		sk_Hiding(i);

	else if (theMessage->data==kEnterStealth)
		sk_Stealth(i);

	else if (theMessage->data==kDetectHidden)
	{
		pl_CheckForActionInterrupted(i, false);
		player[i].revealHidden=player[i].skill[kDetectingHidden].level+10;
	}

	else if (theMessage->data==kEat)
		pl_PlayerEat(i);

	else if (theMessage->data==kLightBlueHairDye)
		pl_HairDying(i, kLightBlue);

	else if (theMessage->data==kRedHairDye)
		pl_HairDying(i, kRed);

	else if (theMessage->data==kBrownHairDye)
		pl_HairDying(i, kBrown);

	else if (theMessage->data==kBlueHairDye)
		pl_HairDying(i, kBlue);

	else if (theMessage->data==kBlackHairDye)
		pl_HairDying(i, kBlack);

	else if (theMessage->data==kGreenHairDye)
		pl_HairDying(i, kGreen);

	else if (theMessage->data==kPurpleHairDye)
		pl_HairDying(i, kPurple);

	else if (theMessage->data==kYellowHairDye)
		pl_HairDying(i, kYellow);
	//Dave
	else if (theMessage->data==kGrayHairDye)
		pl_HairDying(i, kGray);
	
	//else if (theMessage->data==kOrangeHairDye)
		//pl_HairDying(i, kOrange1);
	//Dave^
	else if (theMessage->data==kDrinkLesserHealPotion)
		pl_PlayerDrinkPotion(i,kLesserHealPotion);

	else if (theMessage->data==kDrinkGreaterHealPotion)
		pl_PlayerDrinkPotion(i,kGreaterHealPotion);


	else if (theMessage->data==kDrinkFullHealPotion)
		pl_PlayerDrinkPotion(i,kFullHealPotion);


	else if (theMessage->data==kDrinkManaPotion)
		pl_PlayerDrinkPotion(i,kManaPotion);

	else if (theMessage->data==kDrinkLesserDetoxifyPotion)
		pl_PlayerDrinkPotion(i,kLesserDetoxifyPotion);

	else if (theMessage->data==kDrinkGreaterDetoxifyPotion)
		pl_PlayerDrinkPotion(i,kGreaterDetoxifyPotion);

	else if (theMessage->data==kDrinkFullDetoxifyPotion)
		pl_PlayerDrinkPotion(i,kFullDetoxifyPotion);

	else if (theMessage->data==kDrinkStrengthPotion)
		pl_PlayerDrinkPotion(i,kStrengthPotion);

	else if (theMessage->data==kDrinkAgilityPotion)
		pl_PlayerDrinkPotion(i,kAgilityPotion);

	else if (theMessage->data==kDrinkElevateMindPotion)
		pl_PlayerDrinkPotion(i,kElevateMindPotion);

	else if (theMessage->data==kDrinkNightVisionPotion)
		pl_PlayerDrinkPotion(i,kNightVisionPotion);

	else if (theMessage->data==kDrinkInvisibilityPotion)
		pl_PlayerDrinkPotion(i,kInvisibilityPotion);

	else if (theMessage->data==kDrinkOpportunityPotion)
		pl_PlayerDrinkPotion(i,kOpportunityPotion);

	else if (theMessage->data==kDrinkNourishPotion)
		pl_PlayerDrinkPotion(i,kNourishPotion);

	else if (theMessage->data==kFish)
	{
		pl_CheckForActionInterrupted(i, false);
		creature[player[i].creatureIndex].nextStrikeTime=TickCount();
		creature[player[i].creatureIndex].numberOfStrikes=4;
		player[i].gathering								=	20;//30;//30; beta
		player[i].gatherType							= kFishing;
		player[i].gatherRow								=	0;
		player[i].gatherCol								=	0;
	}

	else if (theMessage->data==kMeditate)
	{
		//pl_CheckForActionInterrupted(i, false);	version 0.1.1
		if (player[i].meditating==false) //  018
		{
			player[i].meditating							=	true;

			if (creature[player[i].creatureIndex].magicPoints<creature[player[i].creatureIndex].intelligence) //  018
				sk_Meditation(i);
		}
	}

	else if (theMessage->data==kTrack)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Tracking(i);
	}

	else if (theMessage->data==kPickLock)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_Lockpicking(i);
	}

	else if (theMessage->data==kDisarmTrap)
	{
		pl_CheckForActionInterrupted(i, false);
		sk_RemoveTrap(i);
	}

	else if (theMessage->data==kGetActionUpdate)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendActionUpdate(i);
	}

	else if (theMessage->data==kGetContainerContents)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendContainerContents(i);
	}

	else if (theMessage->data==kGetBankContents)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendBankContents(i);
	}

	else if (theMessage->data==kGetShopContents)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendShopContents(i);
	}

	else if (theMessage->data==kGetSellList)
	{
		pl_CheckForActionInterrupted(i, false);
		nw_SendSellList(i);
	}

	else if (theMessage->data==kAttackOff)
		creature[player[i].creatureIndex].attackTarget=0;


}

#endif

// ----------------------------------------------------------------- //
void nw_GetRefreshInventory(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	InventoryRefreshMessage 				*theMessage = (InventoryRefreshMessage *) inMessage;
	int															i;
	int                             j;
	UInt32                          sum;

//gr_ShowDebugLocation(111); // server only

	if (windowVisible[kSellListWindow])	//018
		nw_SendAction(kGetSellList, kHighPriority, kSendNow,true);

	nw_ClearQueueOf(kRefreshInventory);

	sum=validate;

	for (i=0;i<kMaxInventorySize;i++) // paranoid error checking
		sum=sum+theMessage->item[i]+theMessage->count[i];

	if (theMessage->checksum!=sum)
	{
		nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,true);
		fi_WriteToErrorLog("Bad Inventory Refresh");
		return;
	}

	for (i=1;i<kMaxInventorySize;i++) // paranoid error checking
		if (theMessage->count[i]==0)
		{
			theMessage->item[i]   = 0;
			theMessage->count[i]  = 0;
		}

	for (i=1;i<kMaxInventorySize;i++) // paranoid error checking
		if ((theMessage->item[i]>kMaxItemsUsed) || (theMessage->count[i]>kMaxInventoryItemCount))
		{
			for (j=0;j<kMaxInventorySize;j++) // paranoid error checking
			{
				theMessage->item[j]   = 0;
				theMessage->count[j]  = 0;
			}

			fi_WriteToErrorLog("Corrupt Inventory Data/Request Again");	// beta debug

			nw_SendAction(kRefreshInventory, kHighPriority, kSendNow,false);
			return;
		}

	gotInventoryRefresh=true;

	for (i=0;i<kMaxInventorySize;i++)
	{
		localInventory[i].itemID		=	theMessage->item[i];
		localInventory[i].itemCount	=	theMessage->count[i];
	}

	localWeight=0;

	for (i=1;i<kMaxInventorySize;i++)
		if (localInventory[i].itemID!=0)
			if (in_CanGroup(localInventory[i].itemID))
				localWeight=localWeight+itemList[localInventory[i].itemID].weight+(itemList[localInventory[i].itemID].weight*localInventory[i].itemCount)/10;
			else
				localWeight=localWeight+itemList[localInventory[i].itemID].weight;

	if (playerDead)
	{
		for (i=0;i<kMaxInventorySize;i++)
		{
			inventorySlotArmed[i]				=	false;

			if (localInventory[i].itemID==kGrayRobe)
			{
				localTorso=i;
				inventorySlotArmed[i]		=	true;
			}
		}
	}

#ifndef _SERVERONLY
	gr_RenderInventoryWindow();

	gr_RenderShopContentsWindow();

	gr_RenderMakeListWindow(currentSkillDisplay, currentCategory);

	gr_RenderStatsWindow();

	//gr_RenderBankWindow();
#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendPlayerLeft(NSpPlayerID id) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];

//gr_ShowDebugLocation(112); // server only

	if (gNetGame)
	{
		NSpClearMessageHeader(&gNSpPlayerLeftMessage.header);
		gNSpPlayerLeftMessage.header.what 				= kNSpPlayerLeft;
		gNSpPlayerLeftMessage.header.to 					= kNSpAllPlayers;
		gNSpPlayerLeftMessage.header.messageLen 	= sizeof(gNSpPlayerLeftMessage);
		gNSpPlayerLeftMessage.playerCount   = 0;
		gNSpPlayerLeftMessage.playerID      = id;
		gNSpPlayerLeftMessage.playerName[0]='\0';
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gNSpPlayerLeftMessage.header.messageLen;

		if (gNSpPlayerLeftMessage.header.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendPlayerLeft");

#endif
		if (gNetGame!=NULL)
		{
			status = NSpMessage_Send(gNetGame, &gNSpPlayerLeftMessage.header, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendPlayerLeft: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();
	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendClearOneName(int i, int name) // server add to carbon 11/24
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];
	//int                   j;

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		//player[i].sentName[name]=false;
		NSpClearMessageHeader(&gPlayerClearMessage.h);
		gPlayerClearMessage.h.what 				= kClearNameMessage;
		gPlayerClearMessage.h.to 					= player[i].id;
		gPlayerClearMessage.h.messageLen 	= sizeof(gPlayerClearMessage);
		gPlayerClearMessage.data				  =	(UInt8)name; // put in carbon 11/20
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gPlayerClearMessage.h.messageLen;

		if (gPlayerClearMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendClearOneName");

#endif
		if (gNetGame!=NULL)
		{
			status = NSpMessage_Send(gNetGame, &gPlayerClearMessage.h, kSendMethod);

			if (status!=noErr)
			{
				strcpy(errorMessage,"nw_SendClearName: ");
				tb_IntegerToString(status,errorNumber);
				strcat(errorMessage,errorNumber);
				fi_WriteToErrorLog(errorMessage);
			}
		}
		else
			nw_RestartServer();
	}

}

#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendClearName(int name) // server
// ----------------------------------------------------------------- //

{
	OSStatus 							status=noErr;
	char                  errorMessage[kStringLength];
	char                  errorNumber[kStringLength];
	int                   j;

	for (j=0;j<kMaxPlayers;j++)
		if (player[j].online)
			if (player[j].sentName[name])
				if ((gNetGame) && (player[j].online))
				{
					player[j].sentName[name]=false;
					NSpClearMessageHeader(&gPlayerClearMessage.h);
					gPlayerClearMessage.h.what 				= kClearNameMessage;
					gPlayerClearMessage.h.to 					= player[j].id;
					gPlayerClearMessage.h.messageLen 	= sizeof(gPlayerClearMessage);
					gPlayerClearMessage.data				  =	(UInt16)player[name].creatureIndex;
#ifdef _SERVERONLY
					messagesSent++;
					bytesSent=bytesSent+gPlayerClearMessage.h.messageLen;

					if (gPlayerClearMessage.h.messageLen>2000)
						fi_WriteToErrorLog("Large message in nw_SendClearName");

#endif
					if (gNetGame!=NULL)
					{
						status = NSpMessage_Send(gNetGame, &gPlayerClearMessage.h, kSendMethod);

						if (status!=noErr)
						{
							strcpy(errorMessage,"nw_SendClearName: ");
							tb_IntegerToString(status,errorNumber);
							strcat(errorMessage,errorNumber);
							fi_WriteToErrorLog(errorMessage);
						}
					}
					else
						nw_RestartServer();
				}

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_GetClearName(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //

{
	PlayerClearMessage 				*theMessage = (PlayerClearMessage *) inMessage;

	if ((theMessage->data<0) || (theMessage->data>=kMaxPlayers))
	{
		fi_WriteToErrorLog("Clear name out of range");
		return;
	}

	strcpy(playerNames[theMessage->data],"");

	playerIDs[theMessage->data]=0;
#ifdef _DEBUGGING
	gr_AddText("Clearing name.",true,true);	// beta
#endif

}
#endif

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendClearSpat(int i)
// ----------------------------------------------------------------- //
{
	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gPlayerClearSpatMessage.h);
		gPlayerClearSpatMessage.h.what 				= kClearSpatMessage;
		gPlayerClearSpatMessage.h.to 					= player[i].id;
		gPlayerClearSpatMessage.h.messageLen 	= sizeof(gPlayerClearSpatMessage);
		
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gPlayerClearSpatMessage.h.messageLen;
#endif

		if (gNetGame!=NULL)
		{
			NSpMessage_Send(gNetGame, &gPlayerClearSpatMessage.h, kSendMethod);
		}
		else
			nw_RestartServer();
	}

}

#endif

#ifndef _SERVERONLY
// ----------------------------------------------------------------- //
void nw_GetClearSpat(NSpMessageHeader *inMessage) // client
// ----------------------------------------------------------------- //
{
	ClearSpatMessage 				*theMessage = (ClearSpatMessage *) inMessage;

	localSpat=0;
	
	gr_RenderMakeListWindow(currentSkillDisplay,currentCategory);
}

#endif

// ----------------------------------------------------------------- //
void nw_GetPlayerLeft(NSpMessageHeader *inMessage)
// ----------------------------------------------------------------- //

{
	NSpPlayerLeftMessage 				*theMessage = (NSpPlayerLeftMessage *) inMessage;
	int													i;

//gr_ShowDebugLocation(115); // server only
#ifdef _SERVERONLY
	bytesReceived=bytesReceived+sizeof(NSpPlayerLeftMessage);
	messagesReceived++;
#endif

#ifdef _DEBUGGING
	fi_WriteToErrorLog("Player Left");
#endif

#ifdef _SERVER

	for (i=1;i<kMaxPlayers;i++)
		if (player[i].id==theMessage->playerID)
		{
			//fi_WriteToErrorLog(player[i].playerName);
			gracefulLogoffs++;
			player[i].online				=	false;
			player[i].lastCheckIn		= TickCount();

			if (sh_StandingInInn(i))  // version 0.1.1 - Instant log out in inn
			{
				cr_ClearAttackTarget(player[i].creatureIndex);
				nw_SendClearName(i);
				fi_Logout(i);
				creatureMap[creature[player[i].creatureIndex].row][creature[player[i].creatureIndex].col]=0;
				cr_ClearCreatureRecord(player[i].creatureIndex);
				pl_ClearPlayerRecord(i);
			}
			else
				nw_SendClearName(i);

#ifdef _SERVERONLY
			gr_DrawServerNameAt(i);

#endif
			return;
		}

#endif

#ifndef _SERVERONLY
#ifdef _DEBUGGING
	gr_AddText("Player left.",true,true);	// beta

#endif
	for (i=1;i<kMaxPlayers;i++)
		if (playerIDs[i]==theMessage->playerID)
		{
#ifdef _DEBUGGING
			gr_AddText("Clearing name.",true,true);	// beta
#endif
			strcpy(playerNames[i],"");
			playerIDs[i]=0;
			return;
		}

#endif

}

#ifdef _SERVER
// ----------------------------------------------------------------- //
void nw_SendNightVision(int i) // server
// ----------------------------------------------------------------- //

{
	//int										j;
	OSStatus 							status;

//gr_ShowDebugLocation(116); // server only

	if ((i<1) || (i>=kMaxPlayers))
		return;

	if (player[i].online==false)
	{
		//fi_WriteToErrorLog("Player not online");
		return;
	}

	if (gNetGame)
	{
		NSpClearMessageHeader(&gNightVisionMessage.h);
		gNightVisionMessage.h.what 					= kNightVisionMessage;
		gNightVisionMessage.h.to 						= player[i].id;
		gNightVisionMessage.h.messageLen 		= sizeof(gNightVisionMessage);
#ifdef _SERVERONLY
		messagesSent++;
		bytesSent=bytesSent+gNightVisionMessage.h.messageLen;

		if (gNightVisionMessage.h.messageLen>2000)
			fi_WriteToErrorLog("Large message in nw_SendNightVision");

#endif
		if (gNetGame!=NULL)
			status = NSpMessage_Send(gNetGame, &gNightVisionMessage.h, kSendMethod);
	}
	else
		nw_RestartServer();

}

#endif

// ----------------------------------------------------------------- //
void nw_DoHandleMessage(NSpMessageHeader *inMessage)	// server/client
// ----------------------------------------------------------------- //

{
	UInt32							playerID;
	OSStatus						status;
	NSpPlayerInfo 			*playerInfo;
	int									playerIndex;
	int									i;
	//int									cpt;
	int									found=-1;

//gr_ShowDebugLocation(117); // server only

	fi_WriteToTraceLog("8");
	

	// ---- check for duplicate messages
#ifdef _SERVER

	messagesReceived++;

	for (i=1;i<kMaxPlayers;i++)
		if (player[i].id==inMessage->from)
		{
			found=i;

			if (player[found].previousMessageID==inMessage->id)
			{
				return;
			}
			else
				break;
		}

	if (found>0)
		player[found].previousMessageID=inMessage->id;

#endif

#ifndef _SERVER
	for (i=0;i<10;i++)
		if (previousMessageID[i]==inMessage->id)
			return;

	for (i=0;i<9;i++)
		previousMessageID[i]=previousMessageID[i+1];

	previousMessageID[9]=inMessage->id;

#endif


//gr_ShowDebugLocation(118); // server only

	switch (inMessage->what)
	{

			//-----------------------------------------------------

		case kMacMessage:
#ifdef _SERVER
			nw_GetMac(found,inMessage);
#endif
			break;

		case kMakeMessage:
#ifdef _SERVER
			nw_GetMake(found,inMessage);
#endif
			break;
			
		case kSpecialAttackMessage:
#ifdef _SERVER
			nw_GetSpecialAttack(found,inMessage);
#endif
		break;

		case kRepairMessage:
#ifdef _SERVER
			nw_GetRepair(found,inMessage);
#endif
			break;

		case kItemRepairedMessage:
#ifndef _SERVERONLY
			nw_GetItemRepaired(inMessage);
#endif
			break;

		case kBuyFromListMessage:
#ifdef _SERVER
			nw_GetBuyFromList(found,inMessage);
#endif
			break;

		case kPetNameMessage:
#ifdef _SERVER
			nw_GetPetName(found,inMessage);
#endif
			break;

		case kTakeFromListMessage:
#ifdef _SERVER
			nw_GetTakeFromList(found,inMessage);
#endif
			break;

		case kTakeFromBankMessage:
#ifdef _SERVER
			nw_GetTakeFromBank(found,inMessage);
#endif
			break;

		case kGiveGoldMessage:
#ifdef _SERVER
			nw_GetGiveGold(found,inMessage);
#endif
			break;

		case kGMMessage:
#ifdef _SERVER
			nw_GetGMCommand(found,inMessage);
#endif
			break;

		case kChessMoveMessage:
#ifdef _SERVER
			nw_GetChessMove(found,inMessage);
#endif
			break;

		case kGoldRemoveMessage:
			nw_GetGoldRemove(inMessage);
			break;

		case kClientCommandMessage:
			nw_GetClientCommand(inMessage);
			break;

		case kClearNameMessage:
#ifndef _SERVERONLY
			nw_GetClearName(inMessage);
#endif
			break;

		case kAlertMessage:
			nw_GetAlertMessage(inMessage);
			break;

		case kUDPMessage:
			nw_GetUDPTest(inMessage);
			break;

		case kUDPOKMessage:
#ifdef _SERVER
			nw_GetUDPOK(found,inMessage);
#endif
			break;

		case kIDMessage:
			nw_GetID(inMessage);
			break;

		case kNightVisionMessage:
			nightVision=true;
			break;

			//-----------------------------------------------------

		case kItemTransferMessage:
#ifdef _SERVER
			nw_GetItemTransferMessage(found,inMessage);
#endif
			break;

		case kInventoryDropToClientMessage:
			nw_GetToClientDropFromInventorySlot(inMessage);
			break;

			//-----------------------------------------------------

		case kLocateMessage:
#ifdef _SERVER
			nw_GetLocateFromClient(found,inMessage);
#endif
			break;

		case kMeetToServerMessage:
#ifdef _SERVER
			nw_GetMeetFromClient(found,inMessage);
#endif
			break;

		case kSummonToServerMessage:
#ifdef _SERVER
			nw_GetSummonFromClient(found,inMessage);
#endif
			break;

		case kGuildToServerMessage:
#ifdef _SERVER
			nw_GetGuildFromClient(found,inMessage);
#endif
			break;

		case kMsgToServerMessage:
#ifdef _SERVER
			nw_GetMsgFromClient(found,inMessage);
#endif
			break;

		case kSignToServerMessage:
#ifdef _SERVER
			nw_GetSignFromClient(found,inMessage);
#endif
			break;

		case kMsayToServerMessage:
#ifdef _SERVER
			nw_GetMsayFromClient(found,inMessage);
#endif
			break;

		case kNoteToServerMessage:
#ifdef _SERVER
			nw_GetNoteFromClient(found,inMessage);
#endif
			break;

		case kHallToServerMessage:
#ifdef _SERVER
			nw_GetHallFromClient(found,inMessage);
#endif
			break;

		case kHallRequestToServerMessage:
#ifdef _SERVER
			nw_GetHallRequest(found,inMessage);
#endif
			break;

		case kHallToClientMessage:
			nw_GetHallInfos(inMessage);
			break;

		case kHallGuildToServerMessage:
#ifdef _SERVER
			nw_GetHallGuildFromClient(found,inMessage);
			break;
#endif

		case kExpToServerMessage:
#ifdef _SERVER
			nw_GetExpFromClient(found,inMessage);
#endif
			break;

		case kCustomTitleToServerMessage:
#ifdef _SERVER
			nw_GetCustomTitleFromClient(found,inMessage);
#endif
			break;

		case kCalendarRequestToServerMessage:
#ifdef _SERVER
			nw_GetCalendarRequest(found,inMessage);
#endif
			break;

		case kRExpToServerMessage:
#ifdef _SERVER
			nw_GetRExpFromClient(found, inMessage);
#endif

		case kCalendarInfosToClientMessage:
			nw_GetCalendarInfo(inMessage);
			break;

		case kNameChangeToServerMessage:
#ifdef _SERVER
			nw_GetNameChangeFromClient(found,inMessage);
#endif
			break;

		case kAnnounceToServerMessage:
#ifdef _SERVER
			nw_GetAnnounceFromClient(found,inMessage);
#endif
			break;

		case kWhisperToServerMessage:
#ifdef _SERVER
			nw_GetWhisperFromClient(found,inMessage);
#endif
			break;

		case kGiftToServerMessage:
#ifdef _SERVER
			nw_GetGiftFromClient(found,inMessage);
#endif
			break;

		case kAddToPartyMessage:
#ifdef _SERVER
			nw_GetAddToParty(found,inMessage);
#endif
			break;

#ifndef _SERVERONLY
		case kTrackingInfosToClientMessage:
			nw_GetTrackingInfosFromServer(inMessage);
			break;
#endif

		case kTextToServerMessage:
#ifdef _SERVER
			nw_GetTextFromClient(found,inMessage);
#endif
			break;

		case kPlayerNameMessage:
			nw_GetPlayerName(inMessage);
			break;

#ifndef _SERVER
		case kVersionToClientMessage:
			nw_GetUpdateRequired(inMessage);
			break;
#endif

		case kQuestDescriptionMessage:
			nw_GetQuestDescription(inMessage);
			break;

		case kRequestPlayerNameMessage:
#ifdef _SERVER
			nw_GetRequestName(found,inMessage);
#endif
			break;

		case kRequestQuestDescriptionMessage:
#ifdef _SERVER
			nw_GetRequestQuestDescription(found,inMessage);
#endif
			break;

		case kRemoveFromPartyMessage:
#ifdef _SERVER
			nw_GetRemoveFromParty(found,inMessage);
#endif
			break;

		case kBroadcastTextToServerMessage:
#ifdef _SERVER
			nw_GetBroadcastTextFromClient(found,inMessage);
#endif
			break;

		case kLogonTextToServerMessage:
#ifdef _SERVER
			nw_GetLogonTextFromClient(found,inMessage);
#endif
			break;

		case kBugTextToServerMessage:
#ifdef _SERVER
			nw_GetBugTextFromClient(found,inMessage);
#endif
			break;

		case kTextToClientMessage:
			nw_GetTextFromServer(inMessage);
			break;

		case kSystemTextToClientMessage:
			nw_GetSystemTextFromServer(inMessage);
			break;

			//-----------------------------------------------------

		case kDisplayMessage:
			nw_GetDisplayMessage(inMessage);
			break;

			//-----------------------------------------------------

			//-----------------------------------------------------

		case kGoldAddMessage:
			nw_GetGoldAdd(inMessage);
			break;

		case kInventoryAddMessage:
			nw_GetInventoryAdd(inMessage);
			break;

		case kInventoryDeleteMessage:
#ifdef _SERVER
			nw_GetDeleteFromInventorySlot(found, inMessage);
#endif
			break;

		case kChessRefreshMessage:
			nw_GetChessUpdate(inMessage);
			break;

		case kInventoryRefreshMessage:
			nw_GetRefreshInventory(inMessage);
			break;

		case kContainerContentsMessage:
			nw_GetContainerContents(inMessage);
			break;

		case kBankContentsMessage:
			nw_GetBankContents(inMessage);
			break;

		case kShopContentsMessage:
			nw_GetShopContents(inMessage);
			break;

		case kSellListMessage:
			nw_GetSellList(inMessage);
			break;

		case kArmMessage:
#ifdef _SERVER
			nw_GetArm(found, inMessage);
#endif
			break;

#ifndef _SERVERONLY
		case kArmRefreshMessage:
			nw_GetArmRefresh(inMessage);
			break;
#endif

#ifndef	_SERVERONLY
		case kSpeedToClientMessage:
			nw_GetSpeed(inMessage);
			break;
			
		case kClearSpatMessage:
			nw_GetClearSpat(inMessage);
			break;
#endif

			//-----------------------------------------------------

		case kSkillsRefreshMessage:
			nw_GetRefreshSkills(inMessage);
			break;

		case kPetNamesRefreshMessage:
			nw_GetRefreshPetNames(inMessage);
			break;

		case kSkillChangeMessage:
			nw_GetSkillChange(inMessage);
			break;

			//-----------------------------------------------------

		case kLeaveMessage:
		{


#ifndef _SERVER
			nw_ShutdownNetworking();
			onLine=false;
			gr_HideAllWindows();
			ShowWindow(logonWindow);
			sp_InitSplash();
			UpdateEvent(tbLogOnWindow);
#endif

		}

		break;

		//-----------------------------------------------------

		case kNSpPlayerJoined:
#ifdef _SERVER
			{
				NSpPlayerJoinedMessage 							*theMessage = (NSpPlayerJoinedMessage *) inMessage;
				char																name[275];

#ifdef _SERVERONLY
				bytesReceived=bytesReceived+sizeof(NSpPlayerJoinedMessage);
				messagesReceived++;

				fi_UpdateIPLog(theMessage->playerInfo.id);
				tb_PascaltoCStr(theMessage->playerInfo.name,name);

				if (strcmp(name,"")==0)
				{
					fi_WriteToErrorLog("Empty Name"); // gs debug
					nw_RemovePlayer(theMessage->playerInfo.id);
					break;
				}

#endif

#ifndef _SERVERONLY
				lastJoin=theMessage->playerInfo.id;//  beta

				fi_UpdateIPLog(theMessage->playerInfo.id);

				tb_PascaltoCStr(theMessage->playerInfo.name,name);

				if (strcmp(name,"")==0)
				{
					nw_RemovePlayer(theMessage->playerInfo.id);
					break;
				}

				if (strcmp(name,"Host")==0)
				{
					pl_AddPlayer("Host", "mac", theMessage->playerInfo.id, &playerIndex);
					it_CalculateArmorRating(playerIndex);
					pl_CalculateStats(playerIndex);
					player[playerIndex].sendStatsRefresh=true;
					player[playerIndex].sendPetsRefresh=true;
					nw_SendInventoryRefresh(playerIndex);
					nw_SendSkillsRefresh(playerIndex);
					nw_SendArmRefresh(playerIndex);
					pl_CalculatePetLoyalty(playerIndex);

					for (i=0;i<kMaxPets;i++)
						if (player[playerIndex].petType[i]!=0)
						{
							nw_SendPetNameRefresh(playerIndex);
							break;
						}

					//if ((player[playerIndex].category==1) || (player[playerIndex].category==2)) // merge 7/27
					//  {
					//   nw_SendSystemTextToClient(playerIndex,"Welcome Counselor!");
					//    nw_SendSystemTextToClient(playerIndex,"For GM news, rules, and commands visit:");
					//    nw_SendSystemTextToClient(playerIndex,"http://www.oberin.com/gmnews.html");
					//  }
				}

#endif

				tb_PascaltoCStr(theMessage->playerInfo.name,name);

				if (strcmp(name,"Host")!=0)
				{
					for (i=1;i<kMaxPlayers;i++)
						if (waitingForAuthorization[i]==0)
						{
							waitingForAuthorization[i]=theMessage->playerInfo.id;
							authorizationTimeOut[i]=TickCount()+(60*10);
							break;
						}
				}
				else
					hostRunning=true;
			}
#endif
			break;

			//-----------------------------------------------------

		case kNSpJoinRequest:
			break;

			//-----------------------------------------------------

		case kNSpJoinApproved:
			//attemptingReconnect=false;
			reconnected=true;
			myID=NSpPlayer_GetMyID(gNetGame);
			nw_SendAuthorization();
			break;

			//-----------------------------------------------------

		case kAuthorizeMessage:
#ifdef _SERVER
			{
				AuthorizeMessage 							*theMessage = (AuthorizeMessage *) inMessage;
				char													name[275];
				TBoolean											authorize=false;
				TBoolean											foundWaiting=false;

#ifdef _SERVERONLY
				bytesReceived=bytesReceived+theMessage->h.messageLen;
				messagesReceived++;
#endif

				playerID=theMessage->h.from;

				for (i=1;i<kMaxPlayers;i++)
					if (waitingForAuthorization[i]==playerID)
					{
						waitingForAuthorization[i]=0;
						authorizationTimeOut[i]=0;
						foundWaiting=true;
						//break;
					}

				if (!foundWaiting)
				{
					fi_WriteToErrorLog("Not on waiting for authorization list.");
					NSpPlayer_Remove(gNetGame,playerID);
				}
				else if ((numberOfPlayersOnline>=kLogOnLimit) && (theMessage->clientType==0)) // client type != 0 can always log on
				{
					serverFull++;
					nw_SendAlertMessage(playerID, "Server full.  Please try again later.");
					NSpPlayer_Remove(gNetGame,playerID);
				}
				else if (theMessage->version<kServerVersion)
				{
					nw_SendUpdateRequired(playerID);
					ToRemove[playerID]=TickCount()+360;
				}
				else
				{
					status=NSpPlayer_GetInfo(gNetGame,playerID,&playerInfo);

					if (status==noErr)
					{
						tb_PascaltoCStr(playerInfo->name,name);

						if (strcmp(name,"")==0)
							authorize=false;
						else if (strlen(name)>14)
							authorize=false;
						else if (strlen(theMessage->password)>32)
							authorize=false;
						else if (strcmp(name,"Host")!=0)
							authorize=pl_AddPlayer(theMessage->name, theMessage->password, playerID, &playerIndex);

						NSpPlayer_ReleaseInfo(gNetGame,playerInfo);

						if ((!authorize) || (playerIndex==0))
						{
							failedLogons++;
							nw_SendAlertMessage(playerID, "Invalid user name or password.");
							ToRemove[playerID]=TickCount()+360;
						}
						else
						{
							numberOfLogons++;
							//nw_SendUDPTest(playerIndex);
							it_CalculateArmorRating(playerIndex);
							pl_CalculateStats(playerIndex);
							player[playerIndex].sendStatsRefresh=true;
							player[playerIndex].sendPetsRefresh=true;
							nw_SendInventoryRefresh(playerIndex);
							nw_SendSkillsRefresh(playerIndex);
							nw_SendArmRefresh(playerIndex);
							nw_SendSpeedRefresh(playerIndex);
							nw_SendShopContents(playerIndex);
							pl_CalculatePetLoyalty(playerIndex);

							for (i=0;i<kMaxPets;i++)
								if (player[playerIndex].petType[i]!=0)
								{
									nw_SendPetNameRefresh(playerIndex);
									break;
								}

							if (showLogonMessage)
							{
								fi_SendLoginMessage(playerIndex);
								//if (strcmp(logonMessage1,"")!=0)
								//  nw_SendSystemTextToClient(playerIndex,logonMessage1);
								//if (strcmp(logonMessage2,"")!=0)
								//  nw_SendSystemTextToClient(playerIndex,logonMessage2);
								//if (strcmp(logonMessage3,"")!=0)
								//  nw_SendSystemTextToClient(playerIndex,logonMessage3);
							}

							//if (theMessage->version<kClientVersion)
							//	{
							//  	nw_SendSystemTextToClient(playerIndex,"Notice:  Version 0.1.6d is available for download.");
							//	}
							//if ((player[playerIndex].category==1) || (player[playerIndex].category==2))
							//  {
							//    nw_SendSystemTextToClient(playerIndex,"Welcome Counselor!");
							//    nw_SendSystemTextToClient(playerIndex,"For GM news, rules, and commands visit:");
							//    nw_SendSystemTextToClient(playerIndex,"http://www.oberin.com/gmnews.html");
							//  }
#ifdef _SERVERONLY
							gr_DrawServerNameAt(playerIndex);

#endif
						}
					}
					else
						NSpPlayer_Remove(gNetGame,playerID);

					//status=NSpPlayer_ReleaseInfo(gNetGame,&playerInfo);
				}
			}
#endif
		break;

			//-----------------------------------------------------

		case kNSpJoinDenied:
		{
#ifndef _SERVER

			if (attemptingReconnect)
			{
				fi_WriteToErrorLog("Reconnect failed");
				attemptingReconnect=false;
				sh_EndSession();
			}
			else
			{
				if (gotFirstRefresh)
				{
					fi_WriteToErrorLog("Join Denied - Ignored");
					return;
				}

				fi_WriteToErrorLog("Join Denied");

				nw_ShutdownNetworking();
#ifndef _SERVER
				onLine=false;
				gr_HideAllWindows();
				ShowWindow(logonWindow);
				sp_InitSplash();
				UpdateEvent(tbLogOnWindow);
#endif
			}
#endif
		}
			break;

		//-----------------------------------------------------

		case kPlayerRefreshMessage:
			nw_GetRefresh(inMessage);
			break;

		//-----------------------------------------------------

		case kPlayerTargetMessage:
#ifdef _SERVER
			nw_GetTargetMessage(found, inMessage);
#endif
			break;

		case kSortMessage:
#ifdef	_SERVER
			nw_GetSortMessage(found, inMessage);
#endif
			break;

		case kPlayerTargetResourceMessage:
#ifdef _SERVER
			nw_GetTargetResourceMessage(found, inMessage);
#endif
			break;

		case kPathMessage:
#ifdef _SERVER
			nw_GetPathMessage(found, inMessage);
#endif
			break;

		case kPlayerActionMessage:
#ifdef _SERVER
			nw_GetPlayerAction(found, inMessage);
#endif
			break;

		//-----------------------------------------------------

		case kNSpPlayerLeft:
			nw_GetPlayerLeft(inMessage);
			break;

		case kNSpGameTerminated:
			{
				fi_WriteToErrorLog("Terminated");
				nw_ShutdownNetworking();

#ifndef _SERVER

				if (attemptingReconnect)
				{
					//sh_}Session();
					if (nw_DoJoin()!=0)
					{
						sh_EndSession();
						fi_WriteToErrorLog("Got termination, but unable to rejoin");
					}
					else
					{
						gotFirstRefresh=true;
						thisIsReconnect=true;
						//nw_InitNetworkingVariables();
					}

				attemptingReconnect=false;
				}
				else
				{
					onLine=false;
					gr_HideAllWindows();

					if (!gUpdating)
					{
						ShowWindow(logonWindow);
						sp_InitSplash();
						UpdateEvent(tbLogOnWindow);
					}
				}

#endif

			}

			break;

		default:
			break;
	}

	//gr_ShowDebugLocation(119); // server only
	fi_WriteToTraceLog("9");

}
