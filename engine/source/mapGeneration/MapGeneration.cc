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

	hexEdgeLength = 20;
	hexOffset = F32(hexEdgeLength*0.5) /*/ (1 / mSqrt(2))*/;
	F32 transAmount = hexEdgeLength * 0.375f;
	area = 2000;

	/*gRandGen.setGlobalRandSeed(19559375);
	seed = gRandGen.getSeed();*/ // the seed for our noise function
	seed = mRandI(0, S32_MAX);
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

	
	//plant Trees
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
			if (cells[i][j].biome == 4){
				plantTrees(&cells[i][j]);
			}
		}
	}
	

	// write cell info to file
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
			/*outFile << cells[i][j].verts[0].x << " " << cells[i][j].verts[0].y << " ";
			outFile << cells[i][j].verts[1].x << " " << cells[i][j].verts[1].y << " ";
			outFile << cells[i][j].verts[2].x << " " << cells[i][j].verts[2].y << " ";
			outFile << cells[i][j].verts[3].x << " " << cells[i][j].verts[3].y << " ";
			outFile << cells[i][j].verts[4].x << " " << cells[i][j].verts[4].y << " ";
			outFile << cells[i][j].verts[5].x << " " << cells[i][j].verts[5].y << ",";*/
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

	//write trees to file
	outFile.open("modules/LightModule/1/MapData/Trees.csv");
	for (i = 0; i < cells.size(); ++i){
		for (j = 0; j < cells[i].size(); ++j){
			for (U32 k = 0; k < U32(cells[i][j].trees.size()); ++k){
				outFile << cells[i][j].trees[k].x << "," << cells[i][j].trees[k].y << "," << cells[i][j].trees[k].radius << "\n";
			}
		}
	}
	outFile.close();
	Con::printf("Island Generated");
	return true;

}

void Island::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// Add my fields here.  
	addProtectedField("scene", TypeSimObjectPtr, Offset(scene, Island), &setScene, &defaultProtectedGetFn, &writeScene, "");
	//addField("chunkManager", TypeSimObjectPtr, Offset(chunkManager, Island), "");
	//addField("Corners", TypeString, Offset(corners, Island), "List of corners.");
	//addField("Corners", , Offset(corners, Island), "List of corners.");
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
}

void Island::plantTrees(HexCell* cell){
	F32 triArea = 0;
	U32 numPoints = 10;
	F32 rad = 6.0f;
	U32 treeCount = 0;
	F32 density = 0;

	//plant trees in each triangle inside the cell
	for (U32 i = 0; i < 6; ++i){
		//find area of triangle and determine the number of trees to plant in the tri
		if (i == 5){
			triArea = 0.5f * getMax(mFabs(cell->verts[i].y - cell->verts[0].y), mFabs(cell->verts[i].y - cell->center.y)) * getMax(mFabs(cell->verts[i].x - cell->verts[0].x), mFabs(cell->verts[i].x - cell->center.x));
			density = ((cell->verts[i].landNoise + cell->verts[0].landNoise + cell->center.landNoise) / 3.0f) * 8.0f;
		}
		else{
			triArea = 0.5f * getMax(mFabs(cell->verts[i].y - cell->verts[i + 1].y), mFabs(cell->verts[i].y - cell->center.y)) * getMax(mFabs(cell->verts[i].x - cell->verts[i + 1].x), mFabs(cell->verts[i].x - cell->center.x));
			density = ((cell->verts[i].landNoise + cell->verts[0].landNoise + cell->center.landNoise) / 3.0f) * 8.0f;
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
			// check if it is the last triangle in the current hex
			if (i == 5){
				temp = HexVert(
					mRandF(getMin(cell->verts[i].x, getMin(cell->verts[0].x, cell->center.x)), getMax(cell->verts[i].x, getMax(cell->verts[0].x, cell->center.x))),
					mRandF(getMin(cell->verts[i].y, getMin(cell->verts[0].y, cell->center.y)), getMax(cell->verts[i].y, getMax(cell->verts[0].y, cell->center.y))));
				
				//check if the random point is within the bounds of the triangle
				if (checkVert(cell->verts[i], cell->verts[0], temp) < 0 && checkVert(cell->verts[0], cell->center, temp) < 0 && checkVert(cell->center, cell->verts[i], temp) < 0){
					//make sure the point is not overlapping another
					if (temp.checkPosition(cell->trees, rad) == 0){
						cell->trees.push_back(Tree(temp.x, temp.y, rad));
						treeCount++;
					}

				}
			}
			else{
				temp = HexVert(
					mRandF(getMin(cell->verts[i].x, getMin(cell->verts[i + 1].x, cell->center.x)), getMax(cell->verts[i].x, getMax(cell->verts[i + 1].x, cell->center.x))),
					mRandF(getMin(cell->verts[i].y, getMin(cell->verts[i + 1].y, cell->center.y)), getMax(cell->verts[i].y, getMax(cell->verts[i + 1].y, cell->center.y))));

				//check if the random point is within the bounds of the triangle
				if (checkVert(cell->verts[i], cell->verts[i + 1], temp) < 0 && checkVert(cell->verts[i + 1], cell->center, temp) < 0 && checkVert(cell->center, cell->verts[i], temp) < 0){
					//make sure the point is not overlapping another
					if (temp.checkPosition(cell->trees, rad) == 0){
						cell->trees.push_back(Tree(temp.x, temp.y, rad));
						treeCount++;
					}

				}
			}
			
			

			
		}
	}

}

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

