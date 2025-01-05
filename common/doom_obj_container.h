//----------------------------------------------------------------------------------------------
// odamex enum definitions with negative indices (similar to id24 specification) are in
// info.h using negative indices prevents overriding during dehacked (though id24 allows
// this)
//
// id24 allows 0x80000000-0x8FFFFFFF (abbreviated as 0x8000-0x8FFF) for negative indices
// for source port implementation id24 spec no longer uses enum to define the indices for
// doom objects, but until that is implemented enums will continue to be used
//----------------------------------------------------------------------------------------------

#pragma once
#include "info.h" // doom object definitions - including enums with negative indices

/*
extern state_t odastates[]; //statenum_t
extern mobjinfo_t odathings[]; //mobjtype_t
extern const char* odasprnames[]; // spritenum_t

state_t* D_GetOdaState(statenum_t statenum);
mobjinfo_t* D_GetOdaMobjinfo(mobjtype_t mobjtype);
const char* D_GetOdaSprName(spritenum_t spritenum);
*/

#include "hashtable.h"
#include "i_system.h"
#include <vector>
#include <cstddef>
#include <typeinfo>

template <typename ObjType, typename IdxType, typename InOrderContainer>
class DoomObjectContainer;

//----------------------------------------------------------------------------------------------
// DoomObjectContainer replaces the global doom object pointers (states, mobjinfo,
// sprnames) with multi-use objects that can handle negative indices. It also
// auto-resizes, similar to vector, and provides a way to get the size and capacity.
// Existing code cannot rely on an index being greater than the number of types now
// because dehacked does not enforce contiguous indices i.e. frame 405 could jump to frame
// 1055
//----------------------------------------------------------------------------------------------

template <typename ObjType, typename IdxType = int32_t, typename InOrderContainer = std::vector<ObjType> >
class DoomObjectContainer
{

	typedef OHashTable<int, ObjType> LookupTable;
	typedef InOrderContainer DoomObjectContainerData;
	typedef DoomObjectContainer<ObjType, IdxType, InOrderContainer> DoomObjectContainerType;

	DoomObjectContainerData container;
	LookupTable lookup_table;

	static void noop(ObjType p, IdxType idx) { }

  public:
	typedef typename DoomObjectContainerData::reference ObjReference;
	typedef typename DoomObjectContainerData::const_reference ConstObjReference;
	typedef typename LookupTable::iterator iterator;
	typedef typename LookupTable::const_iterator const_iterator;

	typedef void (*ResetObjType)(ObjType, IdxType);
	typedef bool (*CompareObjType)(ObjType, ObjType);

	explicit DoomObjectContainer(ResetObjType f = NULL);
	explicit DoomObjectContainer(size_t count, ResetObjType f = NULL);
	~DoomObjectContainer();

	ObjReference operator[](int);
	ConstObjReference operator[](int) const;
	bool operator==(const ObjType* p) const;
	bool operator!=(const ObjType* p) const;
	// convert to ObjType* to allow pointer arithmetic
	operator const ObjType*() const;
	operator ObjType*();
	// direct access similar to STL data()
	ObjType* data();
	const ObjType* data() const;

	size_t capacity() const;
	size_t size() const;
	void clear();
	void resize(size_t count);
	void reserve(size_t new_cap);
	void insert(const ObjType& obj, IdxType idx);
	void append(const DoomObjectContainerType& dObjContainer);

	iterator begin();
	iterator end();
	const_iterator cbegin();
	const_iterator cend();
	iterator find(IdxType idx);
	const_iterator find(IdxType idx) const;
	bool contains(IdxType idx) const;

	friend ObjType operator-(ObjType obj, DoomObjectContainerType& container);
	friend ObjType operator+(DoomObjectContainerType& container, WORD ofs);

  private:
	ResetObjType rf;
};

//----------------------------------------------------------------------------------------------

// Construction and Destruction

