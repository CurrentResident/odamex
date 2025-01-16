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

#include "hashtable.h"
#include "i_system.h"
#include <functional>
#include <vector>
#include <cstddef>
#include <typeinfo>

template <class ObjType, class IdxType, class FreeFunction>
class DoomObjectContainer;

//----------------------------------------------------------------------------------------------
// DoomObjectContainer replaces the global doom object pointers (states, mobjinfo,
// sprnames) with multi-use objects that can handle negative indices. It also
// auto-resizes, similar to vector, and provides a way to get the size and capacity.
// Existing code cannot rely on an index being greater than the number of types now
// because dehacked does not enforce contiguous indices i.e. frame 405 could jump to frame
// 1055. Because of this, iterators should be used when traversing the container.
//----------------------------------------------------------------------------------------------

template <class ObjType, class IdxType = int32_t,
          class FreeFunction = std::function<void(ObjType)>>
class DoomObjectContainer
{

	typedef OHashTable<int, ObjType> LookupTable;
	typedef std::vector<ObjType> DoomObjectContainerData;
	typedef DoomObjectContainer<ObjType, IdxType, FreeFunction> DoomObjectContainerType;

	DoomObjectContainerData container;
	LookupTable lookup_table;

	static void noop(ObjType p, IdxType idx) { }

  public:
	typedef typename DoomObjectContainerData::reference ObjReference;
	typedef typename DoomObjectContainerData::const_reference ConstObjReference;
	typedef typename LookupTable::iterator iterator;
	typedef typename LookupTable::const_iterator const_iterator;

	typedef void (*ResetObjType)(ObjType, IdxType);

	explicit DoomObjectContainer(
	    ResetObjType resetFunc = NULL, 
		FreeFunction freeFunc = [](ObjType) -> void { return; });
	explicit DoomObjectContainer(
	    size_t count, ResetObjType resetFunc = NULL,
	    FreeFunction freeFunc = [](ObjType) -> void { return; });
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
	FreeFunction ff;
};

//----------------------------------------------------------------------------------------------

// Construction and Destruction

template <class ObjType, class IdxType, class FreeFunction>
DoomObjectContainer<ObjType, IdxType, FreeFunction>::DoomObjectContainer(ResetObjType resetFunc,
                                                                         FreeFunction freeFunc)
    : rf(resetFunc == NULL ? &noop : resetFunc), ff(freeFunc)
{
}

template <class ObjType, class IdxType, class FreeFunction>
DoomObjectContainer<ObjType, IdxType, FreeFunction>::DoomObjectContainer(size_t count, ResetObjType resetFunc, FreeFunction freeFunc)
    : rf(resetFunc == NULL ? &noop : resetFunc), ff(freeFunc)
{
	this->container.reserve(count);
}

template <class ObjType, class IdxType, class FreeFunction>
DoomObjectContainer<ObjType, IdxType, FreeFunction>::~DoomObjectContainer()
{
	clear();
}

// Operators

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::ObjReference DoomObjectContainer<
    ObjType, IdxType, FreeFunction>::operator[](int idx)
{
	iterator it = this->lookup_table.find(idx);
    if (it == this->end())
    {
	    I_Error("Attempt to access invalid %s at idx %d", typeid(ObjType).name(), idx);
    }
    return it->second;
}

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::ConstObjReference DoomObjectContainer<
    ObjType, IdxType, FreeFunction>::operator[](int idx) const
{
    const_iterator it = this->lookup_table.find(idx);
    if (it == this->end())
    {
    	I_Error("Attempt to access invalid %s at idx %d", typeid(ObjType).name(), idx);
    }
    return it->second;
}

template <class ObjType, class IdxType, class FreeFunction>
bool DoomObjectContainer<ObjType, IdxType, FreeFunction>::operator==(const ObjType* p) const
{
	return this->container().data() == p;
}

template <class ObjType, class IdxType, class FreeFunction>
bool DoomObjectContainer<ObjType, IdxType, FreeFunction>::operator!=(const ObjType* p) const
{
	return this->container().data() != p;
}

template <class ObjType, class IdxType, class FreeFunction>
DoomObjectContainer<ObjType, IdxType, FreeFunction>::operator const ObjType*() const
{
	return const_cast<ObjType>(this->container.data());
}
template <class ObjType, class IdxType, class FreeFunction>
DoomObjectContainer<ObjType, IdxType, FreeFunction>::operator ObjType*()
{
	return this->container.data();
}

