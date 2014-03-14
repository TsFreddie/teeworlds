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
#include "gskins.h"
#include "pskins.h"

void CMenus::RenderSettingsSkins(CUIRect MainView)
{
	static int s_ControlPage = 0;
	
	CUIRect TabBar, Button;
	MainView.HSplitTop(20.0f, &TabBar, &MainView);
	MainView.Margin(10.0f, &MainView);
	
	// tab bar
	{
		TabBar.VSplitLeft(TabBar.w/4, &Button, &TabBar);
		static int s_Button0 = 0;
		if(DoButton_MenuTab(&s_Button0, Localize("Tee"), s_ControlPage == 0, &Button, 0))
			s_ControlPage = 0;

		TabBar.VSplitLeft(TabBar.w/3, &Button, &TabBar);
		static int s_Button1 = 0;
		if(DoButton_MenuTab(&s_Button1, Localize("Textures"), s_ControlPage == 1, &Button, 0))
			s_ControlPage = 1;
			
		TabBar.VSplitMid(&Button, &TabBar);
		static int s_Button2 = 0;
		if(DoButton_MenuTab(&s_Button2, Localize("Particles"), s_ControlPage == 2, &Button, 0))
			s_ControlPage = 2;
			
		static int s_Button3 = 0;
		if(DoButton_MenuTab(&s_Button3, Localize("Misc"), s_ControlPage == 3, &TabBar, 0))
			s_ControlPage = 3;
	}
	
	// render page
	//MainView.HSplitBottom(ms_ButtonHeight + 5*2, &MainView, &Bottom);

	if(s_ControlPage == 0)
		RenderSettingsTee(MainView);
	else if(s_ControlPage == 1)
		RenderSettingsTextures(MainView);
	else if(s_ControlPage == 2)
		RenderSettingsParticles(MainView);
	else if(s_ControlPage == 3)
		RenderSettingsMisc(MainView);

}

void CMenus::RenderSettingsTee(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);
	// skin info
	const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin));
	CTeeRenderInfo OwnSkinInfo;
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
	OwnSkinInfo.m_Size = 50.0f*UI()->Scale();

	MainView.HSplitTop(20.0f, &Label, &MainView);
	Label.VSplitLeft(230.0f, &Label, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Your skin"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0f, -1);

	MainView.HSplitTop(50.0f, &Label, &MainView);
	Label.VSplitLeft(230.0f, &Label, 0);
	RenderTools()->DrawUIRect(&Label, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 10.0f);
	RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Label.x+30.0f, Label.y+28.0f));
	Label.HSplitTop(15.0f, 0, &Label);;
	Label.VSplitLeft(70.0f, 0, &Label);
	UI()->DoLabelScaled(&Label, g_Config.m_PlayerSkin, 14.0f, -1, 150.0f);


	// custom colour selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(230.0f, &Button, 0);
	if(DoButton_CheckBox(&g_Config.m_PlayerColorBody, Localize("Custom colors"), g_Config.m_PlayerUseCustomColor, &Button))
	{
		g_Config.m_PlayerUseCustomColor = g_Config.m_PlayerUseCustomColor?0:1;
		m_NeedSendinfo = true;
	}

	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(82.5f, &Label, &MainView);
	if(g_Config.m_PlayerUseCustomColor)
	{
		CUIRect aRects[2];
		Label.VSplitMid(&aRects[0], &aRects[1]);
		aRects[0].VSplitRight(10.0f, &aRects[0], 0);
		aRects[1].VSplitLeft(10.0f, 0, &aRects[1]);

		int *paColors[2];
		paColors[0] = &g_Config.m_PlayerColorBody;
		paColors[1] = &g_Config.m_PlayerColorFeet;

		const char *paParts[] = {
			Localize("Body"),
			Localize("Feet")};
		const char *paLabels[] = {
			Localize("Hue"),
			Localize("Sat."),
			Localize("Lht.")};
		static int s_aColorSlider[2][3] = {{0}};

		for(int i = 0; i < 2; i++)
		{
			aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
			UI()->DoLabelScaled(&Label, paParts[i], 14.0f, -1);
			aRects[i].VSplitLeft(20.0f, 0, &aRects[i]);
			aRects[i].HSplitTop(2.5f, 0, &aRects[i]);

			int PrevColor = *paColors[i];
			int Color = 0;
			for(int s = 0; s < 3; s++)
			{
				aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
				Label.VSplitLeft(100.0f, &Label, &Button);
				Button.HMargin(2.0f, &Button);

				float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
				k = DoScrollbarH(&s_aColorSlider[i][s], &Button, k);
				Color <<= 8;
				Color += clamp((int)(k*255), 0, 255);
				UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
			}

			if(PrevColor != Color)
				m_NeedSendinfo = true;

			*paColors[i] = Color;
		}
	}

	// skin selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	static bool s_InitSkinlist = true;
	static sorted_array<const CSkins::CSkin *> s_paSkinList;
	static float s_ScrollValue = 0.0f;
	if(s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_pSkins->Num(); ++i)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = false;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Skins"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); ++i)
	{
		const CSkins::CSkin *s = s_paSkinList[i];
		if(s == 0)
			continue;

		if(str_comp(s->m_aName, g_Config.m_PlayerSkin) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CTeeRenderInfo Info;
			if(g_Config.m_PlayerUseCustomColor)
			{
				Info.m_Texture = s->m_ColorTexture;
				Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
				Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
			}
			else
			{
				Info.m_Texture = s->m_OrgTexture;
				Info.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Info.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			Info.m_Size = UI()->Scale()*50.0f;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, 0, vec2(1.0f, 0.0f), vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));

			if(g_Config.m_Debug)
			{
				vec3 BloodColor = g_Config.m_PlayerUseCustomColor ? m_pClient->m_pSkins->GetColorV3(g_Config.m_PlayerColorBody) : s->m_BloodColor;
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(BloodColor.r, BloodColor.g, BloodColor.b, 1.0f);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 12.0f, 12.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		mem_copy(g_Config.m_PlayerSkin, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerSkin));
		m_NeedSendinfo = true;
	}
}


