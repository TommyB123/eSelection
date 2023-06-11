#include <a_samp>

#define PP_SYNTAX_AWAIT
#include "eSelection.inc"

main() {}

public OnPlayerSpawn(playerid)
{
    ShowSkinModelMenu(playerid);

    return 1;
}

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
