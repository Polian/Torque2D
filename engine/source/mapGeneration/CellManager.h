#ifndef _CELL_MANAGER_H_
#define _CELL_MANAGER_H_

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif

#ifndef _SPRITE_H_
#include "2d/sceneobject/Sprite.h"
#endif

#ifndef _SPRITE_BASE_H_
#include "2d/core/SpriteBase.h"
#endif

#ifndef _MAP_GENERATION_H_
#include "mapGeneration/MapGeneration.h"
#endif

#ifndef _FOLIAGE_SYSTEM_H_
#include "FoliageSystem.h"
#endif

class CellManager : public SimObject
{
private:
	typedef SimObject Parent;

	Island* myIsland;
	HexCell* playerCell;
	SimObjectPtr<FoliageSystem> myFoliageSystem;
	FoliageBed* playerBed;

public:


	CellManager(){};
	virtual ~CellManager() {};
	virtual bool onAdd();
	virtual void onRemove();

	static void initPersistFields();

	// cell management methods
	void checkPlayerCell(Point2F playerPos);
	void setPlayerCell(HexCell* cell);
	HexCell* getPlayerCell() { return playerCell; };
	void getCell(Point2F playerPos);
	void generateCell(HexCell* cell);
	void removeCell(HexCell* cell);

	void setPlayerBed(FoliageBed* pBed){ this->playerBed = pBed; }
	FoliageBed* getPlayerBed() { return this->playerBed; }
	void updateFoliageBeds(Point2F playerPos);
	void generateBed(Point2I index);
	void removeBed(Point2I index);

	// Island getter/setter
	Island* getIsland(){ return myIsland; }

	bool setIsland(const char* island){
		Island* pIsland = dynamic_cast<Island*>(Sim::findObject(island));

		if (pIsland)
		{
			myIsland = pIsland;
		}
		return false;
	};

	// Foliage system getter/setter
	inline FoliageSystem* const     getFoliageSystem(void) const                      { return myFoliageSystem; }

	static bool             setFoliageSystem(void* obj, const char* data)
	{
		FoliageSystem* tempSystem = dynamic_cast<FoliageSystem*>(Sim::findObject(data));
		CellManager* object = static_cast<CellManager*>(obj);
		if (tempSystem)
		{
			object->myFoliageSystem = tempSystem;
			
			// Set the player bed
			object->playerBed = tempSystem->getBed(Point2I(0, 0));
		}


		return false;
	}
	static bool             writeFoliageSystem(void* obj, StringTableEntry pFieldName) { return false; }

	DECLARE_CONOBJECT(CellManager);
};

#endif