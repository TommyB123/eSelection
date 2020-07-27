#if defined __eSelection_included
	#endinput
#endif
#define __eSelection_included

#include <a_samp>
#include <PawnPlus>

#if !defined MAX_ITEM_PER_PAGE
	#define MAX_ITEM_PER_PAGE (18)
#endif

#define MAX_MODEL_TEXT 64

#define MODEL_RESPONSE_CANCEL (0)
#define MODEL_RESPONSE_SELECT (1)

forward OnModelSelectionResponse(playerid, extraid, index, modelid, response);

static Text:g_eMenuRightArrow = Text:INVALID_TEXT_DRAW;
static Text:g_eMenuLeftArrow = Text:INVALID_TEXT_DRAW;
static Text:g_eMenuBackground = Text:INVALID_TEXT_DRAW;
static Text:g_eMenuTopBanner = Text:INVALID_TEXT_DRAW;
static Text:g_eMenuBottomBanner = Text:INVALID_TEXT_DRAW;
static Text:g_eMenuCloseButton = Text:INVALID_TEXT_DRAW;

enum g_ePlayerMenuEnum
{
	PlayerText:g_eMenuHeaderText,
	PlayerText:g_eMenuPageText,
	PlayerText:g_eMenuItems[MAX_ITEM_PER_PAGE],
	PlayerText:g_eMenuTextDisplay[MAX_ITEM_PER_PAGE],
	List:g_eMenuModels,
	g_eMenuTotalItems,
	g_eMenuPage,
	g_eMenuPages,
	g_eMenuShown,
	g_eMenuExtra,
	g_eMenuTime
};

enum _:g_eMenuModelData //data to be used for displayed models in the menu
{
	g_eMenuModel,
	g_eMenuModelText[MAX_MODEL_TEXT],
	bool:g_eItemUseRotation,
	Float:g_eItemRotX,
	Float:g_eItemRotY,
	Float:g_eItemRotZ,
	Float:g_eItemZoom
};

static stock g_ePlayerMenuData[MAX_PLAYERS][g_ePlayerMenuEnum];

//pawnplus task data
enum _:E_MODEL_SELECTION_INFO
{
	E_MODEL_SELECTION_RESPONSE,
	E_MODEL_SELECTION_INDEX,
	E_MODEL_SELECTION_MODELID
}
const PAWN_PLUS_EXTRA_ID = 0x502B; // "P+" in ascii hex (THANKS GRABER)
static Task:ModelSelectionTask[MAX_PLAYERS];

//functions
stock AddModelMenuItem(List:menulist, modelid, const text[] = "", bool:usingrotation = false, Float:rotx = 0.0, Float:roty = 0.0, Float:rotz = 0.0, Float:zoom = 1.0)
{
	new item[g_eMenuModelData];
	item[g_eMenuModel] = modelid;
	format(item[g_eMenuModelText], MAX_MODEL_TEXT, text);
	item[g_eItemUseRotation] = usingrotation;
	item[g_eItemRotX] = rotx;
	item[g_eItemRotY] = roty;
	item[g_eItemRotZ] = rotz;
	item[g_eItemZoom] = zoom;
	list_add_arr(menulist, item);
}

stock ShowModelSelectionMenu(playerid, const header[], extraid, List:items)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(g_ePlayerMenuData[playerid][g_eMenuShown]) HideModelSelectionMenu(playerid);

	g_ePlayerMenuData[playerid][g_eMenuModels] = items;
	g_ePlayerMenuData[playerid][g_eMenuExtra] = extraid;
	g_ePlayerMenuData[playerid][g_eMenuShown] = true;
	g_ePlayerMenuData[playerid][g_eMenuTime] = GetTickCount();
	g_ePlayerMenuData[playerid][g_eMenuPage] = 1;
	g_ePlayerMenuData[playerid][g_eMenuPages] = (list_size(g_ePlayerMenuData[playerid][g_eMenuModels]) / MAX_ITEM_PER_PAGE) + 1;

	ShowModelSelectionMenuTextDraws(playerid, header);
	return 1;
}

