//---------------------------------------------------------------------------
#ifndef SplashH
#define SplashH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
//---------------------------------------------------------------------------
class TSplashForm : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TButton *Button1;
	TLabel *Label6;
	void __fastcall Clicked(TObject *Sender);
	
private:	// User declarations
public:		// User declarations
	__fastcall TSplashForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TSplashForm *SplashForm;
//---------------------------------------------------------------------------
#endif
