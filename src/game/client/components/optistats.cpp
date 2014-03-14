#include <engine/shared/config.h>
#include <engine/textrender.h>
#include <engine/graphics.h>
#include <engine/serverbrowser.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>

#include "optistats.h"

COptiStats::COptiStats()
{
	m_Mode = 0;
	m_StatsClientID = -1;
	m_ScreenshotTaken = false;
	m_ScreenshotTime = -1;
}

void COptiStats::OnReset()
{
	for(int i=0; i<MAX_CLIENTS; i++)
		m_pClient->m_aStats[i].Reset();
	m_Mode = 0;
	m_StatsClientID = -1;
	m_ScreenshotTaken = false;
	m_ScreenshotTime = -1;
}

void COptiStats::ConKeyReset(IConsole::IResult *pResult, void *pUserData)
{
	COptiStats *pStats = (COptiStats *)pUserData;
	pStats->OnReset();
}

void COptiStats::ConKeyStats(IConsole::IResult *pResult, void *pUserData)
{
	if(pResult->GetInteger(0) != 0)
		((COptiStats *)pUserData)->m_Mode = clamp(pResult->GetInteger(1),0, 3);
	else
		((COptiStats *)pUserData)->m_Mode = 0;
}

void COptiStats::ConKeyNext(IConsole::IResult *pResult, void *pUserData)
{
	COptiStats *pStats = (COptiStats *)pUserData;
	if(pStats->m_Mode != 2)
		return;

	if(pResult->GetInteger(0) == 0)
	{
		pStats->m_StatsClientID++;
		pStats->m_StatsClientID %= MAX_CLIENTS;
		pStats->CheckStatsClientID();
	}
}

void COptiStats::OnConsoleInit()
{
	Console()->Register("+stats", "i", CFGFLAG_CLIENT, ConKeyStats, this, "Show stats");
	Console()->Register("+next_stats", "", CFGFLAG_CLIENT, ConKeyNext, this, "Next player Stats");
	Console()->Register("stats_reset", "i", CFGFLAG_CLIENT, ConKeyReset, this, "Reset stats");
}

bool COptiStats::IsActive()
{
	return (m_Mode > 0);
}

void COptiStats::CheckStatsClientID()
{
	if(m_StatsClientID == -1)
		m_StatsClientID = m_pClient->m_Snap.m_LocalClientID;

	int Prev = m_StatsClientID;
	while(!m_pClient->m_aStats[m_StatsClientID].m_Active)
	{
		m_StatsClientID++;
		m_StatsClientID %= MAX_CLIENTS;
		if(m_StatsClientID == Prev)
		{
			m_StatsClientID = -1;
			m_Mode = 0;
			break;
		}
	}
}

void COptiStats::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		CGameClient::CClientStats *pStats = m_pClient->m_aStats;

		pStats[pMsg->m_Victim].m_aKillsBy[pMsg->m_Killer]++;
		
		pStats[pMsg->m_Victim].m_Deaths++;
		pStats[pMsg->m_Victim].m_CurrentSpree = 0;
		if(pMsg->m_Weapon >= 0)
			pStats[pMsg->m_Victim].m_aDeathsFrom[pMsg->m_Weapon]++;
		if(pMsg->m_ModeSpecial & 1)
			pStats[pMsg->m_Victim].m_DeathsCarrying++;
		if(pMsg->m_Victim != pMsg->m_Killer)
		{
			pStats[pMsg->m_Killer].m_Kills++;
			pStats[pMsg->m_Killer].m_CurrentSpree++;
			
			
			if(pStats[pMsg->m_Killer].m_CurrentSpree > pStats[pMsg->m_Killer].m_BestSpree)
				pStats[pMsg->m_Killer].m_BestSpree = pStats[pMsg->m_Killer].m_CurrentSpree;
			if(pMsg->m_Weapon >= 0)
				pStats[pMsg->m_Killer].m_aKillsWith[pMsg->m_Weapon]++;
			if(pMsg->m_ModeSpecial & 1)
				pStats[pMsg->m_Killer].m_CarriersKilled++;
			if(pMsg->m_ModeSpecial & 2)
				pStats[pMsg->m_Killer].m_KillsCarrying++;
		}
		else
			pStats[pMsg->m_Victim].m_Suicides++;
	}
	else if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		if(pMsg->m_ClientID < 0)
		{
			const char *p;
			const char *pLookFor = "flag was captured by '";
			if(str_find(pMsg->m_pMessage, pLookFor) != 0)
			{
				// server info
				CServerInfo CurrentServerInfo;
				Client()->GetServerInfo(&CurrentServerInfo);
			
				p = str_find(pMsg->m_pMessage, pLookFor);
				char aName[64];
				p += str_length(pLookFor);
				str_copy(aName, p, sizeof(aName));
				// remove capture time
				if(str_comp(aName+str_length(aName)-9, " seconds)") == 0)
				{
					char *c = aName+str_length(aName)-10;
					while(c > aName)
					{
						c--;
						if(*c == '(')
						{
							*(c-1) = 0;
							break;
						}
					}
				}
				// remove the ' at the end
				aName[str_length(aName)-1] = 0;
				
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(!m_pClient->m_aStats[i].m_Active)
						continue;

					if(str_comp(m_pClient->m_aClients[i].m_aName, aName) == 0)
					{
						m_pClient->m_aStats[i].m_FlagCaptures++;
						break;
					}
				}
			}
		}
	}
}

