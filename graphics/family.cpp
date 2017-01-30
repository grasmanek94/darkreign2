///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// family.cpp
//
// 4-SEP-1998
//

#include "mesh.h"
#include "console.h"
#include "stdload.h"
//----------------------------------------------------------------------------

void AnimKey::SaveState( FScope * fScope)
{
  StdSave::TypeQuaternion( fScope, "Quaternion", quaternion);

  StdSave::TypeVector( fScope, "Position", position);
  StdSave::TypeVector( fScope, "Scale",    scale);
}
//----------------------------------------------------------------------------

void AnimKey::LoadState( FScope * fScope)
{
  StdLoad::TypeQuaternion( fScope, "Quaternion", quaternion);

  StdLoad::TypeVector( fScope, "Position", position);
  StdLoad::TypeVector( fScope, "Scale",    scale);
}
//----------------------------------------------------------------------------

void FamilyNode::ClearData()
{
  RootObj::ClearData();

  nodeType = nodeNone;
  parent = NULL;
  children.SetNodeMember(&FamilyNode::childNode);

  statePtr = NULL;
}
//----------------------------------------------------------------------------

// inserts 'node' as 'this's' primary/last child depending on 'insert' flag
//
void FamilyNode::Attach( FamilyNode &node, Bool insert) // = TRUE
{
	node.parent = this;

  // put it in the list
  if (insert)
  {
    children.Prepend( &node);
  }
  else
  {
    children.Append( &node);
  }
  node.SetWorldAll();   // FIXME
}
//----------------------------------------------------------------------------

// removes 'this' and all its children from parent
//
void FamilyNode::Detach()
{
	if (parent)
	{
    ASSERT(parent->children.GetCount());

    parent->children.Unlink(this);
		parent = NULL;
	}
}
//----------------------------------------------------------------------------

void FamilyNode::Attach( const char *_name, FamilyNode &node, Bool local) // = FALSE
{
  FamilyNode * part = (FamilyNode *) Find( _name, local);
  if (part)
  {
    part->Attach( node);
  }
}
//----------------------------------------------------------------------------

// removes 'this' from parent leaving its children in place
//
void FamilyNode::Extract()
{
	if (parent)
	{
    // remove from parents list
    parent->children.Unlink( this);

    // add this's children to parent's list
    NList<FamilyNode>::Iterator kids(&children);
    FamilyNode *node;
    while ((node = kids++) != NULL)
    {
      children.Unlink(node);

      node->parent = parent;
      parent->children.Append(node);
    }

    // clear ourselves
		parent = NULL;
 	}
}
//----------------------------------------------------------------------------

//  if local == TRUE then find won't cross to attached objs'
//
FamilyNode * FamilyNode::Find( U32 crc, Bool local) // = FALSE)
{
  if (crc == name.crc)
  {
    return this;
  }

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    if (local && node->nodeType != nodeMesh && node->nodeType != nodeMeshObj)
    {
      continue;
    }
    if (FamilyNode *fn = node->Find( crc, local))
    {
      return fn;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------

// find the root node
//
MeshEnt * FamilyNode::FindMeshEnt( Bool local) // = FALSE
{
  FamilyNode *node = (FamilyNode *) this;

  while (node->parent)
  {
    if (local && node->IsMeshEnt())
    {
      return (MeshEnt *) node;
    }
    node = node->parent;
  }

  return node->IsMeshEnt() ? (MeshEnt *) node : NULL;
}
//----------------------------------------------------------------------------

FamilyNode::~FamilyNode()
{
  parent = NULL;
  children.DisposeAll();
}
//----------------------------------------------------------------------------

void FamilyNode::SetWorldRecurse( const Matrix &world)
{
  CalcWorldMatrix( world);

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    node->SetWorldRecurse( WorldMatrix());
  }
}
//----------------------------------------------------------------------------

void FamilyNode::SetWorldRecurseRender( const Matrix &world, FamilyState *stateArray)
{
  FamilyState & state = stateArray[name.index];

  CalcWorldMatrix( world, state);

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    node->SetWorldRecurseRender( state.WorldMatrix(), stateArray);
  }
}
//----------------------------------------------------------------------------

void FamilyNode::SetWorldAll( const Matrix & world)
{
  ASSERT( statePtr);

  // FIXME: multiply by inverse objectMatrix
  //
  SetWorldRecurse( world);
}
//----------------------------------------------------------------------------

void FamilyNode::SetWorldAll()
{
  ASSERT( statePtr);

  if (parent)
  {
    SetWorldRecurse( parent->WorldMatrix());
    return;
  }
  // FIXME: multiply by inverse objectMatrix
  //
  SetWorldRecurse( WorldMatrix());
}
//----------------------------------------------------------------------------

