#ifndef STATEMASHINE_H
#define STATEMASHINE_H
#include <Arduino.h>
#include "simpleList.h"
#include "Debuging.h"
#include "globalData.h"

void SetSMFlag(SMFlags &flags, SMFlags flag, bool value);

#define AUTOTRSN (SMFlags)0
#define WIFICONECTED (SMFlags)2
#define MDNSFINDED (SMFlags)4
#define TCPCONECTED (SMFlags)8
#define TRANSIVEROPEN (SMFlags)16
#define RESSIVEROPEN (SMFlags)32

struct Condision
{
    SMFlags TrueFlags = 0;
    SMFlags FalseFlags = 0;
    Condision() {};
    Condision(SMFlags _TrueFlags)
    {
        TrueFlags = _TrueFlags;
    };
    Condision(SMFlags _TrueFlags, SMFlags _FalseFlags)
    {
        TrueFlags = _TrueFlags;
        FalseFlags = _FalseFlags;
    }
};

class Transition;

class State
{
private:
    List<Transition *> transitions;
    List<void (*)()> stateActions;

public:
    char *name = "";
    State(char *name);
    State *MakeTransition(SMFlags flags);
    void RunActions();
    void addAction(void (*action)());
    void AddTrsn(Transition *trsn);
    void SetName(char *name);
};

class Transition
{
private:
    State *nextState;
    List<void (*)()> trsnActions = List<void (*)()>();
    Condision condision;

public:
    Transition(State *nextState);
    Transition(State *nextState, void (*action)());
    Transition(State *nextState, Condision condision);
    Transition(State *nextState, void (*action)(), Condision condision);

    bool CheckFlags(SMFlags flags);
    State *GetNext();
    void RunActions();
    void addAction(void (*action)());
};

class stateMashine
{
private:
    // List<State> states;
    State *curentState;
    State *anyState;
    // SMFlags flags;

public:
    stateMashine(State *curentState, State *anyState);
    ~stateMashine();
    void SMIteration();
};

#endif