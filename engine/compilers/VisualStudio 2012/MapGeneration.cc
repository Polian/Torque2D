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

S32 compare(char * const  *a, char* const  *b)
{
	float la=0, lb=0;
	la = strtof(*a, NULL);
	lb = strtof(*b, NULL);
	return (la - lb);
}

bool Island::onAdd()
{
	// Fail if the parent fails.  
	if (!Parent::onAdd())
		return false;

	// Do some work here.
	Taml taml;
	SimSet set, verts, corners, delaunay, tris;
	

	//register the set of verticies and the set of corners.
	verts.registerObject("vertSet");
	corners.registerObject("cornerSet");
	delaunay.registerObject("delaunaySet");
	tris.registerObject("triSet");
	

	//set the area and vert. count
	const int vertCount = 500;
	F32 area = 500;

	//create array of vert. simObjects that will store the vert information and be output to file.
	SimObject tmpVert[vertCount];
	SimObject tmpCorner[3*vertCount]; // there are supposedly 3x as many edges as verts.  NOTE: The actual bound is (3*v-6)
	SimObject tmpDelaunay[3 * vertCount];
	SimObject tmpTri[6 * vertCount];

	Queue<int> fillQueue;

	bool adjCells[vertCount][vertCount] = {0};
	bool visitedCell[vertCount] = {0};

	char* endPtr;
	char name[100];

	Vector<char*> distanceVector;
	char* dist;

	float x1, y1, x2, y2;
	float distanceToVert = 0;
	float midpointX = 0;
	float midpointY = 0;

	float angle1a = 0, angle1b = 0, angle2a = 0, angle2b = 0;
	float min2 = 0;
	float min = 0;

	int index = 0;
	int index2 = 0;

	float xValues[vertCount];
	float yValues[vertCount];
	float noise[vertCount];
	float biomeNoise[vertCount];
	char tempStr[100], nameStr[15], noiseStr[100];
	int counter = 0, edgeCount = 0, dCount = 0;
	float r = 0; // radius from center of map to given point
	float seed = mRandF(0, 100); // the seed for our noise function
	Con::printf("seed: %f", seed);

	//---------------------------
	//-----Generate Vertices-------
	//-------------------------
	//generate random vertices and save the results to file.
	for (int i = 0; i < vertCount; ++i){
		// ---vert calculations---
		// generate random x and y values
		xValues[i] = mRandF(-area, area);
		yValues[i] = mRandF(-area, area);

		//generate noise
		noise[i] = scaled_octave_noise_3d(8.0, 0.0, 0.005, 0.0, 1.0, xValues[i], yValues[i], seed);
		biomeNoise[i] = scaled_octave_noise_3d(8.0, 0.0, 0.05, 0.0, 1.0, xValues[i], yValues[i], seed);
		//biomeNoise[i] = noise[i];

		//make noise more "island-like"
		r = mSqrt(xValues[i] * xValues[i] + yValues[i] * yValues[i]) / area;
		if (noise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2) < 0){
			noise[i] = 0;
		}
		else{
			noise[i] = mPow(noise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2), 2);
		}

		// generate biome noise to be a smaller island
		r = mSqrt(xValues[i] * xValues[i] + yValues[i] * yValues[i]) / (area -(area*0.35));
		if (biomeNoise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2) < 0){
			biomeNoise[i] = 0;
		}
		else{
			biomeNoise[i] = mPow(biomeNoise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2), 2);
		}

		// create a string that contains the x value and the y value of the current vert.
		sprintf(tempStr, "%f %f", xValues[i], yValues[i]);
		sprintf(noiseStr, "%f", noise[i]);

		// create the string that acts as the name for our simObjects, 
		// then register those objects under that name, 
		// and finally set the vertex data to the string created above.
		sprintf(nameStr, "vert%d", i);
		tmpVert[i].registerObject(nameStr);
		tmpVert[i].setDataField("Vertex", NULL, tempStr);
		tmpVert[i].setDataField("Elevation", NULL, noiseStr);
		tmpVert[i].setDataField("Shore", NULL, "1");

		sprintf(noiseStr, "%f", biomeNoise[i]);
		tmpVert[i].setDataField("LandNoise", NULL, noiseStr);
		
		// add the generated object to our vert simSet.
		verts.addObject(Sim::findObject(nameStr));
	}

	

	//---------------------------
	//-----Generate Voronoi-------
	//-------------------------
	// Generate voronoi diagram from verts. generated above.
	VoronoiDiagramGenerator voronoi;
	voronoi.setGenerateDelaunay(true);
	voronoi.generateVoronoi(xValues, yValues, vertCount, -area, area, -area, area, 0, true);
	
	voronoi.resetIterator();
	voronoi.resetDelaunayEdgesIterator();
	float x = 0, y = 0;

	//---------------------------
	//-----Delaunay Edges-------
	//-------------------------
	while (voronoi.getNextDelaunay(x1, y1, x2, y2)){
		// find the index of the two verts
		int i, j;
		for (i = 0; i < vertCount; ++i){
			if (mFabs(xValues[i] - x1) < 0.00001){
				if (mFabs(yValues[i] - y1) < 0.00001){
					break;
				}
			}

		}

		for (j = 0; j < vertCount; ++j){
			if (mFabs(xValues[j] - x2) < 0.00001){
				if (mFabs(yValues[j] - y2) < 0.00001){
					break;
				}
			}

		}

		adjCells[i][j] = true;
		adjCells[j][i] = true;

		// save Delaunay edges in a set
		sprintf(tempStr, "%f %f %f %f", x1, y1, x2, y2);
		
		sprintf(nameStr, "delaunay%d", dCount);
		tmpDelaunay[dCount].registerObject(nameStr);
		tmpDelaunay[dCount].setDataField("DelaunayEdge", NULL, tempStr);

		delaunay.addObject(Sim::findObject(nameStr));

		dCount++;

		assignBiomes(x1, y1, x2, y2, xValues, yValues, vertCount);

	}

	//---------------------------
	//-----Biome Assignment-------
	//-------------------------
	//sprintf(tempStr, "%f %f", x1, y1);
	//Con::printf("%d", DEdges.size());
	//int count = 0;
	//for (iter = DEdges.begin(); count < DEdges.size(); ++iter) {
	//	Con::printf("%f %f", iter.getValue()[2], iter.getValue()[3]);
	//	++count;
	//	//sprintf(tempStr, "%f %f", iter.getValue()[0], iter.getValue()[1]);
	//	Con::printf("%d", count);
	//	//DEdges.erase(tempStr);
	//}

	//int count = DEdges.count(tempStr);
	//for (int i = 0; i < count; ++i){
	//	Con::printf("%d", DEdges.count(tempStr));
	//	Con::printf("%f %f", DEdges.find(tempStr).operator++(i).getValue()[2], DEdges.find(tempStr).operator++(i).getValue()[3]);
	//	//DEdges.find(tempStr).operator++();
	//}
	
	//fillOcean(DEdges, x1, y1, vertCount, xValues, yValues);

	

	//make sure the starting cell is an ocean cell
	for (int i = 0; i < vertCount; ++i){
		sprintf(name, "vert%d", i);
		if (strcmp(Sim::findObject(name)->getDataField("Elevation", NULL), "0.000000") == 0 && (xValues[i] > (area - 25) || xValues[i] < (-area + 25))){
			index = i;
			break;
		}
	}

	fillQueue.enqueue(index);

	while (fillQueue.size() != 0){
		//dequeue an element from the queue
		sprintf(name, "vert%d", fillQueue[0]);
		index = fillQueue[0];
		fillQueue.dequeue();

		if (strcmp(Sim::findObject(name)->getDataField("Elevation", NULL), "0.000000") == 0){

			Sim::findObject(name)->setDataField("Biome", NULL, "Ocean");

			//add adjacent cells to queue
			for (int i = 0; i < vertCount; ++i){

				if (adjCells[i][index]){
					if (visitedCell[i] == 0){
						visitedCell[i] = 1;
						fillQueue.enqueue(i);
					}
					
				}
			}
		}
		else{
			Sim::findObject(name)->setDataField("Biome", NULL, "Shore");
		}

		
		
	}

	//---------------------------
	//-----Edges-------
	//-------------------------
	// get all the edges in the voronoi diagram.
	while (voronoi.getNext(x1, y1, x2, y2)){
		
		// iterate the counter that keeps track of how many edges we are generating
		counter++;

		// the edgeCount is used to actually access the element
		// in the array that represents the current edge
		edgeCount = counter - 1;

		//create the string that represents the edge between two points
		sprintf(tempStr, "%f %f %f %f", x1, y1, x2, y2);

		// create the string that is used as the name of the current object,
		// then register the object with the name,
		// then set the "Edge" field to the current edge
		sprintf(nameStr, "edge%d", edgeCount);
		//tmpCorner[edgeCount].registerObject(nameStr);
		//tmpCorner[edgeCount].setDataField("Edge", NULL, tempStr);

		// ------Cell creation ------
		// find the closest two verts to the midpoint of the current edge.
		midpointX = (x1 + x2) / 2;
		midpointY = (y1 + y2) / 2;

		angle1a = 0, angle1b = 0, angle2a = 0, angle2b = 0;
		min2 = area;
		min = area; // min initialized to area just because area will always be quite large

		index = 0;
		index2 = 0;

		for (int i = 0; i < vertCount; i++){
			distanceToVert = mSqrt(mPow(midpointX - xValues[i], 2) + mPow(midpointY - yValues[i], 2));
			sprintf(tempStr, "%f %f %f", distanceToVert, xValues[i], yValues[i]);
			dist = new char[50];
			sprintf(dist, "%s", tempStr);
			distanceVector.setSize(distanceVector.size() + 1);
			distanceVector[i] = dist;
			//is the current distance less than the previously smallest distance?
			//if (distanceToVert <= min){
			//	//closest vert
			//	min = distanceToVert;
			//	index = i;
			//	angle1a =  mAtan(x1 - xValues[i], y1 - yValues[i]);
			//	angle1b =  mAtan(x2 - xValues[i], y2 - yValues[i]);

			//	//save the cells adjacent to this edge
			//	sprintf(tempStr, "%d", index);
			//	tmpCorner[edgeCount].setDataField("Cell1", NULL, tempStr);
			//	

			//	//save the angle of each of the edge verts to the cell vert
			//	sprintf(tempStr, "%f", angle1a);
			//	tmpCorner[edgeCount].setDataField("Angle1a", NULL, tempStr);
			//	sprintf(tempStr, "%f", angle1b);
			//	tmpCorner[edgeCount].setDataField("Angle1b", NULL, tempStr);
			//	
			//	sprintf(tempStr, "%f %f", midpointX, midpointY);
			//	tmpCorner[edgeCount].setDataField("Midpoint", NULL, tempStr);
			//}
		}


		distanceVector.sort(compare);

		//set all the appropriate fields for the first triangle
		strtof(distanceVector[0], &endPtr);
		x = strtof(endPtr, &endPtr);
		y = strtof(endPtr, NULL);

		sprintf(tempStr, "%f %f %f %f %f %f", x1, y1, x2, y2, x, y);
		tmpTri[edgeCount*2].setDataField("Triangle", NULL, tempStr);

		for (int i = 0; i < vertCount; ++i){
			if (mFabs(xValues[i] - x) < 0.0001 && mFabs(yValues[i] - y) < 0.0001){
					sprintf(tempStr, "vert%d", i);

					tmpTri[edgeCount * 2].setDataField("Biome", NULL, Sim::findObject(tempStr)->getDataField("Biome", NULL));
					sprintf(nameStr, "Tri%d", edgeCount * 2);
					tmpTri[edgeCount * 2].registerObject(nameStr);
					tris.addObject(Sim::findObject(nameStr));
					break;
				
			}
		}
		

		strtof(distanceVector[1], &endPtr);
		x = strtof(endPtr, &endPtr);
		y = strtof(endPtr, NULL);

		sprintf(tempStr, "%f %f %f %f %f %f", x1, y1, x2, y2, x, y);
		tmpTri[edgeCount * 2 + 1].setDataField("Triangle", NULL, tempStr);

		for (int i = 0; i < vertCount; ++i){
			if (mFabs(xValues[i] - x) < 0.00001 && mFabs(yValues[i] - y) < 0.00001){
				sprintf(tempStr, "vert%d", i);

				tmpTri[(edgeCount * 2) + 1].setDataField("Biome", NULL, Sim::findObject(tempStr)->getDataField("Biome", NULL));
				sprintf(nameStr, "Tri%d", (edgeCount * 2) + 1);
				tmpTri[(edgeCount * 2) + 1].registerObject(nameStr);
				tris.addObject(Sim::findObject(nameStr));
				break;
			}
		}

		


		distanceVector.clear();
		distanceVector.setSize(0);

		//find the second closest cell
		/*for (int i = 0; i < vertCount; i++){
			distanceToVert = mSqrt(mPow(midpointX - xValues[i], 2) + mPow(midpointY - yValues[i], 2));

			if (distanceToVert <= min2 && distanceToVert != min){
				min2 = distanceToVert;
				index2 = i;

				angle2a = mAtan(x1 - xValues[i], y1 - yValues[i]);
				angle2b = mAtan(x2 - xValues[i], y2 - yValues[i]);

				sprintf(tempStr, "%d", index2);
				tmpCorner[edgeCount].setDataField("Cell2", NULL, tempStr);

				sprintf(tempStr, "%f", angle2a);
				tmpCorner[edgeCount].setDataField("Angle2a", NULL, tempStr);
				sprintf(tempStr, "%f", angle2b);
				tmpCorner[edgeCount].setDataField("Angle2b", NULL, tempStr);
			}
		}*/

		// Add the current object to the corners simSet
		//corners.addObject(Sim::findObject(nameStr));
		
		
	}

	//----------------------------------------------------
	//--------TAML Writing and Un-registration-----------
	//--------------------------------------------------

	// write the DELAUNAY simSet to file and unregister containers.
	taml.write(Sim::findObject("delaunaySet"), "modules/LightModule/1/MapData/Delaunay.taml");
	for (int i = 0; i < dCount; ++i){
		tmpDelaunay[i].unregisterObject();
	}
	delaunay.unregisterObject();

	//delete[] point;