template <class ObjType, class IdxType, class FreeFunction>
ObjType operator-(ObjType obj, DoomObjectContainer<ObjType, IdxType, FreeFunction>& container)
{
	return obj - container.data();
}
template <class ObjType, class IdxType, class FreeFunction>
ObjType operator+(DoomObjectContainer<ObjType, IdxType, FreeFunction>& container, WORD ofs)
{
	return container.data() + ofs;
}

// data functions for quicker access to all objects presently stored

template <class ObjType, class IdxType, class FreeFunction>
ObjType* DoomObjectContainer<ObjType, IdxType, FreeFunction>::data()
{
	return this->container.data();
}

template <class ObjType, class IdxType, class FreeFunction>
const ObjType* DoomObjectContainer<ObjType, IdxType, FreeFunction>::data() const
{
	return this->container.data();
}

// Capacity and Size

template <class ObjType, class IdxType, class FreeFunction>
size_t DoomObjectContainer<ObjType, IdxType, FreeFunction>::size() const
{
	return this->container.size();
}

template <class ObjType, class IdxType, class FreeFunction>
size_t DoomObjectContainer<ObjType, IdxType, FreeFunction>::capacity() const
{
	return this->container.capacity();
}

template <class ObjType, class IdxType, class FreeFunction>
void DoomObjectContainer<ObjType, IdxType, FreeFunction>::clear()
{
	for (auto& obj : this->container)
	{
		this->ff(obj);
	}
	this->container.clear();
	this->lookup_table.clear();
}

// Allocation changes

template <class ObjType, class IdxType, class FreeFunction>
void DoomObjectContainer<ObjType, IdxType, FreeFunction>::resize(size_t count)
{
	this->container.resize(count);
	// this->lookup_table.resize(count);
}

template <class ObjType, class IdxType, class FreeFunction>
void DoomObjectContainer<ObjType, IdxType, FreeFunction>::reserve(size_t new_cap)
{
	this->container.reserve(new_cap);
	// this->lookup_table.resize(new_cap);
}

// Insertion

template <class ObjType, class IdxType, class FreeFunction>
void DoomObjectContainer<ObjType, IdxType, FreeFunction>::insert(const ObjType& obj, IdxType idx)
{
	this->container.insert(this->container.end(), obj);
	this->lookup_table[static_cast<int>(idx)] = obj;
}

// TODO: more of a copy construct in a sense
template <class ObjType, class IdxType, class FreeFunction>
void DoomObjectContainer<ObjType, IdxType, FreeFunction>::append(
    const DoomObjectContainer<ObjType, IdxType, FreeFunction>& dObjContainer)
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

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::iterator DoomObjectContainer<ObjType, IdxType, FreeFunction>::begin()
{
	return lookup_table.begin();
}

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::iterator DoomObjectContainer<ObjType, IdxType, FreeFunction>::end()
{
	return lookup_table.end();
}

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::const_iterator DoomObjectContainer<ObjType, IdxType, FreeFunction>::cbegin()
{
	return lookup_table.begin();
}

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::const_iterator DoomObjectContainer<ObjType, IdxType, FreeFunction>::cend()
{
	return lookup_table.end();
}

// Lookup

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::iterator DoomObjectContainer<
    ObjType, IdxType, FreeFunction>::find(IdxType idx)
{
	typename LookupTable::iterator it = this->lookup_table.find(idx);
	if (it != this->lookup_table.end())
	{
		return it;
	}
	return this->lookup_table.end();
}

template <class ObjType, class IdxType, class FreeFunction>
typename DoomObjectContainer<ObjType, IdxType, FreeFunction>::const_iterator DoomObjectContainer<
    ObjType, IdxType, FreeFunction>::find(IdxType idx) const
{
	typename LookupTable::iterator it = this->lookup_table.find(idx);
	if (it != this->lookup_table.end())
	{
		return it;
	}
	return this->lookup_table.end();
}

template<class ObjType, class IdxType, class FreeFunction>
bool DoomObjectContainer<ObjType, IdxType, FreeFunction>::contains(IdxType idx) const
{
	return this->find(idx) != this->end();
}

//----------------------------------------------------------------------------------------------
