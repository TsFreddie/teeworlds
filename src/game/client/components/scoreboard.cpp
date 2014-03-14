/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/localization.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>

#include "scoreboard.h"

#include <engine/serverbrowser.h> // For Info

CScoreboard::CScoreboard()
{
	OnReset();
}

void CScoreboard::ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData)
{
	((CScoreboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CScoreboard::OnReset()
{
	m_Active = false;
}

void CScoreboard::OnRelease()
{
	m_Active = false;
}

void CScoreboard::OnConsoleInit()
{
	Console()->Register("+scoreboard", "", CFGFLAG_CLIENT, ConKeyScoreboard, this, "Show scoreboard");
}

void CScoreboard::RenderInfos(float x, float y, float w)
{
	float h = 50.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f,0.0f,0.0f,0.7f);
	RenderTools()->DrawRoundRectExt(x, y, w, h, 17.0f, CUI::CORNER_T);
	Graphics()->QuadsEnd();
	
	// render goals
	y += 10.0f;
	if(m_pClient->m_Snap.m_pGameInfoObj)
	{
		float woffset = 0;
		if(m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum && m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "%s %d/%d", Localize("Round"), m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent, m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum);
			float tw = TextRender()->TextWidth(0, 20.0f, aBuf, -1);
			TextRender()->Text(0, x+w-tw-20.0f, y, 20.0f, aBuf, -1);
			woffset += tw + 10.0f;
		}
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), Localize("Time limit: %d min"), m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit);
			float tw = TextRender()->TextWidth(0, 20.0f, aBuf, -1);
			TextRender()->Text(0, x+w-tw-20.0f-woffset, y, 20.0f, aBuf, -1);
			woffset += tw + 10.0f;
		}
		if(m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Score limit"), m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit);
			float tw = TextRender()->TextWidth(0, 20.0f, aBuf, -1);
			TextRender()->Text(0, x+w-tw-20.0f-woffset, y, 20.0f, aBuf, -1);
			woffset += tw + 10.0f;
		}
		
		float woffset2 = 0;
		CTextCursor Cursor;
		
		if (Client()->State() == IClient::STATE_DEMOPLAYBACK)
		{
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf), "DEMO");
				TextRender()->SetCursor(&Cursor, x+20.0f, y-13.0f, 40.0f, TEXTFLAG_RENDER);
				Cursor.m_MaxLines = 1;
				Cursor.m_LineWidth = w-woffset-40.0f;
		
				TextRender()->TextEx(&Cursor, aBuf, -1);

				float tw = TextRender()->TextWidth(0, 40.0f, aBuf, -1);
				woffset2 += tw + 10.0f;
			}
		}
		else
		{
			CServerInfo CurrentServerInfo;
			Client()->GetServerInfo(&CurrentServerInfo);

			if(CurrentServerInfo.m_aGameType)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf), "%s", CurrentServerInfo.m_aGameType);
				TextRender()->SetCursor(&Cursor, x+20.0f, y-13.0f, 40.0f, TEXTFLAG_RENDER);
				Cursor.m_MaxLines = 1;
				Cursor.m_LineWidth = w-woffset-40.0f;
		
				TextRender()->TextEx(&Cursor, aBuf, -1);

				float tw = TextRender()->TextWidth(0, 40.0f, aBuf, -1);
				woffset2 += tw + 10.0f;
			}
		
			if(CurrentServerInfo.m_aMap)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf), "%s", CurrentServerInfo.m_aMap);
			
				TextRender()->SetCursor(&Cursor, x+woffset2+20.0f, y, 25.0f, TEXTFLAG_RENDER);
				Cursor.m_LineWidth = w-woffset2-woffset-40.0f;
				Cursor.m_MaxLines = 1;
			
				if (Cursor.m_LineWidth > 0)
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
		}

	}
}

