#ifndef _PLAYERS
#define _PLAYERS

#include "Keys.h"
#include "Remap.h"
#include "Network.h"
#include "Inventory.h"
#include "Globals.h"
#include "Common.h"

//- constants ---------------------------------------------

#define         kMaxGMJumps																	1000																
#define         kGMJumpStringLength													32

#define         kBabbleCurse																1 //0x01
#define         kStenchCurse																2 //0x02


#define         kHeardSize                									100 //60

#define         kRegularPlayer                              1
#define         kCanLosePets                                2
#define         kCanBeRevealed                              3
#define         kAttackPets                                 4
#define         kAttackPlayers                              5
#define         kPetCanAttackPlayers                        6
#define         kExtraResistance                            7
#define         kExtraDamage                                8
#define         kCanBan                                     9
#define         kCanAKA                                     10
#define         kIsApprenticeGM                             11
#define         kIsRegularGM                                12
#define         kIsSeniorGM                                 13
#define         kIsDeveloper                                14
#define         kIsAnyGM                                    15
#define         kCanSeeDemoNames                            16
#define         kIsQuestCharacter                           17
#define         kCanSeeQuestCharacters                      18
#define         kCanBroadcast                               19
#define         kSetLogonMessage                            20
#define         kCanUseGMChannel                            21
#define         kCanUseDeveloperChannel                     22
#define         kMeetPlayers                                23
#define         kCanSeeMacroSuspects                        24
#define         kIsQuestScoundrel                           25
#define         kCanSummonPlayers                           26
#define         kCanSetGuildTags                            27
#define         kCanSetLogonMessage                         28
#define         kCanSetSigns                                29
#define         kCanSetHalls                                30
#define         kCanGrantExperience                         31
#define         kCanSetOwner                                32
#define         kCanGoto                                    33
#define         kCanSpawnCreatures                          34
#define         kCanMorph                                   35
#define         kNotifiedOfNewPlayers                       36
#define         kCanMakeContainer                           37
#define         kCanSpawnItems                              38
#define         kCanTalkToGhosts                            39
#define         kPermanentNightVision                       40
#define         kCanGetPosition                             41
#define         kCanControlCharacterCreator                 42
#define         kCanSeeHelpRequestsWithFeedback             43
#define         kCanSeeHelpRequestsWithNoFeedback           44
#define         kCanBanishCreature                          45
#define         kCanPitPlayers                              46
#define         kCanJailPlayers                             47
#define         kCanKickPlayers                             48
#define         kCanControlSkillGain                        49
#define         kCanResPlayers                              50
#define         kCanResetPlayers                            51
#define         kCanSendHome                                52
#define         kCanActivate                                53
#define         kCanGetIP                                   54
#define			kCanMute									55
#define			kCanMuteAll									56
#define			kCanKillPlayers								57
#define			kCanWhisper									58
#define			kCanMsay									59
#define			kCanChangeNames								60
//- types -------------------------------------------------

typedef struct playerLocalSkillsType {
	UInt8 										level;
//	UInt8 										direction;
//	UInt16										counter;
} playerLocalSkillsType;	

typedef struct playerServerSkillsType {
	UInt32 										level;
} playerServerSkillsType;	

typedef struct gmJumpType {
	char 										command[kGMJumpStringLength];
	int											row;
	int											col;
} gmJumpType;	

