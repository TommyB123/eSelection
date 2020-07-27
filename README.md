# eSelection

[![sampctl](https://img.shields.io/badge/sampctl-eSelection-2f2f2f.svg?style=for-the-badge)](https://github.com/TommyB123/eSelection)

This library adds the ability to create dynamic model selection menus in your SA-MP gamemodes. It's an edit of the original eSelection include created by the developers of the Interactive Roleplay SA-MP server back in 2013. I've been using it in my own server for many years and made countless adjustments, fixes, optimizations and feature changes to it. After a bit of consideration, I decided to open source the changes I've made.

This edit (well, it's closer to a rewrite) makes use of dynamic PawnPlus containers, adds in an improved API, descriptive text per model preview and also adds the ability to call task-based menu responses, which will be demonstrated below.

![](https://i.imgur.com/wDal7si.png)
![](https://i.imgur.com/kvkg8t8.png)

## Installation

Simply install to your project:

```bash
sampctl package install TommyB123/eSelection
```

Include in your code and begin using the library:

```pawn
#include <eSelection>
```

## Functions

Adds a new model to a model selection list. Descriptive text and preview rotations are supported as optional arguments.

```pawn
AddModelMenuItem(List:menulist, modelid, const text[] = "", bool:usingrotation = false, Float:rotx = 0.0, Float:roty = 0.0, Float:rotz = 0.0, Float:zoom = 1.0)
```

Shows a model selection menu to a player. `OnModelSelectionResponse` will be called when using this function. 

```pawn
ShowModelSelectionMenu(playerid, const header[], extraid, List:items)
```
Shows a model selection menu to a player. Requires waiting for a PawnPlus task to receive the response results.

```pawn
Task:ShowAsyncModelSelectionMenu(playerid, const header[], List:items)
```

Non-task-based model selection menu responses are handled via the callback below.

```pawn
public OnModelSelectionResponse(playerid, extraid, index, modelid, response)
```

`extraid` is the ID of the model menu provided in `ShowModelSelectionMenu`. This functionality is similar to the dialog ID argument in `ShowPlayerDialog`.

`index` is the list index of the model that was clicked. If the first model in a menu is clicked, `index` will 0. If the second model is clicked, `index` will be 1. You get the point, I hope.

`modelid` is the model ID that was clicked. If a player clicks on skin ID 5 in a model menu, this variable would also be 5.

## Relevant constants
`MODEL_RESPONSE_CANCEL` - Response value when a player cancels a model menu.

`MODEL_RESPONSE_SELECT` - Response value when a player clicks on a model inside of a model menu.

The following constants are used in the array response when handling a task-based model menu response.

`E_MODEL_SELECTION_RESPONSE` - The response value to be compared with the constants described above. Equiavelent to the `response` argument in `OnModelSelectionResponse`

`E_MODEL_SELECTION_INDEX` - The index of the model selection menu response. Equivalent to the `index` argument in `OnModelSelectionResponse`

`E_MODEL_SELECTION_MODELID` - The model ID of the model selection menu response. Equivalent to the `modelid` argument in `OnModelSelectionResponse`

## Usage
Example of a traditional model selection response when using `ShowModelSelectionMenu`
```pawn
// define an ID for the model selection menu below
#define MODEL_SELECTION_SKIN_MENU (0)

ShowSkinModelMenu(playerid)
{
    // create a dynamic PawnPlus list to populate with models.
    // you don't need to worry about deleting this list, it's handled by the include once it's passed to it
    new List:skins = list_new();

    // add skin IDs 0, 1, 29 and 60 with "cool people only" text above skin ID 29.
    AddModelMenuItem(skins, 0);
    AddModelMenuItem(skins, 1);
    AddModelMenuItem(skins, 29, "Cool people only");
    AddModelMenuItem(skins, 60);

    // show the menu to the player
    ShowModelSelectionMenu(playerid, "Skins", MODEL_SELECTION_SKIN_MENU, skins);
}

// model selection response
public OnModelSelectionResponse(playerid, extraid, index, modelid, response)
{
    // make sure the extraid matches the skin menu ID
    if(extraid == MODEL_SELECTION_SKIN_MENU)
    {
        // make sure the player actually clicked on a model and not the close button
        if(response == MODEL_RESPONSE_SELECT)
        {
            // assign the player the skin of their choosing
            SetPlayerSkin(playerid, modelid);
            return 1;
        }
    }
}
```

Example of waiting for a task-based model selection menu response with `ShowAsyncModelSelectionMenu`

```pawn
// enable the "await" syntax from PawnPlus before including it
#define PP_SYNTAX_AWAIT

ShowSkinModelMenu(playerid)
{
    // create a dynamic PawnPlus list to populate with models.
    // you don't need to worry about deleting this list, it's handled by the include once it's passed to it
    new List:skins = list_new();

    // add skin IDs 0, 1, 29 and 60 with "cool people only" text above skin ID 29.
    AddModelMenuItem(skins, 0);
    AddModelMenuItem(skins, 1);
    AddModelMenuItem(skins, 29, "Cool people only");
    AddModelMenuItem(skins, 60);

    // declare an array that will be populated with the model selection menu response data
    new response[E_MODEL_SELECTION_INFO];

    // use await_arr and set the response array to the model selection menu result
    await_arr(response) ShowAsyncModelSelectionMenu(playerid, "Skins", skins);

    // make sure the player actually clicked on a model and not the close button
    if(response[E_MODEL_SELECTION_RESPONSE] == MODEL_RESPONSE_SELECT)
    {
        // assign the player the skin of their choosing
        SetPlayerSkin(playerid, response[E_MODEL_SELECTION_MODELID]);
    }
}
```

## Credits
Interactive Roleplay Developer(s) - Original authors of this library

Me - Numerous feature updates, housekeeping, etc