template <typename ObjType, typename IdxType, typename InOrderContainer>
DoomObjectContainer<ObjType, IdxType, InOrderContainer>::DoomObjectContainer(ResetObjType f)
    : rf(f == NULL ? &noop : f)
{
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
DoomObjectContainer<ObjType, IdxType, InOrderContainer>::DoomObjectContainer(size_t count, ResetObjType f)
    : rf(f == NULL ? &noop : f)
{
	this->container.reserve(count);
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
DoomObjectContainer<ObjType, IdxType, InOrderContainer>::~DoomObjectContainer()
{
	clear();
}

// Operators

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::ObjReference DoomObjectContainer<
    ObjType, IdxType, InOrderContainer>::operator[](int idx)
{
	iterator it = this->lookup_table.find(idx);
    if (it == this->end())
    {
	    I_Error("Attempt to access invalid %s at idx %d", typeid(ObjType).name(), idx);
    }
    return it->second;
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::ConstObjReference DoomObjectContainer<
    ObjType, IdxType, InOrderContainer>::operator[](int idx) const
{
    const_iterator it = this->lookup_table.find(idx);
    if (it == this->end())
    {
    	I_Error("Attempt to access invalid %s at idx %d", typeid(ObjType).name(), idx);
    }
    return it->second;
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
bool DoomObjectContainer<ObjType, IdxType, InOrderContainer>::operator==(const ObjType* p) const
{
	return this->container().data() == p;
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
bool DoomObjectContainer<ObjType, IdxType, InOrderContainer>::operator!=(const ObjType* p) const
{
	return this->container().data() != p;
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
DoomObjectContainer<ObjType, IdxType, InOrderContainer>::operator const ObjType*() const
{
	return const_cast<ObjType>(this->container.data());
}
template <typename ObjType, typename IdxType, typename InOrderContainer>
DoomObjectContainer<ObjType, IdxType, InOrderContainer>::operator ObjType*()
{
	return this->container.data();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
ObjType operator-(ObjType obj, DoomObjectContainer<ObjType, IdxType, InOrderContainer>& container)
{
	return obj - container.data();
}
template <typename ObjType, typename IdxType, typename InOrderContainer>
ObjType operator+(DoomObjectContainer<ObjType, IdxType, InOrderContainer>& container, WORD ofs)
{
	return container.data() + ofs;
}

// data functions for quicker access to all objects presently stored

template <typename ObjType, typename IdxType, typename InOrderContainer>
ObjType* DoomObjectContainer<ObjType, IdxType, InOrderContainer>::data()
{
	return this->container.data();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
const ObjType* DoomObjectContainer<ObjType, IdxType, InOrderContainer>::data() const
{
	return this->container.data();
}

// Capacity and Size

template <typename ObjType, typename IdxType, typename InOrderContainer>
size_t DoomObjectContainer<ObjType, IdxType, InOrderContainer>::size() const
{
	return this->container.size();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
size_t DoomObjectContainer<ObjType, IdxType, InOrderContainer>::capacity() const
{
	return this->container.capacity();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
void DoomObjectContainer<ObjType, IdxType, InOrderContainer>::clear()
{
	this->container.clear();
	this->lookup_table.clear();
}

// Allocation changes

template <typename ObjType, typename IdxType, typename InOrderContainer>
void DoomObjectContainer<ObjType, IdxType, InOrderContainer>::resize(size_t count)
{
	this->container.resize(count);
	// this->lookup_table.resize(count);
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
void DoomObjectContainer<ObjType, IdxType, InOrderContainer>::reserve(size_t new_cap)
{
	this->container.reserve(new_cap);
	// this->lookup_table.resize(new_cap);
}

// Insertion

template <typename ObjType, typename IdxType, typename InOrderContainer>
void DoomObjectContainer<ObjType, IdxType, InOrderContainer>::insert(const ObjType& obj, IdxType idx)
{
	this->container.insert(this->container.end(), obj);
	this->lookup_table[static_cast<int>(idx)] = obj;
}

// TODO: more of a copy construct in a sense
template <typename ObjType, typename IdxType, typename InOrderContainer>
void DoomObjectContainer<ObjType, IdxType, InOrderContainer>::append(
    const DoomObjectContainer<ObjType, IdxType, InOrderContainer>& dObjContainer)
{
	for (DoomObjectContainerType::iterator it = dObjContainer.lookup_table.begin();
	     it != dObjContainer.lookup_table.end(); ++it)
	{
		int idx = it->first;
		ObjType obj = it->second;
		this->insert(static_cast<IdxType>(idx), obj);
	}
}

// Iterators

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::iterator DoomObjectContainer<ObjType, IdxType, InOrderContainer>::begin()
{
	return lookup_table.begin();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::iterator DoomObjectContainer<ObjType, IdxType, InOrderContainer>::end()
{
	return lookup_table.end();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::const_iterator DoomObjectContainer<ObjType, IdxType, InOrderContainer>::cbegin()
{
	return lookup_table.begin();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::const_iterator DoomObjectContainer<ObjType, IdxType, InOrderContainer>::cend()
{
	return lookup_table.end();
}

// Lookup

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::iterator DoomObjectContainer<
    ObjType, IdxType, InOrderContainer>::find(IdxType idx)
{
	typename LookupTable::iterator it = this->lookup_table.find(idx);
	if (it != this->lookup_table.end())
	{
		return it;
	}
	return this->lookup_table.end();
}

template <typename ObjType, typename IdxType, typename InOrderContainer>
typename DoomObjectContainer<ObjType, IdxType, InOrderContainer>::const_iterator DoomObjectContainer<
    ObjType, IdxType, InOrderContainer>::find(IdxType idx) const
{
	typename LookupTable::iterator it = this->lookup_table.find(idx);
	if (it != this->lookup_table.end())
	{
		return it;
	}
	return this->lookup_table.end();
}

template<typename ObjType, typename IdxType, typename InOrderContainer>
bool DoomObjectContainer<ObjType, IdxType, InOrderContainer>::contains(IdxType idx) const
{
	return this->find(idx) != this->end();
}

//----------------------------------------------------------------------------------------------