//	delete [] queueObject;
	delete [] dist;


	// write the CORNERS simSet to file and unregister containers.
	/*taml.write(Sim::findObject("cornerSet"), "modules/LightModule/1/MapData/Edges.taml");
	for (int i = 0; i < counter; ++i){
		tmpCorner[i].unregisterObject();
	}*/
	corners.unregisterObject();


	// write the VERT simSet to file and unregister containers.
	taml.write(Sim::findObject("vertSet"), "modules/LightModule/1/MapData/Verts.taml");
	for (int i = 0; i < vertCount; ++i){
		tmpVert[i].unregisterObject();
	}
	verts.unregisterObject();

	// write the TRI simSet to file and unregister containers.
	taml.write(Sim::findObject("triSet"), "modules/LightModule/1/MapData/Triangles.taml");
	for (int i = 0; i < counter*2; ++i){
		tmpTri[i].unregisterObject();
	}
	tris.unregisterObject();

	

	Con::printf("Voronoi Created");

	
	

	return true;
}

void Island::initPersistFields()
{
	// Call parent.  
	Parent::initPersistFields();

	// Add my fields here.  
	//addField("Vertices", TypeS32, Offset(vertices, Island), "");
	//addField("Corners", TypeString, Offset(corners, Island), "List of corners.");
}