void COptiStats::OnRender()
{
	// auto stat screenshot stuff
	if(g_Config.m_OpStatScreenshot)
	{
		if(m_ScreenshotTime < 0 && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			m_ScreenshotTime = time_get()+time_freq()*3;
		
		if(m_ScreenshotTime > -1 && m_ScreenshotTime < time_get())
			m_Mode = 1;

		if(!m_ScreenshotTaken && m_ScreenshotTime > -1 && m_ScreenshotTime+time_freq()/5 < time_get())
		{
			AutoStatScreenshot();
			m_ScreenshotTaken = true;
		}
	}
	
	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);

	float w = 700.0f;
	
	switch(m_Mode)
	{
		case 1:
			RenderGlobalStats();
			break;
		case 2:
			RenderIndividualStats();
			break;
		case 3:
			RenderHeader(Width/2-w/2, 150.0f, w, "Stats");
			//RenderKillerStats();
			break;
	}
}

void COptiStats::RenderHeader(float x, float y, float w, char *pTitle)
{
	float h = 50.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f,0.0f,0.0f,0.7f);
	RenderTools()->DrawRoundRectExt(x, y, w, h, 17.0f, CUI::CORNER_T);
	Graphics()->QuadsEnd();

	CTextCursor Cursor;
	y += 10.0f;
	
	float woffset = 0;
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "Optimod");
	float tw = TextRender()->TextWidth(0, 20.0f, aBuf, -1);
	TextRender()->Text(0, x+w-tw-20.0f, y, 20.0f, aBuf, -1);
	woffset += tw + 10.0f;
	
	if (Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "DEMO %s", pTitle);
			TextRender()->SetCursor(&Cursor, x+20.0f, y-13.0f, 40.0f, TEXTFLAG_RENDER);
			Cursor.m_MaxLines = 1;
			Cursor.m_LineWidth = w-woffset-40.0f;

			TextRender()->TextEx(&Cursor, aBuf, -1);
		}
	}
	else
	{
		TextRender()->SetCursor(&Cursor, x+20.0f, y-13.0f, 40.0f, TEXTFLAG_RENDER);
		Cursor.m_MaxLines = 1;
		Cursor.m_LineWidth = w-woffset-40.0f;
	
		TextRender()->TextEx(&Cursor, pTitle, -1);
	}
}