void CScoreboard::RenderSpectators(float x, float y, float w, int NumSpectators, bool TeamPlay)
{
	float LineHeight = 45.0f;
	float TeeSizeMod = 0.8f;
	float h = 15.0f + LineHeight;
	if(NumSpectators)
		h = 15.0f+(TeamPlay ? (NumSpectators+1)/2 : NumSpectators) *LineHeight;
	else
		return;
	
	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	if (TeamPlay)
		Graphics()->SetColor(0.6f, 0.0f, 0.6f, 0.6f);
	else
		Graphics()->SetColor(0.0f, 0.6f, 0.0f, 0.6f);
		
	RenderTools()->DrawRoundRectExt(x, y, w, 85.0f, 17.0f, 0);
	
	if (TeamPlay)
		Graphics()->SetColor(0.1f, 0.0f, 0.1f, 0.5f);
	else
		Graphics()->SetColor(0.0f, 0.1f, 0.0f, 0.5f);
	RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h, 17.0f, CUI::CORNER_B);
	
	if (TeamPlay)
	{
		Graphics()->SetColor(0.1f, 0.0f, 0.1f, 0.5f);
		RenderTools()->DrawRoundRectExt(x+w/2-1.0f, y+85.0f, 2.0f, h, 0, 0);
	}

	Graphics()->QuadsEnd();
	
	float TitleFontsize = 40.0f;
	char aBufTitle[128];
	str_format(aBufTitle, sizeof(aBufTitle), "%s (%d)", Localize("Spectators"), NumSpectators);
	
	float tw = TextRender()->TextWidth(0, TitleFontsize, aBufTitle, -1);
	TextRender()->Text(0, x+(w-tw)/2, y, TitleFontsize, aBufTitle, -1);
	
	float TeeOffset = x+80.0f, TeeLength = 60*TeeSizeMod;
	float NameOffset = TeeOffset+TeeLength, NameLength = 300.0f-TeeLength;
	float CountryOffset = x+620.0f-(LineHeight-TeeSizeMod*5.0f)*2.0f, CountryLength = (LineHeight-TeeSizeMod*5.0f)*2.0f;
	float ClanOffset = x+380.0f, ClanLength = 230.0f-CountryLength;

	// render headlines
	y += 50.0f;
	float HeadlineFontsize = 22.0f;
	TextRender()->Text(0, NameOffset, y, HeadlineFontsize, Localize("Name"), -1);
	if (TeamPlay) 
		TextRender()->Text(0, w/2+NameOffset, y, HeadlineFontsize, Localize("Name"), -1);

	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Clan"), -1);
	TextRender()->Text(0, ClanOffset+ClanLength/2-tw/2, y, HeadlineFontsize, Localize("Clan"), -1);
	if (TeamPlay) 
		TextRender()->Text(0, w/2+ClanOffset+ClanLength/2-tw/2, y, HeadlineFontsize, Localize("Clan"), -1);

	// render player entries
	y += HeadlineFontsize*2.0f;
	float FontSize = 24.0f;
	CTextCursor Cursor;	
	
	// For TeamPlay spectaotrs
	int TempCount = 0; 
	float TeamOffset = 0;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// make sure that we render the spectator
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != TEAM_SPECTATORS)
			continue;
			
		if (TeamPlay) 
			TeamOffset = TempCount % 2 ?  w/2 : 0;

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
			IGraphics::CQuadItem QuadItem(TeamOffset+TeeOffset+0.0f, y-5.0f, Size/2.0f, Size);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}

		// avatar
		CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
		TeeInfo.m_Size *= TeeSizeMod;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TeamOffset+TeeOffset+TeeLength/2, y+LineHeight/2));

		// name
		TextRender()->SetCursor(&Cursor, TeamOffset+NameOffset, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = NameLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);

		// clan
		tw = TextRender()->TextWidth(0, TeamOffset+FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
		TextRender()->SetCursor(&Cursor, ClanOffset+ClanLength/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ClanLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);

		// country flag
		vec4 Color(1.0f, 1.0f, 1.0f, 0.5f);
		m_pClient->m_pCountryFlags->Render(m_pClient->m_aClients[pInfo->m_ClientID].m_Country, &Color,
											TeamOffset+CountryOffset, y+(TeeSizeMod*5.0f)/2.0f, CountryLength, LineHeight-TeeSizeMod*5.0f);

		
		if (TeamPlay)
		{
			y += TempCount % 2 ? LineHeight : 0;
			TempCount++;
		}
		else
			y += LineHeight;
	}
}

