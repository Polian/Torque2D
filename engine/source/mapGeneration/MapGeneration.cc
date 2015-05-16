#ifndef _MAP_GENERATION_H_
#include "MapGeneration.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

// Script bindings.
#include "MapGeneration_ScriptBinding.h"

Island::Island()
{
	//vertices = NULL;
	landCellCount = 0;
}


bool Island::onAdd()
{
	// Fail if the parent fails.  
	if (!Parent::onAdd())
		return false;

	// Initializations
	S32 i = 0, j = 0;

	hexEdgeLength = 20;
	hexOffset = F32(hexEdgeLength*0.5) /*/ (1 / mSqrt(2))*/;
	F32 transAmount = hexEdgeLength * 0.375f;
	area = 2000;

	gRandGen.setGlobalRandSeed(1627839297);
	seed = gRandGen.getSeed(); // the seed from which the island is generated
	//seed = mRandI(0, S32_MAX);
	Con::printf("seed: %i", seed);

	// generate vertex postions
	U32 hexNum = U32(mFloor((area * 2) / hexEdgeLength));

	for (i = 0; i < S32(hexNum); ++i){
		points.push_back(Vector<HexVert>());
		for (j = 0; j < S32(hexNum); ++j){
			setPoint(i, j);
		}

	}

	//Deform vert positions
	for (i = 0; i < points.size(); ++i){
		for (j = 0; j < points.at(i).size(); ++j){
			if (!(points[i][j].yIndex % 2 == 0 && points[i][j].xIndex % 3 == 0) && !(points[i][j].yIndex % 2 != 0 && points[i][j].xIndex % 3 == 1)){
				points[i][j].x = points[i][j].x + gRandGen.randRangeF(-transAmount, transAmount);
				points[i][j].y = points[i][j].y + gRandGen.randRangeF(-transAmount, transAmount);
			}
		}
	}

	//create cells
	for (i = 0; i < points.size(); ++i){

		cells.push_back(Vector<HexCell>());
		if (i % 2 == 0){
			for (j = 0; j < mCeil(((area*2)/hexEdgeLength)/3); ++j){
				assignCell(i, j);
			}
		}
		else{
			for (j = 0; j < mFloor(((area * 2) / hexEdgeLength) / 3); ++j){
				assignCell(i, j);
			}
		}
	}

	// establish adjacencies and basic land/water biome assignment
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
				assignCellAdj(i, j);
		}
	}

	// assign land biomes to all interior cells
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
			if (cells[i][j].center.elevation > 0){
				assignLandBiome(&cells[i][j]);
			}
		}
	}

	//fill ocean and make the beaches
	oceanFill(&cells[3][3]);

	
	//plant Trees
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
			if (cells[i][j].biome == 4){
				plantTrees(&cells[i][j]);
			}
		}
	}
	

	// write cell info to file
	//std::ofstream outFile;
	//outFile.open("modules/LightModule/1/MapData/Cells.csv");
	//for (i = 0; i < cells.size(); ++i){
	//	for (j = 0; j < cells[i].size(); ++j){
	//		//Con::printf("%i", cells[i][j].adjacent[0]->center.x);
	//		outFile << cells[i][j].verts[0].x << "," << cells[i][j].verts[0].y << ",";
	//		outFile << cells[i][j].verts[1].x << "," << cells[i][j].verts[1].y << ",";
	//		outFile << cells[i][j].verts[2].x << "," << cells[i][j].verts[2].y << ",";
	//		outFile << cells[i][j].verts[3].x << "," << cells[i][j].verts[3].y << ",";
	//		outFile << cells[i][j].verts[4].x << "," << cells[i][j].verts[4].y << ",";
	//		outFile << cells[i][j].verts[5].x << "," << cells[i][j].verts[5].y << ",";
	//		/*outFile << cells[i][j].verts[0].x << " " << cells[i][j].verts[0].y << " ";
	//		outFile << cells[i][j].verts[1].x << " " << cells[i][j].verts[1].y << " ";
	//		outFile << cells[i][j].verts[2].x << " " << cells[i][j].verts[2].y << " ";
	//		outFile << cells[i][j].verts[3].x << " " << cells[i][j].verts[3].y << " ";
	//		outFile << cells[i][j].verts[4].x << " " << cells[i][j].verts[4].y << " ";
	//		outFile << cells[i][j].verts[5].x << " " << cells[i][j].verts[5].y << ",";*/
	//		if (cells[i][j].adjacent[0] != NULL){
	//			outFile << cells[i][j].adjacent[0]->center.x << "," << cells[i][j].adjacent[0]->center.y << ",";
	//			outFile << cells[i][j].adjacent[1]->center.x << "," << cells[i][j].adjacent[1]->center.y << ",";
	//			outFile << cells[i][j].adjacent[2]->center.x << "," << cells[i][j].adjacent[2]->center.y << ",";
	//			outFile << cells[i][j].adjacent[3]->center.x << "," << cells[i][j].adjacent[3]->center.y << ",";
	//			outFile << cells[i][j].adjacent[4]->center.x << "," << cells[i][j].adjacent[4]->center.y << ",";
	//			outFile << cells[i][j].adjacent[5]->center.x << "," << cells[i][j].adjacent[5]->center.y << ",";
	//		}
	//		else{
	//			outFile << "0,0,";
	//			outFile << "0,0,";
	//			outFile << "0,0,";
	//			outFile << "0,0,";
	//			outFile << "0,0,";
	//			outFile << "0,0,";
	//		}
	//		
	//		outFile << cells[i][j].center.x << "," << cells[i][j].center.y << ",";
	//		outFile << cells[i][j].biome << ",";
	//		outFile << cells[i][j].center.elevation << "\n";
	//	}
	//}

	//outFile.close();

	////write trees to file
	//outFile.open("modules/LightModule/1/MapData/Trees.csv");
	//for (i = 0; i < cells.size(); ++i){
	//	for (j = 0; j < cells[i].size(); ++j){
	//		for (U32 k = 0; k < U32(cells[i][j].trees.size()); ++k){
	//			outFile << cells[i][j].trees[k].x << "," << cells[i][j].trees[k].y << "," << cells[i][j].trees[k].radius << "\n";
	//		}
	//		cells[i][j].trees.clear();
	//	}
	//}
	//outFile.close();
	Con::printf("Island Generated");
	return true;

}

