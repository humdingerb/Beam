/*
	BmSpamFilter.h

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


#ifndef _BmSpamFilter_h
#define _BmSpamFilter_h

#include <Archivable.h>
#include <Autolock.h>

#include <deque>

#include "BmFilterAddon.h"
#include "BmFilterAddonPrefs.h"

#include "BmMemIO.h"

/*------------------------------------------------------------------------------*\
	BmSpamFilter 
		-	implements filtering through SIEVE
\*------------------------------------------------------------------------------*/
class BmSpamFilter : public BmFilterAddon {
	typedef BmFilterAddon inherited;
	
	friend class SpamTest;

	/*------------------------------------------------------------------------------*\
		OsbfClassifier is a C++-"port" of a classifier contained in CRM114 
		(the Controllable	Regex Mutilator, a language for filtering spam, 
		see crm114.sourceforge.net).
		I have simply ripped out the OSBF classifier (OSBF stands for 'orthogonal 
		sparse bigram, Fidelis style') which is used to classify the mails that
		Beam sees. It is a statistical filter, not quite Bayesian, but very similar.
		[the OSBF-classifier is copyright by Fidelis Assis]
		[crm114 is copyright William S. Yerazunis]
		Thanks to these guys and everyone on the crm114-lists!
	\*------------------------------------------------------------------------------*/
	class OsbfClassifier
	{
	public:
		OsbfClassifier();
		~OsbfClassifier();
		void Initialize();
		const BmString& ErrorString() const;
		bool LearnAsSpam( BmMsgContext* msgContext)
													{ return Learn(msgContext, true, false); }
		bool UnlearnAsSpam( BmMsgContext* msgContext)
													{ return Learn(msgContext, true, true); }
		bool LearnAsTofu( BmMsgContext* msgContext)
													{ return Learn(msgContext, false, false); }
		bool UnlearnAsTofu( BmMsgContext* msgContext)
													{ return Learn(msgContext, false, true); }
		bool Classify( BmMsgContext* msgContext);
		
	private:

		typedef struct
		{
		  unsigned long hash;
		  unsigned long key;
		  unsigned long value;
		} FeatureBucket;
		
		typedef struct
		{
			unsigned char version[4];
			unsigned long buckets;	/* number of buckets in the file */
			unsigned long learnings;	/* number of trainings executed */
		} Header;
		
		////////////////////////////////////////////////////////////////////
		//
		//     the hash coefficient table should be full of relatively
		//     prime numbers, and preferably superincreasing, though both of 
		//     those are not strict requirements.
		//
		static const long HashCoeff[20];
		
		static float FeatureWeight[6];

		static unsigned char FileVersion[4];
		
		/* max feature value */
		static const unsigned long WindowLen;

		/* max feature value */
		static const unsigned long FeatureBucketValueMax;
		/* max number of features */
		static const unsigned long DefaultFileLength;
		
		/* shall we try to do small cleanups automatically, if the hash-chains
		   get too long? */ 
		static const bool DoMicrogroom;
		/* max chain len - microgrooming is triggered after this, if enabled */ 
		static const unsigned long MicrogroomChainLength;
		/* maximum number of buckets groom-zeroed */
		static const unsigned long MicrogroomStopAfter;
		/* minimum ratio between max and min P(F|C) */
		static const unsigned long MinPmaxPminRatio;
	
	public:
		static void Microgroom( FeatureBucket* hash, Header* header,
										unsigned long hindex);
	private:
		static void PackData( Header* header, FeatureBucket* hash,
									 unsigned long packstart, unsigned long packlen);
		static void PackDataSeg( Header* header, FeatureBucket* hash,
										 unsigned long packstart, unsigned long packlen);

		/*------------------------------------------------------------------------------*\
			FeatureFilter
				-	filters the mailtext for "features" (words)
		\*------------------------------------------------------------------------------*/
		class FeatureFilter : public BmMemFilter {
			typedef BmMemFilter inherited;
		
		public:
			FeatureFilter( BmMemIBuf* input, uint32 blockSize=nBlockSize);
		
		protected:
			// overrides of BmMailFilter base:
			void Filter( const char* srcBuf, uint32& srcLen, 
							 char* destBuf, uint32& destLen);
		};
	


		/*------------------------------------------------------------------------------*\
			FeatureLearner
				-	implements the learning of features into the given feature-class
		\*------------------------------------------------------------------------------*/
		struct FeatureLearner : public BmMemBufConsumer::Functor {
			FeatureLearner( FeatureBucket* hash, Header* header, bool revert);
			~FeatureLearner();

