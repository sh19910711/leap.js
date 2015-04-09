//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("leapexec.cpp", Form1);
USERES("winleap.res");
USEUNIT("Exec_obj.cpp");
USEUNIT("Listen.cpp");
USEFORM("Splash.cpp", SplashForm);
//---------------------------------------------------------------------------
char parameters[500];

WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR p, int)
{
    strcpy(parameters, p);
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TSplashForm), &SplashForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