void COptiStats::RenderGlobalStats()
{
	if(m_Mode != 1)
		return;

	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;
	float LineHeight = 45.0f;
	float TeeSizeMod = 0.8f;
	
	bool TeamPlay = m_pClient->m_Snap.m_pGameInfoObj ? m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS : false;
	
	
	const CNetObj_PlayerInfo *apPlayers[MAX_CLIENTS] = {0};
	int NumPlayers = 0;
	int i = 0;
	
	// sort red or dm players by score
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];

		if(!pInfo || !m_pClient->m_aStats[pInfo->m_ClientID].m_Active || pInfo->m_Team != TEAM_RED)
			continue;

		apPlayers[NumPlayers] = pInfo;
		NumPlayers++;
	}

	// sort blue players by score after
	if(m_pClient->m_Snap.m_pGameInfoObj && TeamPlay)
	{
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];

			if(!pInfo || !m_pClient->m_aStats[pInfo->m_ClientID].m_Active || pInfo->m_Team != TEAM_BLUE)
				continue;

			apPlayers[NumPlayers] = pInfo;
			NumPlayers++;
		}
	}
	
	float w = 400.0f;

	// calc Width
	for(i = 0; i < 8; i++)
		if(g_Config.m_OpStatboardInfos & (1<<i))
			w += 70;
	if (g_Config.m_OpStatboardInfos&OP_STATS_RATIO) w += 10;
	
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGGRABS)
		w += 70;
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGCAPTURES)
		w += 70;

	bool aDisplayWeapon[NUM_WEAPONS] = {false};
	if(g_Config.m_OpStatboardInfos & OP_STATS_WEAPS)
	{
		w += 10;
		for(i=0; i<NumPlayers; i++)
		{
			const CGameClient::CClientStats pStats = m_pClient->m_aStats[apPlayers[i]->m_ClientID];
			for(int j=0; j<NUM_WEAPONS; j++)
				aDisplayWeapon[j] = aDisplayWeapon[j] || pStats.m_aKillsWith[j] || pStats.m_aDeathsFrom[j];
		}
		for(i=0; i<NUM_WEAPONS; i++)
			if(aDisplayWeapon[i])
				w += 80;
	}
	
	float x = Width/2-w/2;
	float y = 200.0f;
	float h1 = 15.0f + LineHeight;
	float h2 = 15.0f + LineHeight;
	RenderHeader(x, y-50.0f, w, "Global stats");
	// calc height
	if (m_pClient->m_Snap.m_aTeamSize[TEAM_RED] > 0)
	{
		h1 = 15.0f+m_pClient->m_Snap.m_aTeamSize[TEAM_RED]*LineHeight;
	}
	if (TeamPlay && m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE] > 0)
	{
		h2 = 15.0f+m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE]*LineHeight;
	}
	
	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	if (TeamPlay)
		Graphics()->SetColor(0.6f, 0.0f, 0.0f, 0.6f);
	else
		Graphics()->SetColor(0.3f, 0.3f, 0.3f, 0.6f);

	RenderTools()->DrawRoundRectExt(x, y, w, 85.0f, 17.0f, 0);
	if (TeamPlay)
	{
		Graphics()->SetColor(0.1f, 0.0f, 0.0f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h1, 17.0f, 0);
	}
	else
	{
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h1, 17.0f, CUI::CORNER_B);
	}		

	if (TeamPlay)
	{
		Graphics()->SetColor(0.0f, 0.0f, 0.6f, 0.6f);
		RenderTools()->DrawRoundRectExt(x, y+85.0f+h1, w, 85.0f, 17.0f, 0);
		Graphics()->SetColor(0.0f, 0.0f, 0.1f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+170.0f+h1, w, h2, 17.0f, CUI::CORNER_B);
	}
	Graphics()->QuadsEnd();

	// render title
	if (TeamPlay)
	{
		float TitleFontsize = 40.0f;
		char aBufTitle[128];
		str_format(aBufTitle, sizeof(aBufTitle), "%s (%d)",Localize("Red team") , m_pClient->m_Snap.m_aTeamSize[TEAM_RED]);
		TextRender()->Text(0, x+20.0f, y, TitleFontsize, aBufTitle, -1);
		
		str_format(aBufTitle, sizeof(aBufTitle), "%s (%d)",Localize("Blue team") , m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE]);
		TextRender()->Text(0, x+20.0f, y+85.0f+h1, TitleFontsize, aBufTitle, -1);
	}
	else
	{
		float TitleFontsize = 40.0f;
		char aBufTitle[128];
		str_format(aBufTitle, sizeof(aBufTitle), "%s (%d)",Localize("Players") , m_pClient->m_Snap.m_aTeamSize[TEAM_RED]);
		TextRender()->Text(0, x+20.0f, y, TitleFontsize, aBufTitle, -1);
	}

	float tw;
	float px = 425;
	
	x += 10.0f;
	float ScoreOffset = x+10.0f, ScoreLength = 60.0f;
	float TeeOffset = ScoreOffset+ScoreLength, TeeLength = 60*TeeSizeMod;
	float NameOffset = TeeOffset+TeeLength, NameLength = 300.0f-TeeLength;

	y += 50.0f;
	
	float HeadlineFontsize = 22.0f;
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Score"), -1);
	
	TextRender()->Text(0, ScoreOffset+ScoreLength-tw, y, HeadlineFontsize, Localize("Score"), -1);
	TextRender()->Text(0, NameOffset, y, HeadlineFontsize, Localize("Name"), -1);
	if (TeamPlay)
	{
		TextRender()->Text(0, ScoreOffset+ScoreLength-tw, y+85.0f+h1, HeadlineFontsize, Localize("Score"), -1);
		TextRender()->Text(0, NameOffset, y+85.0f+h1, HeadlineFontsize, Localize("Name"), -1);
	}
	
	
	const char *apHeaders[] = { "K", "D", "S", "K/D", "N", "KPM", "SP", "BSP", "G" };
	for(i = 0; i < 8; i++)
		if(g_Config.m_OpStatboardInfos & (1<<i))
		{
			tw = TextRender()->TextWidth(0, HeadlineFontsize, apHeaders[i], -1);
			TextRender()->Text(0, x+px-tw, y, HeadlineFontsize, apHeaders[i], -1);
			if (TeamPlay)
				TextRender()->Text(0, x+px-tw, y+85.0f+h1, HeadlineFontsize, apHeaders[i], -1);
			px += (g_Config.m_OpStatboardInfos&OP_STATS_RATIO && (1<<i+1) == OP_STATS_RATIO ) ? 80 : 70;
		}
	if (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGGRABS)
	{
		tw = TextRender()->TextWidth(0, HeadlineFontsize, apHeaders[8], -1);
		TextRender()->Text(0, x+px-tw, y, HeadlineFontsize, apHeaders[8], -1);
		if (TeamPlay)
			TextRender()->Text(0, x+px-tw, y+85.0f+h1, HeadlineFontsize, apHeaders[8], -1);
		px += 70;
	}
	
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGCAPTURES)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(0.78f);
		RenderTools()->SelectSprite(SPRITE_FLAG_RED);
		RenderTools()->DrawSprite(x+px, y+15, 48);
		if (TeamPlay)
		{
			RenderTools()->SelectSprite(SPRITE_FLAG_BLUE);
			RenderTools()->DrawSprite(x+px, y+100.0f+h1, 48);
		}
		Graphics()->QuadsEnd();
		px += 70;
	}
	
	if(g_Config.m_OpStatboardInfos & OP_STATS_WEAPS)
	{
		px += 50;
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		for(i = 0, px-=40; i < NUM_WEAPONS; i++)
		{
			if(!aDisplayWeapon[i])
				continue;

			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
			if(i == 0)
				RenderTools()->DrawSprite(x+px, y+10, g_pData->m_Weapons.m_aId[i].m_VisualSize*0.8);
			else
				RenderTools()->DrawSprite(x+px, y+10, g_pData->m_Weapons.m_aId[i].m_VisualSize);
				
			if (TeamPlay)
			{
				if(i == 0)
					RenderTools()->DrawSprite(x+px, y+95.0f+h1, g_pData->m_Weapons.m_aId[i].m_VisualSize*0.8);
				else
					RenderTools()->DrawSprite(x+px, y+95.0f+h1, g_pData->m_Weapons.m_aId[i].m_VisualSize);
			}
			px += 80;
		}
		Graphics()->QuadsEnd();
	}
	
	y += HeadlineFontsize*2.0f;
	float FontSize = 24.0f;
	for(int j = 0; j < NumPlayers; j++)
	{
		const CNetObj_PlayerInfo *pInfo = apPlayers[j];
		const CGameClient::CClientStats Stats = m_pClient->m_aStats[pInfo->m_ClientID];

		if (j == m_pClient->m_Snap.m_aTeamSize[TEAM_RED]) y += 100.0f+HeadlineFontsize*2.0f-(j ? LineHeight : 0);

		// background so it's easy to find the local player or the followed one in spectator mode
		if(pInfo->m_Local || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
		{
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
			RenderTools()->DrawRoundRect(x, y, w-20.0f, LineHeight-5.0f, 10.0f);
			Graphics()->QuadsEnd();
		}
		
		CTextCursor Cursor;
		char aBuf[128];
		// score
		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Score, -999, 999));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, ScoreOffset+ScoreLength-tw, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ScoreLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);

		// flag
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS &&
			m_pClient->m_Snap.m_pGameDataObj && (m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed == pInfo->m_ClientID ||
			m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue == pInfo->m_ClientID))
		{
			Graphics()->BlendNormal();
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
			Graphics()->QuadsBegin();

			RenderTools()->SelectSprite(pInfo->m_Team==TEAM_RED ? SPRITE_FLAG_BLUE : SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

			float Size = LineHeight;
			IGraphics::CQuadItem QuadItem(TeeOffset+0.0f, y-5.0f, Size/2.0f, Size);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}

		// avatar
		CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
		TeeInfo.m_Size *= TeeSizeMod;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TeeOffset+TeeLength/2, y+LineHeight/2));

		// name
		TextRender()->SetCursor(&Cursor, NameOffset, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = NameLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);

		px = 425;

		if(g_Config.m_OpStatboardInfos & OP_STATS_KILLS)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_Kills);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_DEATHS)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_Deaths);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_SUICIDES)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_Suicides);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_RATIO)
		{
			px += 10;
			if(Stats.m_Deaths == 0)
				str_format(aBuf, sizeof(aBuf), "--");
			else
				str_format(aBuf, sizeof(aBuf), "%.2f", (float)(Stats.m_Kills)/Stats.m_Deaths);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_NET)
		{
			str_format(aBuf, sizeof(aBuf), "%+d", Stats.m_Kills-Stats.m_Deaths);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_KPM)
		{
			float Kpm = (float)(Stats.m_Kills*60)/((float)(Client()->GameTick()-Stats.m_JoinDate)/Client()->GameTickSpeed());
			str_format(aBuf, sizeof(aBuf), "%.1f", Kpm);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_SPREE)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_CurrentSpree);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(g_Config.m_OpStatboardInfos & OP_STATS_BESTSPREE)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_BestSpree);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGGRABS)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_FlagGrabs);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS && g_Config.m_OpStatboardInfos&OP_STATS_FLAGCAPTURES)
		{
			str_format(aBuf, sizeof(aBuf), "%d", Stats.m_FlagCaptures);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x-tw+px, y, FontSize, aBuf, -1);
			px += 70;
		}
		px += 50;
		for(i = 0, px=px-40; i < NUM_WEAPONS; i++)
		{
			if(!aDisplayWeapon[i])
				continue;

			str_format(aBuf, sizeof(aBuf), "%d/%d", Stats.m_aKillsWith[i], Stats.m_aDeathsFrom[i]);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->Text(0, x+px-tw/2, y, FontSize, aBuf, -1);
			px += 80;
		}
		y += LineHeight;
	}
}

