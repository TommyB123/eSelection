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

//global textdraws
static Text:g_MenuRightArrow = Text:INVALID_TEXT_DRAW;
static Text:g_MenuLeftArrow = Text:INVALID_TEXT_DRAW;
static Text:g_MenuBackground = Text:INVALID_TEXT_DRAW;
static Text:g_MenuTopBanner = Text:INVALID_TEXT_DRAW;
static Text:g_MenuBottomBanner = Text:INVALID_TEXT_DRAW;
static Text:g_MenuCloseButton = Text:INVALID_TEXT_DRAW;

//player textdraws
static PlayerText:g_MenuHeaderText[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};
static PlayerText:g_MenuPageNumber[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};
static PlayerText:g_MenuItems[MAX_PLAYERS][MAX_ITEM_PER_PAGE] = {{PlayerText:INVALID_TEXT_DRAW, ...}, ...};
static PlayerText:g_MenuItemText[MAX_PLAYERS][MAX_ITEM_PER_PAGE] = {{PlayerText:INVALID_TEXT_DRAW, ...}, ...};

//PawnPlus list for models
static List:g_MenuModels[MAX_PLAYERS] = {INVALID_LIST, ...};

//general model menu variables
static g_MenuItemCount[MAX_PLAYERS];
static g_MenuCurrentPage[MAX_PLAYERS] = {1, ...};
static g_MenuPageCount[MAX_PLAYERS];
static bool:g_MenuShown[MAX_PLAYERS];
static g_MenuExtraID[MAX_PLAYERS];
static g_MenuCooldownTick[MAX_PLAYERS];

//enum for model data
static enum _:g_eMenuModelData
{
	g_eMenuModel,
	g_eMenuModelText[MAX_MODEL_TEXT],
	bool:g_eItemUseRotation,
	Float:g_eItemRotX,
	Float:g_eItemRotY,
	Float:g_eItemRotZ,
	Float:g_eItemZoom,
	g_eItemVehicleColor[2]
}

//pawnplus task data
enum _:E_MODEL_SELECTION_INFO
{
	E_MODEL_SELECTION_RESPONSE,
	E_MODEL_SELECTION_INDEX,
	E_MODEL_SELECTION_MODELID
}

//"P+" in ascii hex (THANKS GRABER)
const PAWN_PLUS_EXTRA_ID = 0x502B;

//pawnplus task
static Task:ModelSelectionTask[MAX_PLAYERS];

//functions
stock AddModelMenuItem(List:menulist, modelid, const text[] = "", bool:usingrotation = false, Float:rotx = 0.0, Float:roty = 0.0, Float:rotz = 0.0, Float:zoom = 1.0, vehiclecolor1 = -1, vehiclecolor2 = -1)
{
	new item[g_eMenuModelData];
	item[g_eMenuModel] = modelid;
	format(item[g_eMenuModelText], MAX_MODEL_TEXT, text);
	item[g_eItemUseRotation] = usingrotation;
	item[g_eItemRotX] = rotx;
	item[g_eItemRotY] = roty;
	item[g_eItemRotZ] = rotz;
	item[g_eItemZoom] = zoom;
	item[g_eItemVehicleColor][0] = vehiclecolor1;
	item[g_eItemVehicleColor][1] = vehiclecolor2;
	list_add_arr(menulist, item);
}

