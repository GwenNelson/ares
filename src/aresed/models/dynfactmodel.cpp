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

#include <crystalspace.h>

#include "dynfactmodel.h"
#include "meshfactmodel.h"
#include "../tools/tools.h"
#include "../ui/uimanager.h"
#include "../apparesed.h"


void DynfactCollectionValue::UpdateChildren ()
{
  ReleaseChildren ();
  const csHash<csStringArray,csString>& categories = aresed3d->GetCategories ();
  csHash<csStringArray,csString>::ConstGlobalIterator it = categories.GetIterator ();
  while (it.HasNext ())
  {
    csString category;
    const csStringArray& items = it.Next (category);
    csRef<Ares::ConstantStringValue> strValue;
    for (size_t i = 0 ; i < items.GetSize () ; i++)
    {
      csRef<Ares::CompositeValue> value;
      value.AttachNew (new Ares::CompositeValue ());
      strValue.AttachNew (new Ares::ConstantStringValue (category));
      value->AddChild ("category", strValue);
      strValue.AttachNew (new Ares::ConstantStringValue (items[i]));
      value->AddChild ("item", strValue);
      children.Push (value);
      value->SetParent (this);
    }
  }
}

//--------------------------------------------------------------------------

void DynfactRowModel::SearchNext ()
{
  while (idx >= items.GetSize ())
  {
    if (it.HasNext ())
    {
      items = it.Next (category);
      idx = 0;
    }
    else return;
  }
}

void DynfactRowModel::ResetIterator ()
{
  const csHash<csStringArray,csString>& categories = aresed3d->GetCategories ();
  it = categories.GetIterator ();
  items.DeleteAll ();
  idx = 0;
  SearchNext ();
}

csStringArray DynfactRowModel::NextRow ()
{
  csString cat = category;
  csString item = items[idx];
  idx++;
  SearchNext ();
  return Tools::MakeArray (cat.GetData (), item.GetData (), (const char*)0);
}

csStringArray DynfactRowModel::EditRow (const csStringArray& origRow)
{
  UIManager* ui = aresed3d->GetApp ()->GetUIManager ();
  UIDialog* dialog = ui->CreateDialog ("New dynamic object");
  dialog->AddRow ();
  dialog->AddLabel ("Category:");
  dialog->AddText ("Category");
  dialog->AddRow ();
  //dialog->AddLabel ("Item:");
  //dialog->AddText ("Item");
  csRef<MeshfactRowModel> model = new MeshfactRowModel (aresed3d->GetEngine ());
  dialog->AddList ("Item", model, 0);
  if (origRow.GetSize () >= 1)
    dialog->SetValue ("Category", origRow[0]);
  if (origRow.GetSize () >= 2)
    dialog->SetValue ("Item", origRow[1]);
  if (dialog->Show (0))
  {
    const csHash<csString,csString>& fields = dialog->GetFieldContents ();
    csString category = fields.Get ("Category", "");
    csString item = fields.Get ("Item", "");
    if (item.IsEmpty ())
    {
      ui->Error ("Please select a mesh factory!");
      delete dialog;
      return csStringArray ();
    }
    return Tools::MakeArray (category.GetData (), item.GetData (), (const char*)0);
  }
  delete dialog;
  return csStringArray ();
}

bool DynfactRowModel::DeleteRow (const csStringArray& row)
{
  aresed3d->RemoveItem (row[0], row[1]);
  return true;
}

bool DynfactRowModel::AddRow (const csStringArray& row)
{
  aresed3d->AddItem (row[0], row[1]);
  return true;
}