void FamilyNode::Render( const Array<FamilyState> & stateArray, Color tColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( statePtr);

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    node->Render( stateArray, tColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void FamilyNode::RenderColor( const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( statePtr);

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    node->RenderColor( stateArray, color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

// caller must call matrix.ClearData() before calling
// FIXME: setup array of node points by walking backwards from findNode
//  then multiply in a loop
//
Bool FamilyNode::FindOffset( const FamilyNode *findNode, Matrix &matrix, Bool local) // = FALSE)
{
//  return FindOffsetRecurse( findNode, matrix, local);

  ASSERT( statePtr);

  if (this == findNode)
  {
    return TRUE;
  }

  Matrix newMatrix = matrix;

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    if (local && node->nodeType != nodeMesh && node->nodeType != nodeMeshObj)
    {
      continue;
    }
    matrix = newMatrix;

    if (node->FindOffsetRecurse( findNode, matrix, local))
    {
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------

// caller must call matrix.ClearData() before calling
// FIXME: setup array of node points by walking backwards from findNode
//  then multiply in a loop
//
Bool FamilyNode::FindOffsetRecurse( const FamilyNode *findNode, Matrix &matrix, Bool local) // = FALSE)
{
  ASSERT( statePtr);

  matrix = ObjectMatrix() * matrix;

  if (this == findNode)
  {
    return TRUE;
  }

  Matrix newMatrix = matrix;

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    if (local && node->nodeType != nodeMesh && node->nodeType != nodeMeshObj)
    {
      continue;
    }

    matrix = newMatrix;

    if (node->FindOffsetRecurse( findNode, matrix, local))
    {
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------

FamilyNode * FamilyNode::FindOffset( const char *_name, Matrix &matrix, Bool local) // = FALSE)
{
  FamilyNode *node = Find( _name, local);
  if (node && Child())
  {
    FindOffset( node, matrix, local);
  }
  return node;
}
//----------------------------------------------------------------------------

U32 FamilyNode::GetHierarchy( BuffString *names, U32 &count, Bool local, U32 tabCount, Matrix * matrix) const // = FALSE, = 0, = NULL
{
  if (count >= MAXMESHPERGROUP)
  {
    return count;
  }

  BuffString buff;
  char *p = buff.str;
  U32 i;
  for (i = 0; i < tabCount; i++)
  {
    *p++ = ' ';
  }

  Utils::Strcpy( p, name.str);
  if (names)
  {
    names[count] = buff.str;
  }
  else
  {
    if (matrix)
    {
      CON_DIAG((""));
      LOG_DIAG((""));
    }
    CON_DIAG( ("%s", buff.str) );
    LOG_DIAG( ("%s", buff.str) );

    if (matrix)
    {
      *p = '\0';

      CON_DIAG(("%sright %f,%f,%f", buff.str, matrix->right.x, matrix->right.y, matrix->right.z));
      CON_DIAG(("%sup    %f,%f,%f", buff.str, matrix->up.x,    matrix->up.y,    matrix->up.z));
      CON_DIAG(("%sfront %f,%f,%f", buff.str, matrix->front.x, matrix->front.y, matrix->front.z));
      CON_DIAG(("%sposit %f,%f,%f", buff.str, matrix->posit.x, matrix->posit.y, matrix->posit.z));

      LOG_DIAG(("%sright %f,%f,%f", buff.str, matrix->right.x, matrix->right.y, matrix->right.z));
      LOG_DIAG(("%sup    %f,%f,%f", buff.str, matrix->up.x,    matrix->up.y,    matrix->up.z));
      LOG_DIAG(("%sfront %f,%f,%f", buff.str, matrix->front.x, matrix->front.y, matrix->front.z));
      LOG_DIAG(("%sposit %f,%f,%f", buff.str, matrix->posit.x, matrix->posit.y, matrix->posit.z));
    }
  }

  Matrix newMatrix;
  if (matrix)
  {
    if (count > 0)
    {
      *matrix = ObjectMatrix() * *matrix;
    }
    newMatrix = *matrix;
  }
  count++;

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    if (local && node->nodeType != nodeMesh && node->nodeType != nodeMeshObj)
    {
      continue;
    }
    if (matrix)
    {
      *matrix = newMatrix;
    }

    tabCount++;
    node->GetHierarchy( names, count, local, tabCount, matrix);
    tabCount--;
  }
  return count;
}
//----------------------------------------------------------------------------

void AnimKey::ClearData()
{
  quaternion.ClearData();
  position.ClearData();
  scale.Set( 1.0f, 1.0f, 1.0f);

  frame = 0.0f;
  type = animNONE;
}
//----------------------------------------------------------------------------

void FamilyState::ClearData()
{
  AnimKey::ClearData();

  objectMatrix.ClearData();
  worldMatrix.ClearData();
}
//----------------------------------------------------------------------------

#if 0
void FamilyState::operator=( FamilyState & state)
{
  *((AnimKey *)this) = state;

  objectMatrix = state.objectMatrix;
  worldMatrix  = state.worldMatrix;
}
//----------------------------------------------------------------------------
#endif

void FamilyState::SetNode( const FamilyNode &_node)
{
  node = (FamilyNode *) &_node;
}
//----------------------------------------------------------------------------

