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
	//addProtectedField("objectSet", TypeSimObjectPtr, Offset(objectSet, CellManager), &setObjectSet, &defaultProtectedGetFn, &writeObjectSet, "");
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

				break;
			}
		}


	}
}

void CellManager::setPlayerCell(HexCell* cell){
	//Con::printf("Player Cell: %i %i", cell->center.xIndex, cell->center.yIndex);
	playerCell = cell;
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

IMPLEMENT_CONOBJECT(CellManager);