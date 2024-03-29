/*
The MIT License

Copyright (c) 2012 by Jorrit Tyberghein

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
#include "apparesed.h"
#include "ui/uimanager.h"
#include "config.h"

#include <wx/wx.h>

//---------------------------------------------------------------------------

AresConfig::AresConfig (AppAresEditWX* app) :
  scfImplementationType (this), app (app)
{
}

bool AresConfig::ParsePlugins (iDocumentNode* pluginsNode)
{
  csRef<iDocumentNodeIterator> it = pluginsNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "plugin")
    {
      PluginConfig mc;
      mc.plugin = child->GetAttributeValue ("plugin");
      mc.tooltip = child->GetAttributeValue ("tooltip");
      mc.mainMode = child->GetAttributeValueAsBool ("main");
      csRef<iDocumentNodeIterator> childit = child->GetNodes ();
      while (childit->HasNext ())
      {
	csRef<iDocumentNode> resourceChild = childit->Next ();
	if (resourceChild->GetType () != CS_NODE_ELEMENT) continue;
        csString resourceValue = resourceChild->GetValue ();
        if (resourceValue == "resource")
	{
	  csString file = resourceChild->GetAttributeValue ("file");
	  mc.resources.Push (file);
	}
        else
        {
          return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", resourceValue.GetData ());
        }
      }
      plugins.Push (mc);
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

bool AresConfig::ParseWizard (iDocumentNode* node, Wizard* wizard)
{
  wizard->name = node->GetAttributeValue ("name");
  wizard->description = node->GetAttributeValue ("description");
  wizard->node = node;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "ask")
    {
      WizardParameter par;
      par.name = child->GetAttributeValue ("name");
      par.type = child->GetAttributeValue ("type");
      par.defaultValue = child->GetAttributeValue ("default");
      par.description = child->GetAttributeValue ("description");
      wizard->parameters.Push (par);
    }
  }

  return true;
}

bool AresConfig::ParseWizards (iDocumentNode* wizardsNode)
{
  csRef<iDocumentNodeIterator> it = wizardsNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "template")
    {
      Wizard* wizard = new Wizard ();;
      if (!ParseWizard (child, wizard))
	return false;
      templateWizards.Push (wizard);
    }
    else if (value == "quest")
    {
      Wizard* wizard = new Wizard ();;
      if (!ParseWizard (child, wizard))
	return false;
      questWizards.Push (wizard);
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

Wizard* AresConfig::FindTemplateWizard (const char* name) const
{
  for (size_t i = 0 ; i < templateWizards.GetSize () ; i++)
    if (templateWizards.Get (i)->name == name)
      return templateWizards.Get (i);
  return 0;
}

Wizard* AresConfig::FindQuestWizard (const char* name) const
{
  for (size_t i = 0 ; i < questWizards.GetSize () ; i++)
    if (questWizards.Get (i)->name == name)
      return questWizards.Get (i);
  return 0;
}

bool AresConfig::ParseKnownMessages (iDocumentNode* knownmessagesNode)
{
  csRef<iDocumentNodeIterator> it = knownmessagesNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "message")
    {
      KnownMessage msg;
      msg.name = child->GetAttributeValue ("name");
      msg.description = child->GetAttributeValue ("description");
      csRef<iDocumentNodeIterator> parit = child->GetNodes ();
      while (parit->HasNext ())
      {
        csRef<iDocumentNode> parNode = parit->Next ();
        if (parNode->GetType () != CS_NODE_ELEMENT) continue;
        csString parValue = parNode->GetValue ();
	if (parValue == "par")
	{
	  KnownParameter par;
	  par.name = parNode->GetAttributeValue ("name");
	  par.value = parNode->GetAttributeValue ("value");
	  csString typeS = parNode->GetAttributeValue ("type");
	  celDataType type;
	  if (typeS == "string") type = CEL_DATA_STRING;
	  else if (typeS == "long") type = CEL_DATA_LONG;
	  else if (typeS == "bool") type = CEL_DATA_BOOL;
	  else if (typeS == "float") type = CEL_DATA_FLOAT;
	  else if (typeS == "vector2") type = CEL_DATA_VECTOR2;
	  else if (typeS == "vector3") type = CEL_DATA_VECTOR3;
	  else if (typeS == "vector4") type = CEL_DATA_VECTOR4;
	  else if (typeS == "color") type = CEL_DATA_COLOR;
	  else if (typeS == "color4") type = CEL_DATA_COLOR4;
	  else type = CEL_DATA_NONE;
	  par.type = type;
	  msg.parameters.Push (par);
	}
	else
	{
          return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", parValue.GetData ());
	}
      }
      messages.Push (msg);
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

csRef<iDocument> AresConfig::ReadConfigDocument ()
{
  if (doc) return doc;
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (app->GetObjectRegistry ());
  csRef<iDocumentSystem> docsys;
  docsys = csQueryRegistry<iDocumentSystem> (app->GetObjectRegistry ());
  if (!docsys)
    docsys.AttachNew (new csTinyDocumentSystem ());

  doc = docsys->CreateDocument ();
  csRef<iDataBuffer> buf = vfs->ReadFile ("/appdata/aresedconfig.xml");
  if (!buf)
  {
    app->ReportError ("Could not open config file '/appdata/aresedconfig.xml'!");
    return 0;
  }
  const char* error = doc->Parse (buf->GetData ());
  if (error)
  {
    app->ReportError ("Error parsing 'aresedconfig.xml': %s!", error);
    return 0;
  }
  return doc;
}

bool AresConfig::ReadConfig ()
{
  ReadConfigDocument ();
  if (!doc) return false;
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> configNode = root->GetNode ("config");
  if (!configNode)
    return app->ReportError ("Error 'aresedconfig.xml' is missing a 'config' node!");
  csRef<iDocumentNode> knownmessagesNode = configNode->GetNode ("knownmessages");
  if (knownmessagesNode)
  {
    if (!ParseKnownMessages (knownmessagesNode))
      return false;
  }
  csRef<iDocumentNode> pluginsNode = configNode->GetNode ("plugins");
  if (pluginsNode)
  {
    if (!ParsePlugins (pluginsNode))
      return false;
  }
  csRef<iDocumentNode> wizardsNode = configNode->GetNode ("wizards");
  if (wizardsNode)
  {
    if (!ParseWizards (wizardsNode))
      return false;
  }
  return true;
}

const KnownMessage* AresConfig::GetKnownMessage (const char* name) const
{
  for (size_t i = 0 ; i < messages.GetSize () ; i++)
  {
    if (messages.Get (i).name == name)
      return &messages.Get (i);
  }
  return 0;
}

static int StringToId (csString s)
{
  s.Downcase ();
  if (s == "new") return wxID_NEW;
  if (s == "copy") return wxID_COPY;
  if (s == "quit") return wxID_EXIT;
  if (s == "open") return wxID_OPEN;
  if (s == "save") return wxID_SAVE;
  if (s == "saveas") return wxID_SAVEAS;
  if (s == "paste") return wxID_PASTE;
  if (s == "delete") return wxID_DELETE;
  if (s == "preview") return wxID_PREVIEW;
  if (s == "find") return wxID_FIND;
  if (s == "help") return wxID_HELP_INDEX;
  if (s == "about") return wxID_ABOUT;
  return wxID_ANY;
}

bool AresConfig::ParseMenuItems (wxMenu* menu, iDocumentNode* itemsNode)
{
  csRef<iDocumentNodeIterator> it = itemsNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "item")
    {
      csString name = child->GetAttributeValue ("name");
      csString hotkey = child->GetAttributeValue ("key");
      csString idString = child->GetAttributeValue ("id");
      csString target = child->GetAttributeValue ("target");
      csString command = child->GetAttributeValue ("command");
      csString help = child->GetAttributeValue ("help");
      csString args = child->GetAttributeValue ("args");
      int id = StringToId (idString);
      if (id == wxID_ANY)
	id = app->GetUIManager ()->AllocContextMenuID ();
      if (!hotkey.IsEmpty())
      {
        name.SetCapacity (name.Length() + 1 + hotkey.Length());
        name.Append ("\t");
        name.Append (hotkey);
      }
      app->AllocateMenuCommand (id, name, target, command, args, help);
      app->AppendMenuItem (menu, id, name, help);
    }
    else if (value == "sep")
    {
      menu->AppendSeparator ();
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

bool AresConfig::ParseMenus (wxMenuBar* menuBar, iDocumentNode* menusNode)
{
  csRef<iDocumentNodeIterator> it = menusNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "menu")
    {
      csString name = child->GetAttributeValue ("name");
      wxMenu* menu = new wxMenu ();
      if (!ParseMenuItems (menu, child))
      {
	delete menu;
	return false;
      }
      menuBar->Append (menu, wxString::FromUTF8 (name));
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

wxMenuBar* AresConfig::BuildMenuBar ()
{
  ReadConfigDocument ();
  if (!doc) return 0;
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> configNode = root->GetNode ("config");
  if (!configNode)
  {
    app->ReportError ("Error 'aresedconfig.xml' is missing a 'config' node!");
    return 0;
  }
  csRef<iDocumentNode> menusNode = configNode->GetNode ("menus");
  if (menusNode)
  {
    wxMenuBar* menuBar = new wxMenuBar ();
    if (!ParseMenus (menuBar, menusNode))
    {
      delete menuBar;
      return 0;
    }
    return menuBar;
  }
  return 0;
}

bool AresConfig::ParseToolbarItems (wxToolBar* toolbar, iDocumentNode* sectionNode)
{
  csRef<iDataBuffer> buf = app->GetVFS ()->GetRealPath ("/ares/data/icons/toolbar/32x32");
  csString path (buf->GetData ());
  path += CS_PATH_SEPARATOR;
  wxImage image (32, 32);

  csRef<iDocumentNodeIterator> it = sectionNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "item")
    {
      csString name = child->GetAttributeValue ("name");
      csString idString = child->GetAttributeValue ("id");
      csString target = child->GetAttributeValue ("target");
      csString imageFile = child->GetAttributeValue ("image");
      csString command = child->GetAttributeValue ("command");
      csString help = child->GetAttributeValue ("help");
      csString args = child->GetAttributeValue ("args");
      bool toggle = child->GetAttributeValueAsBool ("toggle");
      int id = StringToId (idString);
      if (id == wxID_ANY)
	id = app->GetUIManager ()->AllocContextMenuID ();
      app->AllocateMenuCommand (id, name, target, command, args, help);
      image.LoadFile (wxString::FromUTF8 (path + imageFile));
      toolbar->AddTool (id, wxString::FromUTF8 (name), wxBitmap (image), wxString::FromUTF8 (help),
	  toggle ? wxITEM_CHECK : wxITEM_NORMAL);
    }
    else if (value == "sep")
    {
      toolbar->AddSeparator ();
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

bool AresConfig::ParseToolbar (wxToolBar* toolbar, iDocumentNode* toolbarNode)
{
  bool first = true;
  csRef<iDocumentNodeIterator> it = toolbarNode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csString value = child->GetValue ();
    if (value == "section")
    {
      csString name = child->GetAttributeValue ("name");
      if (first)
      {
	toolbar->AddSeparator ();
	first = false;
      }
      if (!ParseToolbarItems (toolbar, child))
      {
	return false;
      }
    }
    else
    {
      return app->ReportError ("Error parsing 'aresedconfig.xml', unknown element '%s'!", value.GetData ());
    }
  }
  return true;
}

bool AresConfig::BuildToolBar (wxToolBar* toolbar)
{
  ReadConfigDocument ();
  if (!doc) return 0;
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> configNode = root->GetNode ("config");
  if (!configNode)
  {
    app->ReportError ("Error 'aresedconfig.xml' is missing a 'config' node!");
    return false;
  }
  csRef<iDocumentNode> toolbarNode = configNode->GetNode ("toolbar");
  if (toolbarNode)
  {
    if (!ParseToolbar (toolbar, toolbarNode))
      return false;
    return true;
  }
  return true;
}

