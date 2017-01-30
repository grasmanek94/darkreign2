/*
	makeball.cpp

	Generates the vertices for a ball approximated with polygonal facets

	by John Cooke
*/
//----------------------------------------------------------------------------

#include "mathtypes.h"
#include "vertex.h"
//----------------------------------------------------------------------------

Bool MakeBall( F32 radius, U32 bands, DWORD &vertCount, Vertex **vertListP, DWORD &indexCount, WORD **indexListP) 
{
	U32 i, j, LastIndex, BandsX2, LastBandStartIndex;
	U32 TopBandStartIndex, BottomBandStartIndex, faces, verts, zvert;
	U32 dowrap = 1;
//	U32 dowrap = 0;
	Vector BaseVec, BandVec, WVec, TempVec;
	F32 zmin, c1, c2, c0, stickwid, r1, r2;
	F32 *rads;
		
	BandsX2 = bands*2;

	Quaternion att;
	Matrix matY, matZ;
  matY.ClearData();
  matZ.ClearData();

	att.Set( PI / bands, Matrix::I.Up());
	matY.Set( att);
	
	att.Set( PI / bands, Matrix::I.Front());
	matZ.Set( att);

	verts = 2 + (BandsX2 + 1) * (bands - 1);
	faces = BandsX2 * bands + BandsX2 * (bands - 2);

	rads = new F32[bands + 1];
	if (!rads) 
	{
		return FALSE;
	}

	Vertex *vlP = new Vertex[verts + 1];
	if (!vlP)
	{
		delete [] rads;
		return FALSE;
	}
	WORD *ilP = new WORD[faces * 3 + 1];
	if (!ilP)
	{
		delete [] vlP;
		delete [] rads;
		return FALSE;
	}
	if (*vertListP)
  {
    delete [] *vertListP;
  }
	if (*indexListP)
  {
    delete [] *indexListP;
  }
	*vertListP = vlP;
	*indexListP = ilP;
	vertCount = indexCount = 0;

	zmin = 32000.0f;
	// point at the top
	BaseVec.x = 0.0f;
	BaseVec.y = radius;
	BaseVec.z = 0.0f;
	(*vertListP)[vertCount].vv = BaseVec;
	(*vertListP)[vertCount].nv = (*vertListP)[vertCount].vv;
	(*vertListP)[vertCount].nv.Normalize();
	vertCount++;

	BandVec = BaseVec;

	// create the vertices in each band in turn
	for (i = 1; i < bands; i++)
	{
		// rotate around Z to the next band's latitude
		matZ.Transform( TempVec, BandVec);

		WVec = BandVec = TempVec;
		rads[i] = (F32) sqrt( WVec.x * WVec.x + WVec.z * WVec.z);
		// do the vertices in this band
		U32 startVert = vertCount;
		for (j = 0; j < BandsX2; j++)
		{
			WVec = TempVec;
			(*vertListP)[vertCount].vv = WVec;
			(*vertListP)[vertCount].nv = (*vertListP)[vertCount].vv;
			(*vertListP)[vertCount].nv.Normalize();
			vertCount++;

			if (WVec.z < zmin)
			{
				zmin = WVec.z;
				zvert = vertCount;
			}
			// rotate around Y to the next vertex's longitude
			matY.Transform( TempVec, WVec);
		}
		// duplicate band start vert at the end (for texture wrapping)
		(*vertListP)[vertCount].vv = (*vertListP)[startVert].vv;
		(*vertListP)[vertCount].nv = (*vertListP)[vertCount].vv;
		(*vertListP)[vertCount].nv.Normalize();
		vertCount++;
	}
	// point at the bottom
	BaseVec.y = -radius;
	(*vertListP)[vertCount].vv = BaseVec;
	(*vertListP)[vertCount].nv = (*vertListP)[vertCount].vv;
	(*vertListP)[vertCount].nv.Normalize();
	LastIndex = vertCount;
	vertCount++;

	r1 = 0.0f;
	r2 = 1.0f / (F32) bands;
	// top band
	stickwid = 100 * rads[1] / radius;
	stickwid += (100 - stickwid)/2;
	U32 i0, i1, i2, i3, i4, i5;
	for (i = 0; i < BandsX2; i++)
	{
		i0 = indexCount;
		i1 = indexCount + 1;
		i2 = indexCount + 2;
		indexCount += 3;

		(*indexListP)[i0] = 0;
//		(*indexListP)[i1] = ((i + 1) % BandsX2) + 1;
		(*indexListP)[i1] = (WORD) (((i + 1) % (BandsX2 + 1)) + 1);
		(*indexListP)[i2] = (WORD) (i + 1);

		if (dowrap == 1)
		{
			c1 = (F32) i / (F32) BandsX2;
			c2 = (F32) (i + 1) / (F32) BandsX2;
			c0 = (c1 + 0.5f) / (F32) BandsX2;

			(*vertListP)[(*indexListP)[i0]].u = c0;
			(*vertListP)[(*indexListP)[i0]].v = r1;

			(*vertListP)[(*indexListP)[i1]].u = c1;
			(*vertListP)[(*indexListP)[i1]].v = r2;

			(*vertListP)[(*indexListP)[i2]].u = c2;
			(*vertListP)[(*indexListP)[i2]].v = r2;
		}
		else
		{
			(*vertListP)[(*indexListP)[i0]].u = 0.5f;
			(*vertListP)[(*indexListP)[i0]].v = 0.0f;

			(*vertListP)[(*indexListP)[i1]].u = 0.0f;
			(*vertListP)[(*indexListP)[i1]].v = 1.0f;

			(*vertListP)[(*indexListP)[i2]].u = 1.0f;
			(*vertListP)[(*indexListP)[i2]].v = 1.0f;
		}
	}

	// middle bands
	for (j = 0; j < bands - 2; j++)
	{
//		TopBandStartIndex = j * BandsX2 + 1;
//		BottomBandStartIndex = (j + 1) * BandsX2 + 1;
		TopBandStartIndex = j * (BandsX2 + 1) + 1;
		BottomBandStartIndex = (j + 1) * (BandsX2 + 1) + 1;
		r1 = (F32) (j + 1) / (F32) bands;
		r2 = (F32) (j + 2) / (F32) bands;
		for (i = 0; i < BandsX2; i++)
		{
			i0 = indexCount;
			i1 = indexCount + 1;
			i2 = indexCount + 2;
			i3 = indexCount + 3;
			i4 = indexCount + 4;
			i5 = indexCount + 5;
			indexCount += 6;

			(*indexListP)[i0] = (WORD) (i + TopBandStartIndex);
//			(*indexListP)[i1] = ((i + 1) % BandsX2) + TopBandStartIndex;
//			(*indexListP)[i2] = ((i + 1) % BandsX2) + BottomBandStartIndex;
			(*indexListP)[i1] = (WORD) (((i + 1) % (BandsX2 + 1)) + TopBandStartIndex);
			(*indexListP)[i2] = (WORD) (((i + 1) % (BandsX2 + 1)) + BottomBandStartIndex);

//			(*indexListP)[i3] = ((i + 1) % BandsX2) + BottomBandStartIndex;
			(*indexListP)[i3] = (WORD) (((i + 1) % (BandsX2 + 1)) + BottomBandStartIndex);
			(*indexListP)[i4] = (WORD) (i + BottomBandStartIndex);
			(*indexListP)[i5] = (WORD) (i + TopBandStartIndex);

			if (dowrap == 1)
			{
				c1 = (F32) i / (F32) BandsX2;
				c2 = (F32) (i + 1) / (F32) BandsX2;

				(*vertListP)[(*indexListP)[i0]].u = c1;
				(*vertListP)[(*indexListP)[i0]].v = r1;

				(*vertListP)[(*indexListP)[i1]].u = c2;
				(*vertListP)[(*indexListP)[i1]].v = r1;

				(*vertListP)[(*indexListP)[i2]].u = c2;
				(*vertListP)[(*indexListP)[i2]].v = r2;

				(*vertListP)[(*indexListP)[i3]].u = c2;
				(*vertListP)[(*indexListP)[i3]].v = r2;

				(*vertListP)[(*indexListP)[i4]].u = c1;
				(*vertListP)[(*indexListP)[i4]].v = r2;

				(*vertListP)[(*indexListP)[i5]].u = c1;
				(*vertListP)[(*indexListP)[i5]].v = r1;
			}
			else
			{
				(*vertListP)[(*indexListP)[i0]].u = 1.0f;
				(*vertListP)[(*indexListP)[i0]].v = 0.0f;
										     
				(*vertListP)[(*indexListP)[i1]].u = 0.0f;
				(*vertListP)[(*indexListP)[i1]].v = 0.0f;
										     
				(*vertListP)[(*indexListP)[i2]].u = 0.0f;
				(*vertListP)[(*indexListP)[i2]].v = 1.0f;
										     
				(*vertListP)[(*indexListP)[i3]].u = 0.0f;
				(*vertListP)[(*indexListP)[i3]].v = 1.0f;
										     
				(*vertListP)[(*indexListP)[i4]].u = 1.0f;
				(*vertListP)[(*indexListP)[i4]].v = 1.0f;
										     
				(*vertListP)[(*indexListP)[i5]].u = 1.0f;
				(*vertListP)[(*indexListP)[i5]].v = 0.0f;
			}
		}
	}

	// bottom band
//	LastBandStartIndex = BandsX2 * (bands - 2) + 1;
	LastBandStartIndex = (BandsX2 + 1) * (bands - 2) + 1;
	r1 = (F32) (bands - 1) / (F32) bands;
	r2 = 1.0f;
	for (i = 0; i < BandsX2; i++)
	{
		i0 = indexCount;
		i1 = indexCount + 1;
		i2 = indexCount + 2;
		indexCount += 3;

		(*indexListP)[i0] = (WORD) LastIndex;
		(*indexListP)[i1] = (WORD) (LastBandStartIndex + i);
//		(*indexListP)[i2] = LastBandStartIndex + ((i + 1) % BandsX2);
		(*indexListP)[i2] = (WORD) (LastBandStartIndex + ((i + 1) % (BandsX2 + 1)));

		if (dowrap == 1)
		{
			c1 = (F32) i / (F32) BandsX2;
			c2 = (F32) (i + 1) / (F32) BandsX2;
			c0 = c1 - 0.5f / (F32) BandsX2;

			(*vertListP)[(*indexListP)[i0]].u = c0;
			(*vertListP)[(*indexListP)[i0]].v = r2;
									     
			(*vertListP)[(*indexListP)[i1]].u = c1;
			(*vertListP)[(*indexListP)[i1]].v = r1;
									     
			(*vertListP)[(*indexListP)[i2]].u = c2;
			(*vertListP)[(*indexListP)[i2]].v = r1;
		}
		else
		{
			(*vertListP)[(*indexListP)[i0]].u = 0.5f;
			(*vertListP)[(*indexListP)[i0]].v = 0.0f;
									     
			(*vertListP)[(*indexListP)[i1]].u = 0.0f;
			(*vertListP)[(*indexListP)[i1]].v = 1.0f;
									     
			(*vertListP)[(*indexListP)[i2]].u = 1.0f;
			(*vertListP)[(*indexListP)[i2]].v = 1.0f;
		}
	}
	delete [] rads;

	return TRUE;
}
//----------------------------------------------------------------------------
