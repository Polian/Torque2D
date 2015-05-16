#ifndef _FOLIAGE_SYSTEM_H_
#include "FoliageSystem.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#include "FoliageSystem_ScriptBinding.h"

//--------------------------
// Constructors
//--------------------------
FoliageSystem::FoliageSystem(){
	this->beds_x = 200;
	this->beds_y = this->beds_x;
	this->foliageSimTime = 0;
}

FoliageBed::FoliageBed(){
	this->fertility = 0;
	
	for (U32 i = 0; i < numberOfNutrients; ++i){
		this->availableNutrients[i] = 0;
	}

	this->plantCap = 0;
	this->plantManager = NULL;
}

FoliageBed::FoliageBed(Point2I pIndex){
	this->fertility = 0;

	for (U32 i = 0; i < numberOfNutrients; ++i){
		this->availableNutrients[i] = 0;
	}

	this->plantCap = 0;

	this->index.x = pIndex.x;
	this->index.y = pIndex.y;

	this->plantManager = NULL;
}

//-------------------
// Persist fields
//-------------------
void FoliageSystem::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// Add my fields here.  
	addProtectedField("island", TypeSimObjectPtr, Offset(island, FoliageSystem), &setIsland, &defaultProtectedGetFn, &writeIsland, "");
	addProtectedField("fertilitySprite", TypeSimObjectPtr, Offset(fertilitySprite, FoliageSystem), &setFertilitySprite, &defaultProtectedGetFn, &writeFertilitySprite, "");
}


bool FoliageSystem::onAdd(){

	// Fail if the parent fails.  
	if (!Parent::onAdd())
		return false;

	// Initialize the grid of foliage beds
	for (U32 x = 0; x < this->beds_x; ++x){
		this->foliageGrid.push_back(Vector<FoliageBed>());
		for (U32 y = 0; y < this->beds_y; ++y){
			this->foliageGrid[x].push_back(FoliageBed(Point2I(x, y)));

			this->foliageGrid[x][y].setFoliageSystem(this);

			// set the plant cap's
			this->foliageGrid[x][y].setPlantCap(PLANT_CAP);
		}
	}

	return true;
}

void FoliageSystem::onRemove(){
	this->foliageGrid.clear();
}