stock Task:ShowAsyncModelSelectionMenu(playerid, const header[], List:items)
{
	if(task_valid(ModelSelectionTask[playerid])) task_delete(ModelSelectionTask[playerid]);
	ModelSelectionTask[playerid] = task_new();

	ShowModelSelectionMenu(playerid, header, PAWN_PLUS_EXTRA_ID, items);
	return ModelSelectionTask[playerid];
}

static stock ShowModelSelectionMenuTextDraws(playerid, const header[])
{
	DestroyModelSelectionPlayerTDs(playerid);
	CreateModelSelectionPlayerTDs(playerid);

	for(new Iter:i = list_iter(g_ePlayerMenuData[playerid][g_eMenuModels]), count, model[g_eMenuModelData]; iter_inside(i); iter_move_next(i))
	{
		if(count >= MAX_ITEM_PER_PAGE) break;

		iter_get_arr(i, model);
		SetModelSelectionModelBox(playerid, count, model);
		count ++;
	}

	//page TD
	new page[8];
	format(page, sizeof(page), "1/%d", (list_size(g_ePlayerMenuData[playerid][g_eMenuModels]) / MAX_ITEM_PER_PAGE) + 1);
	PlayerTextDrawSetString(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], page);

	//header test
	PlayerTextDrawSetString(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], header);
	PlayerTextDrawShow(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText]);
	PlayerTextDrawShow(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText]);

	TextDrawShowForPlayer(playerid, g_eMenuRightArrow);
	TextDrawShowForPlayer(playerid, g_eMenuLeftArrow);
	TextDrawShowForPlayer(playerid, g_eMenuBackground);
	TextDrawShowForPlayer(playerid, g_eMenuTopBanner);
	TextDrawShowForPlayer(playerid, g_eMenuBottomBanner);
	TextDrawShowForPlayer(playerid, g_eMenuCloseButton);
	SelectTextDraw(playerid, -1);
}

static stock HideModelSelectionMenu(playerid)
{
	if(!IsPlayerConnected(playerid) || !g_ePlayerMenuData[playerid][g_eMenuShown]) return 0;

	TextDrawHideForPlayer(playerid, g_eMenuRightArrow);
	TextDrawHideForPlayer(playerid, g_eMenuLeftArrow);
	TextDrawHideForPlayer(playerid, g_eMenuBackground);
	TextDrawHideForPlayer(playerid, g_eMenuTopBanner);
	TextDrawHideForPlayer(playerid, g_eMenuBottomBanner);
	TextDrawHideForPlayer(playerid, g_eMenuCloseButton);

	DestroyModelSelectionPlayerTDs(playerid);

	g_ePlayerMenuData[playerid][g_eMenuShown] = false;
	g_ePlayerMenuData[playerid][g_eMenuTotalItems] = 0;
	g_ePlayerMenuData[playerid][g_eMenuExtra] = 0;
	g_ePlayerMenuData[playerid][g_eMenuPage] = 1;
	g_ePlayerMenuData[playerid][g_eMenuPages] = 0;

	list_delete(g_ePlayerMenuData[playerid][g_eMenuModels]);
	CancelSelectTextDraw(playerid);
	return 1;
}

static stock Menu_SetPage(playerid, page)
{
	if(!g_ePlayerMenuData[playerid][g_eMenuShown]) return 0;
	if(page < 1 || page > g_ePlayerMenuData[playerid][g_eMenuPages]) return 0;

	new start = (MAX_ITEM_PER_PAGE * (page - 1)), str[16];

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i ++)
	{
		PlayerTextDrawHide(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i]);
		PlayerTextDrawHide(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i]);
	}

	for(new Iter:i = list_iter(g_ePlayerMenuData[playerid][g_eMenuModels], start), count, model[g_eMenuModelData]; iter_inside(i); iter_move_next(i))
	{
		if(count >= MAX_ITEM_PER_PAGE) break;

		iter_get_arr(i, model);
		SetModelSelectionModelBox(playerid, count, model);
		count ++;
	}
	g_ePlayerMenuData[playerid][g_eMenuPage] = page;

	format(str, 8, "%d/%d", page, g_ePlayerMenuData[playerid][g_eMenuPages]);
	PlayerTextDrawSetString(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], str);
	return 1;
}