void Island::onRemove(){
	
}

void Island::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// Add my fields here.  
	addProtectedField("scene", TypeSimObjectPtr, Offset(scene, Island), &setScene, &defaultProtectedGetFn, &writeScene, "");
	addProtectedField("map", TypeSimObjectPtr, Offset(map, Island), &setMap, &defaultProtectedGetFn, &writeMap, "");
}


void Island::setPoint(U32 i, U32 j){
	F32 offset = F32(area);
	//offset vert placement every other row
	if (i%2 == 0){
		points[i].push_back(HexVert(j*hexEdgeLength-offset, i*hexEdgeLength-offset, j, i, genNoise(j*hexEdgeLength, i*hexEdgeLength), genLandNoise(j*hexEdgeLength, i*hexEdgeLength)));
	}
	else{
		points[i].push_back(HexVert(j*hexEdgeLength+hexOffset - offset, i*hexEdgeLength - offset, j, i, genNoise(j*hexEdgeLength+hexOffset, i*hexEdgeLength), genLandNoise(j*hexEdgeLength+hexOffset, i*hexEdgeLength)));
	}
}

F32 Island::genNoise(F32 x, F32 y){
	
	F32 r = 0;
	F32 noise = 0;
	

	//offset x and y location
	x = x - area;
	y = y - area;

	//generate noise
	//Scale for area=500 and edgeLenght=10 is 0.005
	noise = scaled_octave_noise_3d(8.0f, 0.0f, 0.001f, 0.0f, 1.0f, x, y, (const float) (seed));

	//make noise more "island-like"
	r = mSqrt(x * x + y * y) / (area - (area*0.1f));
	if (noise * exp(-mPow(r / 4, 2)) - mPow(r, 2) < 0){
		noise = 0;
	}
	else{
		noise = mPow(noise * exp(-mPow(r / 4, 2)) - mPow(r, 2), 2);
	}

	return noise;
}

F32 Island::genLandNoise(F32 x, F32 y){
	
	F32 r = 0;
	F32 noise = 0;
	F32 islandNoise = 0;

	islandNoise = genNoise(x, y);

	//offset x and y location
	x = x - area;
	y = y - area;

	//generate noise
	//Scale for area=500 and edgeLenght=10 is 0.015
	noise = scaled_octave_noise_3d(8.0f, 0.0f, 0.00325f, 0.0f, 1.0f, x, y, (const float) (seed));
	

	// generate biome noise to be a smaller island
	//r = mSqrt(x * x + y * y) / (area - (area*0.1f));

	noise = (islandNoise + (noise/6)) / 2;

	return noise;
}

void Island::assignCell(U32 i, U32 j){
	HexCell cell;


	// set up verts
	if (i%2 == 0){
		cell.init(j, i, points[i][j * 3]);
		cell.setVertsEven(points);
	}
	else{
		cell.init(j, i, points[i][j * 3 + 1]);
		cell.setVertsOdd(points);
	}
	
	cells[i].push_back(cell);
}