void FoliageSystem::initializeBedData(){
	// make sure the island exists
	AssertFatal(this->island, "FoliageSystem's island has not been set.");

	// Set up bed data
	for (U32 x = 0; x < this->beds_x; ++x){
		for (U32 y = 0; y < this->beds_y; ++y){

			F32 edgeLength = F32(this->getIsland()->area) / F32(beds_x);
			this->foliageGrid[x][y].setEdgeLength(edgeLength);

			// set the fertility data
			// NOTE: making the scale smaller makes the features larger
			U32 biome = this->getIsland()->getCell(this->foliageGrid[x][y].getMapLocation())->biome;
			F32 fertility = 0;
			if (biome != 0 && biome != 2){
				 fertility = scaled_octave_noise_3d(8.0f, 0.0f, 0.0007f, 0.0f, 1.0f, x*500.0f, y*500.0f, (const float) (this->island->seed));
			}
			
			this->foliageGrid[x][y].setFertility(fertility);

			// Set the nutrient values
			Point2F bedPosition = this->foliageGrid[x][y].getMapLocation();
			U32 bedBiome = this->getIsland()->getCell(bedPosition)->biome;
			F32 biomeNutrientField = 0, biomeNutrientForest = 0, biomeNutrientShore = 0, lightNutrient = 0;

			if (bedBiome == 3){			//Field
				biomeNutrientField = 1;
				biomeNutrientForest = mRandF(0.1f, 0.2f);
				biomeNutrientShore = mRandF(0.1f, 0.2f);
				lightNutrient = mRandF(0.1f, 0.2f);
			}
			else if (bedBiome == 4){	//Forest
				biomeNutrientForest = 1;
				biomeNutrientField = mRandF(0.1f, 0.2f);
				biomeNutrientShore = mRandF(0.1f, 0.2f);
				lightNutrient = mRandF(0.1f, 0.2f);
			}
			else if (bedBiome == 1){	//Shore
				biomeNutrientShore = 1;
				biomeNutrientForest = mRandF(0.1f, 0.2f);
				biomeNutrientField = mRandF(0.1f, 0.2f);
				lightNutrient = mRandF(0.1f, 0.2f);
			}

			// Light producing nutrient:
			// Designate beds randomly with a % chance of any land biome 
			// having some concentration of light producing nutrients
			if (mRandI(0, 1000) < 1 && bedBiome != 2 && bedBiome != 0){
				lightNutrient = 1;
			}

			this->foliageGrid[x][y].setAvailableNutrient(biomeNutrientField*fertility, 0);
			this->foliageGrid[x][y].setAvailableNutrient(biomeNutrientForest*fertility, 1);
			this->foliageGrid[x][y].setAvailableNutrient(biomeNutrientShore*fertility, 2);
			this->foliageGrid[x][y].setAvailableNutrient(lightNutrient*fertility, 3);


			// Set initial plant spawns
			if (fertility > 0){
				for (U32 i = 0; i < U32(this->foliageGrid[x][y].getPlantCap()*fertility); ++i){
					Point4F bounds = this->foliageGrid[x][y].getBounds();
					F32 plantX = mRandF(bounds.x, bounds.z);
					F32 plantY = mRandF(bounds.y, bounds.w);

					this->foliageGrid[x][y].addPlant(plantX, plantY, i);
				}
			}

			//this->foliageGrid[x][y].setFertility(lightNutrient);
			
		}
	}

	


	//Export bed data for visualization
	// TODO: remove for final shipping version
	/*std::ofstream outFile;
	outFile.open("modules/LightModule/1/MapData/FoliageData.csv");
	for (U32 x = 0; x < this->beds_x; ++x){
		for (U32 y = 0; y < this->beds_y; ++y){
			outFile << this->foliageGrid[x][y].getFertility() << ",";

		}
		outFile << "\n";
	}
	outFile.close();*/

	this->initializeVisualization();
	Con::printf("Finished Writing foliage data\n");

}

FoliageBed* FoliageSystem::getBed(Point2F worldPoint){
	// get the index of the bed from the world point
	Point2I index;

	// NOTE: the denominator is simply the edge length of a bed
	//- (this->getIsland()->area / F32(beds_x))
	index.x = S32(((worldPoint.x + this->getIsland()->area) * F32(beds_x)) / (F32(this->getIsland()->area * 2)));
	index.y = S32(((worldPoint.y + this->getIsland()->area) * F32(beds_x)) / (F32(this->getIsland()->area * 2)));

	return &this->foliageGrid[index.x][index.y];
}

FoliageBed* FoliageSystem::getBed(Point2I index){
	// Simply return the bed at the given index
	return &this->foliageGrid[index.x][index.y];
}

void FoliageSystem::initializeVisualization(){
	
	// construct the bitmap
	Point2I extent;
	extent.x = this->beds_x;
	extent.y = this->beds_y;

	fertilityBitmap = new GBitmap(extent.x, extent.y, false, GBitmap::BitmapFormat::RGBA);
	U8 * pixels = fertilityBitmap->getWritableBits();

	for (S32 i = 0; i<extent.x; i++)
	{
		for (S32 j = 0; j<extent.y; j++)
		{
			U8 value = U8(this->foliageGrid[i][j].getFertility() * 255);
			pixels[i * 4 + j * 4 * extent.x + 0] = value;
			pixels[i * 4 + j * 4 * extent.x + 1] = value;
			pixels[i * 4 + j * 4 * extent.x + 2] = value;
			pixels[i * 4 + j * 4 * extent.x + 3] = 255;
		}
	}

	this->debugHandle = new TextureHandle();
	this->debugHandle->set(TextureManager::getUniqueTextureKey(), fertilityBitmap, TextureHandle::TextureHandleType::BitmapKeepTexture);


	this->debugAsset = new ImageAsset();
	this->debugAsset->setAssetInternal(true);
	this->debugAsset->setAssetAutoUnload(false);
	this->debugAsset->setImageTexture(*this->debugHandle);
	AssetDatabase.addPrivateAsset(this->debugAsset);

	fertilitySprite = new Sprite();
	fertilitySprite->registerObject();

	static_cast<ImageFrameProvider*>(fertilitySprite)->setImage(this->debugAsset->getAssetId());

	fertilitySprite->setPosition(Vector2(0, 0));
	fertilitySprite->setFlipY(true);
	fertilitySprite->setSize(Vector2(20, 20)); // NOTE: the size is reset to be the size of the map in the script
	fertilitySprite->setSceneLayer(10);
	this->island->scene->addToScene(fertilitySprite);
}