stock ShowModelSelectionMenu(playerid, const header[], extraid, List:items)
{
	if(!IsPlayerConnected(playerid)) return false;
	if(g_MenuShown[playerid]) HideModelSelectionMenu(playerid);

	g_MenuModels[playerid] = items;
	g_MenuExtraID[playerid] = extraid;
	g_MenuShown[playerid] = true;
	g_MenuCooldownTick[playerid] = GetTickCount();
	g_MenuCurrentPage[playerid] = 1;
	g_MenuPageCount[playerid] = (list_size(items) / MAX_ITEM_PER_PAGE) + 1;

	ShowModelSelectionMenuTextDraws(playerid, header);
	return true;
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

	for(new Iter:i = list_iter(g_MenuModels[playerid]), count, model[g_eMenuModelData]; iter_inside(i); iter_move_next(i))
	{
		if(count >= MAX_ITEM_PER_PAGE) break;

		iter_get_arr(i, model);
		SetModelSelectionModelBox(playerid, count, model);
		count ++;
	}

	//page TD
	new page[8];
	format(page, sizeof(page), "1/%d", (list_size(g_MenuModels[playerid]) / MAX_ITEM_PER_PAGE) + 1);
	PlayerTextDrawSetString(playerid, g_MenuPageNumber[playerid], page);

	//header text
	PlayerTextDrawSetString(playerid, g_MenuHeaderText[playerid], header);
	PlayerTextDrawShow(playerid, g_MenuHeaderText[playerid]);
	PlayerTextDrawShow(playerid, g_MenuPageNumber[playerid]);

	TextDrawShowForPlayer(playerid, g_MenuRightArrow);
	TextDrawShowForPlayer(playerid, g_MenuLeftArrow);
	TextDrawShowForPlayer(playerid, g_MenuBackground);
	TextDrawShowForPlayer(playerid, g_MenuTopBanner);
	TextDrawShowForPlayer(playerid, g_MenuBottomBanner);
	TextDrawShowForPlayer(playerid, g_MenuCloseButton);
	SelectTextDraw(playerid, -1);
}

static stock HideModelSelectionMenu(playerid)
{
	if(!IsPlayerConnected(playerid) || !g_MenuShown[playerid]) return false;

	TextDrawHideForPlayer(playerid, g_MenuRightArrow);
	TextDrawHideForPlayer(playerid, g_MenuLeftArrow);
	TextDrawHideForPlayer(playerid, g_MenuBackground);
	TextDrawHideForPlayer(playerid, g_MenuTopBanner);
	TextDrawHideForPlayer(playerid, g_MenuBottomBanner);
	TextDrawHideForPlayer(playerid, g_MenuCloseButton);

	DestroyModelSelectionPlayerTDs(playerid);

	g_MenuShown[playerid] = false;
	g_MenuItemCount[playerid] = 0;
	g_MenuExtraID[playerid] = 0;
	g_MenuCurrentPage[playerid] = 1;
	g_MenuPageCount[playerid] = 0;

	if(list_valid(g_MenuModels[playerid])) list_delete(g_MenuModels[playerid]);
	g_MenuModels[playerid] = INVALID_LIST;

	CancelSelectTextDraw(playerid);
	return true;
}

static stock SetModelSelectionMenuPage(playerid, page)
{
	if(!g_MenuShown[playerid]) return false;
	if(page < 1 || page > g_MenuPageCount[playerid]) return false;

	new start = (MAX_ITEM_PER_PAGE * (page - 1));

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i ++)
	{
		PlayerTextDrawHide(playerid, g_MenuItems[playerid][i]);
		PlayerTextDrawHide(playerid, g_MenuItemText[playerid][i]);
	}

	for(new Iter:i = list_iter(g_MenuModels[playerid], start), count, model[g_eMenuModelData]; iter_inside(i); iter_move_next(i))
	{
		if(count >= MAX_ITEM_PER_PAGE) break;

		iter_get_arr(i, model);
		SetModelSelectionModelBox(playerid, count, model);
		count ++;
	}
	g_MenuCurrentPage[playerid] = page;

	new pagetext[12];
	format(pagetext, sizeof(pagetext), "%d/%d", page,  g_MenuPageCount[playerid]);
	PlayerTextDrawSetString(playerid, g_MenuPageNumber[playerid], pagetext);
	return true;
}

static stock SetModelSelectionModelBox(playerid, count, const model[g_eMenuModelData])
{
	PlayerTextDrawSetPreviewModel(playerid, g_MenuItems[playerid][count], model[g_eMenuModel]);
	if(model[g_eItemUseRotation])
	{
		PlayerTextDrawSetPreviewRot(playerid, g_MenuItems[playerid][count], model[g_eItemRotX], model[g_eItemRotY], model[g_eItemRotZ], model[g_eItemZoom]);
	}
	else
	{
		PlayerTextDrawSetPreviewRot(playerid, g_MenuItems[playerid][count], 0.0, 0.0, 0.0);
	}

	if(model[g_eItemVehicleColor][0] != -1)
	{
		new secondarycolor = model[g_eItemVehicleColor][1] == -1 ? model[g_eItemVehicleColor][0] : model[g_eItemVehicleColor][1];
		PlayerTextDrawSetPreviewVehCol(playerid, g_MenuItems[playerid][count], model[g_eItemVehicleColor][0], secondarycolor);
	}

	PlayerTextDrawShow(playerid, g_MenuItems[playerid][count]);

	if(model[g_eMenuModelText][0])
	{
		PlayerTextDrawSetString(playerid, g_MenuItemText[playerid][count], model[g_eMenuModelText]);
		PlayerTextDrawShow(playerid, g_MenuItemText[playerid][count]);
	}
}

