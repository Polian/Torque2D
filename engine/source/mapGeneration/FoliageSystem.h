#ifndef _FOLIAGE_SYSTEM_H_
#define _FOLIAGE_SYSTEM_H_

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif 

#ifndef _SPRITE_H_
#include "2d/sceneobject/Sprite.h"
#endif

#ifndef _COMPOSITE_SPRITE_H_
#include "2d/sceneobject/CompositeSprite.h"
#endif

#ifndef _IMAGE_ASSET_H_
#include "2d/assets/ImageAsset.h"
#endif

#ifndef _MAP_GENERATION_H_
#include "MapGeneration.h"
#endif

// Forward declarations
class FoliageBed;

#define numberOfNutrients 4
#define lifespanMax 1000
#define lifespanMin 100
#define childrenMax 10
#define childrenMin 2
#define PLANT_CAP 30 // This is for initial spawning of plants only

class Plant{
private:
	U32 index;
	F32 x, y;
	U32 birthTime;
	U32 lifespan;
	U32 seed;

	F32 nutrientComposition[numberOfNutrients];

	U32 potentialChildren;
	U32 childNumber;

	U8 stage;

	FoliageBed* bed;
public:
	Plant();
	Plant(Plant* pPlant);
	Plant(Plant* pPlant, Point2F spawnPoint);
	Plant(F32 x, F32 y, U32 pIndex, FoliageBed* pBed);

	~Plant();

	void setPosition(Point2F pPos)			{ this->x = pPos.x; this->y = pPos.y; }
	Point2F getPosition()					{ return Point2F(this->x, this->y); }

	void setIndex(U32 pIndex)				{ this->index = pIndex; }
	U32 getIndex()							{ return this->index; }

	void setBirthTime(U32 pTime)			{ this->birthTime = pTime; }
	U32 getBirthTime()						{ return this->birthTime; }

	void setLifeSpan(U32 pTime)				{ this->lifespan = pTime; }
	U32 getLifeSpan()						{ return this->lifespan; }

	void setStage(U8 pStage)				{ this->stage = pStage; }
	U8 getStage()							{ return stage; }

	void setFoliageBed(FoliageBed* pBed)	{ this->bed = pBed; }

	F32 getNutrient(U32 pIndex)				{ return this->nutrientComposition[pIndex]; }

	void setInitialNutrients(Plant* pParent);
	void setInitialNutrients();
	bool updateNutrients();

	bool nutrientsAreAvailable(FoliageBed* pBed);

	void updatePlant();
	void copyGeneticsTo(Plant* pPlant);
};

class FoliageSystem : public SimObject{
private:
	typedef SimObject Parent;
	Vector < Vector< FoliageBed > > foliageGrid;
	U32 beds_x, beds_y;
	SimObjectPtr<Island> island;

	SimObjectPtr<Sprite> fertilitySprite;
	GBitmap* fertilityBitmap;
	ImageAsset* debugAsset;
	TextureHandle* debugHandle;

	U32 foliageSimTime;

public:
	FoliageSystem();
	virtual ~FoliageSystem(){}

	static void initPersistFields();

	virtual bool onAdd();
	virtual void onRemove();

	void initializeBedData();

	void initializeVisualization();
	void updateVisualization(FoliageBed* pBed);

	FoliageBed* getBed(Point2F point);
	FoliageBed* getBed(Point2I index);

	U32 getFoliageSimTime(){ return this->foliageSimTime; }
	void setFoliageSimTime(U32 pTime){ this->foliageSimTime = pTime; }

	U32 getGridDimension(){ return this->beds_x; }

	//Island
	inline Island* const     getIsland(void) const                      { return island; }

	static bool             setIsland(void* obj, const char* data)
	{
		Island* pIsland = dynamic_cast<Island*>(Sim::findObject(data));
		FoliageSystem* object = static_cast<FoliageSystem*>(obj);
		if (pIsland)
		{
			object->island = pIsland;
		}
		return false;
	}
	static bool             writeIsland(void* obj, StringTableEntry pFieldName) { return false; }

	// Fertility Sprite
	inline Sprite* const     getFertilitySprite(void) const                      { return fertilitySprite; }

	static bool             setFertilitySprite(void* obj, const char* data)
	{
		Sprite* pSprite = dynamic_cast<Sprite*>(Sim::findObject(data));
		FoliageSystem* object = static_cast<FoliageSystem*>(obj);
		if (pSprite)
		{
			object->fertilitySprite = pSprite;
		}
		return false;
	}
	static bool             writeFertilitySprite(void* obj, StringTableEntry pFieldName) { return false; }

	DECLARE_CONOBJECT(FoliageSystem);
};


class FoliageBed{
private:
	Point2I index;
	F32 fertility;
	F32 availableNutrients[numberOfNutrients];
	U32 plantCap;
	F32 bedEdgeLength;
	Vector < Plant > plants;
	FoliageSystem * bedFoliageSystem;
	SimObject* plantManager;
public:
	FoliageBed();
	FoliageBed(Point2I pIndex);
	~FoliageBed(){}

	F32 getFertility(){ return this->fertility; }
	void setFertility(F32 pFertility){ this->fertility = pFertility; }

	F32 getAvailableNutrient(U32 pIndex) { return this->availableNutrients[pIndex]; }
	void setAvailableNutrient(F32 pValue, U32 pIndex) { this->availableNutrients[pIndex] = pValue; }

	U32 getPlantCap(){ return this->plantCap; }
	void setPlantCap(U32 pCap){ this->plantCap = pCap; }

	Point2I getIndex(){ return this->index; }

	void addPlant(F32 x, F32 y, U32 index){ this->plants.push_back(Plant(x, y, index, this)); }
	void addPlant(Plant* pPlant);

	void removePlant(U32 pIndex);

	Plant getPlant(U32 pIndex);
	U32 getNumberOfPlants(){ return (U32)this->plants.size(); }

	void setEdgeLength(F32 pEdgeLength){ this->bedEdgeLength = pEdgeLength; }

	void setFoliageSystem(FoliageSystem* pSystem){ this->bedFoliageSystem = pSystem; }
	FoliageSystem* getFoliageSystem(){ return this->bedFoliageSystem; }

	Point4F getBounds();
	Point2F getMapLocation();

	void setPlantManager(SimObject* pObject)	{ this->plantManager = pObject; }
	SimObject* getPlantManager()				{ return this->plantManager; }

	void updateBed();

	// Methods for adding and removing plants from loaded beds (called from the CellManager)
	void addPlantsToScene();
	void removePlantsFromScene();
};

class Seed : public Sprite{
private:
	typedef Sprite Parent;
	Plant* plant;
	FoliageSystem* mFoliageSystem;
public:
	Seed(){
		plant = NULL;
		mFoliageSystem = NULL;
	}
	~Seed();

	void setPlant(Point2I pBedIndex, U32 pParentIndex);

	bool spawnPlant(Point2F pPosition);

	void setFoliageSystem(FoliageSystem* pSystem){ this->mFoliageSystem = pSystem; }

	DECLARE_CONOBJECT(Seed);
};

#endif