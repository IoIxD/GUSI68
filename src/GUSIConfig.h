
#ifndef _GUSIConfig_
#define _GUSIConfig_

#ifdef GUSI_SOURCE

#include "GUSIFileSpec.h"

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif

class GUSIConfiguration
{
public:
	enum
	{
		kNoResource = -1,
		kDefaultResourceID = 10240
	};

	static GUSIConfiguration *Instance();
	static GUSIConfiguration *CreateInstance(short resourceID = kDefaultResourceID);

	struct FileSuffix
	{
		char suffix[4];
		OSType suffType;
		OSType suffCreator;
	};
	short fNumSuffices;
	FileSuffix *fSuffices;

	void ConfigureSuffices(short numSuffices, FileSuffix *suffices);

	OSType fDefaultType;
	OSType fDefaultCreator;

	void ConfigureDefaultTypeCreator(OSType defaultType, OSType defaultCreator);
	void SetDefaultFType(const GUSIFileSpec &name) const;

	bool fAutoSpin;

	void ConfigureAutoSpin(bool autoSpin);
	void AutoSpin() const;

	bool fAutoInitGraf;

	void ConfigureAutoInitGraf(bool autoInitGraf);
	void AutoInitGraf();

	bool fAccurateStat;

	void ConfigureAccurateStat(bool accurateState);

	bool fSigPipe;

	void ConfigureSigPipe(bool sigPipe);
	void BrokenPipe();

	bool fSigInt;

	void ConfigureSigInt(bool sigInt);
	void CheckInterrupt();

	bool fSharedOpen;

	void ConfigureSharedOpen(bool sharedOpen);

	bool fHandleAppleEvents;

	void ConfigureHandleAppleEvents(bool handleAppleEvents);

protected:
	GUSIConfiguration(short resourceID = kDefaultResourceID);

private:
	static GUSIConfiguration *sInstance;

	bool fWeOwnSuffices;

	void DoAutoSpin() const;

	void DoAutoInitGraf();

	bool CmdPeriod(const EventRecord *event);
};

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

#ifdef __MRC__
#pragma noinline_func GUSISetupConfig
#endif

extern "C" void GUSISetupConfig();

inline GUSIConfiguration *GUSIConfiguration::Instance()
{
	if (!sInstance)
		GUSISetupConfig();
	if (!sInstance)
		sInstance = new GUSIConfiguration();

	return sInstance;
}

inline GUSIConfiguration *GUSIConfiguration::CreateInstance(short resourceID)
{
	if (!sInstance)
		sInstance = new GUSIConfiguration(resourceID);

	return sInstance;
}

inline void GUSIConfiguration::ConfigureDefaultTypeCreator(OSType defaultType, OSType defaultCreator)
{
	fDefaultType = defaultType;
	fDefaultCreator = defaultCreator;
}

inline void GUSIConfiguration::ConfigureAutoSpin(bool autoSpin)
{
	fAutoSpin = autoSpin;
}

inline void GUSIConfiguration::ConfigureAutoInitGraf(bool autoInitGraf)
{
	fAutoInitGraf = autoInitGraf;
}

inline void GUSIConfiguration::ConfigureSigPipe(bool sigPipe)
{
	fSigPipe = sigPipe;
}

inline void GUSIConfiguration::ConfigureSigInt(bool sigInt)
{
	fSigInt = sigInt;
}

inline void GUSIConfiguration::ConfigureAccurateStat(bool accurateStat)
{
	fAccurateStat = accurateStat;
}
inline void GUSIConfiguration::ConfigureSharedOpen(bool sharedOpen)
{
	fSharedOpen = sharedOpen;
}

inline void GUSIConfiguration::AutoInitGraf()
{
	if (fAutoInitGraf)
		DoAutoInitGraf();
}

#endif /* GUSI_SOURCE */

#endif /* _GUSIConfig_ */
