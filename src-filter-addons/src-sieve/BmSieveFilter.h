/*
	BmSieveFilter.h

		$Id$
*/
/*************************************************************************/
/*                                                                       */
/*  Beam - BEware Another Mailer                                         */
/*                                                                       */
/*  http://www.hirschkaefer.de/beam                                      */
/*                                                                       */
/*  Copyright (C) 2002 Oliver Tappe <beam@hirschkaefer.de>               */
/*                                                                       */
/*  This program is free software; you can redistribute it and/or        */
/*  modify it under the terms of the GNU General Public License          */
/*  as published by the Free Software Foundation; either version 2       */
/*  of the License, or (at your option) any later version.               */
/*                                                                       */
/*  This program is distributed in the hope that it will be useful,      */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*  General Public License for more details.                             */
/*                                                                       */
/*  You should have received a copy of the GNU General Public            */
/*  License along with this program; if not, write to the                */
/*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,         */
/*  Boston, MA  02111-1307, USA.                                         */
/*                                                                       */
/*************************************************************************/


#ifndef _BmSieveFilter_h
#define _BmSieveFilter_h

#include <Archivable.h>
#include <Autolock.h>

extern "C" {
	#include "sieve_interface.h"
}

#include "BmFilterAddon.h"
#include "BmFilterAddonPrefs.h"

#define BM_MAX_MATCH_COUNT 5

/*------------------------------------------------------------------------------*\
	BmSieveFilter 
		-	implements filtering through SIEVE
\*------------------------------------------------------------------------------*/
class BmSieveFilter : public BmFilterAddon {
	typedef BmFilterAddon inherited;

	// archivable components:
	static const char* const MSG_VERSION;
	static const char* const MSG_CONTENT;
	static const int16 nArchiveVersion;

public:
	BmSieveFilter( const BmString& name, const BMessage* archive);
	virtual ~BmSieveFilter();
	
	// native methods:
	bool CompileScript();
	void RegisterCallbacks( sieve_interp_t* interp);
	BLocker* SieveLock();

	// implementations for abstract BmFilterAddon-methods:
	bool Execute( void* msgContext);
	bool SanityCheck( BmString& complaint, BmString& fieldName);
	status_t Archive( BMessage* archive, bool deep = true) const;
	BmString ErrorString() const;

	// SIEVE-callbacks:
	static int sieve_redirect( void* action_context, void* interp_context, 
			   						void* script_context, void* message_context, 
			   						const char** errmsg);
	static int sieve_keep( void* action_context, void* interp_context, 
			   				  void* script_context, void* message_context, 
			   				  const char** errmsg);
	static int sieve_discard( void* action_context, void* interp_context, 
			   				     void* script_context, void* message_context, 
			   				     const char** errmsg);
	static int sieve_fileinto( void* action_context, void* interp_context, 
			   				  		void* script_context, void* message_context, 
			   				  		const char** errmsg);
	static int sieve_notify( void* action_context, void*, 
			   				  	 void*, void* message_context, 
			   				  	 const char**);
	static int sieve_get_size( void* message_context, int* size);
	static int sieve_get_header( void* message_context, const char* header,
			  							  const char*** contents);
	static int sieve_parse_error( int lineno, const char *msg, 
											void *interp_context, void *script_context);
	// SIEVE-helpers:
	static void SetMailFlags( sieve_imapflags_t* flags, BmMsgContext* msgContext);

	static int sieve_execute_error( const char* msg, void* interp_context,
											  void* script_context, void* message_context);

	// getters:
	inline const BmString &Content() const	{ return mContent; }
	inline const BmString &Name() const	{ return mName; }

	inline int LastErrVal() const			{ return mLastErrVal; }
	inline const BmString &LastErr() const	{ return mLastErr; }
	inline const BmString &LastSieveErr() const { return mLastSieveErr; }

	// setters:
	inline void Content( const BmString &s){ mContent = s; }

protected:
	BmString mName;
							// the name of this filter-implementation
	BmString mContent;
							// the SIEVE-script represented by this filter
	sieve_script_t* mCompiledScript;
							// the compiled SIEVE-script, ready to be thrown at messages
	sieve_interp_t* mSieveInterp;
							// the interpreter that compiled the SIEVE-script
	int mLastErrVal;
							// last error-value we got
	BmString mLastErr;
							// the last (general) error that occurred
	BmString mLastSieveErr;
							// the last SIEVE-error that occurred
	static BLocker* nSieveLock;

private:
	BmSieveFilter();									// hide default constructor
	// Hide copy-constructor and assignment:
	BmSieveFilter( const BmSieveFilter&);
	BmSieveFilter operator=( const BmSieveFilter&);

};



/*------------------------------------------------------------------------------*\
	BmGraphicalSieveFilter 
		-	additionally supports graphical editing of the filter
\*------------------------------------------------------------------------------*/
class BmGraphicalSieveFilter : public BmSieveFilter {
	typedef BmSieveFilter inherited;

	friend class BmSieveFilterPrefs;

	// archivable components:
	static const char* const MSG_MATCH_COUNT;
	static const char* const MSG_MATCH_ANYALL;
	static const char* const MSG_MATCH_MAILPART;
	static const char* const MSG_MATCH_FIELDNAME;
	static const char* const MSG_MATCH_OPERATOR;
	static const char* const MSG_MATCH_VALUE;
	static const char* const MSG_FILEINTO;
	static const char* const MSG_FILEINTO_VALUE;
	static const char* const MSG_DISCARD;
	static const char* const MSG_SET_STATUS;
	static const char* const MSG_SET_STATUS_VALUE;
	static const char* const MSG_SET_IDENTITY;
	static const char* const MSG_SET_IDENTITY_VALUE;
	static const char* const MSG_STOP_PROCESSING;

public:
	BmGraphicalSieveFilter( const BmString& name, const BMessage* archive);
	