F32 *createIslandImage(U32 width, U32 height, Vector<Vector<HexCell>> cells, U32 area, F32 hexEdgeLength){
	F32 *buffer = (F32 *) malloc(width*height*sizeof(F32));
	if (buffer == NULL){
		return NULL;
	}

	//create image based on biome of nearest cell
	U32 xIndex, yIndex, i, j;
	F32 xPos = 0, yPos = 0;
	Point2I cellIndex;

	for (yIndex = 0; yIndex < height; ++yIndex){
		for (xIndex = 0; xIndex < width; ++xIndex){
			//calculate the position of the pixel (without the offset)
			xPos = F32(xIndex)*(F32(area) * 2) / F32(width);
			yPos = F32(yIndex)*(F32(area) * 2) / F32(height);

			//which cell are we dealing with?
			i = U32(2*mFloor((yPos) / (2*hexEdgeLength)));
			j = U32(mFloor((xPos) / (2 * hexEdgeLength)) - mRound(mFloor((xPos) / (2 * hexEdgeLength)) / 3));
			
			//Get the coordinates of the cell that the pixel is in
			cellIndex = findHex(i, j, xPos - F32(area), yPos - F32(area), cells);

			//only calculate for cells on the interior of the image
			if (i < U32(mFloor((2 * area) / (hexEdgeLength)) - 1) && i > 0 && j < U32(mFloor((2 * area) / (hexEdgeLength * 3)) - 1) && j > 0){
				buffer[yIndex*width + xIndex] = F32(cells.at(cellIndex.x).at(cellIndex.y).biome);
			}
			else{
				buffer[yIndex*width + xIndex] = 2;
			}
		}
	}

	return buffer;
}

inline void setRGB(png_byte *ptr, float val)
{
	if (val == 0) { //Lake
		ptr[0] = 46; ptr[1] = 94; ptr[2] = 110;
	}
	else if (val == 1) { //Shore
		ptr[0] = 166; ptr[1] = 155; ptr[2] = 121;
	}
	else if (val == 2) { //Ocean
		ptr[0] = 33; ptr[1] = 68; ptr[2] = 82;
	}
	else if (val == 3) { //Field
		ptr[0] = 53; ptr[1] = 145; ptr[2] = 35;
	}
	else if (val == 4) { //Forest
		ptr[0] = 54; ptr[1] = 102; ptr[2] = 21;
	}
}

U32 writeImage(char* filename, int width, int height, float *buffer, char* title)
{
	int code = 0;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row;

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set title
	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	row = (png_bytep) malloc(3 * width * sizeof(png_byte));

	// Write image data
	int x, y;
	for (y = 0; y<height; y++) {
		for (x = 0; x<width; x++) {
			setRGB(&(row[x * 3]), buffer[y*width + x]);
		}
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	if (row != NULL) free(row);

	return code;
}


void Island::renderMap(){
	// Specify an output image size
	U32 width = 256;
	U32 height = 256;

	// The output is a 1D array of floats, length: width * height
	Con::printf("Creating Image");
	F32 *buffer = createIslandImage(width, height, this->cells, this->area, this->hexEdgeLength);

	if (buffer == NULL) {
		return;
	}

	// Save the image to a PNG file
	// The 'title' string is stored as part of the PNG file
	Con::printf("Saving PNG");
	U32 result = writeImage("modules/LightModule/1/assets/images/Maps/mainMap.png", width, height, buffer, "Main Map");

	

	// Free up the memory used to store the image
	free(buffer);

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