static stock SetModelSelectionModelBox(playerid, count, const model[g_eMenuModelData])
{
	PlayerTextDrawSetPreviewModel(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][count], model[g_eMenuModel]);
	if(model[g_eItemUseRotation])
	{
		PlayerTextDrawSetPreviewRot(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][count], model[g_eItemRotX], model[g_eItemRotY], model[g_eItemRotZ], model[g_eItemZoom]);
	}
	else
	{
		PlayerTextDrawSetPreviewRot(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][count], 0.0, 0.0, 0.0);
	}

	PlayerTextDrawShow(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][count]);

	if(model[g_eMenuModelText][0])
	{
		PlayerTextDrawSetString(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][count], model[g_eMenuModelText]);
		PlayerTextDrawShow(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][count]);
	}
}

static stock CreateModelSelectionPlayerTDs(playerid)
{
	//page TD
	g_ePlayerMenuData[playerid][g_eMenuPageText] = CreatePlayerTextDraw(playerid, 523.333251, 139.792648, "0/1");
	PlayerTextDrawLetterSize(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 0.190666, 1.110518);
	PlayerTextDrawAlignment(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 3);
	PlayerTextDrawColor(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 0xC0C0C0FF);
	PlayerTextDrawSetShadow(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 0);
	PlayerTextDrawSetOutline(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 1);
	PlayerTextDrawBackgroundColor(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 0x00000033);
	PlayerTextDrawFont(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 2);
	PlayerTextDrawSetProportional(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText], 1);

	//header test
	g_ePlayerMenuData[playerid][g_eMenuHeaderText] = CreatePlayerTextDraw(playerid, 128.333312, 139.377761, "header");
	PlayerTextDrawLetterSize(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 0.315000, 1.247407);
	PlayerTextDrawAlignment(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 1);
	PlayerTextDrawColor(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 0xC0C0C0FF);
	PlayerTextDrawSetShadow(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 0);
	PlayerTextDrawSetOutline(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 1);
	PlayerTextDrawBackgroundColor(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 0x00000033);
	PlayerTextDrawFont(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 2);
	PlayerTextDrawSetProportional(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText], 1);

	new Float:x = 78.0, Float:y = 162.0;
	for(new i = 0, idx = 0; i < MAX_ITEM_PER_PAGE; i ++)
	{
		if(idx > 0 && (idx % 6) == 0)
		{
			x = 140.0;
			y += 55.0;
		}
		else
		{
			x += 62.0;
		}
		idx++;

		g_ePlayerMenuData[playerid][g_eMenuItems][i] = CreatePlayerTextDraw(playerid, x, y, "_");
		PlayerTextDrawBackgroundColor(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 0xD3D3D344);
		PlayerTextDrawFont(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 5);
		PlayerTextDrawLetterSize(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 1.430000, 5.700000);
		PlayerTextDrawColor(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], -1);
		PlayerTextDrawSetOutline(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 1);
		PlayerTextDrawSetProportional(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 1);
		PlayerTextDrawUseBox(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 1);
		PlayerTextDrawBoxColor(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 0);
		PlayerTextDrawTextSize(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 61.000000, 54.000000);
		PlayerTextDrawSetSelectable(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i], 1);

		g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i] = CreatePlayerTextDraw(playerid, x + 31.0, y, "_");
		PlayerTextDrawFont(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 2);
		PlayerTextDrawLetterSize(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 0.199999, 0.6);
		PlayerTextDrawAlignment(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 2);
		PlayerTextDrawSetOutline(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 0);
		PlayerTextDrawSetProportional(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 1);
		PlayerTextDrawTextSize(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 0.0, 62.0);
		PlayerTextDrawSetShadow(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 0);
		PlayerTextDrawColor(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i], 0xD3D3D3AA);
	}
}

