//---------------------------------------------------------------------------
#ifndef ListenH
#define ListenH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
//---------------------------------------------------------------------------
class Listener : public TThread
{
private:
    HANDLE read_hand;
    void __fastcall ClearIt();
    void __fastcall Consolidate();
    TMemo * memo;
    bool done;
    char * buf;
    unsigned long read_bytes;
    /*
    logger l("c:\\projects\\dos_exec\\listen.log");
    This cannot go in listen.cpp or there will be
    thread problems. If we need it again initialize from
    constructor.
    */
protected:
    void __fastcall Execute();
public:
	__fastcall Listener(bool CreateSuspended);
    void __fastcall EndListen();
	void __fastcall SetHandle(HANDLE r, TMemo * m);
};
//---------------------------------------------------------------------------
#endif
