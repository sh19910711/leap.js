//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#include <assert.h>
#include "exec_obj.h"
#include "listen.h"
#include "splash.h"

//#define TEST

#pragma hdrstop

#include "leapexec.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TForm1 *Form1;
bool first;
exec_obj e, browser;
Listener * earout, * earerr;
HANDLE his_stdin=NULL, his_stdout=NULL, his_stderr=NULL;
extern char parameters[];
const int maxcmds = 10;
int R_lines[maxcmds], M_lines[maxcmds];
int current, R_count, M_count;
TStringList * commands = new TStringList();     // from edit window
char srcdir[500];
bool merge = false;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
commands->Add(" "); // prevents overrunning bounds
for(int i=0;i<maxcmds;i++)
  R_lines[i] = M_lines[i] = 0;
current = 0;
R_count = M_count = 0;
//Application->MessageBox("WinLeap - Windows Interface to Leap\nThis is free software - type Copying for details\nCopyright 1998 - Curt Hill","WinLeap",MB_OK);
first = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LetsGo(TObject *Sender)
{
  if(first) {
   SplashForm->Show();
   e.set_title("Leap Console"); // but this shouldnt show
   char pgmdir[500],dir[500];
   #ifdef TEST
     strcpy(dir,"C:\\leap\\src");
   #else
     GetCurrentDirectory(500,dir);
   #endif
   strcpy(pgmdir,dir);
   strcpy(srcdir,dir);
   strcat(pgmdir,"\\leap.exe ");
   strcat(dir," ");
   strcat(pgmdir,parameters);
   e.set_startdir(dir);
   e.set_create_console(true);
   his_stdin = e.get_stdin_pipe();
   assert(his_stdin!=NULL);
   his_stdout = e.get_stdout_pipe();
   assert(his_stdout!=NULL);
   his_stderr = e.get_stderr_pipe();
   assert(his_stderr!=NULL);
   e.set_timeout(10);
   earout = new Listener(true);
   earout->SetHandle(his_stdout,Receive);
   earerr = new Listener(true);
   earerr->SetHandle(his_stderr,Messages);
   earout->Resume();
   earerr->Resume();
   // look for --merge-stderr
   if(strstr(parameters,"--merge-stderr")) {
     merge = true;
     Messages->Visible = false;
     }

   if(!e.execute(pgmdir)){
     SplashForm->Close();
     Application->MessageBox("Error - LEAP not found.\nWinLeap should be in same directory as LEAP","Error",MB_OK);
     Close();
     }
   else
     Timer1->Enabled = true;  
   first = false;
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CloseForm(TObject *Sender, TCloseAction &Action)
{
 if(e.process_active()) {
    //send the quit
    unsigned long bytes_sent;
    WriteFile(his_stdin,"quit\n",5,&bytes_sent,NULL);
    earout->EndListen();
    earerr->EndListen();
    }
 earout->Terminate();
 earerr->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Exit(TObject *Sender)
{
 Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::About(TObject *Sender)
{
 SplashForm->Show();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Check_Key(TObject *Sender, WORD &Key, TShiftState Shift)
{
 static int position=0, length;

 switch (Key) {
 case VK_RETURN: {
   char s[500];
   static unsigned long bytes_sent = 0;
   static unsigned long len;
   strcpy(s,Send->Text.c_str());
   strcat(s,"\n");
   len = strlen(s);
   int c = Receive->Lines->Count-R_count;
   if(R_lines[current] > 0) {
      int len=0;
      for(int i=0;i<R_lines[current];i++) {
        len += Receive->Lines->Strings[i].Length()+2;
        }
      AnsiString st = Receive->Lines->Text;
      st.Delete(1,len);
      Receive->Lines->Text = st;
      }
   int d = c;
   R_count = Receive->Lines->Count;

   c = Messages->Lines->Count-M_count;
   if(M_lines[current] > 0) {
      int len=0;
      for(int i=0;i<M_lines[current];i++) {
        len += Messages->Lines->Strings[i].Length()+2;
        }
      AnsiString st = Messages->Lines->Text;
      st.Delete(1,len);
      Messages->Lines->Text = st;
      }
   M_lines[current] = c;

   M_count = Messages->Lines->Count;
   R_lines[current] = d;
   M_lines[current] = c;
   current = (current+1)%maxcmds;
//   Receive->Lines->Clear();
//   Messages->Lines->Clear();
   WriteFile(his_stdin,s,len,&bytes_sent,NULL);
   if(s[0]!='\0' && AnsiString(s) != commands->Strings[commands->Count-1]) {
     s[strlen(s)-1] = ' '; // one padding blank
     commands->Add(s);
     }
   position = commands->Count;
   Send->Clear();
   }
   break;

 case VK_UP:
   if(position>0)
     position--;
   Send->Text = commands->Strings[position];
   // This gets the position of the end of line
   length = SendMessage(Send->Handle,EM_LINELENGTH,(WPARAM)-1,0)+1;

   // This sets the cursor to there
   SendMessage(Send->Handle,EM_SETSEL,(WPARAM)(length+1),(LPARAM)(length+1));
   break;
 case VK_DOWN:
   if(position<commands->Count-1)
     position++;
   Send->Text = commands->Strings[position];
   // This gets the position of the end of line
   length = SendMessage(Send->Handle,EM_LINELENGTH,(WPARAM)-1,0)+1;
   #ifdef debug
      tlog.log("VK_Down, length",length);
      tlog.log("Command",commands->Strings[position].c_str());
      #endif
   // This sets the cursor to there
   SendMessage(Send->Handle,EM_SETSEL,(WPARAM)(length+1),(LPARAM)(length+1));
   break;
  }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::SizeChange(TObject *Sender)
{
  Form1->VertScrollBar->Visible = false;
  Form1->HorzScrollBar->Visible = false;
  int horz = GetDeviceCaps(GetDC(ClientHandle),LOGPIXELSX);
  int width = horz/12;
//  int waste = 58 * horz / 100; // Accounts for menu and title
  int waste = 58 * horz / 100 + 2; // Accounts for menu and title
  int s_height = - Send->Font->Height; // Height is negative
  s_height += s_height/2;
  Send->Top = Form1->Height - s_height - waste;
  Send->Width = Form1->Width - width;
  Receive->Width = Form1->Width - width;
  Messages->Width = Form1->Width - width;
  Messages->Height = Form1->Height / 5;
  if (merge) {
    Receive->Top = 1;
    Receive->Height = Send->Top - 8;
    }
  else {
    Receive->Top = Messages->Height + 4;
    Receive->Height = Send->Top - Messages->Height - 8;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Leap1Click(TObject *Sender)
{
 char html_file[500];
 strcpy(html_file,srcdir);
 // peel off the last directory, which should be src
 for(int i=strlen(html_file);html_file[i]!='\\' && i>0;html_file[i--]='\0');
 strcat(html_file,"doc\\user_guide.htm");
 browser.execute_registered(html_file);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SelectFont(TObject *Sender)
{

  if(FontDialog->Execute()) {
    Receive->Font = FontDialog->Font;
    Send->Font = FontDialog->Font;
    Messages->Font = FontDialog->Font;
    SizeChange(Sender);
    Repaint();
    }


}
//---------------------------------------------------------------------------
void __fastcall TForm1::TimeOut(TObject *Sender)
{
 // clear the splash screen
 if(SplashForm->Visible)
   SplashForm->Close();
 Timer1->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Clear_It(TObject *Sender)
{
 Receive->Clear();
 Messages->Clear();
 for(int i = 0;i<maxcmds;i++) {
  R_lines[i]= 0;
  M_lines[i] = 0;
  }
}
//---------------------------------------------------------------------------
