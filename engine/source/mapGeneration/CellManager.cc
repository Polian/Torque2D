#ifndef _CELL_MANAGER_H_
#include "mapGeneration/CellManager.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

// Script bindings.
#include "mapGeneration/CellManager_ScriptBinding.h"

bool CellManager::onAdd(){

	// Fail if the parent fails.  
	if (!Parent::onAdd())
		return false;

	//do stuff here

	return true;
}

void CellManager::onRemove(){

	//do stuff

	Parent::onRemove();
}

void CellManager::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// My fields here.
	addProtectedField("FoliageSystem", TypeSimObjectPtr, Offset(myFoliageSystem, CellManager), &setFoliageSystem, &defaultProtectedGetFn, &writeFoliageSystem, "");
}

void CellManager::checkPlayerCell(Point2F playerPos){
	//check the current cell
	F32 vertx[6] = { playerCell->verts[0].x, playerCell->verts[1].x, playerCell->verts[2].x, playerCell->verts[3].x, playerCell->verts[4].x, playerCell->verts[5].x };
	F32 verty[6] = { playerCell->verts[0].y, playerCell->verts[1].y, playerCell->verts[2].y, playerCell->verts[3].y, playerCell->verts[4].y, playerCell->verts[5].y };

	// The player is in the player cell, do nothing.
	if (pnpoly(6, vertx, verty, playerPos.x, playerPos.y)){
		return;
	}

	// The player is not in the "player cell".
	// Thus, we need to set the new cell to be the player cell and load/unload adjacents
	else{
		// Determine which cell we have entered
		for (U32 index = 0; index < 6; ++index){
			vertx[0] = playerCell->adjacent[index]->verts[0].x;
			vertx[1] = playerCell->adjacent[index]->verts[1].x;
			vertx[2] = playerCell->adjacent[index]->verts[2].x;
			vertx[3] = playerCell->adjacent[index]->verts[3].x;
			vertx[4] = playerCell->adjacent[index]->verts[4].x;
			vertx[5] = playerCell->adjacent[index]->verts[5].x;

			verty[0] = playerCell->adjacent[index]->verts[0].y;
			verty[1] = playerCell->adjacent[index]->verts[1].y;
			verty[2] = playerCell->adjacent[index]->verts[2].y;
			verty[3] = playerCell->adjacent[index]->verts[3].y;
			verty[4] = playerCell->adjacent[index]->verts[4].y;
			verty[5] = playerCell->adjacent[index]->verts[5].y;

			if (pnpoly(6, vertx, verty, playerPos.x, playerPos.y)){

				//clear old cells
				removeCell(playerCell->adjacent[modulo(index - 4, 6)]);
				removeCell(playerCell->adjacent[modulo(index - 3, 6)]);
				removeCell(playerCell->adjacent[modulo(index - 2, 6)]);

				// Set the new player cell
				setPlayerCell(playerCell->adjacent[index]);

				//generate new cells
				generateCell(playerCell->adjacent[modulo(index - 1, 6)]);
				generateCell(playerCell->adjacent[index]);
				generateCell(playerCell->adjacent[modulo(index + 1, 6)]);

				return;
			}
		}


	}
}

void CellManager::setPlayerCell(HexCell* cell){
	//Con::printf("Player Cell: %i %i", cell->center.xIndex, cell->center.yIndex);
	this->playerCell = cell;
}

void CellManager::getCell(Point2F playerPos){
	playerCell = myIsland->getCell(playerPos);
}

void CellManager::generateCell(HexCell* cell){

	// If the cell is a forest cell place trees
	if (cell->biome == 4){
		// Set tree positions		
		myIsland->plantTrees(cell);

		// Place objects in the scene at tree positions
		for (S32 k = 0; k < cell->trees.size(); ++k){
			SceneObject* treeMarker = new SceneObject();

			treeMarker->registerObject();
			treeMarker->setPosition(Vector2(cell->trees[k].x, cell->trees[k].y));
			treeMarker->setSize(Vector2("1 1"));

			treeMarker->setField("Rendered", "false");

			treeMarker->setSceneLayer(dAtoi(Con::getVariable("TreeLayer")));
			treeMarker->setSceneGroup(dAtoi(Con::getVariable("TreeGroup")));

			myIsland->scene->addToScene(treeMarker);

			cell->trees[k].object = treeMarker;
		}
	}

	// Place dynamic objects
	char s[32];
	dItoa(cell->landIndex, s);
	Con::executef(2, "placeDynamicObjects", s);
}