void FoliageSystem::updateVisualization(FoliageBed* pBed){
	if (fertilityBitmap){

		Point2I extent;
		extent.x = this->beds_x;
		extent.y = this->beds_y;

		this->fertilityBitmap = static_cast<ImageFrameProvider*>(this->fertilitySprite)->getProviderTexture().getBitmap();

		U8 * pixels = fertilityBitmap->getWritableBits();

		/*for (S32 i = 0; i<extent.x; i++)
		{
			for (S32 j = 0; j<extent.y; j++)
			{
				U8 value = U8(this->foliageGrid[i][j].getFertility() * 255);
				pixels[i * 4 + j * 4 * extent.x + 0] = U8(pBed->getAvailableNutrient(0) * 255);
				pixels[i * 4 + j * 4 * extent.x + 1] = U8(pBed->getAvailableNutrient(1) * 255);
				pixels[i * 4 + j * 4 * extent.x + 2] = U8(pBed->getAvailableNutrient(3) * 255);
				pixels[i * 4 + j * 4 * extent.x + 3] = 255;
			}
		}*/

		//U8 value = U8(this->foliageGrid[i][j].getAvailableNutrient(0) * 255);
		pixels[pBed->getIndex().x * 4 + pBed->getIndex().y * 4 * extent.x + 0] = U8(pBed->getAvailableNutrient(0) * 255);
		pixels[pBed->getIndex().x * 4 + pBed->getIndex().y * 4 * extent.x + 1] = U8(pBed->getAvailableNutrient(1) * 255);
		pixels[pBed->getIndex().x * 4 + pBed->getIndex().y * 4 * extent.x + 2] = U8(pBed->getAvailableNutrient(3) * 255);
		pixels[pBed->getIndex().x * 4 + pBed->getIndex().y * 4 * extent.x + 3] = 255;

		this->debugHandle->set(TextureManager::getUniqueTextureKey(), fertilityBitmap, TextureHandle::TextureHandleType::BitmapKeepTexture);

		//this->debugAsset->setAssetInternal(true);
		this->debugAsset->setImageTexture(*this->debugHandle);
		//AssetDatabase.addPrivateAsset(this->debugAsset);

		static_cast<ImageFrameProvider*>(this->fertilitySprite)->setImage(this->debugAsset->getAssetId());
	}
}


//--------------
// Foliage Bed
//--------------

