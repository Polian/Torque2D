ConsoleMethodWithDocs(CellManager, setIsland, ConsoleVoid, 3, 3, ())
{
	object->setIsland(argv[2]);
}

ConsoleMethodWithDocs(CellManager, setPlayerCell, ConsoleVoid, 3, 3, ())
{
	Point2F playerPos;

	dSscanf(argv[2], "%f %f", &playerPos.x, &playerPos.y);

	// Set the player cell based on the players position
	object->getCell(playerPos);

	// Generate the cells surrounding the player
	object->generateCell(object->getPlayerCell());
	object->generateCell(object->getPlayerCell()->adjacent[0]);
	object->generateCell(object->getPlayerCell()->adjacent[1]);
	object->generateCell(object->getPlayerCell()->adjacent[2]);
	object->generateCell(object->getPlayerCell()->adjacent[3]);
	object->generateCell(object->getPlayerCell()->adjacent[4]);
	object->generateCell(object->getPlayerCell()->adjacent[5]);
}

ConsoleMethodWithDocs(CellManager, unloadPlayerCell, ConsoleVoid, 3, 3, ())
{
	Point2F playerPos;

	dSscanf(argv[2], "%f %f", &playerPos.x, &playerPos.y);

	// Set the player cell based on the players position
	object->getCell(playerPos);

	//remove any trees that have been added to the scene
	Con::executef(1, "cleanupFoliage");
	

	// Remove the cells surrounding the player
	object->removeCell(object->getPlayerCell());
	object->removeCell(object->getPlayerCell()->adjacent[0]);
	object->removeCell(object->getPlayerCell()->adjacent[1]);
	object->removeCell(object->getPlayerCell()->adjacent[2]);
	object->removeCell(object->getPlayerCell()->adjacent[3]);
	object->removeCell(object->getPlayerCell()->adjacent[4]);
	object->removeCell(object->getPlayerCell()->adjacent[5]);
}

ConsoleMethodWithDocs(CellManager, updateCells, ConsoleVoid, 3, 3, ())
{
	Point2F playerPos;

	dSscanf(argv[2], "%f %f", &playerPos.x, &playerPos.y);

	object->checkPlayerCell(playerPos);
}

ConsoleMethodWithDocs(CellManager, getLandCellCount, ConsoleInt, 2, 2, ())
{
	return object->getIsland()->landCellCount;
}

/*! Get the 1D Index of the cell the given point resides in.
@param x y position that is being checked.
@return The index of the cell.
*/
ConsoleMethodWithDocs(CellManager, getCellIndex, ConsoleInt, 3, 3, ())
{
	Point2F pos;

	dSscanf(argv[2], "%f %f", &pos.x, &pos.y);

	return object->getIsland()->getCell(pos)->landIndex;
}

ConsoleMethodWithDocs(CellManager, getSeed, ConsoleInt, 2, 2, ())
{
	return object->getIsland()->seed;
}
