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