float CScoreboard::RenderScoreboard(float x, float y, float w, int Team, const char *pTitle, bool HasSpectator,  bool TeamPlay)
{
	if(Team == TEAM_SPECTATORS)
		return 0;
		
	float LineHeight = 45.0f;
	float TeeSizeMod = 0.8f;
	float h = 15.0f + LineHeight;
	int TeamSize = 0;
	
	//Fix Team
	if (TeamPlay)
		TeamSize = max(m_pClient->m_Snap.m_aTeamSize[0], m_pClient->m_Snap.m_aTeamSize[1]);
	else
		TeamSize = m_pClient->m_Snap.m_aTeamSize[Team];

	if (TeamSize > 0)
	{
		h = 15.0f+TeamSize*LineHeight;
	}

	//
	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	if (TeamPlay)
		Team ? Graphics()->SetColor(0.0f, 0.0f, 0.6f, 0.6f) : Graphics()->SetColor(0.6f, 0.0f, 0.0f, 0.6f);
	else
		Graphics()->SetColor(0.3f, 0.3f, 0.3f, 0.6f);

	RenderTools()->DrawRoundRectExt(x, y, w, 85.0f, 17.0f, 0);
	if (TeamPlay)
	{
		Team ? Graphics()->SetColor(0.0f, 0.0f, 0.1f, 0.5f):Graphics()->SetColor(0.1f, 0.0f, 0.0f, 0.5f);
		Team ? RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h, 17.0f, HasSpectator ? 0 : CUI::CORNER_BR) : RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h, 17.0f, HasSpectator ? 0 : CUI::CORNER_BL);
	}
	else
	{
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+85.0f, w, h, 17.0f, HasSpectator ? 0 : CUI::CORNER_B);
	}		
	
	Graphics()->QuadsEnd();

	// render title
	float TitleFontsize = 40.0f;
	if(!pTitle)
	{
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			pTitle = Localize("Game over");
		else
			pTitle = Localize("Score board");
	}
	char aBufTitle[128];
	str_format(aBufTitle, sizeof(aBufTitle), "%s (%d)", pTitle, m_pClient->m_Snap.m_aTeamSize[Team]);
	TextRender()->Text(0, x+20.0f, y, TitleFontsize, aBufTitle, -1);

	char aBuf[128] = {0};
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		if(m_pClient->m_Snap.m_pGameDataObj)
		{
			int Score = Team == TEAM_RED ? m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed : m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
	}
	else
	{
		if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW &&
			m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID])
		{
			int Score = m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID]->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
		else if(m_pClient->m_Snap.m_pLocalInfo)
		{
			int Score = m_pClient->m_Snap.m_pLocalInfo->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
	}
	float tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
	TextRender()->Text(0, x+w-tw-20.0f, y, TitleFontsize, aBuf, -1);

	x += 10.0f;

	float ScoreOffset = x+10.0f, ScoreLength = 60.0f;
	float TeeOffset = ScoreOffset+ScoreLength, TeeLength = 60*TeeSizeMod;
	float NameOffset = TeeOffset+TeeLength, NameLength = 300.0f-TeeLength;
	float PingOffset = x+610.0f, PingLength = 65.0f;
	float CountryOffset = PingOffset-(LineHeight-TeeSizeMod*5.0f)*2.0f, CountryLength = (LineHeight-TeeSizeMod*5.0f)*2.0f;
	float ClanOffset = x+370.0f, ClanLength = 230.0f-CountryLength;

	// render headlines
	y += 50.0f;
	float HeadlineFontsize = 22.0f;
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Score"), -1);
	TextRender()->Text(0, ScoreOffset+ScoreLength-tw, y, HeadlineFontsize, Localize("Score"), -1);

	TextRender()->Text(0, NameOffset, y, HeadlineFontsize, Localize("Name"), -1);

	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Clan"), -1);
	TextRender()->Text(0, ClanOffset+ClanLength/2-tw/2, y, HeadlineFontsize, Localize("Clan"), -1);

	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Ping"), -1);
	TextRender()->Text(0, PingOffset+PingLength-tw, y, HeadlineFontsize, Localize("Ping"), -1);

	// render player entries
	y += HeadlineFontsize*2.0f;
	float FontSize = 24.0f;
	CTextCursor Cursor;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// make sure that we render the correct team
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;

		// background so it's easy to find the local player or the followed one in spectator mode
		if(pInfo->m_Local || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
		{
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
			RenderTools()->DrawRoundRect(x, y, w-20.0f, LineHeight-5.0f, 10.0f);
			Graphics()->QuadsEnd();
		}

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

		// clan
		tw = TextRender()->TextWidth(0, FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
		TextRender()->SetCursor(&Cursor, ClanOffset+ClanLength/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ClanLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);

		// country flag
		vec4 Color(1.0f, 1.0f, 1.0f, 0.5f);
		m_pClient->m_pCountryFlags->Render(m_pClient->m_aClients[pInfo->m_ClientID].m_Country, &Color,
											CountryOffset, y+(TeeSizeMod*5.0f)/2.0f, CountryLength, LineHeight-TeeSizeMod*5.0f);

		// ping
		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Latency, 0, 1000));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, PingOffset+PingLength-tw, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = PingLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);

		y += LineHeight;
	}
	
	return h+85.0f;
}

