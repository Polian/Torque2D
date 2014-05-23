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
}

bool Island::onAdd()
{
	// Fail if the parent fails.  
	if (!Parent::onAdd())
		return false;

	// Initializations
	S32 i = 0, j = 0;

	hexEdgeLength = 10;
	hexOffset = F32(hexEdgeLength*0.5) /*/ (1 / mSqrt(2))*/;
	F32 transAmount = 3.75;
	area = 500;
	seed = mRandI(0, S32_MAX); // the seed for our noise function
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
				points[i][j].x = points[i][j].x + mRandF(-transAmount, transAmount);
				points[i][j].y = points[i][j].y + mRandF(-transAmount, transAmount);
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

	
	
	

	// write cell info to file
	Con::printf("%i, %i", cells[0].size(), cells[1].size());
	std::ofstream outFile;
	outFile.open("modules/LightModule/1/MapData/Cells.csv");
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
			//Con::printf("%i", cells[i][j].adjacent[0]->center.x);
			outFile << cells[i][j].verts[0].x << "," << cells[i][j].verts[0].y << ",";
			outFile << cells[i][j].verts[1].x << "," << cells[i][j].verts[1].y << ",";
			outFile << cells[i][j].verts[2].x << "," << cells[i][j].verts[2].y << ",";
			outFile << cells[i][j].verts[3].x << "," << cells[i][j].verts[3].y << ",";
			outFile << cells[i][j].verts[4].x << "," << cells[i][j].verts[4].y << ",";
			outFile << cells[i][j].verts[5].x << "," << cells[i][j].verts[5].y << ",";
			if (cells[i][j].adjacent[0] != NULL){
				outFile << cells[i][j].adjacent[0]->center.x << "," << cells[i][j].adjacent[0]->center.y << ",";
				outFile << cells[i][j].adjacent[1]->center.x << "," << cells[i][j].adjacent[1]->center.y << ",";
				outFile << cells[i][j].adjacent[2]->center.x << "," << cells[i][j].adjacent[2]->center.y << ",";
				outFile << cells[i][j].adjacent[3]->center.x << "," << cells[i][j].adjacent[3]->center.y << ",";
				outFile << cells[i][j].adjacent[4]->center.x << "," << cells[i][j].adjacent[4]->center.y << ",";
				outFile << cells[i][j].adjacent[5]->center.x << "," << cells[i][j].adjacent[5]->center.y << ",";
			}
			else{
				outFile << "0,0,";
				outFile << "0,0,";
				outFile << "0,0,";
				outFile << "0,0,";
				outFile << "0,0,";
				outFile << "0,0,";
			}
			
			outFile << cells[i][j].center.x << "," << cells[i][j].center.y << ",";
			outFile << cells[i][j].biome << ",";
			outFile << cells[i][j].center.elevation << "\n";
		}
	}

	outFile.close();
	

	return true;
}

void Island::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// Add my fields here.  
	//addField("Vertices", TypeS32, Offset(vertices, Island), "");
	//addField("Corners", TypeString, Offset(corners, Island), "List of corners.");
	//addField("Corners", , Offset(corners, Island), "List of corners.");
}


void Island::setPoint(U32 i, U32 j){

	//offset vert placement every other row
	if (i%2 == 0){
		points[i].push_back(HexVert(j*hexEdgeLength, i*hexEdgeLength, j, i, genNoise(j*hexEdgeLength, i*hexEdgeLength), genLandNoise(j*hexEdgeLength, i*hexEdgeLength)));
	}
	else{
		points[i].push_back(HexVert(j*hexEdgeLength+hexOffset, i*hexEdgeLength, j, i, genNoise(j*hexEdgeLength+hexOffset, i*hexEdgeLength), genLandNoise(j*hexEdgeLength+hexOffset, i*hexEdgeLength)));
	}
}

F32 Island::genNoise(F32 x, F32 y){
	
	F32 r = 0;
	F32 noise = 0;

	//offset x and y location
	x = x - area;
	y = y - area;

	//generate noise
	noise = scaled_octave_noise_3d(8.0, 0.0, 0.005f, 0.0, 1.0, x, y, (const float)(seed));

	//make noise more "island-like"
	r = mSqrt(x * x + y * y) / (area - (area*0.1));
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

	//offset x and y location
	x = x - area;
	y = y - area;

	//generate noise
	noise = scaled_octave_noise_3d(8.0, 0.0, 0.015f, 0.0, 1.0, x, y, (const float) (seed));

	// generate biome noise to be a smaller island
	r = mSqrt(x * x + y * y) / (area - (area*0.1));
	if (noise * exp(-mPow(r / 4, 2)) - mPow(r, 2) < 0){
		noise = 0;
	}
	else{
		noise = mPow(noise * exp(-mPow(r / 4, 2)) - mPow(r, 2), 2);
	}

	return noise;
}

void Island::assignCell(U32 i, U32 j){
	HexCell cell;


	// set up verts
	if (i%2 == 0){
		cell.init(j, i, points[i][j * 3]);
		cell.setVertsEven(points);
		//cell.setAdjEven(points);
	}
	else{
		cell.init(j, i, points[i][j * 3 + 1]);
		cell.setVertsOdd(points);
		//cell.setAdjOdd(points);
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
	F32 beachThreshold = 0.025f;

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

	F32 forestThreshold = 0.035f;
	F32 beachThreshold = 0.03f;

	if (cell->center.landNoise > forestThreshold){
		cell->biome = 4;
	}
	else{
		cell->biome = 3;
	}
}

IMPLEMENT_CONOBJECT(Island);