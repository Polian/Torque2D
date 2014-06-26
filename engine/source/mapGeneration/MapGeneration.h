#ifndef _MAP_GENERATION_H_
#define _MAP_GENERATION_H_

#include <iostream>
#include <fstream>
#include <thread>

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif 

#ifndef SIMPLEX_H_
#include "perlinNoise/simplexnoise.h"
#endif

#ifndef _VECTOREXT_H
#include "collection/vectorQueue.h"
#endif

#ifndef _SCENE_H_
#include "2d/scene/Scene.h"
#endif

#ifndef _SHAPE_VECTOR_H_
#include "2d\sceneobject\ShapeVector.h"
#endif

#ifndef _PLATFORM_THREADS_THREAD_H_
#include "platform\threads\thread.h"
#endif

#include <png.h>

//NOTE: Not regular Hexagons for the sake of more easily filling the given area

class Tree{
public:
	F32 x, y;
	F32 radius;

public:
	Tree(){};
	virtual ~Tree(){};

	Tree(F32 vert_x, F32 vert_y, F32 _rad){
		x = vert_x;
		y = vert_y;
		radius = _rad;
	};
};

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

	HexVert(F32 _x, F32 _y){
		x = _x;
		y = _y;
		xIndex = 0;
		yIndex = 0;
		elevation = 0;
		landNoise = 0;
	};

	U32 checkPosition(Vector<Tree> trees, F32 radius){
		F32 distance = 0;

		for (U32 i = 0; i < U32(trees.size()); ++i){
			distance = mSqrt(mPow(x - trees[i].x, 2) + mPow(y - trees[i].y, 2));

			if (distance < radius){
				return 1;
			}
		}
		return 0;
	};

};

class HexCell{

public:
	Point2I index;
	HexCell* adjacent[6];
	HexVert verts[6];
	HexVert center;
	U32 biome;
	Vector<Tree> trees;

public:
	HexCell(){};
	virtual ~HexCell(){};

	void init(U32 x, U32 y, HexVert _center){
		index = Point2I(x, y);
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
		else{
			verts[0] = HexVert(0, 0);
			verts[1] = HexVert(0, 0);
			verts[2] = HexVert(0, 0);
			verts[3] = HexVert(0, 0);
			verts[4] = HexVert(0, 0);
			verts[5] = HexVert(0, 0);
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
		else{
			verts[0] = HexVert(0, 0);
			verts[1] = HexVert(0, 0);
			verts[2] = HexVert(0, 0);
			verts[3] = HexVert(0, 0);
			verts[4] = HexVert(0, 0);
			verts[5] = HexVert(0, 0);
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
	SimObjectPtr<Scene>  scene;
	

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
	void plantTrees(HexCell* cell);
	void renderMap();

	F32 checkVert(HexVert p1, HexVert p2, HexVert testVert);

	//scene
	inline Scene* const     getScene(void) const                      { return scene; }

	static bool             setScene(void* obj, const char* data)
	{
		Scene* pScene = dynamic_cast<Scene*>(Sim::findObject(data));
		Island* object = static_cast<Island*>(obj);
		if (pScene)
		{
			object->scene = pScene;
		}
		return false;
	}
	static bool             writeScene(void* obj, StringTableEntry pFieldName) { return false; }

	void placeTrees(){
		S32 i, j, k;
		for (i = 0; i < cells.size(); ++i){
			for (j = 0; j < cells[i].size(); ++j){
				for (k = 0; k < cells[i][j].trees.size(); ++k){
					ShapeVector* shape = new ShapeVector();

					shape->registerObject();
					shape->setPosition(Vector2(cells[i][j].trees[k].x, cells[i][j].trees[k].y));
					shape->setSize(Vector2("1 1"));
					shape->setIsCircle(true);
					shape->setLineColor(ColorF(0.5f, 0.9f, 0.2f, 1.0f));
					shape->setCircleRadius(0.5f);
					shape->setSceneLayer(4);

					scene->addToScene(shape);
				}
				
			}
		}
		
	};

	DECLARE_CONOBJECT(Island);
};


void loadTaml(const char* name, const char* extension);
void unloadTaml(const char* name, const char* extension);

#endif