/*
The MIT License

Copyright (c) 2010 by Jorrit Tyberghein

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

#include "apparesed.h"
#include "curvemode.h"

//---------------------------------------------------------------------------

CurveMode::CurveMode (AppAresEdit* aresed, AresEdit3DView* aresed3d)
  : EditingMode (aresed, aresed3d, "Curve")
{
  editingCurveFactory = 0;
  autoSmooth = true;

#if 0
  CEGUI::WindowManager* winMgr = aresed3d->GetCEGUI ()->GetWindowManagerPtr ();
  CEGUI::Window* btn;

  btn = winMgr->getWindow("Ares/CurveWindow/RotLeft");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&CurveMode::OnRotLeftButtonClicked, this));
  btn = winMgr->getWindow("Ares/CurveWindow/RotRight");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&CurveMode::OnRotRightButtonClicked, this));
  btn = winMgr->getWindow("Ares/CurveWindow/RotReset");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&CurveMode::OnRotResetButtonClicked, this));

  btn = winMgr->getWindow("Ares/CurveWindow/Flatten");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&CurveMode::OnFlattenButtonClicked, this));

  autoSmoothCheck = static_cast<CEGUI::Checkbox*>(winMgr->getWindow("Ares/CurveWindow/AutoSmooth"));
  autoSmoothCheck->setSelected (autoSmooth);
  autoSmoothCheck->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,
    CEGUI::Event::Subscriber(&CurveMode::OnAutoSmoothSelected, this));
#endif
}

void CurveMode::UpdateMarkers ()
{
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  if (editingCurveFactory->GetPointCount () != markers.GetSize ())
  {
    Stop ();
    for (size_t i = 0 ; i < editingCurveFactory->GetPointCount () ; i++)
    {
      iMarker* marker = aresed3d->GetMarkerManager ()->CreateMarker ();
      markers.Push (marker);
      marker->AttachMesh (mesh);
    }
  }

  iMarkerColor* red = aresed3d->GetMarkerManager ()->FindMarkerColor ("red");
  iMarkerColor* green = aresed3d->GetMarkerManager ()->FindMarkerColor ("green");
  iMarkerColor* yellow = aresed3d->GetMarkerManager ()->FindMarkerColor ("yellow");

  for (size_t i = 0 ; i < editingCurveFactory->GetPointCount () ; i++)
  {
    csVector3 pos = editingCurveFactory->GetPosition (i);
    csVector3 front = editingCurveFactory->GetFront (i);
    csVector3 up = editingCurveFactory->GetUp (i);
    iMarker* marker = markers[i];
    marker->Clear ();
    marker->Line (MARKER_OBJECT, pos, pos+front, green, true);
    marker->Line (MARKER_OBJECT, pos, pos+up, red, true);
    marker->ClearHitAreas ();
    iMarkerHitArea* hitArea = marker->HitArea (MARKER_OBJECT, pos, .1f, i, yellow);
    csRef<MarkerCallback> cb;
    cb.AttachNew (new MarkerCallback (this));
    hitArea->DefineDrag (0, 0, MARKER_WORLD, CONSTRAIN_MESH, cb);
    hitArea->DefineDrag (0, CSMASK_SHIFT, MARKER_WORLD, CONSTRAIN_MESH, cb);
    hitArea->DefineDrag (0, CSMASK_ALT, MARKER_WORLD, CONSTRAIN_YPLANE, cb);
    hitArea->DefineDrag (0, CSMASK_SHIFT + CSMASK_ALT, MARKER_WORLD, CONSTRAIN_YPLANE, cb);
    hitArea->DefineDrag (0, CSMASK_CTRL, MARKER_WORLD, CONSTRAIN_NONE, cb);
    hitArea->DefineDrag (0, CSMASK_SHIFT + CSMASK_CTRL, MARKER_WORLD, CONSTRAIN_NONE, cb);
  }

  UpdateMarkerSelection ();
}

void CurveMode::UpdateMarkerSelection ()
{
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    markers[i]->SetSelectionLevel (SELECTION_NONE);
  for (size_t i = 0 ; i < selectedPoints.GetSize () ; i++)
    markers[selectedPoints[i]]->SetSelectionLevel (SELECTION_SELECTED);
}

void CurveMode::Start ()
{
  csString name = aresed3d->GetSelection ()->GetFirst ()->GetFactory ()->GetName ();
  editingCurveFactory = aresed3d->GetCurvedMeshCreator ()->GetCurvedFactory (name);
  ggen = scfQueryInterface<iGeometryGenerator> (editingCurveFactory);
  selectedPoints.Empty ();
  dragPoints.Empty ();

  UpdateMarkers ();
}

void CurveMode::Stop ()
{
  for (size_t i = 0 ; i < markers.GetSize () ; i++)
    aresed3d->GetMarkerManager ()->DestroyMarker (markers[i]);
  markers.Empty ();
}

void CurveMode::MarkerStartDragging (iMarker* marker, iMarkerHitArea* area,
    const csVector3& pos, uint button, uint32 modifiers)
{
  size_t idx = size_t (area->GetData ());

  if (modifiers & CSMASK_SHIFT)
    AddCurrentPoint (idx);
  else
    SetCurrentPoint (idx);

  csArray<size_t>::Iterator it = selectedPoints.GetIterator ();
  while (it.HasNext ())
  {
    size_t id = it.Next ();
    csVector3 wpos = GetWorldPosPoint (id);
    DragPoint dob;

    dob.kineOffset = pos - wpos;
    dob.idx = id;
    dragPoints.Push (dob);
  }
}

void CurveMode::MarkerWantsMove (iMarker* marker, iMarkerHitArea* area,
      const csVector3& pos)
{
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  const csReversibleTransform& meshtrans = mesh->GetMovable ()->GetTransform ();
  for (size_t i = 0 ; i < dragPoints.GetSize () ; i++)
  {
    csVector3 np = pos - dragPoints[i].kineOffset;
    size_t idx = dragPoints[i].idx;
    csVector3 rp = meshtrans.Other2This (np);
    editingCurveFactory->ChangePoint (idx,
	  rp,
	  editingCurveFactory->GetFront (idx),
	  editingCurveFactory->GetUp (idx));
    UpdateMarkers ();
  }
  if (autoSmooth) DoAutoSmooth ();
  ggen->GenerateGeometry (mesh);
  aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
}

void CurveMode::MarkerStopDragging (iMarker* marker, iMarkerHitArea* area)
{
  StopDrag ();
}

void CurveMode::SetCurrentPoint (size_t idx)
{
  if (selectedPoints.Find (idx) != csArrayItemNotFound) return;
  selectedPoints.DeleteAll ();
  selectedPoints.Push (idx);
  UpdateMarkerSelection ();
}

void CurveMode::AddCurrentPoint (size_t idx)
{
  if (selectedPoints.Find (idx) != csArrayItemNotFound)
  {
    selectedPoints.Delete (idx);
  }
  else
  {
    selectedPoints.Push (idx);
  }
  UpdateMarkerSelection ();
}

void CurveMode::StopDrag ()
{
  dragPoints.DeleteAll ();
}

void CurveMode::FramePre()
{
}

void CurveMode::Frame3D()
{
}

void CurveMode::Frame2D()
{
}

void CurveMode::DoAutoSmooth ()
{
  if (editingCurveFactory->GetPointCount () <= 2) return;
  for (size_t i = 1 ; i < editingCurveFactory->GetPointCount ()-1 ; i++)
    SmoothPoint (i, false);
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  ggen->GenerateGeometry (mesh);
  aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
}

void CurveMode::SmoothPoint (size_t idx, bool regen)
{
  if (idx == 0 || idx >= editingCurveFactory->GetPointCount ()-1)
    return;
  const csVector3& p1 = editingCurveFactory->GetPosition (idx-1);
  const csVector3& pos = editingCurveFactory->GetPosition (idx);
  const csVector3& p2 = editingCurveFactory->GetPosition (idx+1);
  csVector3 fr = ((p2 - pos).Unit () + (pos - p1).Unit ()) / 2.0;
  csVector3 up = editingCurveFactory->GetUp (idx);
  //csVector3 right = fr % up;
  //up = - (fr % right).Unit ();
  editingCurveFactory->ChangePoint (idx, pos, fr, up);
  UpdateMarkers ();
  if (regen)
  {
    iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
    ggen->GenerateGeometry (mesh);
    aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
  }
}

csVector3 CurveMode::GetWorldPosPoint (size_t idx)
{
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  const csReversibleTransform& meshtrans = mesh->GetMovable ()->GetTransform ();
  const csVector3& pos = editingCurveFactory->GetPosition (idx);
  return meshtrans.This2Other (pos);
}

size_t CurveMode::FindCurvePoint (int mouseX, int mouseY)
{
  int data;
  iMarker* marker = aresed3d->GetMarkerManager ()->FindHitMarker (mouseX, mouseY, data);
  if (!marker) return csArrayItemNotFound;
  size_t idx = 0;
  while (idx < markers.GetSize ())
    if (markers[idx] == marker) return idx;
    else idx++;
  return csArrayItemNotFound;
}

void CurveMode::RotateCurrent (float baseAngle)
{
  bool slow = aresed3d->GetKeyboardDriver ()->GetKeyState (CSKEY_CTRL);
  bool fast = aresed3d->GetKeyboardDriver ()->GetKeyState (CSKEY_SHIFT);
  float angle = baseAngle;
  if (slow) angle /= 180.0;
  else if (fast) angle /= 2.0;
  else angle /= 8.0;

  csArray<size_t>::Iterator it = selectedPoints.GetIterator ();
  while (it.HasNext ())
  {
    size_t id = it.Next ();
    // @TODO: optimize!!!
    const csVector3& pos = editingCurveFactory->GetPosition (id);
    const csVector3& f = editingCurveFactory->GetFront (id);
    const csVector3& u = editingCurveFactory->GetUp (id);
    csVector3 r = f % u;
    csMatrix3 m (r.x, r.y, r.z, u.x, u.y, u.z, f.x, f.y, f.z);
    csReversibleTransform tr (m, pos);
    tr.RotateOther (u, angle);
    editingCurveFactory->ChangePoint (id, pos, tr.GetFront (), u);
    UpdateMarkers ();
  }
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  ggen->GenerateGeometry (mesh);
  aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
}

void CurveMode::FlatPoint (size_t idx)
{
  float sideHeight = editingCurveFactory->GetSideHeight ();
  iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
  const csReversibleTransform& meshtrans = mesh->GetMovable ()->GetTransform ();
  csVector3 pos = editingCurveFactory->GetPosition (idx);
  pos = meshtrans.This2Other (pos);
  const csVector3& f = editingCurveFactory->GetFront (idx);
  const csVector3& u = editingCurveFactory->GetUp (idx);
  pos.y += 100.0;
  iSector* sector = aresed3d->GetCsCamera ()->GetSector ();

  csFlags oldFlags = mesh->GetFlags ();
  mesh->GetFlags ().Set (CS_ENTITY_NOHITBEAM);
  csSectorHitBeamResult result = sector->HitBeamPortals (pos, pos - csVector3 (0, 1000, 0));
  mesh->GetFlags ().SetAll (oldFlags.Get ());

  if (result.mesh)
  {
    pos = result.isect;
    pos.y += sideHeight / 2.0;
    pos = meshtrans.Other2This (pos);
    editingCurveFactory->ChangePoint (idx, pos, f, u);
    UpdateMarkers ();
    if (autoSmooth)
      DoAutoSmooth ();
    else
      ggen->GenerateGeometry (mesh);
    aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
  }
}

bool CurveMode::OnFlattenButtonClicked (const CEGUI::EventArgs&)
{
  if (selectedPoints.GetSize () == 0)
  {
    for (size_t i = 0 ; i < editingCurveFactory->GetPointCount () ; i++)
      FlatPoint (i);
  }
  else
  {
    csArray<size_t>::Iterator it = selectedPoints.GetIterator ();
    while (it.HasNext ())
    {
      size_t id = it.Next ();
      FlatPoint (id);
    }
  }

  return true;
}

bool CurveMode::OnRotLeftButtonClicked (const CEGUI::EventArgs&)
{
  RotateCurrent (-PI);
  return true;
}

bool CurveMode::OnRotRightButtonClicked (const CEGUI::EventArgs&)
{
  RotateCurrent (PI);
  return true;
}

bool CurveMode::OnRotResetButtonClicked (const CEGUI::EventArgs&)
{
  csArray<size_t>::Iterator it = selectedPoints.GetIterator ();
  while (it.HasNext ())
  {
    size_t id = it.Next ();
    SmoothPoint (id);
  }
  return true;
}

bool CurveMode::OnAutoSmoothSelected (const CEGUI::EventArgs&)
{
  autoSmooth = autoSmoothCheck->isSelected ();
  return true;
}

bool CurveMode::OnKeyboard(iEvent& ev, utf32_char code)
{
  if (code == 'e')
  {
    size_t id = editingCurveFactory->GetPointCount ()-1;
    const csVector3& pos = editingCurveFactory->GetPosition (id);
    const csVector3& front = editingCurveFactory->GetFront (id);
    const csVector3& up = editingCurveFactory->GetUp (id);
    editingCurveFactory->AddPoint (pos + front, front, up);
    if (autoSmooth) DoAutoSmooth ();

    iMeshWrapper* mesh = aresed3d->GetSelection ()->GetFirst ()->GetMesh ();
    ggen->GenerateGeometry (mesh);
    aresed3d->GetSelection ()->GetFirst ()->RefreshColliders ();
    UpdateMarkers ();
  }
  return false;
}

bool CurveMode::OnMouseDown (iEvent& ev, uint but, int mouseX, int mouseY)
{
  return false;
}

bool CurveMode::OnMouseUp(iEvent& ev, uint but, int mouseX, int mouseY)
{
  return false;
}

bool CurveMode::OnMouseMove (iEvent& ev, int mouseX, int mouseY)
{
  return false;
}