typedef struct playerType {
	TBoolean									active;
	TBoolean									online;
	TBoolean									skipNextRefresh;
	TBoolean									sendStatsRefresh;
	TBoolean									sendPetsRefresh;
	TBoolean                 					demo;
	TBoolean                  					helper;
	TBoolean                  					noGain;
	TBoolean                  					slow;
	TBoolean									afk;
	char										userName	    		[kStringLength];
	char										password	    		[kStringLength];	// saved
	char										playerName    			[kStringLength];	// saved
	char										playerNameTemp			[kStringLength];    // saved
	char										playerNameLC  			[kStringLength];	// saved
	char										guildName    			[kStringLength];	// saved
	char										friendName   			[16][kStringLength];	// saved
	char                      					macID         			[kStringLength];
	int                       					friendID      			[16];
	UInt8                     					morph;
	UInt8                     					totalRefresh;
	int                       					anatomyTarget           [kMaxMacroTrace];
	int                       					tamingTarget            [kMaxMacroTrace];
	int                       					detectingHiddenTarget   [kMaxMacroTrace];
	int                       					thieveryTarget          [kMaxMacroTrace];
	UInt16                    					lastHideRow;
	UInt16                    					lastHideCol;
	UInt16                    					lastStealthRow;
	UInt16                    					lastStealthCol;
	UInt16                    					lastTrackingRow;
	UInt16                    					lastTrackingCol;
	UInt8										category;	// see note below	// saved
	TBoolean                  					god;
	int                       					returnRow;
	int                       					returnCol;
	int										    accountNumber;
	UInt16										creatureIndex;
	TBoolean									dead;	// saved
	NSpPlayerID									id;
	UInt32										previousMessageID;
	unsigned long								lastRefresh;
	unsigned long								lastTalkRefresh;
	unsigned long								lastCheckIn;
	unsigned long								lastMove;
	unsigned long								logon;
	unsigned long             					thiefUntil;
	unsigned long             					saveAt;
	playerServerSkillsType						skill[kMaxSkills];	// saved
	UInt8										gathering;
	UInt8										gatherType;
	UInt16										gatherRow;
	UInt16										gatherCol;
	UInt8										strengthCounter;	// saved
	UInt8										intelligenceCounter;	// saved
	UInt8										dexterityCounter;	// saved
//	UInt16										hunger;	// saved
	TBoolean									meditating;
	TBoolean									healing;
	UInt8										arm[13];	// saved
	//char										talk[kStringLength];
	//unsigned long								talkTime;
	//unsigned long             				talkID;
	unsigned long             					heard[kHeardSize];
	unsigned long             					heardSound[kHeardSize];
	inventoryType								bank[kMaxInventorySize];	// saved
	UInt16										rowTrail[kMaxTrail];
	UInt16										colTrail[kMaxTrail];
	UInt32										pet[kMaxPets];
	TBoolean									petInStable[kMaxPets];
	UInt16										petType[kMaxPets];	// saved
	char                      					petName[kMaxPets][kStringLength];
	UInt8                     					petLoyalty[kMaxPets];
	UInt32										petExperience[kMaxPets];
	int											petHealth[kMaxPets];
	UInt8										revealHidden;
	int   										title;	// saved
	char										titleCustom[kStringLength]; //saved
	UInt8                     					gender;
	TBoolean                  					router;
	TBoolean									stealth;
	unsigned long								stealthCool;
	int											meet;
	UInt32										turnUntil;
	UInt8                     					characterClass;
	TBoolean                  					sharing;
	TBoolean                  					sentName[kMaxPlayers];
	TBoolean                  					newPlayer;
	UInt8										paralyzeField;
	UInt8										poisonField;
	UInt8										massheal;
	UInt8										disorientfield;
	UInt16										previousRow;
	UInt16										previousCol;
	UInt32                    					magicAttacker;
	UInt16                    					resistanceGained;
	UInt32                    					normalAttacker;
	UInt16                    					fightingSkillGained;
	UInt32                    					lastSkillGain;
	int											maxAttackers;
	int											currentlyAttacking;
	unsigned long								playerRefreshTime;
	UInt8										language;
	UInt16										curse;
	unsigned long								previousStenchMessage;
	char										msay[kStringLength]; //Used to make monsters or NPC talk
	UInt8										hairColor; //Saved
	UInt8										hairStyle; //Saved
	UInt8										stealthMoves;
	int											lastBackstabTarget;
	TBoolean									possibleBackstab;
	TBoolean									noTrack;
	UInt8										speed;
	int											npcDressup;
	int											creatureField;
	int											fieldFrom;
	unsigned long								morphTime;
} playerType;	

// Player categories
//
// 0 - regular
// 1 - no obstacles/won't be attacked
// 2 - won't be attacked

//- variables ----------------------------------------------

