ConsoleMethodWithDocs(FoliageSystem, initializeBedData, ConsoleVoid, 2, 2, ())
{
	object->initializeBedData();
}

ConsoleMethodWithDocs(FoliageSystem, incrementFoliageSimTime, ConsoleVoid, 2, 2, ()){
	object->setFoliageSimTime(object->getFoliageSimTime() + 1);
}

ConsoleMethodWithDocs(FoliageSystem, updateBed, ConsoleVoid, 3, 3, ()){
	U32 x = 0, y = 0;

	dSscanf(argv[2], "%i %i", &x, &y);

	object->getBed(Point2I(x, y))->updateBed();
}

ConsoleMethodWithDocs(Seed, setPlant, ConsoleVoid, 3, 3, ()){
	// The bed index and plant index are passed in as well as the position of the new plant

	U32 bedX = 0, bedY = 0, parentPlantIndex = 0;

	// Parse indices
	dSscanf(argv[2], "%i,%i:%i", &bedX, &bedY, &parentPlantIndex);

	object->setPlant(Point2I(bedX, bedY), parentPlantIndex);

}

ConsoleMethodWithDocs(Seed, setFoliageSystem, ConsoleVoid, 3, 3, ()){
	object->setFoliageSystem(static_cast<FoliageSystem*>(Sim::findObject(argv[2])));
}

ConsoleMethodWithDocs(Seed, spawnPlant, ConsoleBool, 3, 3, ()){
	F32 x = 0, y = 0;

	dSscanf(argv[2], "%f %f", &x, &y);

	return object->spawnPlant(Point2F(x, y));
}

ConsoleMethodWithDocs(FoliageSystem, updateFoliageSystem, ConsoleVoid, 3, 3, ()){
	// We are passed in the 1D index of the bed to update
	U32 IndexArg = 0;
	dSscanf(argv[2], "%i", &IndexArg);

	U32 x = modulo(IndexArg, object->getGridDimension());
	U32 y = IndexArg / object->getGridDimension();

	FoliageBed* tempBed = object->getBed(Point2I(x, y));
	if (tempBed->getPlantManager() == NULL){
		tempBed->updateBed();
	}
	
	//object->updateVisualization(object->getBed(Point2I(x, y)));

}

ConsoleMethodWithDocs(FoliageSystem, getFoliageGridDimension, ConsoleInt, 2, 2, ()){
	return object->getGridDimension();
}

ConsoleMethodWithDocs(FoliageSystem, getNutrientLevels, ConsoleVoid, 3, 3, ())
{
	F32 x = 0, y = 0;

	dSscanf(argv[2], "%f %f", &x, &y);

	for (U32 i = 0; i < numberOfNutrients; ++i){
		Con::printf("Nutrient%i:%f", i, object->getBed(Point2F(x, y))->getAvailableNutrient(i));
	}
	
}