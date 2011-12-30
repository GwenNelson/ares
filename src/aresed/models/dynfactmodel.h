/*
The MIT License

Copyright (c) 2011 by Jorrit Tyberghein

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#ifndef __aresed_dynfactmodel_h
#define __aresed_dynfactmodel_h

#include "rowmodel.h"
#include "model.h"

class AresEdit3DView;

/**
 * A value representing a category.
 * The children of this value are of type ConstantStringValue.
 */
class CategoryCollectionValue : public Ares::StandardCollectionValue
{
private:
  AresEdit3DView* aresed3d;
  csString category;
  bool dirty;

protected:
  virtual void UpdateChildren ();
  virtual void ChildChanged (Value* child)
  {
    FireValueChanged ();
  }

public:
  CategoryCollectionValue (AresEdit3DView* aresed3d, const char* category) :
    aresed3d (aresed3d), category (category), dirty (true) { }
  virtual ~CategoryCollectionValue () { }

  virtual const char* GetStringValue () { return category; }

  virtual bool DeleteValue (Value* child) { return false; }
  virtual Value* NewValue (size_t idx, Value* selectedValue) { return 0; }

  virtual csString Dump (bool verbose = false)
  {
    csString dump = "[DFCat*]";
    dump += Ares::StandardCollectionValue::Dump (verbose);
    return dump;
  }
};

/**
 * A value representing the tree of dynamic factories.
 * The children of this value are of type CategoryCollectionValue.
 */
class DynfactCollectionValue : public Ares::StandardCollectionValue
{
private:
  AresEdit3DView* aresed3d;
  bool dirty;

protected:
  virtual void UpdateChildren ();
  virtual void ChildChanged (Value* child)
  {
    FireValueChanged ();
  }

public:
  DynfactCollectionValue (AresEdit3DView* aresed3d) : aresed3d (aresed3d), dirty (true) { }
  virtual ~DynfactCollectionValue () { }

  /**
   * Call this when you want to refresh this value because external data (i.e.
   * the factories and items) changed.
   */
  void Refresh () { dirty = true; FireValueChanged (); }

  virtual const char* GetStringValue () { return "Factories"; }

  virtual bool DeleteValue (Value* child) { return false; }
  virtual Value* NewValue (size_t idx, Value* selectedValue);

  virtual csString Dump (bool verbose = false)
  {
    csString dump = "[DFRoot*]";
    dump += Ares::StandardCollectionValue::Dump (verbose);
    return dump;
  }
};

class DynfactRowModel : public RowModel
{
private:
  AresEdit3DView* aresed3d;
  csHash<csStringArray,csString>::ConstGlobalIterator it;
  csString category;
  csStringArray items;
  size_t idx;

  void SearchNext ();

public:
  DynfactRowModel (AresEdit3DView* aresed3d) : aresed3d (aresed3d) { }
  virtual ~DynfactRowModel () { }

  virtual void ResetIterator ();
  virtual bool HasRows () { return idx < items.GetSize (); }
  virtual csStringArray NextRow ();

  virtual bool DeleteRow (const csStringArray& row);
  virtual bool AddRow (const csStringArray& row);

  virtual const char* GetColumns () { return "Category,Item"; }
  virtual bool IsEditAllowed () const { return false; }
  virtual csStringArray EditRow (const csStringArray& origRow);
};


#endif // __aresed_dynfactmodel_h