Point4F FoliageBed::getBounds(){
	F32 lowX = ((F32(this->index.x) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) - (this->bedFoliageSystem->getIsland()->area);
	F32 lowY = ((F32(this->index.y) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) - (this->bedFoliageSystem->getIsland()->area);

	F32 highX = ((F32(this->index.x) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) + (2*this->bedEdgeLength) - (this->bedFoliageSystem->getIsland()->area);
	F32 highY = ((F32(this->index.y) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) + (2*this->bedEdgeLength) - (this->bedFoliageSystem->getIsland()->area);

	return Point4F(lowX, lowY, highX, highY);
}

Point2F FoliageBed::getMapLocation(){
	F32 x = ((F32(this->index.x) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) + (this->bedEdgeLength) - (this->bedFoliageSystem->getIsland()->area);
	F32 y = ((F32(this->index.y) * this->bedFoliageSystem->getIsland()->area * 2) / (this->bedFoliageSystem->getGridDimension())) + (this->bedEdgeLength) - (this->bedFoliageSystem->getIsland()->area);

	return Point2F(x, y);
}


void FoliageBed::addPlantsToScene(){
	// Add the composite sprite to the scene
	char* managerPosition = Con::getReturnBuffer(32);
	char bedIndexStr[32];

	dSprintf(managerPosition, 32, "%f %f", this->getMapLocation().x, this->getMapLocation().y);
	dSprintf(bedIndexStr, 32, "%i %i", this->index.x, this->index.y);

	const char* plantID = Con::executef(3, "createPlantManager", managerPosition, bedIndexStr);

	SimObject* plantManager = Sim::findObject(plantID);

	this->setPlantManager(plantManager);

	// Place objects in the scene at plant positions
	for (S32 k = 0; k < this->plants.size(); ++k){
		char position[32];
		char color[256];
		char frame[32];

		dSprintf(position, 32, "%f %f", this->plants[k].getPosition().x, this->plants[k].getPosition().y);
		dSprintf(color, 256, "%f %f %f", this->plants[k].getNutrient(0) * 255, this->plants[k].getNutrient(1) * 255, this->plants[k].getNutrient(3) * 255);
		dSprintf(frame, 32, "%i", this->plants[k].getStage());

		//Con::printf("Pos: %s, Bed: %s, Plant: %s", position, bedIndex, plantIndex);
		this->plants[k].setIndex(dAtoi(Con::executef(this->plantManager, 4, "createPlant", position, color, frame)));
	}
}

void FoliageBed::removePlantsFromScene(){
	// Remove plants
	if (this->plantManager){
		char* plantManagerId = Con::getReturnBuffer(32);
		dSprintf(plantManagerId, 32, "%s", this->getPlantManager()->getIdString());
		Con::executef(2, "removePlantManager", plantManagerId);

		this->plantManager = NULL;
	}
	
}

void FoliageBed::addPlant(Plant* pPlant){
	pPlant->setFoliageBed(this);
	this->plants.push_back(pPlant);
}

void FoliageBed::removePlant(U32 pIndex){
	for (U32 i = 0; i < (U32)this->plants.size(); i++){
		if (this->plants[i].getIndex() == pIndex){

			// add the nutrients stored in the plant back to the bed
			for (U32 j = 0; j < numberOfNutrients; ++j){
				this->availableNutrients[j] += this->plants[i].getNutrient(j);
			}
			
			this->plants.erase_fast(i);
			
			return;
		}
	}
}

Plant FoliageBed::getPlant(U32 pIndex){
	for (U32 i = 0; i < (U32)this->plants.size(); ++i){
		if (this->plants[i].getIndex() == pIndex){
			return this->plants[i];
		}
	}

	return NULL;
}

void FoliageBed::updateBed(){
	if (this->fertility == 0){
		return;
	}
	
	for (U32 i = 0; i < (U32)this->plants.size(); ++i){
		this->plants[i].updatePlant();
	}
}


//-----------
// Plants
//-----------

// NOTE: this constructor is used for creating the plants associated with seeds
Plant::Plant(){
	this->x = 0;
	this->y = 0;
	this->index = 0;
	this->stage = 0;
	this->birthTime = 0;
	this->lifespan = 0;
	this->potentialChildren = mRandI(childrenMin, childrenMax);
	this->childNumber = 0;
	this->bed = NULL;

	// NOTE: The nutrients are set in the seed code
}

Plant::Plant(Plant* pPlant){
	if (pPlant){
		x = pPlant->x;
		y = pPlant->y;
		index = pPlant->index;
		birthTime = pPlant->birthTime;
		lifespan = pPlant->lifespan;
		seed = pPlant->seed;
		potentialChildren = pPlant->potentialChildren;
		childNumber = pPlant->childNumber;
		stage = pPlant->stage;
		bed = pPlant->bed;

		this->setInitialNutrients(pPlant);
		this->updateNutrients();
		
	}
}

Plant::Plant(Plant* pPlant, Point2F spawnPoint){
	if (pPlant){
		x = spawnPoint.x;
		y = spawnPoint.y;
		index = pPlant->index;
		birthTime = pPlant->bed->getFoliageSystem()->getFoliageSimTime();
		lifespan = mRandI(lifespanMin, lifespanMax);
		//seed = pPlant->seed;
		potentialChildren = mRandI(childrenMin, childrenMax);
		childNumber = 0;
		stage = 0;
		bed = pPlant->bed->getFoliageSystem()->getBed(spawnPoint);

		// Set up the nutrient composition of the 
		// new plant based on its parent's composition
		this->setInitialNutrients(pPlant);
		this->updateNutrients();
	}
}

// NOTE: This is the constructor used when adding plants initially.
Plant::Plant(F32 x, F32 y, U32 pIndex, FoliageBed* pBed){
	this->x = x;
	this->y = y;
	this->index = pIndex;
	this->bed = pBed;
	this->birthTime = pBed->getFoliageSystem()->getFoliageSimTime();
	this->lifespan = mRandI(lifespanMin, lifespanMax);
	this->potentialChildren = mRandI(childrenMin, childrenMax);
	this->childNumber = 0;
	this->stage = 0;

	// Set inital nutrient levels
	this->setInitialNutrients();
}

Plant::~Plant(){
	//Con::printf("Plant removed %i", index);
	index = -1;
}

void Plant::updatePlant(){
	U8 currentStage = (U8) mFloor((F32) (this->bed->getFoliageSystem()->getFoliageSimTime() - this->birthTime) / (F32) (lifespan / 5));

	if (currentStage > 4){
		// remove the sprite (if it's rendered)
		if (this->bed->getPlantManager() != NULL){
			char name[32];
			dSprintf(name, 32, "%i", this->getIndex());
			Con::executef(this->bed->getPlantManager(), 2, "removePlant", name);
		}

		this->bed->removePlant(this->getIndex());

		return;
		
	}
	else if (currentStage > this->stage){
		this->stage = currentStage;
		
		// if the plant is visible, call the update script
		if (this->bed->getPlantManager() != NULL){
			char name[32];
			char stageArg[32];

			dSprintf(name, 32, "%i", this->index);
			dSprintf(stageArg, 32, "%i", currentStage);

			Con::executef(this->bed->getPlantManager(), 3, "updatePlantStage", name, stageArg);
		}
	}

	// Spawn children plants if possible
	U32 currentChild = (U8) mFloor((F32) (this->bed->getFoliageSystem()->getFoliageSimTime() - this->birthTime) / (F32) (lifespan / this->potentialChildren));
	if (this->childNumber < currentChild){
		this->childNumber = currentChild;

		// Select a spawn point and create a plant if the nutrients are available
		U32 spawnSize = 10;
		Point2F spawnPoint = Point2F(mRandF(this->getPosition().x - spawnSize, this->getPosition().x + spawnSize), mRandF(this->getPosition().y - spawnSize, this->getPosition().y + spawnSize));
		
		if (this->nutrientsAreAvailable(this->bed->getFoliageSystem()->getBed(spawnPoint))){
			Plant* newPlant = new Plant(this, spawnPoint);
			FoliageBed* tempBed = this->bed->getFoliageSystem()->getBed(spawnPoint);

			// Add the sprite to the scene
			if (tempBed->getPlantManager()){
				char posStr[32];
				char color[256];
				dSprintf(posStr, 32, "%f %f", spawnPoint.x, spawnPoint.y);
				dSprintf(color, 256, "%f %f %f", newPlant->getNutrient(0) * 255, newPlant->getNutrient(1) * 255, newPlant->getNutrient(3) * 255);
				newPlant->setIndex(dAtoi(Con::executef(tempBed->getPlantManager(), 3, "createPlant", posStr, color)));
			}

			tempBed->addPlant(newPlant);

			/*for (U32 i = 0; i < numberOfNutrients; ++i){
				tempBed->setAvailableNutrient(tempBed->getAvailableNutrient(i) - newPlant->getNutrient(i), i);
				
			}*/
		}
	}
}

void Plant::setInitialNutrients(Plant* pParent){

	// Initially, make an exact copy of the parent plant
	for (U32 i = 0; i < numberOfNutrients; ++i){
		this->nutrientComposition[i] = pParent->nutrientComposition[i];
	}
		
}

// Set the nutrient levels when plants are being created without a parent
void Plant::setInitialNutrients(){
	
	for (U32 i = 0; i < numberOfNutrients; ++i){
		F32 fraction = mRandF(0.1f, 0.25f);
		this->nutrientComposition[i] = this->bed->getAvailableNutrient(i) * fraction;
		this->bed->setAvailableNutrient(this->bed->getAvailableNutrient(i) - this->nutrientComposition[i], i);
		//Con::printf("%i: %f", i, this->bed->getAvailableNutrient(i));
	}
}

bool Plant::updateNutrients(){
	F32 fraction = 0.1f;

	for (U32 i = 0; i < numberOfNutrients; ++i){

		if (this->bed->getAvailableNutrient(i) > this->nutrientComposition[i]){
			// If there is an abundance of a nutrient, give it more than it asks for
			if (this->bed->getAvailableNutrient(i) > (this->nutrientComposition[i] * fraction) + this->nutrientComposition[i]){
				this->nutrientComposition[i] += this->nutrientComposition[i] * fraction;
				this->bed->setAvailableNutrient(this->bed->getAvailableNutrient(i) - this->nutrientComposition[i], i);
			}
			else{
				// leave nutrient levels as they are
			}
		}
		// There is less available than is requested, but only by a small amount, so give it a bit less than it asked for
		else if (this->bed->getAvailableNutrient(i) > this->nutrientComposition[i] - (this->nutrientComposition[i] * fraction) && this->nutrientComposition[i] - (this->nutrientComposition[i] * fraction) > 0){
			this->nutrientComposition[i] -= this->nutrientComposition[i] * fraction;
			this->bed->setAvailableNutrient(this->bed->getAvailableNutrient(i) - this->nutrientComposition[i], i);
		}
		// There is not enough for the plant to grow here
		else{
			return false;
		}

		
	}

	return true;
}

bool Plant::nutrientsAreAvailable(FoliageBed* pBed){
	for (U32 i = 0; i < numberOfNutrients; ++i){
		if (this->getNutrient(i) > pBed->getAvailableNutrient(i)){
			return false;
		}


		/*if (pBed->getAvailableNutrient(i) < 0.0009 || this->getNutrient(i) < 0.0009){
			return false;
		}*/
	}

	return true;
}

void Plant::copyGeneticsTo(Plant* pPlant){
	// Copy over genetic information
	//TODO: remove this if seed is not used, since the nutrient info is set at spawn-time.
	pPlant->seed = this->seed;
	
}

//---------
// SEED
//---------
Seed::~Seed(){
	if (this->plant){
		delete this->plant;
	}
}

void Seed::setPlant(Point2I pBedIndex, U32 pParentIndex){
	this->plant = new Plant();

	AssertFatal(this->mFoliageSystem, "Seed's Foliage System has not been set.")
	FoliageBed* tempBed = this->mFoliageSystem->getBed(pBedIndex);
	

	tempBed->getPlant(pParentIndex).copyGeneticsTo(this->plant);
	this->plant->setInitialNutrients(&tempBed->getPlant(pParentIndex));
}

bool Seed::spawnPlant(Point2F pPosition){
	FoliageBed* tempBed = this->mFoliageSystem->getBed(pPosition);

	this->plant->setFoliageBed(tempBed);
	if (this->plant->updateNutrients()){

		this->plant->setPosition(pPosition);
		this->plant->setBirthTime(this->mFoliageSystem->getFoliageSimTime());
		this->plant->setLifeSpan(mRandI(lifespanMin, lifespanMax));
		this->plant->setStage(0);
		

		// Add the sprite to the scene
		if (tempBed->getPlantManager()){
			char posStr[32];
			//char color[32];
			dSprintf(posStr, 32, "%f %f", pPosition.x, pPosition.y);
			//dSprintf(color, 32, "%f %f %f", this->plant->getNutrient(0)*255, this->plant->getNutrient(1)*255, this->plant->getNutrient(3)*255);
			this->plant->setIndex(dAtoi(Con::executef(tempBed->getPlantManager(), 2, "createPlant", posStr)));
		}



		tempBed->addPlant(this->plant);

		return true;
	}
	else{
		return false;
	}
	
}

IMPLEMENT_CONOBJECT(Seed);
IMPLEMENT_CONOBJECT(FoliageSystem);

