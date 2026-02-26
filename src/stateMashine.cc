#include "stateMashine.h"

void SetSMFlag(SMFlags &flags, SMFlags flag, bool value)
{
    // PrintLog("BF" + String(flags));
    flags = value ? flags | flag : flags & ~flag;
    // PrintLog("ff" + String(flags));
}

State::State(char *name) : transitions(), stateActions()
{
    SetName(name);
}

State *State::MakeTransition(SMFlags flags)
{
    for (size_t i = 0; i < transitions.size(); i++)
    {
        auto trsn = transitions[i];
        if (trsn->CheckFlags(flags))
        {
            trsn->RunActions();
            return trsn->GetNext();
        }
    }
    return this;
}
void State::RunActions()
{
    for (size_t i = 0; i < stateActions.size(); i++)
    {
        stateActions[i]();
    }
}
void State::addAction(void (*action)())
{
    stateActions.add(action);
}
void State::AddTrsn(Transition *trsn)
{
    transitions.add(trsn);
}

void State::SetName(char *name)
{
    this->name = name;
    PrintLog(name);
}

Transition::Transition(State *nextState) : nextState(nextState)
{
    condision = AUTOTRSN;
}
Transition::Transition(State *nextState, void (*action)()) : nextState(nextState)
{
    trsnActions.add(action);
    condision = AUTOTRSN;
}
Transition::Transition(State *nextState, Condision condision) : nextState(nextState), condision(condision) {};
Transition::Transition(State *nextState, void (*action)(), Condision condision) : nextState(nextState), condision(condision)
{
    trsnActions.add(action);
}

bool Transition::CheckFlags(SMFlags flags)
{
    SMFlags Tmasked = flags & condision.TrueFlags;
    SMFlags Fmasked = flags & condision.FalseFlags;
    // PrintLog(String(masked) + " " + String(condision));
    return Tmasked == condision.TrueFlags && Fmasked == 0;
}

State *Transition::GetNext() { return nextState; };
void Transition::RunActions()
{
    for (size_t i = 0; i < trsnActions.size(); i++)
    {
        trsnActions[i]();
    }
}
void Transition::addAction(void (*action)())
{
    trsnActions.add(action);
}

stateMashine::stateMashine(State *curentState, State *anyState) : curentState(curentState), anyState(anyState) {};
stateMashine::~stateMashine() {};
void stateMashine::SMIteration()
{
    // PrintLog("IN state ");
    // PrintLogln(curentState->name);
    curentState->RunActions();
    curentState = curentState->MakeTransition(GlobalSMFlags);
}