void COptiStats::RenderIndividualStats()
{
	if(m_Mode != 2)
		return;
	CheckStatsClientID();
	if(m_Mode != 2)
		return;
	int m_ClientID = m_StatsClientID;
	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;
	float w = 1200.0f;
	float x = Width/2-w/2;
	float y = 100.0f;
	float xo = 200.0f;
	float HeadlineFontsize = 30.0f;
	float LineHeight = 40.0f;
	const CGameClient::CClientStats m_aStats = m_pClient->m_aStats[m_ClientID];

	Graphics()->MapScreen(0, 0, Width, Height);

	// header with name and score
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->DrawRoundRect(x-10.f, y-10.f, w, 120.0f, 17.0f);
	Graphics()->QuadsEnd();

	CTeeRenderInfo Teeinfo = m_pClient->m_aClients[m_ClientID].m_RenderInfo;
	Teeinfo.m_Size *= 1.5f;
	RenderTools()->RenderTee(CAnimState::GetIdle(), &Teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(x+xo+32, y+36));
	TextRender()->Text(0, x+xo+128, y, 48.0f, m_pClient->m_aClients[m_ClientID].m_aName, -1);

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Score"), m_pClient->m_Snap.m_paPlayerInfos[m_ClientID]->m_Score);
	TextRender()->Text(0, x+xo, y+64, HeadlineFontsize, aBuf, -1);
	int Seconds = (float)(Client()->GameTick()-m_aStats.m_JoinDate)/Client()->GameTickSpeed();
	str_format(aBuf, sizeof(aBuf), "%s: %02d:%02d", Localize("Time played"), Seconds/60, Seconds%60);
	TextRender()->Text(0, x+xo+256, y+64, HeadlineFontsize, aBuf, -1);

	y += 150.0f;

	// Kills, etc. stats
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->DrawRoundRect(x-10.f, y-10.f, w, 100.0f, 17.0f);
	Graphics()->QuadsEnd();

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f,1.0f,1.0f,1.0f);
	RenderTools()->SelectSprite(SPRITE_EYES);
	IGraphics::CQuadItem QuadItem(x+xo/2, y+40, 128, 128);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();

	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Kills"), m_aStats.m_Kills);
	TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Deaths"), m_aStats.m_Deaths);
	TextRender()->Text(0, x+xo+200.0f, y, HeadlineFontsize, aBuf, -1);
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Suicides"), m_aStats.m_Suicides);
	TextRender()->Text(0, x+xo+400.0f, y, HeadlineFontsize, aBuf, -1);
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Spree"), m_aStats.m_CurrentSpree);
	TextRender()->Text(0, x+xo+600.0f, y, HeadlineFontsize, aBuf, -1);
	y += LineHeight;

	if(m_aStats.m_Deaths == 0)
		str_format(aBuf, sizeof(aBuf), "%s: --", Localize("K/D"));
	else
		str_format(aBuf, sizeof(aBuf), "%s: %.2f", Localize("K/D"), (float)(m_aStats.m_Kills)/m_aStats.m_Deaths);
	TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Net"), m_aStats.m_Kills-m_aStats.m_Deaths);
	TextRender()->Text(0, x+xo+200.0f, y, HeadlineFontsize, aBuf, -1);
	float Kpm = (float)(m_aStats.m_Kills*60)/((float)(Client()->GameTick()-m_aStats.m_JoinDate)/Client()->GameTickSpeed());
	str_format(aBuf, sizeof(aBuf), "%s: %.1f", Localize("KPM"), Kpm);
	TextRender()->Text(0, x+xo+400.0f, y, HeadlineFontsize, aBuf, -1);
	str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Best spree"), m_aStats.m_BestSpree);
	TextRender()->Text(0, x+xo+600.0f, y, HeadlineFontsize, aBuf, -1);
	y+= LineHeight + 30.0f;

	// Weapon stats
	bool aDisplayWeapon[NUM_WEAPONS] = {false};
	int NumWeaps = 0;
	for(int i=0; i<NUM_WEAPONS; i++)
		if(m_aStats.m_aKillsWith[i] || m_aStats.m_aDeathsFrom[i])
		{
			aDisplayWeapon[i] = true;
			NumWeaps++;
		}

	if(NumWeaps)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(0,0,0,0.5f);
		RenderTools()->DrawRoundRect(x-10.f, y-10.f, w, LineHeight*(1+NumWeaps)+20.0f, 17.0f);
		Graphics()->QuadsEnd();

		TextRender()->Text(0, x+xo, y, HeadlineFontsize, Localize("Kills"), -1);
		TextRender()->Text(0, x+xo+200.0f, y, HeadlineFontsize, Localize("Deaths"), -1);
		y += LineHeight;

		for(int i=0; i<NUM_WEAPONS; i++)
		{
			if(!aDisplayWeapon[i])
				continue;

			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody, 0);
			RenderTools()->DrawSprite(x+xo/2, y+24, g_pData->m_Weapons.m_aId[i].m_VisualSize);
			Graphics()->QuadsEnd();

			str_format(aBuf, sizeof(aBuf), "%d", m_aStats.m_aKillsWith[i]);
			TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
			str_format(aBuf, sizeof(aBuf), "%d", m_aStats.m_aDeathsFrom[i]);
			TextRender()->Text(0, x+xo+200.0f, y, HeadlineFontsize, aBuf, -1);
			y += LineHeight;
		}
		y += 30.0f;
	}

	// Flag stats
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS)
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(0,0,0,0.5f);
		RenderTools()->DrawRoundRect(x-10.f, y-10.f, w, LineHeight*5+20.0f, 17.0f);
		Graphics()->QuadsEnd();

		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_FLAG_RED);
		RenderTools()->DrawSprite(x+xo/2, y+100.0f, 192);
		Graphics()->QuadsEnd();

		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Grabs"), m_aStats.m_FlagGrabs);
		TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
		y += LineHeight;
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Captures"), m_aStats.m_FlagCaptures);
		TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
		y += LineHeight;
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Kills holding flag"), m_aStats.m_KillsCarrying);
		TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
		y += LineHeight;
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Deaths with flag"), m_aStats.m_DeathsCarrying);
		TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
		y += LineHeight;
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Carriers killed"), m_aStats.m_CarriersKilled);
		TextRender()->Text(0, x+xo, y, HeadlineFontsize, aBuf, -1);
		y += LineHeight;
	}
}

void COptiStats::AutoStatScreenshot()
{
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		Client()->AutoStatScreenshot_Start();
}
