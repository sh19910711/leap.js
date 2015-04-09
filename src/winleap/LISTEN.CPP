//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#include "leapexec.h"
#pragma hdrstop
/*
  Since this object is used more than once and is threaded
  you absolutely cannot have any global variables here.
  If you do race errors will occur.
*/  
#include "Listen.h"
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Listener::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall Listener::Listener(bool CreateSuspended)
	: TThread(CreateSuspended)
{
  done = false;
  buf = NULL;
  memo = NULL;
}
//---------------------------------------------------------------------------
void __fastcall Listener::SetHandle(HANDLE r, TMemo * m)
{
  read_hand = r;
  memo = m;
}
//---------------------------------------------------------------------------
// Next two are Synchronized to avoid race problems
void __fastcall Listener::ClearIt()
{
    memo->Clear();
}

void __fastcall Listener::Consolidate() {
        // clean prompt out of buf
        bool found_prompt;
        do {  // This works for latest leap, but not earlier
          found_prompt = false;
          char * left = strchr(buf,'[');
          char * right = strchr(buf,']');
          if (left != NULL && right != NULL) {
            if(right[1] == ' ' && right[2] == ':' && right[3] == '-') { // prompt
              char * newbuf = new char[strlen(buf)];
              strcpy(newbuf,&right[5]);
              strcpy(left,newbuf);
              found_prompt = true;
              delete [] newbuf;
              }
            }
           } while (found_prompt);

        do{
          found_prompt = false;
          char * newline = strchr(buf,'\r');
          if(newline != NULL) {
              newline[0]=' ';
              found_prompt = true;
              }
        }while (found_prompt);

        char * s = new char[strlen(buf)+2];
        do{
          found_prompt = false;
          char * newline = strchr(buf,'\n');
          if(newline != NULL) {
            newline[0]='\0';
            memo->Lines->Add(buf);
            strcpy(s,&newline[1]); // copy last part
            strcpy(buf,s); // now copy back
            found_prompt = true;
            }
        }while (found_prompt);
        delete [] s;
        SendMessage(memo->Handle,EM_LINESCROLL,0,memo->Lines->Count-1);
        /*
        AnsiString s = memo->Text;
        s += buf;
//        l.log("Buffer received",buf);
        memo->Text = s;
        SendMessage(memo->Handle,EM_LINESCROLL,0,memo->Lines->Count-1);
        int len = strlen(buf),i;
        for(i=0; i<len; i++)
           buf[i] = '\0';
        buf[i] = '\0';
//        l.log("Consolidate exited");
        */
}

void __fastcall Listener::Execute()
{
	//---- Place thread code here ----
    buf = new char[5000];
    Synchronize(ClearIt);
    while (!Terminated && !done) {
      strcpy(buf,"              ");
      buf[0] = '\0';
      read_bytes = 0;
      for(int i=0;i<5000;i++)
        buf[i]='\0';
      ReadFile(read_hand,buf,5000,&read_bytes,NULL);
      if(read_bytes>0) {
        Synchronize(Consolidate);
        }
      }
    delete [] buf;
}

//---------------------------------------------------------------------------
void __fastcall Listener::EndListen(){
    done = true;
    }
