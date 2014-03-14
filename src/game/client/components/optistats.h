#include <game/client/component.h>

enum {
	OP_STATS_KILLS=1,
	OP_STATS_DEATHS=2,
	OP_STATS_SUICIDES=4,
	OP_STATS_RATIO=8,
	OP_STATS_NET=16,
	OP_STATS_KPM=32,
	OP_STATS_SPREE=64,
	OP_STATS_BESTSPREE=128,
	OP_STATS_FLAGGRABS=256,
	OP_STATS_WEAPS=512,
	OP_STATS_FLAGCAPTURES=1024
};

class COptiStats: public CComponent
{
private:
	int m_Mode;
	int m_StatsClientID;
	bool m_ScreenshotTaken;
	int64 m_ScreenshotTime;
	static void ConKeyStats(IConsole::IResult *pResult, void *pUserData);
	static void ConKeyNext(IConsole::IResult *pResult, void *pUserData);
	static void ConKeyReset(IConsole::IResult *pResult, void *pUserData);
	void RenderHeader(float x, float y, float w, char *pTitle);
	void RenderGlobalStats(/*float x, float y, float w*/);
	void RenderIndividualStats();
	void RenderKillerStats();
	void CheckStatsClientID();
	void AutoStatScreenshot();

public:
	COptiStats();
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	bool IsActive();
};