void CMenus::RenderSettingsTextures(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);
	
	// skin selector
	//MainView.HSplitTop(20.0f, 0, &MainView);
	static bool s_InitSkinlist = true;
	static sorted_array<const CgSkins::CgSkin *> s_paSkinList;
	static float s_ScrollValue = 0.0f;
	if(s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_pgSkins->Num(); ++i)
		{
			const CgSkins::CgSkin *s = m_pClient->m_pgSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = false;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 160.0f, Localize("Texture"), "", s_paSkinList.size(), 2, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); ++i)
	{
		const CgSkins::CgSkin *s = s_paSkinList[i];
		if(s == 0)
			continue;

		if(str_comp(s->m_aName, g_Config.m_GameTexture) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CUIRect Label;
			Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
					
			int gTexture = s->m_Texture;
			char gName[512];
			str_format(gName, sizeof(gName), "%s", s->m_aName);;
			//Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			Graphics()->TextureSet(gTexture);
			Graphics()->QuadsBegin();
			IGraphics::CQuadItem QuadItem(Item.m_Rect.x+Item.m_Rect.w/2 - 120.0f, Item.m_Rect.y+Item.m_Rect.h/2 - 60.0f, 240.0f, 120.0f);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
			UI()->DoLabel(&Label, gName, 10.0f, 0);
			

			//if(g_Config.m_Debug)
			//{
			//	Graphics()->TextureSet(Info.m_Texture);
			//	Graphics()->QuadsBegin();
			//	IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 240.0f, 120.0f);
			//	Graphics()->QuadsDrawTL(&QuadItem, 1);
			//	Graphics()->QuadsEnd();
			//}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		mem_copy(g_Config.m_GameTexture, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameTexture));
		g_pData->m_aImages[IMAGE_GAME].m_Id = s_paSkinList[NewSelected]->m_Texture; 
		//m_NeedSendinfo = true;
	}
}