static stock DestroyModelSelectionPlayerTDs(playerid)
{
	PlayerTextDrawDestroy(playerid, g_ePlayerMenuData[playerid][g_eMenuHeaderText]);
	PlayerTextDrawDestroy(playerid, g_ePlayerMenuData[playerid][g_eMenuPageText]);

	g_ePlayerMenuData[playerid][g_eMenuHeaderText] = PlayerText:INVALID_TEXT_DRAW;
	g_ePlayerMenuData[playerid][g_eMenuPageText] = PlayerText:INVALID_TEXT_DRAW;

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i++)
	{
		PlayerTextDrawDestroy(playerid, g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i]);
		PlayerTextDrawDestroy(playerid, g_ePlayerMenuData[playerid][g_eMenuItems][i]);

		g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i] = PlayerText:INVALID_TEXT_DRAW;
		g_ePlayerMenuData[playerid][g_eMenuItems][i] = PlayerText:INVALID_TEXT_DRAW;
	}
}

//callbacks
public OnGameModeInit()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		g_ePlayerMenuData[i][g_eMenuPageText] = PlayerText:INVALID_TEXT_DRAW;
		g_ePlayerMenuData[i][g_eMenuHeaderText] = PlayerText:INVALID_TEXT_DRAW;

		for(new p = 0; p < MAX_ITEM_PER_PAGE; p++)
		{
			g_ePlayerMenuData[i][g_eMenuItems][p] = PlayerText:INVALID_TEXT_DRAW;
			g_ePlayerMenuData[i][g_eMenuTextDisplay][p] = PlayerText:INVALID_TEXT_DRAW;
		}
	}

	//textdraw background
	g_eMenuBackground = TextDrawCreate(531.333374, 140.877777, "_");
	TextDrawBackgroundColor(g_eMenuBackground, 0);
	TextDrawAlignment(g_eMenuBackground, 1);
	TextDrawFont(g_eMenuBackground, 0);
	TextDrawLetterSize(g_eMenuBackground, 0.000000, 22.912965);
	TextDrawColor(g_eMenuBackground, 0);
	TextDrawSetOutline(g_eMenuBackground, 0);
	TextDrawSetProportional(g_eMenuBackground, 1);
	TextDrawSetShadow(g_eMenuBackground, 0);
	TextDrawUseBox(g_eMenuBackground, 1);
	TextDrawBoxColor(g_eMenuBackground, 0x000000DD);
	TextDrawTextSize(g_eMenuBackground, 121.333328, 0.000000);
	TextDrawSetSelectable(g_eMenuBackground, 0);

	//right arrow
	g_eMenuRightArrow = TextDrawCreate(521.333374, 339.318542, "LD_BEAT:right");
	TextDrawLetterSize(g_eMenuRightArrow, 0.000000, 0.000000);
	TextDrawTextSize(g_eMenuRightArrow, 5.999938, 7.051818);
	TextDrawAlignment(g_eMenuRightArrow, 1);
	TextDrawColor(g_eMenuRightArrow, -1);
	TextDrawSetShadow(g_eMenuRightArrow, 0);
	TextDrawSetOutline(g_eMenuRightArrow, 0);
	TextDrawFont(g_eMenuRightArrow, 4);
	TextDrawSetSelectable(g_eMenuRightArrow, true);

	//left arrow
	g_eMenuLeftArrow = TextDrawCreate(507.000305, 339.074066, "LD_BEAT:left");
	TextDrawLetterSize(g_eMenuLeftArrow, 0.000000, 0.000000);
	TextDrawTextSize(g_eMenuLeftArrow, 5.999938, 7.051818);
	TextDrawAlignment(g_eMenuLeftArrow, 1);
	TextDrawColor(g_eMenuLeftArrow, -1);
	TextDrawSetShadow(g_eMenuLeftArrow, 0);
	TextDrawSetOutline(g_eMenuLeftArrow, 0);
	TextDrawFont(g_eMenuLeftArrow, 4);
	TextDrawSetSelectable(g_eMenuLeftArrow, true);

	//top banner strip
	g_eMenuTopBanner = TextDrawCreate(531.000244, 155.811111, "TopBanner");
	TextDrawLetterSize(g_eMenuTopBanner, 0.000000, -0.447120);
	TextDrawTextSize(g_eMenuTopBanner, 121.333328, 0.000000);
	TextDrawAlignment(g_eMenuTopBanner, 1);
	TextDrawColor(g_eMenuTopBanner, 0);
	TextDrawUseBox(g_eMenuTopBanner, true);
	TextDrawBoxColor(g_eMenuTopBanner, 0x808080FF);
	TextDrawSetShadow(g_eMenuTopBanner, 0);
	TextDrawSetOutline(g_eMenuTopBanner, 0);
	TextDrawFont(g_eMenuTopBanner, 0);

	//bottom banner strip
	g_eMenuBottomBanner = TextDrawCreate(531.333618, 338.500305, "BottomBanner");
	TextDrawLetterSize(g_eMenuBottomBanner, 0.000000, -0.447120);
	TextDrawTextSize(g_eMenuBottomBanner, 120.666656, 0.000000);
	TextDrawAlignment(g_eMenuBottomBanner, 1);
	TextDrawColor(g_eMenuBottomBanner, 0);
	TextDrawUseBox(g_eMenuBottomBanner, true);
	TextDrawBoxColor(g_eMenuBottomBanner, 0x808080FF);
	TextDrawSetShadow(g_eMenuBottomBanner, 0);
	TextDrawSetOutline(g_eMenuBottomBanner, 0);
	TextDrawFont(g_eMenuBottomBanner, 0);

	//close button
	g_eMenuCloseButton = TextDrawCreate(490.666809, 337.829711, "CLOSE");
	TextDrawLetterSize(g_eMenuCloseButton, 0.128333, 0.957036);
	TextDrawTextSize(g_eMenuCloseButton, 10.5021, 10.0187);
	TextDrawAlignment(g_eMenuCloseButton, 2);
	TextDrawColor(g_eMenuCloseButton, 0xC0C0C0FF);
	TextDrawSetShadow(g_eMenuCloseButton, 0);
	TextDrawSetOutline(g_eMenuCloseButton, 0);
	TextDrawBackgroundColor(g_eMenuCloseButton, 0x00000033);
	TextDrawFont(g_eMenuCloseButton, 2);
	TextDrawSetProportional(g_eMenuCloseButton, 1);
	TextDrawSetSelectable(g_eMenuCloseButton, true);

	#if defined MS_OnGameModeInit
		return MS_OnGameModeInit();
	#else
		return 1;
	#endif
}

