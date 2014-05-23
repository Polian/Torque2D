#ifndef _MAP_GENERATION_H_
#define _MAP_GENERATION_H_

#include <iostream>
#include <fstream>

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif 

#ifndef SIMPLEX_H_
#include "perlinNoise/simplexnoise.h"
#endif

#ifndef _VECTOREXT_H
#include "collection/vectorQueue.h"
#endif

//NOTE: Not regular Hexagons for the sake of more easily filling the given area

class HexVert{

public:
	U32 xIndex, yIndex, xCell, yCell;
	F32 x, y;
	F32 elevation;
	F32 landNoise;

public:
	HexVert(){};
	virtual ~HexVert(){};

	HexVert(F32 _x, F32 _y, U32 _ix, U32 _iy, F32 _elevation, F32 _landNoise){
		x = _x;
		y = _y;
		xIndex = _ix;
		yIndex = _iy;
		elevation = _elevation;
		landNoise = _landNoise;
	};

};

class HexCell{

public:
	U32 index;
	HexCell* adjacent[6];
	HexVert verts[6];
	HexVert center;
	U32 biome;

public:
	HexCell(){};
	virtual ~HexCell(){};

	void init(U32 x, U32 y, HexVert _center){
		index = y;
		center = _center;
		center.xCell = x;
		center.yCell = y;		
	};

	void setVertsEven(Vector<Vector<HexVert>> points){
		U32 upperBound = points.size() - 1;
		//check to see if this is an edge cell
		if (center.xIndex > 0 && center.xIndex < upperBound && center.yIndex > 0 && center.yIndex < upperBound){
			verts[0] = points[center.yIndex][center.xIndex-1];
			verts[1] = points[center.yIndex+1]	[center.xIndex - 1];
			verts[2] = points[center.yIndex+1][center.xIndex];
			verts[3] = points[center.yIndex][center.xIndex+1];
			verts[4] = points[center.yIndex-1][center.xIndex];
			verts[5] = points[center.yIndex - 1][center.xIndex - 1];
		}
		
	};

	void setVertsOdd(Vector<Vector<HexVert>> points){
		U32 upperBound = points.size() - 1;
		//check to see if this is an edge cell
		if (center.xIndex > 0 && center.xIndex < upperBound && center.yIndex > 0 && center.yIndex < upperBound){
			verts[0] = points[center.yIndex][center.xIndex-1];
			verts[1] = points[center.yIndex + 1][center.xIndex];
			verts[2] = points[center.yIndex + 1][center.xIndex+1];
			verts[3] = points[center.yIndex][center.xIndex+1];
			verts[4] = points[center.yIndex - 1][center.xIndex+1];
			verts[5] = points[center.yIndex - 1][center.xIndex];
		}

	};

	void setAdjEven(Vector<Vector<HexCell>> cells){
		U32 xUpperBound = cells[0].size() - 1;
		U32 yUpperBound = cells.size() - 2;
		//index = mRound((points[center.yIndex][center.xIndex].xIndex + points.size() * points[center.yIndex][center.xIndex].yIndex) / 3) - 1;
		//only find adjacent nodes to interior cells that have neighbors on all sides
		if (center.xCell > 0 && center.xCell < xUpperBound && center.yCell > 1 && center.yCell < yUpperBound){
			adjacent[0] = &cells[center.yCell+1][center.xCell - 1];
			adjacent[1] = &cells[center.yCell + 2][center.xCell];
			adjacent[2] = &cells[center.yCell + 1][center.xCell];
			adjacent[3] = &cells[center.yCell - 1][center.xCell];
			adjacent[4] = &cells[center.yCell - 2][center.xCell];
			adjacent[5] = &cells[center.yCell - 1][center.xCell - 1];
		}
		else{
			adjacent[0] = NULL;
			adjacent[1] = NULL;
			adjacent[2] = NULL;
			adjacent[3] = NULL;
			adjacent[4] = NULL;
			adjacent[5] = NULL;
		}
	};

	void setAdjOdd(Vector<Vector<HexCell>> cells){
		U32 xUpperBound = cells[0].size() - 1;
		U32 yUpperBound = cells.size() - 2;
		//index = mRound((points[center.yIndex][center.xIndex].xIndex + points.size() * points[center.yIndex][center.xIndex].yIndex) / 3) - 1;
		//only find adjacent nodes to interior cells that have neighbors on all sides
		if (center.xCell > 0 && center.xCell < xUpperBound && center.yCell > 1 && center.yCell < yUpperBound){
			adjacent[0] = &cells[center.yCell + 1][center.xCell];
			adjacent[1] = &cells[center.yCell + 2][center.xCell];
			adjacent[2] = &cells[center.yCell + 1][center.xCell + 1];
			adjacent[3] = &cells[center.yCell - 1][center.xCell + 1];
			adjacent[4] = &cells[center.yCell - 2][center.xCell];
			adjacent[5] = &cells[center.yCell - 1][center.xCell];
		}
		else{
			adjacent[0] = NULL;
			adjacent[1] = NULL;
			adjacent[2] = NULL;
			adjacent[3] = NULL;
			adjacent[4] = NULL;
			adjacent[5] = NULL;
		}
	};
};



// Island Class
class Island : public SimObject
{
private:
	typedef SimObject Parent;

public:
	F32 hexEdgeLength;
	F32 hexOffset;
	U32 seed;
	U32 area;
	Vector<Vector<HexVert>> points;
	Vector<Vector<HexCell>> cells;

	Island();
	virtual ~Island() {};
	virtual bool onAdd();

	static void initPersistFields();

	void assignLandBiome(HexCell* cell);

	void setPoint(U32 i, U32 j);
	F32 genNoise(F32 x, F32 y);
	F32 genLandNoise(F32 x, F32 y);
	void assignCell(U32 i, U32 j);
	void assignCellAdj(U32 i, U32 j);
	void oceanFill(HexCell* cell);

	DECLARE_CONOBJECT(Island);
};




#endif