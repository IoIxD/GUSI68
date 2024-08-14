
#ifndef _GUSISpecific_
#define _GUSISpecific_

#ifndef GUSI_SOURCE

typedef struct GUSISpecific GUSISpecific;

#else

#include <Types.h>
#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif

extern "C"
{
	typedef void (*GUSISpecificDestructor)(void *);
}

class GUSISpecific
{
	friend class GUSISpecificTable;

public:
	GUSISpecific(GUSISpecificDestructor destructor = nil);
	~GUSISpecific();

	void Destruct(void *data);

private:
	GUSISpecificDestructor fDestructor;
	unsigned fID;
	static unsigned sNextID;
};

class GUSISpecificTable
{
	friend class GUSISpecific;

public:
	GUSISpecificTable();
	~GUSISpecificTable();
	void *GetSpecific(const GUSISpecific *key) const;
	void SetSpecific(const GUSISpecific *key, void *value);
	void DeleteSpecific(const GUSISpecific *key);

private:
	static void Register(GUSISpecific *key);
	static void Destruct(GUSISpecific *key);

	void ***fValues;
	unsigned fAlloc;

	bool Valid(const GUSISpecific *key) const;

	static GUSISpecific ***sKeys;
	static unsigned sKeyAlloc;
};

template <class T, GUSISpecificDestructor D>
class GUSISpecificData
{
public:
	GUSISpecificData() : fKey(D) {}
	T &operator*() { return *get(); }
	T *operator->() { return get(); }

	const GUSISpecific *Key() const { return &fKey; }
	T *get(GUSISpecificTable *table);
	T *get();

protected:
	GUSISpecific fKey;
};

template <class T, GUSISpecificDestructor D>
T *GUSISpecificData<T, D>::get(GUSISpecificTable *table)
{
	void *data = table->GetSpecific(&fKey);

	if (!data)
		table->SetSpecific(&fKey, data = new T);

	return static_cast<T *>(data);
}

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

inline GUSISpecific::GUSISpecific(GUSISpecificDestructor destructor)
	: fDestructor(destructor), fID(sNextID++)
{
	GUSISpecificTable::Register(this);
}

inline GUSISpecific::~GUSISpecific()
{
	GUSISpecificTable::Destruct(this);
}

inline void GUSISpecific::Destruct(void *data)
{
	if (fDestructor)
		fDestructor(data);
}

inline bool GUSISpecificTable::Valid(const GUSISpecific *key) const
{
	return key && key->fID < fAlloc;
}

inline GUSISpecificTable::GUSISpecificTable()
	: fValues(nil), fAlloc(0)
{
}

inline void *GUSISpecificTable::GetSpecific(const GUSISpecific *key) const
{
	if (Valid(key))
		return fValues[0][key->fID];
	else
		return nil;
}

#endif /* GUSI_SOURCE */

#endif /* _GUSISpecific_ */