public OnPlayerDisconnect(playerid, reason)
{
	g_ePlayerMenuData[playerid][g_eMenuPageText] = PlayerText:INVALID_TEXT_DRAW;
	g_ePlayerMenuData[playerid][g_eMenuHeaderText] = PlayerText:INVALID_TEXT_DRAW;

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i++)
	{
		g_ePlayerMenuData[playerid][g_eMenuItems][i] = PlayerText:INVALID_TEXT_DRAW;
		g_ePlayerMenuData[playerid][g_eMenuTextDisplay][i] = PlayerText:INVALID_TEXT_DRAW;
	}

	#if defined MS_OnPlayerDisconnect
		return MS_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
	if(g_ePlayerMenuData[playerid][g_eMenuShown] && (clickedid == Text:INVALID_TEXT_DRAW || clickedid == g_eMenuCloseButton))
	{
		if(g_ePlayerMenuData[playerid][g_eMenuExtra] != PAWN_PLUS_EXTRA_ID)
		{
			CallLocalFunction("OnModelSelectionResponse", "iiiii", playerid, g_ePlayerMenuData[playerid][g_eMenuExtra], 0, 0, MODEL_RESPONSE_CANCEL);
			HideModelSelectionMenu(playerid);
		}
		else
		{
			new model_response[E_MODEL_SELECTION_INFO];
			model_response[E_MODEL_SELECTION_INDEX] = 0;
			model_response[E_MODEL_SELECTION_MODELID] = 0;
			model_response[E_MODEL_SELECTION_RESPONSE] = MODEL_RESPONSE_CANCEL;

			HideModelSelectionMenu(playerid);

			new const Task:task = ModelSelectionTask[playerid];
			ModelSelectionTask[playerid] = Task:0;
			task_set_result_arr(task, model_response);
		}
	}
	else if(clickedid == g_eMenuRightArrow)
	{
		if(g_ePlayerMenuData[playerid][g_eMenuPage] == g_ePlayerMenuData[playerid][g_eMenuPages]) return false;

		Menu_SetPage(playerid, g_ePlayerMenuData[playerid][g_eMenuPage] + 1);
		return true;
	}
	else if(clickedid == g_eMenuLeftArrow)
	{
		if(g_ePlayerMenuData[playerid][g_eMenuPage] < 2) return false;

		else Menu_SetPage(playerid, g_ePlayerMenuData[playerid][g_eMenuPage] - 1);
		return true;
	}

	#if defined MS_OnPlayerClickTextDraw
		return MS_OnPlayerClickTextDraw(playerid, Text:clickedid);
	#else
		return 1;
	#endif
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
	if(g_ePlayerMenuData[playerid][g_eMenuShown] && (GetTickCount() - g_ePlayerMenuData[playerid][g_eMenuTime]) > 600)
	{
		for(new i = 0; i < MAX_ITEM_PER_PAGE; i ++)
		{
			if(g_ePlayerMenuData[playerid][g_eMenuItems][i] == playertextid)
			{
				new index, extraid, modelid;
				index = (i + (g_ePlayerMenuData[playerid][g_eMenuPage] - 1) * MAX_ITEM_PER_PAGE);
				extraid = g_ePlayerMenuData[playerid][g_eMenuExtra];
				modelid = list_get(g_ePlayerMenuData[playerid][g_eMenuModels], index);

				if(extraid != PAWN_PLUS_EXTRA_ID)
				{
					CallLocalFunction("OnModelSelectionResponse", "iiiii", playerid, extraid, index, modelid, MODEL_RESPONSE_SELECT);
					HideModelSelectionMenu(playerid);
				}
				else
				{
					new model_response[E_MODEL_SELECTION_INFO];
					model_response[E_MODEL_SELECTION_INDEX] = index;
					model_response[E_MODEL_SELECTION_MODELID] = modelid;
					model_response[E_MODEL_SELECTION_RESPONSE] = MODEL_RESPONSE_SELECT;

					HideModelSelectionMenu(playerid);

					new const Task:task = ModelSelectionTask[playerid];
					ModelSelectionTask[playerid] = Task:0;
					task_set_result_arr(task, model_response);
				}
			}
		}
	}

	#if defined MS_OnPlayerClickPlayerTextDraw
		return MS_OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid);
	#else
		return 1;
	#endif
}

//hooks
#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif

#define OnGameModeInit MS_OnGameModeInit
#if defined MS_OnGameModeInit
	forward MS_OnGameModeInit();
#endif

#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect MS_OnPlayerDisconnect
#if defined MS_OnPlayerDisconnect
	forward MS_OnPlayerDisconnect(playerid, reason);
#endif

#if defined _ALS_OnPlayerClickTextDraw
	#undef OnPlayerClickTextDraw
#else
	#define _ALS_OnPlayerClickTextDraw
#endif

#define OnPlayerClickTextDraw MS_OnPlayerClickTextDraw
#if defined MS_OnPlayerClickTextDraw
	forward MS_OnPlayerClickTextDraw(playerid, Text:clickedid);
#endif

#if defined _ALS_OnPlayerClickPlayerTD
	#undef OnPlayerClickPlayerTextDraw
#else
	#define _ALS_OnPlayerClickPlayerTD
#endif

#define OnPlayerClickPlayerTextDraw MS_OnPlayerClickPlayerTextDraw
#if defined MS_OnPlayerClickPlayerTextDraw
	forward MS_OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid);
#endif