void CMenus::RenderSettingsParticles(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);
	
	// skin selector
	//MainView.HSplitTop(20.0f, 0, &MainView);
	static bool s_InitSkinlist = true;
	static sorted_array<const CpSkins::CpSkin *> s_paSkinList;
	static float s_ScrollValue = 0.0f;
	if(s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_ppSkins->Num(); ++i)
		{
			const CpSkins::CpSkin *s = m_pClient->m_ppSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = false;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 160.0f, Localize("Particles"), "", s_paSkinList.size(), 3, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); ++i)
	{
		const CpSkins::CpSkin *s = s_paSkinList[i];
		if(s == 0)
			continue;

		if(str_comp(s->m_aName, g_Config.m_GameParticles) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CUIRect Label;
			Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
					
			int gTexture = s->m_Texture;
			char gName[512];
			str_format(gName, sizeof(gName), "%s", s->m_aName);;
			//Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			Graphics()->TextureSet(gTexture);
			Graphics()->QuadsBegin();
			IGraphics::CQuadItem QuadItem(Item.m_Rect.x+Item.m_Rect.w/2 - 60.0f, Item.m_Rect.y+Item.m_Rect.h/2 - 60.0f, 120.0f, 120.0f);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
			UI()->DoLabel(&Label, gName, 10.0f, 0);
			

			//if(g_Config.m_Debug)
			//{
			//	Graphics()->TextureSet(Info.m_Texture);
			//	Graphics()->QuadsBegin();
			//	IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 240.0f, 120.0f);
			//	Graphics()->QuadsDrawTL(&QuadItem, 1);
			//	Graphics()->QuadsEnd();
			//}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		mem_copy(g_Config.m_GameParticles, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_GameParticles));
		g_pData->m_aImages[IMAGE_PARTICLES].m_Id = s_paSkinList[NewSelected]->m_Texture; 
		//m_NeedSendinfo = true;
	}
}