void Island::assignBiomes(float x1, float y1, float x2, float y2, float* xValues, float* yValues, int vertCount){
	char vertStr1[100], vertStr2[100];

	//// init shore info
	//for (int i = 0; i < vertCount; ++i){
	//	// assign ocean values
	//	if (xValues[i] == x1 && yValues[i] == y1){
	//		sprintf(vertStr1, "vert%d", i); // form the name of the current vert

	//		if (strcmp(Sim::findObject(vertStr1)->getDataField("Elevation", NULL), "0.000000") == 0){
	//			//Sim::findObject(vertStr1)->setDataField("Ocean", NULL, "0");
	//			Sim::findObject(vertStr1)->setDataField("Shore", NULL, "1");
	//		}
	//		else{
	//			//Sim::findObject(vertStr1)->setDataField("Ocean", NULL, "1");
	//			Sim::findObject(vertStr1)->setDataField("Shore", NULL, "1");
	//		}
	//	}
	//	else if (xValues[i] == x2 && yValues[i] == y2){
	//		sprintf(vertStr1, "vert%d", i); // form the name of the current vert

	//		if (strcmp(Sim::findObject(vertStr1)->getDataField("Elevation", NULL), "0.000000") == 0){
	//			//Sim::findObject(vertStr1)->setDataField("Ocean", NULL, "0");
	//			Sim::findObject(vertStr1)->setDataField("Shore", NULL, "1");
	//		}
	//		else{
	//			//Sim::findObject(vertStr1)->setDataField("Ocean", NULL, "1");
	//			Sim::findObject(vertStr1)->setDataField("Shore", NULL, "1");
	//		}
	//	}

	//}

	// check if ocean borders land
	for (int i = 0; i < vertCount; ++i){

		if (mFabs(xValues[i] - x1) < 0.00001 && mFabs(yValues[i] - y1) < 0.00001){
			sprintf(vertStr1, "vert%d", i); // form the name of the first vert
			for (int j = 0; j < vertCount; ++j){

				if (mFabs(xValues[j] - x2) < 0.00001 && mFabs(yValues[j] - y2) < 0.00001){
					sprintf(vertStr2, "vert%d", j); // form the name of the second vert

					// assign shore biome if one cell is water and the other is land (from the elevation)
					//if (dAtof(Sim::findObject(vertStr1)->getDataField("Elevation", NULL)) == 0 &&
					//	dAtof(Sim::findObject(vertStr2)->getDataField("Elevation", NULL)) != 0){

					//	Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Ocean");
					//	Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Shore");
					//	Sim::findObject(vertStr2)->setDataField("Shore", NULL, "0");

					//}
					//else if (	dAtof(Sim::findObject(vertStr1)->getDataField("Elevation", NULL)) != 0 &&
					//			dAtof(Sim::findObject(vertStr2)->getDataField("Elevation", NULL)) == 0){

					//	Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Shore");
					//	Sim::findObject(vertStr1)->setDataField("Shore", NULL, "0");
					//	Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Ocean");

					//}
					////both are ocean
					//else if (	dAtof(Sim::findObject(vertStr1)->getDataField("Elevation", NULL)) == 0 &&
					//			dAtof(Sim::findObject(vertStr2)->getDataField("Elevation", NULL)) == 0){

					//	Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Ocean");
					//	Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Ocean");

					//}
					////both are land
					/*if (	dAtof(Sim::findObject(vertStr1)->getDataField("Elevation", NULL)) != 0 &&
								dAtof(Sim::findObject(vertStr2)->getDataField("Elevation", NULL)) != 0){

						if (dAtof(Sim::findObject(vertStr1)->getDataField("Shore", NULL)) == 0 &&
							dAtof(Sim::findObject(vertStr2)->getDataField("Shore", NULL)) == 0){

							Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Shore");
							Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Shore");

						}else if (dAtof(Sim::findObject(vertStr1)->getDataField("Shore", NULL)) == 0 &&
							dAtof(Sim::findObject(vertStr2)->getDataField("Shore", NULL)) == 1){

							Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Shore");
							assignLandBiome(vertStr2);

						}
						else if (	dAtof(Sim::findObject(vertStr1)->getDataField("Shore", NULL)) == 1 &&
									dAtof(Sim::findObject(vertStr2)->getDataField("Shore", NULL)) == 0){

							assignLandBiome(vertStr1);
							Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Shore");

						}
						else{*/

					if (dAtof(Sim::findObject(vertStr1)->getDataField("Elevation", NULL)) == 0){
						Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Lake");
					}
					else if (dAtof(Sim::findObject(vertStr2)->getDataField("Elevation", NULL)) == 0){
						Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Lake");
					}
					else{
						assignLandBiome(vertStr1);
						assignLandBiome(vertStr2);
					}
							
						/*}
						
					}
					else{
						Sim::findObject(vertStr1)->setDataField("Biome", NULL, "Ocean");
						Sim::findObject(vertStr2)->setDataField("Biome", NULL, "Ocean");
					}*/
				}
			}
		}
	}
}


