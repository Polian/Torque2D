//ConsoleMethodGroupBeginWithDocs(Island)

ConsoleMethodWithDocs(Island, placeTrees, ConsoleVoid, 2, 2, ())
{
	object->placeTrees();
}

ConsoleFunctionWithDocs(threadedLoad, ConsoleVoid, 2, 4, ())
{
	//argv[2] is the file name and argv[3] is the extension
	std::thread t(loadTaml, argv[1], argv[2]);
	t.join();
}

ConsoleMethodWithDocs(Island, renderMap, ConsoleVoid, 2, 2, ())
{
	object->renderMap();
}

ConsoleMethodWithDocs(Island, getBiome, ConsoleInt, 2, 4, ())
{
	return object->getBiome(argv[2], argv[3]);
}

ConsoleFunctionWithDocs(project3d, ConsoleString, 2, 2, ())
{
	// Create Returnable Buffer.
	char* point = Con::getReturnBuffer(32);
	F32 ax = 0, ay = 0, az = 0;
	F32 cx = 0, cy = 0, cz = 0;
	
	//set the camera and traget points
	dSscanf(argv[1], "%f %f %f", &ax, &ay, &az);
	dSscanf(Con::executef(1, "getCameraPos"), "%f %f %f", &cx, &cy, &cz);

	//make sure we do not divide by zero (this should almost never happen)
	if (cz-az == 0){
		dSprintf(point, 32, "%f %f", 0, 0);
	}
	else{
		dSprintf(point, 32, "%f %f", ((cz * (ax - cx)) / (cz - az)) + cx, ((cz * (ay - cy)) / (cz - az)) + cy);
	}
	
	
	return point;
}