void CMenus::RenderSettingsMisc(CUIRect MainView)
{
		// laser color
		CUIRect Button, Laser, Label, OutLine, Right, Bottom;
		MainView.HSplitBottom(100.0f, &MainView, &Bottom);
		Bottom.Margin(20.0f,&Bottom);

		MainView.VSplitMid(&MainView, &Right);
	{
		
		MainView.VSplitLeft(10.0f, 0, &MainView);
		MainView.HSplitTop(100.0f, &MainView, &OutLine);

		int *pColor;
		pColor = &g_Config.m_GameLaserInnerColor;
		int *pAlpha;
		pAlpha = &g_Config.m_GameLaserInnerAlpha;
		int *pColor2;
		pColor2 = &g_Config.m_GameLaserOuterColor;
		int *pAlpha2;
		pAlpha2 = &g_Config.m_GameLaserOuterAlpha;
		

		const char *pParts[] = {
			Localize("Line Inner Color"),
			Localize("Line Outline Color")};
		const char *paLabels[] = {
			Localize("R."),
			Localize("G."),
			Localize("B."),
			Localize("Alp.")};
		static int s_aColorSlider[4] = {0};
		static int s_aColorSlider2[4] = {0};

		MainView.HSplitTop(15.0f, &Label, &MainView);
		UI()->DoLabelScaled(&Label, pParts[0], 14.0f, -1);
		MainView.VSplitLeft(10.0f, 0, &MainView);
		MainView.HSplitTop(2.5f, 0, &MainView);

		int PrevColor = *pColor;
		int Color = 0;
		for(int s = 0; s < 3; s++)
		{
			MainView.HSplitTop(20.0f, &Label, &MainView);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = ((PrevColor>>((2-s)*8))&0xff)  / 255.0f;
			k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
			Color <<= 8;
			Color += clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
		}
		
		*pColor = Color;
		
		int PrevAlpha = *pAlpha;
		int Alpha = 0;
		{
			MainView.HSplitTop(20.0f, &Label, &MainView);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = PrevAlpha / 255.0f;
			k = DoScrollbarH(&s_aColorSlider[3], &Button, k);
			Alpha = clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[3], 14.0f, -1);
		}
		
		*pAlpha = Alpha;

		OutLine.HSplitTop(15.0f, &Label, &OutLine);
		UI()->DoLabelScaled(&Label, pParts[1], 14.0f, -1);
		OutLine.VSplitLeft(10.0f, 0, &OutLine);
		OutLine.HSplitTop(2.5f, 0, &OutLine);

		int PrevColor2 = *pColor2;
		int Color2 = 0;
		for(int s = 0; s < 3; s++)
		{
			OutLine.HSplitTop(20.0f, &Label, &OutLine);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = ((PrevColor2>>((2-s)*8))&0xff)  / 255.0f;
			k = DoScrollbarH(&s_aColorSlider2[s], &Button, k);
			Color2 <<= 8;
			Color2 += clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
		}

		*pColor2 = Color2;

		int PrevAlpha2 = *pAlpha2;
		int Alpha2 = 0;
		{
			OutLine.HSplitTop(20.0f, &Label, &OutLine);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = PrevAlpha2 / 255.0f;
			k = DoScrollbarH(&s_aColorSlider2[3], &Button, k);
			Alpha2 = clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[3], 14.0f, -1);
		}
		
		*pAlpha2 = Alpha2;

	}
	
	{
		Right.VSplitLeft(10.0f, 0, &Right);
		Right.HSplitTop(100.0f, &Right, &OutLine);

		int *pColor;
		pColor = &g_Config.m_GameDotInnerColor;
		int *pAlpha;
		pAlpha = &g_Config.m_GameDotInnerAlpha;
		int *pColor2;
		pColor2 = &g_Config.m_GameDotOuterColor;
		int *pAlpha2;
		pAlpha2 = &g_Config.m_GameDotOuterAlpha;
		

		const char *pParts[] = {
			Localize("Dot Inner Color"),
			Localize("Dot Outline Color")};
		const char *paLabels[] = {
			Localize("R."),
			Localize("G."),
			Localize("B."),
			Localize("Alp.")};
		static int s_aColorSlider[4] = {0};
		static int s_aColorSlider2[4] = {0};

		Right.HSplitTop(15.0f, &Label, &Right);
		UI()->DoLabelScaled(&Label, pParts[0], 14.0f, -1);
		Right.VSplitLeft(10.0f, 0, &Right);
		Right.HSplitTop(2.5f, 0, &Right);

		int PrevColor = *pColor;
		int Color = 0;
		for(int s = 0; s < 3; s++)
		{
			Right.HSplitTop(20.0f, &Label, &Right);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = ((PrevColor>>((2-s)*8))&0xff)  / 255.0f;
			k = DoScrollbarH(&s_aColorSlider[s], &Button, k);
			Color <<= 8;
			Color += clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
		}
		
		*pColor = Color;
		
		int PrevAlpha = *pAlpha;
		int Alpha = 0;
		{
			Right.HSplitTop(20.0f, &Label, &Right);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = PrevAlpha / 255.0f;
			k = DoScrollbarH(&s_aColorSlider[3], &Button, k);
			Alpha = clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[3], 14.0f, -1);
		}
		
		*pAlpha = Alpha;

		OutLine.HSplitTop(15.0f, &Label, &OutLine);
		UI()->DoLabelScaled(&Label, pParts[1], 14.0f, -1);
		OutLine.VSplitLeft(10.0f, 0, &OutLine);
		OutLine.HSplitTop(2.5f, 0, &OutLine);

		int PrevColor2 = *pColor2;
		int Color2 = 0;
		for(int s = 0; s < 3; s++)
		{
			OutLine.HSplitTop(20.0f, &Label, &OutLine);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = ((PrevColor2>>((2-s)*8))&0xff)  / 255.0f;
			k = DoScrollbarH(&s_aColorSlider2[s], &Button, k);
			Color2 <<= 8;
			Color2 += clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
		}

		*pColor2 = Color2;

		int PrevAlpha2 = *pAlpha2;
		int Alpha2 = 0;
		{
			OutLine.HSplitTop(20.0f, &Label, &OutLine);
			Label.VSplitLeft(100.0f, &Label, &Button);
			Button.HMargin(2.0f, &Button);

			float k = PrevAlpha2 / 255.0f;
			k = DoScrollbarH(&s_aColorSlider2[3], &Button, k);
			Alpha2 = clamp((int)(k*255), 0, 255);
			UI()->DoLabelScaled(&Label, paLabels[3], 14.0f, -1);
		}
		
		*pAlpha2 = Alpha2;
	}
	
		Bottom.HSplitTop(15.0f, 0, &Laser);
		Laser.HSplitTop(18.0f, 0, &Button);
		Button.VSplitRight(20.0f, &Button, 0);
		Laser.VSplitLeft(5.0f, 0, &Laser);
		
		// darw laser
		vec2 From = vec2(Laser.x, Laser.y);
		vec2 Pos = vec2(Laser.x+Laser.w-10.0f, Laser.y);

		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();

		// do outline
		vec4 RgbOut;
		RgbOut.r = ((g_Config.m_GameLaserOuterColor>>16)&0xff)/255.0f;
		RgbOut.g = ((g_Config.m_GameLaserOuterColor>>8)&0xff)/255.0f;
		RgbOut.b = (g_Config.m_GameLaserOuterColor&0xff)/255.0f;
		RgbOut.a = g_Config.m_GameLaserOuterAlpha/255.0f;
		vec4 RgbDOut;
		RgbDOut.r = ((g_Config.m_GameDotOuterColor>>16)&0xff)/255.0f;
		RgbDOut.g = ((g_Config.m_GameDotOuterColor>>8)&0xff)/255.0f;
		RgbDOut.b = (g_Config.m_GameDotOuterColor&0xff)/255.0f;
		RgbDOut.a = g_Config.m_GameDotOuterAlpha/255.0f;
		Graphics()->SetColor(RgbOut.r, RgbOut.g, RgbOut.b, RgbOut.a); // outline
		vec2 Out = vec2(0.0f, -1.0f) * (3.15f);

		IGraphics::CFreeformItem Freeform(
				From.x-Out.x, From.y-Out.y,
				From.x+Out.x, From.y+Out.y,
				Pos.x-Out.x, Pos.y-Out.y,
				Pos.x+Out.x, Pos.y+Out.y);
		Graphics()->QuadsDrawFreeform(&Freeform, 1);

		// do inner	
		Out = vec2(0.0f, -1.0f) * (2.25f);
		vec4 RgbInner;
		RgbInner.r = ((g_Config.m_GameLaserInnerColor>>16)&0xff)/255.0f;
		RgbInner.g = ((g_Config.m_GameLaserInnerColor>>8)&0xff)/255.0f;
		RgbInner.b = (g_Config.m_GameLaserInnerColor&0xff)/255.0f;
		RgbInner.a = g_Config.m_GameLaserInnerAlpha/255.0f;
		vec4 RgbDInner;
		RgbDInner.r = ((g_Config.m_GameDotInnerColor>>16)&0xff)/255.0f;
		RgbDInner.g = ((g_Config.m_GameDotInnerColor>>8)&0xff)/255.0f;
		RgbDInner.b = (g_Config.m_GameDotInnerColor&0xff)/255.0f;
		RgbDInner.a = g_Config.m_GameDotInnerAlpha/255.0f;
		Graphics()->SetColor(RgbInner.r, RgbInner.g, RgbInner.b, RgbInner.a); // center

		Freeform = IGraphics::CFreeformItem(
				From.x-Out.x, From.y-Out.y,
				From.x+Out.x, From.y+Out.y,
				Pos.x-Out.x, Pos.y-Out.y,
				Pos.x+Out.x, Pos.y+Out.y);
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	
		Graphics()->QuadsEnd();
		
		// render head
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		Graphics()->QuadsBegin();

		RenderTools()->SelectSprite(SPRITE_PART_SPLAT01);
		Graphics()->SetColor(RgbDOut.r, RgbDOut.g, RgbDOut.b, RgbDOut.a);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, 24, 24);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->SetColor(RgbDInner.r, RgbDInner.g, RgbDInner.b, RgbDInner.a);
		QuadItem = IGraphics::CQuadItem(Pos.x, Pos.y, 20, 20);
		Graphics()->QuadsDraw(&QuadItem, 1);

		Graphics()->QuadsEnd();

		// draw laser weapon
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();

		RenderTools()->SelectSprite(SPRITE_WEAPON_RIFLE_BODY);
		RenderTools()->DrawSprite(Laser.x, Laser.y, 60.0f);

		Graphics()->QuadsEnd();
				
		//Button.Margin(10, &Button);
		Button.HSplitTop(20.0f, &Button, 0);
		static int s_ResetButton = 0;
		if(DoButton_Menu(&s_ResetButton, Localize("Reset color"), 0, &Button))
		{
			g_Config.m_GameLaserInnerColor = 8421631;
			g_Config.m_GameLaserInnerAlpha = 255;
			g_Config.m_GameLaserOuterColor = 1250112;
			g_Config.m_GameLaserOuterAlpha = 255;
			g_Config.m_GameDotInnerColor = 8421631;
			g_Config.m_GameDotOuterAlpha = 255;
			g_Config.m_GameDotOuterColor = 1250112;
			g_Config.m_GameDotOuterAlpha = 255;
		}
		//MainView.HSplitTop(10.0f, &Button, &MainView);
}