void Island::assignCellAdj(U32 i, U32 j){
	
	if (i%2 == 0){
		cells[i][j].setAdjEven(cells);
	}
	else{
		cells[i][j].setAdjOdd(cells);
	}

	//assign basic biomes (land and water)
	if (cells[i][j].center.elevation > 0){
		cells[i][j].biome = 3;
	}
	else{
		cells[i][j].biome = 0;
	}

	
}

void Island::oceanFill(HexCell* cell){
	Queue<HexCell*> fillQueue;
	Queue<HexCell*> beachQueue;
	HexCell* n;
	F32 beachThreshold = 0.004f;

	fillQueue.enqueue(cell);

	while (fillQueue.size() != 0){
		//dequeue an element from the queue
		n = fillQueue[0];
		fillQueue.dequeue();

		if (n == NULL){
			continue;
		}

		if (n->biome == 0){

			n->biome = 2;

			//add adjacent cells to queue
			for (U32 i = 0; i < 6; ++i){
				fillQueue.enqueue(n->adjacent[i]);
			}
		}
		else if (n->center.elevation > 0){
			n->biome = 1;
			beachQueue.enqueue(n);

		}



	}

	while (beachQueue.size() != 0){

		n = beachQueue[0];
		beachQueue.dequeue();

		for (U32 i = 0; i < 6; ++i){
			if (n->adjacent[i]->biome == 3 && n->center.elevation < beachThreshold){
				n->adjacent[i]->biome = 1;
				beachQueue.enqueue(n->adjacent[i]);
			}
		}
		
	}


}

void Island::assignLandBiome(HexCell* cell){

	F32 forestThreshold = 0.05f;

	if (cell->center.landNoise > forestThreshold){
		cell->biome = 4; // forest
	}
	else{
		cell->biome = 3; // field
	}

	cell->landIndex = landCellCount;
	landCellCount++;
}

void Island::plantTrees(HexCell* cell){
	F32 triArea = 0;
	U32 numPoints = 10;
	F32 rad = 5.5f;
	U32 treeCount = 0;
	F32 density = 0;

	//Make sure the random seed matches the island seed
	gRandGen.setSeed(seed);

	//plant trees in each triangle inside the cell
	for (U32 i = 0; i < 6; ++i){
		//find area of triangle and determine the number of trees to plant in the tri
		if (i == 5){
			triArea = 0.5f * getMax(mFabs(cell->verts[i].y - cell->verts[0].y), mFabs(cell->verts[i].y - cell->center.y)) * getMax(mFabs(cell->verts[i].x - cell->verts[0].x), mFabs(cell->verts[i].x - cell->center.x));
			density = ((cell->verts[i].landNoise + cell->verts[0].landNoise + cell->center.landNoise) / 3.0f) * 9.0f;
		}
		else{
			triArea = 0.5f * getMax(mFabs(cell->verts[i].y - cell->verts[i + 1].y), mFabs(cell->verts[i].y - cell->center.y)) * getMax(mFabs(cell->verts[i].x - cell->verts[i + 1].x), mFabs(cell->verts[i].x - cell->center.x));
			density = ((cell->verts[i].landNoise + cell->verts[0].landNoise + cell->center.landNoise) / 3.0f) * 9.0f;
		}

		//make sure the density stays below 1
		if (density > 1){
			density = 1;
		}
		
		numPoints = U32(mFloor(F32(triArea / (M_PI * rad*rad)) * density));
		treeCount = 0;

		//plant the trees
		while (treeCount < numPoints){
			HexVert temp;

			temp = HexVert(
				gRandGen.randRangeF(
					getMin(cell->verts[i].x, getMin(cell->verts[modulo(i + 1, 6)].x, cell->center.x)), 
					getMax(cell->verts[i].x, getMax(cell->verts[modulo(i + 1, 6)].x, cell->center.x))),
				gRandGen.randRangeF(
					getMin(cell->verts[i].y, getMin(cell->verts[modulo(i + 1, 6)].y, cell->center.y)),
					getMax(cell->verts[i].y, getMax(cell->verts[modulo(i + 1, 6)].y, cell->center.y))));

			//check if the random point is within the bounds of the triangle
			F32 vertx[3] = { cell->verts[i].x, cell->verts[modulo(i + 1, 6)].x, cell->center.x};
			F32 verty[3] = { cell->verts[i].y, cell->verts[modulo(i + 1, 6)].y, cell->center.y };

			if (pnpoly(3, vertx, verty, temp.x, temp.y)){
				//make sure the point is not overlapping another
				if (temp.checkPosition(cell->trees, rad) == 0){
					cell->trees.push_back(Tree(temp.x, temp.y, rad));
					treeCount++;
				}

			}
		}
			
	}

}