extern playerType								player					[kMaxPlayers];
extern int										playerState				[kMaxPlayers][2][kMaxScreenElements];
extern gmJumpType								gmJump					[kMaxGMJumps];

//- functions ----------------------------------------------

void 			pl_InitPlayers					(void);
TBoolean 		pl_AddPlayer					(char userName[275], char password[kStringLength], UInt32 playerID, int *playerIndex);
TBoolean 		pl_PlayerMove					(int i, int direction);
void 			pl_ClearPlayerRecord			(int i);
void 			pl_PlayerEat					(int i); // server
void 			pl_PlayerDrinkPotion			(int i, int potionType); // server
void 			pl_CheckForActionInterrupted	(int i, TBoolean move); // server
void 			pl_Attack						(int i, int c); // server
void 			pl_PlayerMoveTo					(int i, UInt16 row, UInt16 col); // server
void 			pl_PetCommand					(int i, char	playerText[kStringLength]);
void 			pl_StablePets					(int i);
void 			pl_Insult						(int i); // server
TBoolean    	pl_PlayerInStable               (int i); // server
TBoolean    	pl_IsThief                      (int c); // server
void        	pl_PlayerFaceDirection          (int i, int direction); // server
void        	pl_GetLocationName              (char location[kStringLength], int row, int col);		// server
void        	pl_AddToTitle                   (int i, int amount);		// server
void        	pl_ActivatePlayer               (int i, int p);		// server
void 			pl_SendPlayerHome				(int i, int p);		// server
void        	pl_ResetPlayer                  (int i, int p);		// server
void 			pl_ResPlayer					(int i, int p);		// server
void 			pl_KillPlayer					(int i, int p);		// server
int         	pl_GetCharacterLevel            (int p);	// server
void        	pl_CalculateStats               (int p);	// server
void        	pl_StablePet                    (int i, int j);
TBoolean    	pl_IsPetCommand                 (char s[kStringLength]);
void        	pl_CalculatePetLoyalty          (int p);	// server
void        	pl_BanishCreature               (int i, int p);		// server
void 			pl_BanPlayer					(int i, int p);		// server
void 			pl_JailPlayer					(int i, int p);		// server
void 			pl_MutePlayer					(int i, int p);		// server
void 			pl_UnmutePlayer					(int i, int p);		// server
void        	pl_KickPlayer                   (int i, int p);		// server
void        	pl_SendMacroWarning             (int i); // server
void        	pl_SendMacroWarning             (int i); // server
TBoolean    	pl_PlayerHeard                  (int i, unsigned long talkID);
TBoolean    	pl_PlayerHeardSound             (int i, unsigned long sndID, unsigned long sndTime);
void 			pl_GetPetPosition				(int i, int p, UInt16 *row, UInt16 *col);	// server
TBoolean    	pl_Teleporting                  (int i, int crystal);	// server
TBoolean    	pl_OnTeleporter                 (int pRow, int pCol, int row, int col);	// server mod 8/4 new function
void        	pl_NoGainPlayer                 (int i, int p);		// server
void        	pl_GainPlayer                   (int i, int p);		// server
void        	pl_PitPlayer                    (int i, int p);		// server
TBoolean    	pl_AuthorizedTo                 (int c, int command);	// put in carbon 11/27
void 			pl_CursePlayer					(int p);	// server
void			pl_DumpPlayerClass				(int i, int p); //server
void			pl_MakeEvil						(int i, int p); //server
void			pl_MakeGood						(int i, int p); //server
void			pl_MakeSay						(int i, int p); //server
void 			pl_DumpPlayerSkills				(int i, int p); //server
void 			pl_HairDying					(int i, int color); //server
void			pl_SetSpeed						(int i, UInt8 speed); //server
TBoolean		pl_CheckSpecialAttack			(int spat); //client
TBoolean		pl_BehindTarget					(); //client
TBoolean		pl_NearTarget					(); //client
void			pl_AddSpecialAttack				(int i, int spat); //server
void			pl_DumpPlayerInv				(int, int);
void			pl_DumpPlayerBank				(int, int);
TBoolean		pl_BehindTargetServer			(int, int);

//----------------------------------------------------------

#endif