static stock CreateModelSelectionPlayerTDs(playerid)
{
	//page TD
	g_MenuPageNumber[playerid] = CreatePlayerTextDraw(playerid, 523.333251, 139.792648, "0/1");
	PlayerTextDrawLetterSize(playerid, g_MenuPageNumber[playerid], 0.190666, 1.110518);
	PlayerTextDrawAlignment(playerid, g_MenuPageNumber[playerid], 3);
	PlayerTextDrawColor(playerid, g_MenuPageNumber[playerid], 0xC0C0C0FF);
	PlayerTextDrawSetShadow(playerid, g_MenuPageNumber[playerid], 0);
	PlayerTextDrawSetOutline(playerid, g_MenuPageNumber[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, g_MenuPageNumber[playerid], 0x00000033);
	PlayerTextDrawFont(playerid, g_MenuPageNumber[playerid], 2);
	PlayerTextDrawSetProportional(playerid, g_MenuPageNumber[playerid], 1);

	//header test
	g_MenuHeaderText[playerid] = CreatePlayerTextDraw(playerid, 128.333312, 139.377761, "header");
	PlayerTextDrawLetterSize(playerid, g_MenuHeaderText[playerid], 0.315000, 1.247407);
	PlayerTextDrawAlignment(playerid, g_MenuHeaderText[playerid], 1);
	PlayerTextDrawColor(playerid, g_MenuHeaderText[playerid], 0xC0C0C0FF);
	PlayerTextDrawSetShadow(playerid, g_MenuHeaderText[playerid], 0);
	PlayerTextDrawSetOutline(playerid, g_MenuHeaderText[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, g_MenuHeaderText[playerid], 0x00000033);
	PlayerTextDrawFont(playerid, g_MenuHeaderText[playerid], 2);
	PlayerTextDrawSetProportional(playerid, g_MenuHeaderText[playerid], 1);

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

		g_MenuItems[playerid][i] = CreatePlayerTextDraw(playerid, x, y, "_");
		PlayerTextDrawBackgroundColor(playerid, g_MenuItems[playerid][i], 0xD3D3D344);
		PlayerTextDrawFont(playerid, g_MenuItems[playerid][i], 5);
		PlayerTextDrawLetterSize(playerid, g_MenuItems[playerid][i], 1.430000, 5.700000);
		PlayerTextDrawColor(playerid, g_MenuItems[playerid][i], -1);
		PlayerTextDrawSetOutline(playerid, g_MenuItems[playerid][i], 1);
		PlayerTextDrawSetProportional(playerid, g_MenuItems[playerid][i], 1);
		PlayerTextDrawUseBox(playerid, g_MenuItems[playerid][i], 1);
		PlayerTextDrawBoxColor(playerid, g_MenuItems[playerid][i], 0);
		PlayerTextDrawTextSize(playerid, g_MenuItems[playerid][i], 61.000000, 54.000000);
		PlayerTextDrawSetSelectable(playerid, g_MenuItems[playerid][i], 1);

		g_MenuItemText[playerid][i] = CreatePlayerTextDraw(playerid, x + 31.0, y, "_");
		PlayerTextDrawFont(playerid, g_MenuItemText[playerid][i], 2);
		PlayerTextDrawLetterSize(playerid, g_MenuItemText[playerid][i], 0.199999, 0.6);
		PlayerTextDrawAlignment(playerid, g_MenuItemText[playerid][i], 2);
		PlayerTextDrawSetOutline(playerid, g_MenuItemText[playerid][i], 0);
		PlayerTextDrawSetProportional(playerid, g_MenuItemText[playerid][i], 1);
		PlayerTextDrawTextSize(playerid, g_MenuItemText[playerid][i], 0.0, 62.0);
		PlayerTextDrawSetShadow(playerid, g_MenuItemText[playerid][i], 0);
		PlayerTextDrawColor(playerid, g_MenuItemText[playerid][i], 0xD3D3D3AA);
	}
}

static stock DestroyModelSelectionPlayerTDs(playerid)
{
	if(g_MenuHeaderText[playerid] != PlayerText:INVALID_TEXT_DRAW) PlayerTextDrawDestroy(playerid, g_MenuHeaderText[playerid]);
	if(g_MenuPageNumber[playerid] != PlayerText:INVALID_TEXT_DRAW) PlayerTextDrawDestroy(playerid, g_MenuPageNumber[playerid]);

	g_MenuHeaderText[playerid] = PlayerText:INVALID_TEXT_DRAW;
	g_MenuPageNumber[playerid] = PlayerText:INVALID_TEXT_DRAW;

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i++)
	{
		if(g_MenuItems[playerid][i] != PlayerText:INVALID_TEXT_DRAW) PlayerTextDrawDestroy(playerid, g_MenuItems[playerid][i]);
		if(g_MenuItemText[playerid][i] != PlayerText:INVALID_TEXT_DRAW) PlayerTextDrawDestroy(playerid, g_MenuItemText[playerid][i]);

		g_MenuItems[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
		g_MenuItemText[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
	}
}

//callbacks
public OnGameModeInit()
{
	//textdraw background
	g_MenuBackground = TextDrawCreate(531.333374, 140.877777, "_");
	TextDrawBackgroundColor(g_MenuBackground, 0);
	TextDrawAlignment(g_MenuBackground, 1);
	TextDrawFont(g_MenuBackground, 0);
	TextDrawLetterSize(g_MenuBackground, 0.000000, 22.912965);
	TextDrawColor(g_MenuBackground, 0);
	TextDrawSetOutline(g_MenuBackground, 0);
	TextDrawSetProportional(g_MenuBackground, true);
	TextDrawSetShadow(g_MenuBackground, 0);
	TextDrawUseBox(g_MenuBackground, true);
	TextDrawBoxColor(g_MenuBackground, 0x000000DD);
	TextDrawTextSize(g_MenuBackground, 121.333328, 0.000000);
	TextDrawSetSelectable(g_MenuBackground, false);

	//right arrow
	g_MenuRightArrow = TextDrawCreate(521.333374, 339.318542, "LD_BEAT:right");
	TextDrawLetterSize(g_MenuRightArrow, 0.000000, 0.000000);
	TextDrawTextSize(g_MenuRightArrow, 5.999938, 7.051818);
	TextDrawAlignment(g_MenuRightArrow, 1);
	TextDrawColor(g_MenuRightArrow, -1);
	TextDrawSetShadow(g_MenuRightArrow, 0);
	TextDrawSetOutline(g_MenuRightArrow, 0);
	TextDrawFont(g_MenuRightArrow, 4);
	TextDrawSetSelectable(g_MenuRightArrow, true);

	//left arrow
	g_MenuLeftArrow = TextDrawCreate(507.000305, 339.074066, "LD_BEAT:left");
	TextDrawLetterSize(g_MenuLeftArrow, 0.000000, 0.000000);
	TextDrawTextSize(g_MenuLeftArrow, 5.999938, 7.051818);
	TextDrawAlignment(g_MenuLeftArrow, 1);
	TextDrawColor(g_MenuLeftArrow, -1);
	TextDrawSetShadow(g_MenuLeftArrow, 0);
	TextDrawSetOutline(g_MenuLeftArrow, 0);
	TextDrawFont(g_MenuLeftArrow, 4);
	TextDrawSetSelectable(g_MenuLeftArrow, true);

	//top banner strip
	g_MenuTopBanner = TextDrawCreate(531.000244, 155.811111, "TopBanner");
	TextDrawLetterSize(g_MenuTopBanner, 0.000000, -0.447120);
	TextDrawTextSize(g_MenuTopBanner, 121.333328, 0.000000);
	TextDrawAlignment(g_MenuTopBanner, 1);
	TextDrawColor(g_MenuTopBanner, 0);
	TextDrawUseBox(g_MenuTopBanner, true);
	TextDrawBoxColor(g_MenuTopBanner, 0x808080FF);
	TextDrawSetShadow(g_MenuTopBanner, 0);
	TextDrawSetOutline(g_MenuTopBanner, 0);
	TextDrawFont(g_MenuTopBanner, 0);

	//bottom banner strip
	g_MenuBottomBanner = TextDrawCreate(531.333618, 338.500305, "BottomBanner");
	TextDrawLetterSize(g_MenuBottomBanner, 0.000000, -0.447120);
	TextDrawTextSize(g_MenuBottomBanner, 120.666656, 0.000000);
	TextDrawAlignment(g_MenuBottomBanner, 1);
	TextDrawColor(g_MenuBottomBanner, 0);
	TextDrawUseBox(g_MenuBottomBanner, true);
	TextDrawBoxColor(g_MenuBottomBanner, 0x808080FF);
	TextDrawSetShadow(g_MenuBottomBanner, 0);
	TextDrawSetOutline(g_MenuBottomBanner, 0);
	TextDrawFont(g_MenuBottomBanner, 0);

	//close button
	g_MenuCloseButton = TextDrawCreate(490.666809, 337.829711, "CLOSE");
	TextDrawLetterSize(g_MenuCloseButton, 0.128333, 0.957036);
	TextDrawTextSize(g_MenuCloseButton, 10.5021, 10.0187);
	TextDrawAlignment(g_MenuCloseButton, 2);
	TextDrawColor(g_MenuCloseButton, 0xC0C0C0FF);
	TextDrawSetShadow(g_MenuCloseButton, 0);
	TextDrawSetOutline(g_MenuCloseButton, 0);
	TextDrawBackgroundColor(g_MenuCloseButton, 0x00000033);
	TextDrawFont(g_MenuCloseButton, 2);
	TextDrawSetProportional(g_MenuCloseButton, true);
	TextDrawSetSelectable(g_MenuCloseButton, true);

	#if defined MS_OnGameModeInit
		return MS_OnGameModeInit();
	#else
		return true;
	#endif
}

public OnPlayerDisconnect(playerid, reason)
{
	g_MenuHeaderText[playerid] = PlayerText:INVALID_TEXT_DRAW;
	g_MenuPageNumber[playerid] = PlayerText:INVALID_TEXT_DRAW;

	for(new i = 0; i < MAX_ITEM_PER_PAGE; i++)
	{
		g_MenuItems[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
		g_MenuItemText[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
	}

	#if defined MS_OnPlayerDisconnect
		return MS_OnPlayerDisconnect(playerid, reason);
	#else
		return true;
	#endif
}

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
	if(g_MenuShown[playerid] && (clickedid == Text:INVALID_TEXT_DRAW || clickedid == g_MenuCloseButton))
	{
		if(g_MenuExtraID[playerid] != PAWN_PLUS_EXTRA_ID)
		{
			CallLocalFunction("OnModelSelectionResponse", "iiiii", playerid, g_MenuExtraID[playerid], 0, 0, MODEL_RESPONSE_CANCEL);
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
	else if(clickedid == g_MenuRightArrow)
	{
		if(g_MenuCurrentPage[playerid] == g_MenuPageCount[playerid]) return false;

		SetModelSelectionMenuPage(playerid, g_MenuCurrentPage[playerid] + 1);
		return true;
	}
	else if(clickedid == g_MenuLeftArrow)
	{
		if(g_MenuCurrentPage[playerid] <= 1) return false;

		else SetModelSelectionMenuPage(playerid, g_MenuCurrentPage[playerid] - 1);
		return true;
	}

	#if defined MS_OnPlayerClickTextDraw
		return MS_OnPlayerClickTextDraw(playerid, Text:clickedid);
	#else
		return true;
	#endif
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
	if(g_MenuShown[playerid] && (GetTickCount() - g_MenuCooldownTick[playerid]) > 600)
	{
		for(new i = 0; i < MAX_ITEM_PER_PAGE; i ++)
		{
			if(g_MenuItems[playerid][i] == playertextid)
			{
				new index, extraid, modelid;
				index = (i + (g_MenuCurrentPage[playerid] - 1) * MAX_ITEM_PER_PAGE);
				extraid = g_MenuExtraID[playerid];
				modelid = list_get(g_MenuModels[playerid], index);

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
		return true;
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
