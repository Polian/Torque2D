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

	// Do some work here.
	Taml taml;
	SimSet set, verts, corners, delaunay;
	

	//register the set of verticies and the set of corners.
	verts.registerObject("vertSet");
	corners.registerObject("cornerSet");
	delaunay.registerObject("delaunaySet");
	

	//set the area and vert. count
	const int vertCount = 1000;
	F32 area = 500;

	//create array of vert. simObjects that will store the vert information and be output to file.
	SimObject tmpVert[vertCount];
	SimObject tmpCorner[3*vertCount]; // there are supposedly 3x as many edges as verts.  NOTE: The actual bound is (3*v-6)
	SimObject tmpDelaunay[3 * vertCount];

	HashMap<Point2D, SimObject*> vertLookup;
	Point2D vertPoint;
	SimObject* hashedVert;

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
	char tempStr[100], nameStr[15], noiseStr[100];
	int counter = 0, edgeCount = 0, dCount = 0;
	float r = 0; // radius from center of map to given point
	float seed = mRandF(0, 100); // the seed for our noise function

	//generate random vertices and save the results to file.
	for (int i = 0; i < vertCount; ++i){
		// ---vert calculations---
		// generate random x and y values
		xValues[i] = mRandF(-area, area);
		yValues[i] = mRandF(-area, area);

		//generate noise
		noise[i] = scaled_octave_noise_3d(8, 0.0, 0.005, 0.0, 1.0, xValues[i], yValues[i], seed);

		//make noise more "island-like"
		r = mSqrt(xValues[i] * xValues[i] + yValues[i] * yValues[i]) / area;
		if (noise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2) < 0){
			noise[i] = 0;
		}
		else{
			noise[i] = mPow(noise[i] * exp(-mPow(r / 4, 2)) - mPow(r, 2), 2);
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

		// add the generated object to our vert simSet.
		verts.addObject(Sim::findObject(nameStr));

		//add the vert to the hashmap
		vertPoint.x = xValues[i];
		vertPoint.y = yValues[i];
		vertLookup.insert(vertPoint, Sim::findObject(nameStr));
	}

	


	// Generate voronoi diagram from verts. generated above.
	VoronoiDiagramGenerator voronoi;
	voronoi.setGenerateDelaunay(true);
	voronoi.generateVoronoi(xValues, yValues, vertCount, -area, area, -area, area, 2, true);
	
	voronoi.resetIterator();
	voronoi.resetDelaunayEdgesIterator();

	

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
		tmpCorner[edgeCount].registerObject(nameStr);
		tmpCorner[edgeCount].setDataField("Edge", NULL, tempStr);

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

			//is the current distance less than the previously smallest distance?
			if (distanceToVert <= min){
				//closest vert
				min = distanceToVert;
				index = i;
				angle1a =  mAtan(x1 - xValues[i], y1 - yValues[i]);
				angle1b =  mAtan(x2 - xValues[i], y2 - yValues[i]);

				//save the cells adjacent to this edge
				sprintf(tempStr, "%d", index);
				tmpCorner[edgeCount].setDataField("Cell1", NULL, tempStr);
				

				//save the angle of each of the edge verts to the cell vert
				sprintf(tempStr, "%f", angle1a);
				tmpCorner[edgeCount].setDataField("Angle1a", NULL, tempStr);
				sprintf(tempStr, "%f", angle1b);
				tmpCorner[edgeCount].setDataField("Angle1b", NULL, tempStr);
				
				sprintf(tempStr, "%f %f", midpointX, midpointY);
				tmpCorner[edgeCount].setDataField("Midpoint", NULL, tempStr);
			}
		}

		//find the second closest cell
		for (int i = 0; i < vertCount; i++){
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
		}

		// Add the current object to the corners simSet
		corners.addObject(Sim::findObject(nameStr));
		
	}

	


	//---------------------------
	//-----Delaunay Edges-------
	//-------------------------
	while (voronoi.getNextDelaunay(x1, y1, x2, y2)){

		sprintf(tempStr, "%f %f %f %f", x1, y1, x2, y2);

		sprintf(nameStr, "delaunay%d", dCount);
		tmpDelaunay[dCount].registerObject(nameStr);
		tmpDelaunay[dCount].setDataField("DelaunayEdge", NULL, tempStr);

		delaunay.addObject(Sim::findObject(nameStr));

		dCount++;

		// check if current vert borders water verts.
		/*vertPoint.x = x1;
		vertPoint.y = y1;
		hashedVert = vertLookup.find(vertPoint).getValue();
		Con::printf("test %s", hashedVert->getDataField("Elevation", NULL));
		if (hashedVert->getDataField("Elevation", NULL) == 0){
			hashedVert->setDataField("Ocean", NULL, "1");
			vertLookup.erase(vertPoint);
			vertLookup.insert(vertPoint, hashedVert);
		}
		else{
			hashedVert->setDataField("Ocean", NULL, "0");
			vertLookup.erase(vertPoint);
			vertLookup.insert(vertPoint, hashedVert);
		}*/
		
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


	// write the CORNERS simSet to file and unregister containers.
	taml.write(Sim::findObject("cornerSet"), "modules/LightModule/1/MapData/Edges.taml");
	for (int i = 0; i < counter; ++i){
		tmpCorner[i].unregisterObject();
	}
	corners.unregisterObject();


	// write the VERT simSet to file and unregister containers.
	taml.write(Sim::findObject("vertSet"), "modules/LightModule/1/MapData/Verts.taml");
	for (int i = 0; i < vertCount; ++i){
		tmpVert[i].unregisterObject();
	}
	verts.unregisterObject();

	

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

IMPLEMENT_CONOBJECT(Island);