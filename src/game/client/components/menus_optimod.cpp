/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

//#include <base/math.h>

#include <engine/graphics.h>
//#include <engine/textrender.h>
#include <engine/shared/config.h>

//#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

//#include <game/client/ui.h>
//#include <game/client/render.h>
//#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/localization.h>

#include "menus.h"
#include "skins.h"


void CMenus::RenderSettingsOptimod(CUIRect MainView)
{
	CUIRect Button;
	MainView.HSplitTop(20.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	
	//if(DoButton_CheckBox(&g_Config.m_GameShowGhost, &s_Fade[8], Localize("Show ghost"), g_Config.m_GameShowGhost, &Button))
	if(DoButton_CheckBox_Number(&g_Config.m_GameShowGhost, g_Config.m_GameShowGhost==0?Localize("Don't show ghost"):g_Config.m_GameShowGhost==1?Localize("Show ghost"):Localize("Show only ghost"), g_Config.m_GameShowGhost, &Button))
	{		
		g_Config.m_GameShowGhost = (g_Config.m_GameShowGhost+1)%3;
	}
	
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GameAntiPingGrenade, Localize("Grenade Antiping"), g_Config.m_GameAntiPingGrenade, &Button))
		g_Config.m_GameAntiPingGrenade ^=1;
		
	MainView.HSplitTop(20.0f, &Button, &MainView);	
	if(DoButton_CheckBox(&g_Config.m_GameAntiPing, Localize("Antiping"), g_Config.m_GameAntiPing, &Button))
		g_Config.m_GameAntiPing = g_Config.m_GameAntiPing?0:1;
	
	if(g_Config.m_GameAntiPing == 1 || g_Config.m_GameShowGhost)
	{
		CUIRect aRects, Label;
		//MainView.VSplitLeft(15.0f, 0, &MainView);
		MainView.HSplitTop(20.0f, &Label, &MainView);	
		
		if(DoButton_CheckBox(&g_Config.m_GameAntiPingTeeColor, Localize("Use tee color"), g_Config.m_GameAntiPingTeeColor, &Label))
			g_Config.m_GameAntiPingTeeColor ^=1;
			
		MainView.HSplitTop(20.0f, &Label, &aRects);	
		
		int *paColors;
		paColors = &g_Config.m_GamePlayerColorGhost;

		const char *paLabels[] = {
			Localize("Hue"),
			Localize("Sat."),
			Localize("Lht."),
			Localize("Alpha")};
		static int s_aColorSlider[3] = {0};

		UI()->DoLabelScaled(&Label, Localize("Ghost color"), 14.0f, -1);
		aRects.VSplitLeft(20.0f, 0, &aRects);
		//aRects.HSplitTop(2.5f, 0, &aRects);

		static float s_FadeScroll2[4][3] = {{0}};
		int PrevColor = *paColors;
		int Color = 0;
		if(g_Config.m_GameAntiPingTeeColor == 0)
		{
			for(int s = 0; s < 3; s++)
			{
				aRects.HSplitTop(20.0f, &Label, &aRects);
				Label.VSplitLeft(100.0f, &Label, &Button);
				Button.HMargin(2.0f, &Button);

				float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
				k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
				Color <<= 8;
				Color += clamp((int)(k*255), 0, 255);
				UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
			}
		}
		
		*paColors = Color;

		aRects.HSplitTop(20.0f, &Label, &aRects);
		Label.VSplitLeft(100.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);

		float k = (g_Config.m_GamePlayerColorGhostAlpha) / 255.0f;
		k = DoScrollbarH(&g_Config.m_GamePlayerColorGhostAlpha, &Button, k);
		g_Config.m_GamePlayerColorGhostAlpha = (int)(k*255.0f);
		UI()->DoLabelScaled(&Label, paLabels[3], 14.0f, -1);	
		
		// skin info
		const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin));
		CTeeRenderInfo OwnSkinInfo;
		
		if(g_Config.m_GameAntiPingTeeColor == 1)
		{
			if(g_Config.m_PlayerUseCustomColor)
			{
				OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
				OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
				OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
			}
			else
			{
				OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
				OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			OwnSkinInfo.m_ColorBody.a = g_Config.m_GamePlayerColorGhostAlpha/255.0f;
			OwnSkinInfo.m_ColorFeet.a = g_Config.m_GamePlayerColorGhostAlpha/255.0f;
		}
		else
		{
			OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
			OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_GamePlayerColorGhost);
			OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_GamePlayerColorGhost);
			OwnSkinInfo.m_ColorBody.a = g_Config.m_GamePlayerColorGhostAlpha/255.0f;
			OwnSkinInfo.m_ColorFeet.a = g_Config.m_GamePlayerColorGhostAlpha/255.0f;
		}

		
		OwnSkinInfo.m_Size = 50.0f*UI()->Scale();
		
		aRects.HSplitTop(5.0f, 0, &Label);
		Label.VSplitLeft(230.0f, &Label, 0);
		Label.HSplitTop(50.0f, &Label, 0);
		RenderTools()->DrawUIRect(&Label, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 5.0f);
		RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Label.x+30.0f, Label.y+28.0f));
		Label.HSplitTop(15.0f, 0, &Label);
		Label.VSplitLeft(70.0f, 0, &Label);
		UI()->DoLabelScaled(&Label, Localize("Antiping ghost"), 14.0f, -1, 150.0f);
	}
}