void CellManager::removeCell(HexCell* cell){

	// If the cell is a forest remove all the trees
	if (cell->biome == 4){

		// Remove Trees
		for (S32 k = 0; k < cell->trees.size(); ++k){
			cell->trees[k].object->deleteObject();
		}
		cell->trees.clear();
	}

	// Remove dynamic objects
	char s[32];
	dItoa(cell->landIndex, s);
	Con::executef(2, "removeDynamicObjects", s);
}


// Update Foliage Beds:
// add and remove plant objects from the scene
void CellManager::updateFoliageBeds(Point2F playerPos){
	//Check if the player bed has changed
	FoliageBed* tempBed = static_cast<FoliageSystem*>(this->myFoliageSystem)->getBed(playerPos);
	if (this->playerBed->getIndex() == tempBed->getIndex()){
		return;
	}
	// The player is in a new bed 
	else{
		// Determine which beds are going to be loaded and removed
		Point2I unloadList[3] = {};
		Point2I loadList[3] = {};

		S32 oldXIndex = this->playerBed->getIndex().x;
		S32 oldYIndex = this->playerBed->getIndex().y;
		S32 newXIndex = tempBed->getIndex().x;
		S32 newYIndex = tempBed->getIndex().y;

		if (oldXIndex < newXIndex){
			unloadList[0] = Point2I(oldXIndex - 1, oldYIndex + 1);
			unloadList[1] = Point2I(oldXIndex - 1, newYIndex);
			unloadList[2] = Point2I(oldXIndex - 1, oldYIndex - 1);

			loadList[0] = Point2I(newXIndex + 1, newYIndex + 1);
			loadList[1] = Point2I(newXIndex + 1, newYIndex);
			loadList[2] = Point2I(newXIndex + 1, newYIndex - 1);
		}
		else if (oldXIndex > newXIndex){
			unloadList[0] = Point2I(oldXIndex + 1, oldYIndex + 1);
			unloadList[1] = Point2I(oldXIndex + 1, oldYIndex);
			unloadList[2] = Point2I(oldXIndex + 1, oldYIndex - 1);

			loadList[0] = Point2I(newXIndex - 1, newYIndex + 1);
			loadList[1] = Point2I(newXIndex - 1, newYIndex);
			loadList[2] = Point2I(newXIndex - 1, newYIndex - 1);
		}
		else if (oldYIndex < newYIndex){
			unloadList[0] = Point2I(oldXIndex + 1, oldYIndex - 1);
			unloadList[1] = Point2I(oldXIndex, oldYIndex - 1);
			unloadList[2] = Point2I(oldXIndex - 1, oldYIndex - 1);

			loadList[0] = Point2I(newXIndex + 1, newYIndex + 1);
			loadList[1] = Point2I(newXIndex, newYIndex + 1);
			loadList[2] = Point2I(newXIndex - 1, newYIndex + 1);
		}
		else if (oldYIndex > newYIndex){
			unloadList[0] = Point2I(oldXIndex + 1, oldYIndex + 1);
			unloadList[1] = Point2I(oldXIndex, oldYIndex + 1);
			unloadList[2] = Point2I(oldXIndex - 1, oldYIndex + 1);

			loadList[0] = Point2I(newXIndex + 1, newYIndex - 1);
			loadList[1] = Point2I(newXIndex, newYIndex - 1);
			loadList[2] = Point2I(newXIndex - 1, newYIndex - 1);
		}

		// Unload old beds (needs to happen before we update the player bed)
		this->removeBed(unloadList[0]);
		this->removeBed(unloadList[1]);
		this->removeBed(unloadList[2]);

		// Load adjacent beds
		this->generateBed(loadList[0]);
		this->generateBed(loadList[1]);
		this->generateBed(loadList[2]);

		// Update the player bed
		this->playerBed = tempBed;
		//Con::printf("New Bed: %d %d\tLoad beds: %d %d, %d %d, %d %d", tempBed->getIndex().x, tempBed->getIndex().y, loadList[0].x, loadList[0].y, loadList[1].x, loadList[1].y, loadList[2].x, loadList[2].y);
		//Con::executef(1, "printPlantCount");
		
	}
	

}

void CellManager::generateBed(Point2I index){
	this->myFoliageSystem->getBed(index)->addPlantsToScene();
}

void CellManager::removeBed(Point2I index){
	this->myFoliageSystem->getBed(index)->removePlantsFromScene();
}
IMPLEMENT_CONOBJECT(CellManager);