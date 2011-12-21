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

#ifndef __appares_dynfactdialog_h
#define __appares_dynfactdialog_h

#include <crystalspace.h>

#include "../models/model.h"

#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/xrc/xmlres.h>

class MeshView;
class TreeCtrlView;
class ListCtrlView;
class FactoryEditorModel;

class ColliderCollectionValue;

class UIManager;

using namespace Ares;

class DynfactDialog : public wxDialog
{
private:
  UIManager* uiManager;
  long selIndex;
  csRef<iTimerEvent> timerOp;
  size_t normalPen;
  size_t hilightPen;

  MeshView* meshView;
  TreeCtrlView* meshTreeView;
  csRef<FactoryEditorModel> factoryEditorModel;

  csRef<View> colliderView;
  csRef<ColliderCollectionValue> colliderCollectionValue;
  csRef<SelectedValue> colliderSelectedValue;

  void OnOkButton (wxCommandEvent& event);
  void SetupColliderGeometry ();

public:
  DynfactDialog (wxWindow* parent, UIManager* uiManager);
  ~DynfactDialog ();

  void Show ();
  void Tick ();

  void EditCollider (const char* type);
  void EditFactory (const char* factory);
  iDynamicFactory* GetCurrentFactory ();

  DECLARE_EVENT_TABLE ();
};

#endif // __appares_dynfactdialog_h

