//---------------------------------------------------------------------------
#ifndef leapexecH
#define leapexecH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TEdit *Send;
	TMemo *Receive;
	TMainMenu *MainMenu1;
	TMenuItem *File1;
	TMenuItem *Exit1;
	TMenuItem *Help1;
	TMenuItem *About1;
	TMemo *Messages;
	TMenuItem *Leap1;
	TFontDialog *FontDialog;
	TMenuItem *ChooseFont1;
	TMenuItem *Options1;
	TTimer *Timer1;
	TMenuItem *Clear1;
	void __fastcall LetsGo(TObject *Sender);
	void __fastcall CloseForm(TObject *Sender, TCloseAction &Action);
	void __fastcall SizeChange(TObject *Sender);
	void __fastcall Exit(TObject *Sender);
	void __fastcall About(TObject *Sender);
	void __fastcall Check_Key(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Leap1Click(TObject *Sender);
	void __fastcall SelectFont(TObject *Sender);
	void __fastcall TimeOut(TObject *Sender);
	void __fastcall Clear_It(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
