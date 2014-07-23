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

class CellManager : public SimObject
{
private:
	typedef SimObject Parent;

	Island* myIsland;
	HexCell* playerCell;

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

	Island* getIsland(){ return myIsland; }

	bool setIsland(const char* island){
		Island* pIsland = dynamic_cast<Island*>(Sim::findObject(island));

		if (pIsland)
		{
			myIsland = pIsland;
		}
		return false;
	};

	DECLARE_CONOBJECT(CellManager);
};

#endif