U32 modulo(S32 a, S32 b) { return a >= 0 ? a % b : (b - mAbs(a%b)) % b; }

F32 Island::checkVert(HexVert p1, HexVert p2, HexVert testVert){

	return (p2.x - p1.x) * (testVert.y - p1.y) - (p2.y - p1.y) * (testVert.x - p1.x);

}

void loadTaml(const char* name, const char* extension){
	//Con::printf("Loading: %s.%s", name, extension);
	Con::executef(3, "loadTaml", name, extension);
}

// Algorithm for checking if a given point is inside a polygon
// Source: http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
S32 pnpoly(S32 nvert, F32 *vertx, F32 *verty, F32 testx, F32 testy)
{
	S32 i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i]>testy) != (verty[j]>testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
			c = !c;
	}
	return c;
}

//find which hex the point x, y is in
Point2I findHex(U32 i, U32 j, F32 x, F32 y, Vector<Vector<HexCell>> cells){
	//check the current cell
	F32 vertx[6] = { cells[i][j].verts[0].x, cells[i][j].verts[1].x, cells[i][j].verts[2].x, cells[i][j].verts[3].x, cells[i][j].verts[4].x, cells[i][j].verts[5].x };
	F32 verty[6] = { cells[i][j].verts[0].y, cells[i][j].verts[1].y, cells[i][j].verts[2].y, cells[i][j].verts[3].y, cells[i][j].verts[4].y, cells[i][j].verts[5].y };

	if (pnpoly(6, vertx, verty, x, y)){
		return Point2I(i, j);
	}

	//not in current, so check adjacent
	for (U32 index = 0; index < 6; ++index){
		if (cells[i][j].adjacent[index] != NULL){
			vertx[0] = cells[i][j].adjacent[index]->verts[0].x;
			vertx[1] = cells[i][j].adjacent[index]->verts[1].x;
			vertx[2] = cells[i][j].adjacent[index]->verts[2].x;
			vertx[3] = cells[i][j].adjacent[index]->verts[3].x;
			vertx[4] = cells[i][j].adjacent[index]->verts[4].x;
			vertx[5] = cells[i][j].adjacent[index]->verts[5].x;

			verty[0] = cells[i][j].adjacent[index]->verts[0].y;
			verty[1] = cells[i][j].adjacent[index]->verts[1].y;
			verty[2] = cells[i][j].adjacent[index]->verts[2].y;
			verty[3] = cells[i][j].adjacent[index]->verts[3].y;
			verty[4] = cells[i][j].adjacent[index]->verts[4].y;
			verty[5] = cells[i][j].adjacent[index]->verts[5].y;

			if (pnpoly(6, vertx, verty, x, y)){
				return Point2I(cells[i][j].adjacent[index]->center.yCell, cells[i][j].adjacent[index]->center.xCell);
			}
		}
	}

	//not in adjacent, so check adjacent to adjacent
	for (U32 index = 0; index < 6; ++index){
		if (cells[i][j].adjacent[index] != NULL){
			for (U32 index2 = 0; index2 < 6; ++index2){
				if (cells[i][j].adjacent[index]->adjacent[index2] != NULL){
					vertx[0] = cells[i][j].adjacent[index]->adjacent[index2]->verts[0].x;
					vertx[1] = cells[i][j].adjacent[index]->adjacent[index2]->verts[1].x;
					vertx[2] = cells[i][j].adjacent[index]->adjacent[index2]->verts[2].x;
					vertx[3] = cells[i][j].adjacent[index]->adjacent[index2]->verts[3].x;
					vertx[4] = cells[i][j].adjacent[index]->adjacent[index2]->verts[4].x;
					vertx[5] = cells[i][j].adjacent[index]->adjacent[index2]->verts[5].x;

					verty[0] = cells[i][j].adjacent[index]->adjacent[index2]->verts[0].y;
					verty[1] = cells[i][j].adjacent[index]->adjacent[index2]->verts[1].y;
					verty[2] = cells[i][j].adjacent[index]->adjacent[index2]->verts[2].y;
					verty[3] = cells[i][j].adjacent[index]->adjacent[index2]->verts[3].y;
					verty[4] = cells[i][j].adjacent[index]->adjacent[index2]->verts[4].y;
					verty[5] = cells[i][j].adjacent[index]->adjacent[index2]->verts[5].y;

					if (pnpoly(6, vertx, verty, x, y)){
						return Point2I(cells[i][j].adjacent[index]->adjacent[index2]->center.yCell, cells[i][j].adjacent[index]->adjacent[index2]->center.xCell);
					}
				}
			}
		}
		
	}
	
	//something went wrong so just return a cell that consists of ocean
	return Point2I(1, 1);
}



