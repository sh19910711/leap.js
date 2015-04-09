//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "Splash.h"
#include "leapexec.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TSplashForm * SplashForm;
//---------------------------------------------------------------------------
__fastcall TSplashForm::TSplashForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSplashForm::Clicked(TObject *Sender)
{
 Close();
 //Form1->Show();
}
//---------------------------------------------------------------------------