void Island::assignLandBiome(char* vert){
	
	float threshold = 0;

	if (dAtof(Sim::findObject(vert)->getDataField("LandNoise", NULL)) > threshold){
		Sim::findObject(vert)->setDataField("Biome", NULL, "Forest");
	}
	else{
		Sim::findObject(vert)->setDataField("Biome", NULL, "Field");
	}
}



//void Island::fillOcean(HashTable<char*, float*> DEdges, float x, float y, int vertCount, float* xValues, float* yValues){
//	char key[100];
//	int counter = 0;
//
//	char tempStr[100];
//
//	for (int i = 0; i < vertCount; ++i){
//		if (xValues[i] == x && yValues[i] == y){
//			sprintf(tempStr, "vert%d", i);
//			
//		}
//	}
//
//	Con::printf("fill begin");
//	// if the current node is above sea level return without setting the biome
//	if (dAtof(Sim::findObject(tempStr)->getDataField("Elevation", NULL)) > 0){
//		Con::printf("current not an ocean cell");
//		return;
//	}
//
//	Sim::findObject(tempStr)->setDataField("Biome", NULL, "Ocean2");
//	Con::printf("found object and set biome data");
//	sprintf(key, "%f %f", x, y);
//	Con::printf("assign key value");
//	counter = DEdges.count(key);
//	Con::printf("assign done");
//	//for (int i = 0; i < counter; ++i){
//	Con::printf("recurrsively fill all adjacent %f %f", DEdges.find(key)->value[2], DEdges.find(key)->value[3]);
//		fillOcean(DEdges, DEdges.find(key)->value[2], DEdges.find(key)->value[3], vertCount, xValues, yValues);
//		Con::printf("erase current entry");
//		DEdges.erase(DEdges.find(key));
//	//}
//}
//
//char* Island::findVert(float x, float y, int vertCount, float* xValues, float* yValues){
//	char tempStr[100];
//
//	for (int i = 0; i < vertCount; ++i){
//		if (xValues[i] == x && yValues[i] == y){
//			sprintf(tempStr, "vert%d", i);
//			return tempStr;
//		}
//	}
//}

IMPLEMENT_CONOBJECT(Island);