void Island::renderMap(){
	// Specify an output image size
	Point2I extent;
	extent.x = 256;
	extent.y = 256;

	GBitmap* bitmap = new GBitmap(extent.x, extent.y, false, GBitmap::BitmapFormat::RGBA);
	U8 * pixels = bitmap->getWritableBits();

	for (S32 i = 0; i<extent.x; i++)
	{
		for (S32 j = 0; j<extent.y; j++)
		{
			U32 a, b, biome;
			F32 xPos = 0, yPos = 0;
			U8 red, green, blue;
			Point2I cellIndex;

			//calculate the position of the pixel
			xPos = F32(i)*(F32(area) * 2) / F32(extent.x);
			xPos += F32(area) / F32(extent.x);
			yPos = F32(j)*(F32(area) * 2) / F32(extent.y);
			yPos += F32(area) / F32(extent.y);

			//which cell are we dealing with?
			a = U32(2 * mFloor((yPos) / (2 * hexEdgeLength)));
			b = U32(mFloor((xPos) / (2 * hexEdgeLength)) - mRound(mFloor((xPos) / (2 * hexEdgeLength)) / 3));

			//Get the coordinates of the cell that the pixel is in
			cellIndex = findHex(a, b, xPos - F32(area), yPos - F32(area), cells);

			//only calculate for cells on the interior of the image
			if (a < U32(mFloor((2 * area) / (hexEdgeLength)) - 1) && a > 0 && b < U32(mFloor((2 * area) / (hexEdgeLength * 3)) - 1) && b > 0){
				biome = cells.at(cellIndex.x).at(cellIndex.y).biome;
			}
			else{
				biome = 2;
			}

			

			if (biome == 0) { //Lake
				red = 46; green = 94; blue = 110;
			}
			else if (biome == 1) { //Shore
				red = 166; green = 155; blue = 121;
			}
			else if (biome == 2) { //Ocean
				red = 33; green = 68; blue = 82;
			}
			else if (biome == 3) { //Field
				red = 53; green = 145; blue = 35;
			}
			else if (biome == 4) { //Forest
				red = 54; green = 102; blue = 21;
			}

			pixels[i * 4 + j * 4 * extent.x + 0] = red;
			pixels[i * 4 + j * 4 * extent.x + 1] = green;
			pixels[i * 4 + j * 4 * extent.x + 2] = blue;
			pixels[i * 4 + j * 4 * extent.x + 3] = 255;
		}
	}

	TextureHandle* handle = new TextureHandle();
	handle->set(TextureManager::getUniqueTextureKey(), bitmap, TextureHandle::TextureHandleType::BitmapKeepTexture);


	ImageAsset* asset = new ImageAsset();
	//asset->setAssetInternal(true);
	asset->setImageTexture(*handle);
	AssetDatabase.addPrivateAsset(asset);


	static_cast<ImageFrameProvider*>(map)->setImage(asset->getAssetId());

	return;
}

//function that returns the biome of the given point
U32 Island::getBiome(const char* x, const char* y){
	Point2I cellIndex;
	U32 i, j;

	//approximately which cell are we dealing with?
	i = U32(2 * mFloor((dAtof(y)+area) / (2 * hexEdgeLength)));
	j = U32(mFloor((dAtof(x)+area) / (2 * hexEdgeLength)) - mRound(mFloor((dAtof(x) + area) / (2 * hexEdgeLength)) / 3));
	
	//Get the coordinates of the cell that the pixel is in
	cellIndex = findHex(i, j, dAtof(x), dAtof(y), cells);

	return cells[cellIndex.x][cellIndex.y].biome;
}

HexCell* Island::getCell(Point2F pos){
	Point2I cellIndex;
	U32 i, j;

	//approximately which cell are we dealing with?
	i = U32(2 * mFloor((pos.y + area) / (2 * hexEdgeLength)));
	j = U32(mFloor((pos.x + area) / (2 * hexEdgeLength)) - mRound(mFloor((pos.x + area) / (2 * hexEdgeLength)) / 3));

	//Get the coordinates of the cell that the pixel is in
	cellIndex = findHex(i, j, pos.x, pos.y, cells);

	return &cells[cellIndex.x][cellIndex.y];
}

IMPLEMENT_CONOBJECT(Island);