void CScoreboard::RenderRecordingNotification(float x)
{
	if(!m_pClient->DemoRecorder()->IsRecording())
		return;

	//draw the box
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.4f);
	RenderTools()->DrawRoundRectExt(x, 0.0f, 180.0f, 50.0f, 15.0f, CUI::CORNER_B);
	Graphics()->QuadsEnd();

	//draw the red dot
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	RenderTools()->DrawRoundRect(x+20, 15.0f, 20.0f, 20.0f, 10.0f);
	Graphics()->QuadsEnd();

	//draw the text
	char aBuf[64];
	int Seconds = m_pClient->DemoRecorder()->Length();
	str_format(aBuf, sizeof(aBuf), Localize("REC %3d:%02d"), Seconds/60, Seconds%60);
	TextRender()->Text(0, x+50.0f, 10.0f, 20.0f, aBuf, -1);
}

void CScoreboard::OnRender()
{
	if(!Active())
		return;

	// if the score board is active, then we should clear the motd message aswell
	if(m_pClient->m_pMotd->IsActive())
		m_pClient->m_pMotd->Clear();


	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);

	float w = 700.0f;
	
	if(m_pClient->m_Snap.m_pGameInfoObj)
	{
		int NumSpectators = 0;
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paPlayerInfos[i];
			if(pInfo && pInfo->m_Team == TEAM_SPECTATORS)
				NumSpectators++;
		}
		
		if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS))
		{
			RenderInfos(Width/2-w/2, 150.0f, w);
			float h = RenderScoreboard(Width/2-w/2, 200.0f, w, 0, 0,NumSpectators ? true : false);
			RenderSpectators(Width/2-w/2, 200.0f+h, w, NumSpectators);
		}	
		else
		{
			const char *pRedClanName = GetClanName(TEAM_RED);
			const char *pBlueClanName = GetClanName(TEAM_BLUE);

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER && m_pClient->m_Snap.m_pGameDataObj)
			{
				char aText[256];
				str_copy(aText, Localize("Draw!"), sizeof(aText));

				if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue)
				{
					if(pRedClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pRedClanName);
					else
						str_copy(aText, Localize("Red team wins!"), sizeof(aText));
				}
				else if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed)
				{
					if(pBlueClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pBlueClanName);
					else
						str_copy(aText, Localize("Blue team wins!"), sizeof(aText));
				}

				float w = TextRender()->TextWidth(0, 86.0f, aText, -1);
				TextRender()->Text(0, Width/2-w/2, 39, 86.0f, aText, -1);
			}
			
			RenderInfos(Width/2-w, 150.0f, w*2);
			RenderScoreboard(Width/2-w, 200.0f, w, TEAM_RED, pRedClanName ? pRedClanName : Localize("Red team"),NumSpectators ? true : false, true);
			float h = RenderScoreboard(Width/2, 200.0f, w, TEAM_BLUE, pBlueClanName ? pBlueClanName : Localize("Blue team"),NumSpectators ? true : false, true);
			RenderSpectators(Width/2-w, 200.0f+h, w*2, NumSpectators, true);
			
		}
	}
	//150+760+10
	RenderRecordingNotification((Width/7)*4);
}

bool CScoreboard::Active()
{
	// if we activly wanna look on the scoreboard
	if(m_Active)
		return true;

	if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
	{
		// we are not a spectator, check if we are dead
		if(!m_pClient->m_Snap.m_pLocalCharacter)
			return true;
	}

	// if the game is over
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
		return true;

	return false;
}

const char *CScoreboard::GetClanName(int Team)
{
	int ClanPlayers = 0;
	const char *pClanName = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;

		if(!pClanName)
		{
			pClanName = m_pClient->m_aClients[pInfo->m_ClientID].m_aClan;
			ClanPlayers++;
		}
		else
		{
			if(str_comp(m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, pClanName) == 0)
				ClanPlayers++;
			else
				return 0;
		}
	}

	if(ClanPlayers > 1 && pClanName[0])
		return pClanName;
	else
		return 0;
}
