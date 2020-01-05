#pragma once

class GameMode
{
    bool ready = false;
    bool Loop(bool *bError);
    bool Init(bool *bError);
    bool DeInit(bool *bError);
}