	// native methods:
	bool BuildScriptFromStrings();

	// overrides of BmSieve-base:
	bool SanityCheck( BmString& complaint, BmString& fieldName);
	status_t Archive( BMessage* archive, bool deep = true) const;

private:

	// stuff used by graphical editor:
	int16 mMatchCount;
	BmString mMatchAnyAll;
	BmString mMatchMailPart[BM_MAX_MATCH_COUNT];
	BmString mMatchFieldName[BM_MAX_MATCH_COUNT];
	BmString mMatchOperator[BM_MAX_MATCH_COUNT];
	BmString mMatchValue[BM_MAX_MATCH_COUNT];
	bool mActionFileInto;
	BmString mActionFileIntoValue;
	bool mActionDiscard;
	bool mActionSetStatus;
	BmString mActionSetStatusValue;
	bool mActionSetIdentity;
	BmString mActionSetIdentityValue;
	bool mStopProcessing;

	BmGraphicalSieveFilter();									// hide default constructor
	// Hide copy-constructor and assignment:
	BmGraphicalSieveFilter( const BmGraphicalSieveFilter&);
	BmGraphicalSieveFilter operator=( const BmGraphicalSieveFilter&);

};



/*------------------------------------------------------------------------------*\
	BmSieveFilterPrefs
		-	
\*------------------------------------------------------------------------------*/

class BmCheckControl;
class BmMenuControl;
class BmTextControl;

#define BM_ANY_ALL_SELECTED			'bmTa'
#define BM_ACTION_SELECTED				'bmTb'
#define BM_MAILPART_SELECTED			'bmTc'
#define BM_OPERATOR_SELECTED			'bmTd'
#define BM_SELECT_FILTER_VALUE		'bmTe'
#define BM_SELECT_ACTION_VALUE		'bmTf'
#define BM_ADD_FILTER_LINE				'bmTg'
#define BM_REMOVE_FILTER_LINE			'bmTh'

#define BM_FILEINTO_CHANGED			'bmTi'
#define BM_FILEINTO_SELECTED			'bmTj'
#define BM_DISCARD_CHANGED				'bmTk'
#define BM_SET_STATUS_CHANGED			'bmTl'
#define BM_SET_STATUS_SELECTED		'bmTm'
#define BM_SET_IDENTITY_CHANGED		'bmTn'
#define BM_SET_IDENTITY_SELECTED		'bmTo'
#define BM_STOP_PROCESSING_CHANGED	'bmTp'

class BmSieveFilterPrefs : public BmFilterAddonPrefsView {
	typedef BmFilterAddonPrefsView inherited;

public:
	BmSieveFilterPrefs( minimax minmax);
	virtual ~BmSieveFilterPrefs();
	
	// native methods:

	// implementations for abstract base-class methods:
	const char *Kind() const;
	void ShowFilter( BmFilterAddon* addon);
	void Initialize();
	void Activate();

	// BView overrides:
	void MessageReceived( BMessage* msg);

private:

	static const char* const MSG_IDX;

	void AddFilterLine();
	void RemoveFilterLine();
	void UpdateState();

	VGroup* mFilterGroup;
	VGroup* mActionGroup;
	
	BmMenuControl* mAnyAllControl;
	MButton* mAddButton;
	MButton* mRemoveButton;
	HGroup* mFilterLine[BM_MAX_MATCH_COUNT];
	BmMenuControl* mMailPartControl[BM_MAX_MATCH_COUNT];
	BmMenuControl* mOperatorControl[BM_MAX_MATCH_COUNT];
	BmTextControl* mFieldNameControl[BM_MAX_MATCH_COUNT];
	BmTextControl* mValueControl[BM_MAX_MATCH_COUNT];
	int32 mVisibleLines;
	
	BmCheckControl* mFileIntoControl;
	BmMenuControl* mFileIntoValueControl;
	BmCheckControl* mDiscardControl;
	BmCheckControl* mSetStatusControl;
	BmMenuControl* mSetStatusValueControl;
	BmCheckControl* mSetIdentityControl;
	BmMenuControl* mSetIdentityValueControl;
	BmCheckControl* mStopProcessingControl;

	BmGraphicalSieveFilter* mCurrFilterAddon;

	// Hide copy-constructor and assignment:
	BmSieveFilterPrefs( const BmSieveFilterPrefs&);
	BmSieveFilterPrefs operator=( const BmSieveFilterPrefs&);
};



/*------------------------------------------------------------------------------*\
	BmSieveScriptFilterPrefs
		-	
\*------------------------------------------------------------------------------*/

class BmMultiLineTextControl;

#define BM_TEST_FILTER			'bmTs'

class BmSieveScriptFilterPrefs : public BmFilterAddonPrefsView {
	typedef BmFilterAddonPrefsView inherited;

public:
	BmSieveScriptFilterPrefs( minimax minmax);
	virtual ~BmSieveScriptFilterPrefs();
	
	// native methods:

	// implementations for abstract base-class methods:
	const char *Kind() const;
	void ShowFilter( BmFilterAddon* addon);
	void Initialize();

	// BView overrides:
	void MessageReceived( BMessage* msg);

private:

	BmMultiLineTextControl* mContentControl;
	MButton* mTestButton;

	BmSieveFilter* mCurrFilterAddon;

	// Hide copy-constructor and assignment:
	BmSieveScriptFilterPrefs( const BmSieveScriptFilterPrefs&);
	BmSieveScriptFilterPrefs operator=( const BmSieveScriptFilterPrefs&);
};

#endif
