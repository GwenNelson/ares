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

#ifndef __uimanager_h
#define __uimanager_h

#include <wx/wx.h>
#include "edcommon/model.h"
#include "editor/iuidialog.h"
#include "editor/iuimanager.h"

class AppAresEditWX;
class FileReq;
class NewProjectDialog;
class MessageFrame;
class CellDialog;
class UIDialog;
class SimpleListCtrlView;

class wxBoxSizer;
class wxTextCtrl;
class wxChoice;
class wxButton;
class wxCommandEvent;
class wxWindow;
class wxListCtrl;

typedef csHash<csString,csString> DialogResult;

struct ValueListInfo
{
  wxListCtrl* list;
  size_t col;
  csRef<Ares::Value> collectionValue;
};

class UIDialog : public scfImplementation1<UIDialog, iUIDialog>,
  public wxDialog
{
private:
  Ares::View view;

  wxPanel* mainPanel;
  wxBoxSizer* sizer;
  wxBoxSizer* lastRowSizer;
  csHash<wxTextCtrl*,csString> textFields;
  csHash<wxChoice*,csString> choiceFields;
  csHash<wxComboBox*,csString> comboFields;

  csHash<ValueListInfo,csString> valueListFields;

  csArray<wxButton*> buttons;
  csRef<iUIDialogCallback> callback;

  DialogResult fieldContents;

  bool okCancelAdded;
  void AddOkCancel ();

  virtual void OnButtonClicked (wxCommandEvent& event);

public:
  UIDialog (wxWindow* parent, const char* title, int width = -1, int height = -1);
  virtual ~UIDialog ();

  /// Add a new row in this dialog.
  virtual void AddRow (int proportion = 0);

  /// Add a label in the current row.
  virtual void AddLabel (const char* str);
  /// Add a single line text control in the current row.
  virtual void AddText (const char* name);
  /// Add a multi line text control in the current row.
  virtual void AddMultiText (const char* name);
  /// Add a button in the current row.
  virtual void AddButton (const char* str);
  /// Add a combobox control in the current row with the given choices (end with 0).
  virtual void AddCombo (const char* name, ...);
  virtual void AddCombo (const char* name, const csStringArray& choiceArray);
  /// Add a choice control in the current row with the given choices (end with 0).
  virtual void AddChoice (const char* name, ...);
  virtual void AddChoice (const char* name, const csStringArray& choiceArray);
  /// Add a horizontal spacer in the current row.
  virtual void AddSpace ();
  /**
   * Add a list. The given column index is used as the 'value'
   * from the model. The value should be a collection with composites
   * as children and the requested value out of the composite should
   * be a string.
   */
  virtual void AddList (const char* name, Ares::Value* collectionValue,
      size_t valueColumn, int height, const char* heading, const char* names);
  virtual void AddListIndexed (const char* name, Ares::Value* collectionValue,
      size_t valueColumn, int height, const char* heading, ...);

  // Clear all input fields to empty or default values.
  virtual void Clear ();

  /**
   * Set the value of a given control. This will do the right thing depending
   * on the type of the control.
   */
  virtual void SetValue (const char* name, const char* value);

  /// Set the value of the given text control.
  virtual void SetText (const char* name, const char* value);
  /// Set the value of the given choice.
  virtual void SetChoice (const char* name, const char* value);
  /// Set the value of the given combo.
  virtual void SetCombo (const char* name, const char* value);
  /// Set the vlaue of the given list.
  virtual void SetList (const char* name, const char* value);

  /**
   * Show the dialog in a modal manner.
   * Returns 1 if Ok was pressed and 0 otherwise.
   */
  virtual int Show (iUIDialogCallback* cb);

  /**
   * When any button is pressed (including Ok and Cancel) this will return
   * the contents of all text controls and choices.
   */
  virtual const DialogResult& GetFieldContents () const { return fieldContents; }

  /**
   * Fill this dialog with the DialogResult contents.
   */
  virtual void SetFieldContents (const DialogResult& result);
};


class UIManager : public scfImplementation1<UIManager, iUIManager>
{
private:
  AppAresEditWX* app;
  wxWindow* parent;

  FileReq* filereqDialog;
  NewProjectDialog* newprojectDialog;
  CellDialog* cellDialog;
  MessageFrame* messageFrame;

  int contextMenuID;

public:
  UIManager (AppAresEditWX* app, wxWindow* parent);
  virtual ~UIManager ();

  AppAresEditWX* GetApp () const { return app; }

  virtual void Message (const char* description, ...);
  virtual bool Error (const char* description, ...);
  virtual bool Ask (const char* description, ...);
  virtual csRef<iString> AskDialog (const char* description, const char* label);

  FileReq* GetFileReqDialog () const { return filereqDialog; }
  NewProjectDialog* GetNewProjectDialog () const { return newprojectDialog; }
  CellDialog* GetCellDialog () const { return cellDialog; }
  MessageFrame* GetMessageFrame () const { return messageFrame; }

  /// Create a dynamically buildable dialog.
  virtual csPtr<iUIDialog> CreateDialog (const char* title);
  virtual csPtr<iUIDialog> CreateDialog (wxWindow* par, const char* title);

  virtual int AllocContextMenuID () { contextMenuID++; return contextMenuID; }
};

#endif // __uimanager_h

