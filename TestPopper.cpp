/*
	TestPopper.cpp
		$Id$
*/

#include <stdio.h>
#include <Application.h>
#include <MButton.h>
#include <MWindow.h>

#include "BmConnectionWin.h"

static char buf[30];

// for testing:
#define BM_MSG_NOCH_EINER 'bmt1'

class GenericWin : public MWindow
{
public:
	GenericWin();
	~GenericWin();
	bool QuitRequested();
};

class GenericApp : public BApplication
{
	BmConnectionWin *win;
	GenericWin *gw;
	int32 count;
public:
	GenericApp();
	~GenericApp();
	void MessageReceived(BMessage*);
};

int main()
{
	GenericApp *testApp = new GenericApp;
	try {
		be_app->Run();
	} 
	catch( exception &e) {
		BM_LOGERR( BString("Oops: %s") << e.what());
	}
	delete testApp;
	delete Beam::Prefs;
}

GenericWin::GenericWin()
: MWindow( BRect(5,20,0,0), "hit me!",
					B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 
					B_ASYNCHRONOUS_CONTROLS)
{
	MView *mOuterGroup = 
		new VGroup(
				new MButton("Noch einer...",new BMessage(BM_MSG_NOCH_EINER), be_app),
			0
		);
	AddChild( dynamic_cast<BView*>(mOuterGroup));
}

GenericWin::~GenericWin()
{ }

bool GenericWin::QuitRequested() {
	be_app->PostMessage( B_QUIT_REQUESTED);
	return true;
}

GenericApp::GenericApp()
: BApplication("application/x-vnd.OT-Generic"), win(0), count(0)
{
	if (!BmPrefs::InitPrefs())
		exit(10);
	Beam::LogHandler = new BmLogHandler();
	win = new BmConnectionWin( "ConnectionWin", this);
	win->Hide();
	win->Show();
	gw = new GenericWin();
	gw->Show();
}

GenericApp::~GenericApp()
{
	delete Beam::LogHandler;
}

void GenericApp::MessageReceived(BMessage* msg) {
	BMessage* archive;
	BmPopAccount acc;
	switch( msg->what) {
		case BM_MSG_NOCH_EINER: 

			count++;
			if (count % 3 == 2) {
				archive = new BMessage(BM_POPWIN_FETCHMSGS);
				sprintf(buf, "mailtest@kiwi:110");
				acc.Name( buf);
				acc.Username( "mailtest");
				acc.Password( "mailtest");
				acc.POPServer( "kiwi");
				acc.PortNr( 110);
				acc.SMTPPortNr( 25);
				acc.Archive( archive, false);
				win->PostMessage( archive);
				delete archive;
			} else if (count % 3 == 0) {
				archive = new BMessage(BM_POPWIN_FETCHMSGS);
				sprintf(buf, "zooey@kiwi:110");
				acc.Name( buf);
				acc.Username( "zooey");
				acc.Password( "leeds#42");
				acc.POPServer( "kiwi");
				acc.PortNr( 110);
				acc.SMTPPortNr( 25);
				acc.Archive( archive, false);
				win->PostMessage( archive);
				delete archive;
			} else if (count % 3 == 1) {
				archive = new BMessage(BM_POPWIN_FETCHMSGS);
				sprintf(buf, "mailtest2@kiwi:114");
				acc.Name( buf);
				acc.Username( "mailtest2");
				acc.Password( "mailtest2");
				acc.POPServer( "kiwi");
				acc.PortNr( 114);
				acc.SMTPPortNr( 25);
				acc.Archive( archive, false);
				win->PostMessage( archive);
				delete archive;
			}

/*
			archive = new BMessage(BM_POPWIN_FETCHMSGS);
			sprintf(buf, "mailtest@kiwi:112");
			acc.Name( buf);
			acc.Username( "mailtest");
			acc.Password( "mailtest");
			acc.POPServer( "kiwi");
			acc.PortNr( 112);
			acc.SMTPPortNr( 25);
			acc.Archive( archive, false);
			win->PostMessage( archive);
			delete archive;
*/

			break;
		case BM_POPWIN_DONE: 
//			PostMessage( B_QUIT_REQUESTED);
			break;
		default:
			BApplication::MessageReceived( msg);
	}
}