			status_t operator() (char* buf, uint32 bufLen);

			void Finalize();

			FeatureBucket* mHash;
			Header* mHeader;
			bool mRevert;
			deque<unsigned long> mHashpipe;
			char *mSeenFeatures;
			status_t mStatus;
		};



		/*------------------------------------------------------------------------------*\
			FeatureClassifier
				-	implements the classifying of features into one of two (or more)
					feature-classes
		\*------------------------------------------------------------------------------*/
		struct FeatureClassifier : public BmMemBufConsumer::Functor {
			FeatureClassifier( FeatureBucket* spamHash, Header* spamHeader,
									 FeatureBucket* tofuHash, Header* tofuHeader);
			~FeatureClassifier();

			status_t operator() (char* buf, uint32 bufLen);
			
			void Finalize();

			static const uint32 MaxHash = 2;
					// we only have to hashes: spam and tofu

			FeatureBucket* mHash[MaxHash];
			Header* mHeader[MaxHash];

			deque<unsigned long> mHashpipe;
			status_t mStatus;
			
			bool mClassifiedAsSpam;
			bool mClassifiedAsTofu;

			char *mSeenFeatures[MaxHash];
			unsigned long mHits[MaxHash];
				// actual hits per feature per classifier
			unsigned long mTotalHits[MaxHash];
				// actual total hits per classifier
			unsigned long mLearnings[MaxHash];
				// total learnings per classifier
			unsigned long mTotalLearnings;
			unsigned long mTotalFeatures;
				//  total features
			unsigned long mUniqueFeatures[MaxHash];
				//  found features per class
			unsigned long mMissedFeatures[MaxHash];
				//  missed features per class
			double mPtc[MaxHash];	
				// current running probability of this class
			double mPltc[MaxHash];	
				// current local probability of this class
			unsigned long mHashLen[MaxHash];
			const char *mHashName[MaxHash];
		};



		bool Learn( BmMsgContext* msgContext, bool learnAsSpam, bool revert);
		void Store();
		status_t CreateDataFile( const BmString& filename);
		status_t ReadDataFile( const BmString& filename, Header& header,
									  FeatureBucket*& hash);
		status_t WriteDataFile( const BmString& filename, Header& header,
										FeatureBucket*& hash);
		
		Header mSpamHeader;
		FeatureBucket* mSpamHash;
		bool mNeedToStoreSpam;
		Header mTofuHeader;
		FeatureBucket* mTofuHash;
		bool mNeedToStoreTofu;
		BLocker mLock;
		BmString mLastErr;
	};

public:
	BmSpamFilter( const BmString& name, const BMessage* archive);
	virtual ~BmSpamFilter();
	
	// implementations for abstract BmFilterAddon-methods:
	bool Execute( BmMsgContext* msgContext);
	bool SanityCheck( BmString& complaint, BmString& fieldName);
	status_t Archive( BMessage* archive, bool deep = true) const;
	BmString ErrorString() const;
	void Initialize()							{ nClassifier.Initialize(); }

	// getters:
	inline const BmString &Name() const	{ return mName; }

	// setters:

	// archivable components:
	static const char* const MSG_VERSION;
	static const int16 nArchiveVersion;

protected:
	BmString mName;
							// the name of this filter-implementation

	static OsbfClassifier nClassifier;
	
private:
	BmSpamFilter();									// hide default constructor
	// Hide copy-constructor and assignment:
	BmSpamFilter( const BmSpamFilter&);
	BmSpamFilter operator=( const BmSpamFilter&);

};



/*------------------------------------------------------------------------------*\
	BmSpamFilterPrefs
		-	
\*------------------------------------------------------------------------------*/

class MButton;

enum {
	BM_SHOW_STATISTICS		= 'bmTa',
};


class BmSpamFilterPrefs : public BmFilterAddonPrefsView {
	typedef BmFilterAddonPrefsView inherited;

public:
	BmSpamFilterPrefs( minimax minmax);
	virtual ~BmSpamFilterPrefs();
	
	// native methods:

	// implementations for abstract base-class methods:
	const char *Kind() const;
	void ShowFilter( BmFilterAddon* addon);
	void Initialize();
	void Activate();

	// BView overrides:
	void MessageReceived( BMessage* msg);

private:

	MButton* mStatisticsButton;

	BmSpamFilter* mCurrFilterAddon;

	// Hide copy-constructor and assignment:
	BmSpamFilterPrefs( const BmSpamFilterPrefs&);
	BmSpamFilterPrefs operator=( const BmSpamFilterPrefs&);
};



#endif