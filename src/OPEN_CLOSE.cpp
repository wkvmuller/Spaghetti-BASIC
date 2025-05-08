
void executeOPEN(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, filenameToken, forToken, modeToken, asToken, hashChannel;
    iss >> cmd >> filenameToken >> forToken >> modeToken >> asToken >> hashChannel;

    // Remove quotes from filename
    if (filenameToken.front() == '\"' && filenameToken.back() == '\"') {
        filenameToken = filenameToken.substr(1, filenameToken.length() - 2);
    }

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);
    OpenMode mode;
    std::ios::openmode openFlags;

    if (modeToken == "INPUT") {
        mode = MODE_INPUT;
        openFlags = std::ios::in;
    } else if (modeToken == "OUTPUT") {
        mode = MODE_OUTPUT;
        openFlags = std::ios::out | std::ios::trunc;
    } else if (modeToken == "APPEND") {
        mode = MODE_APPEND;
        openFlags = std::ios::out | std::ios::app;
    } else {
        std::cerr << "ERROR: Invalid file mode: " << modeToken << std::endl;
        return;
    }

    FileHandle fh;
    fh.filename = filenameToken;
    fh.channel = channel;
    fh.mode = mode;
    fh.stream.open(filenameToken, openFlags);

    if (!fh.stream.is_open()) {
        std::cerr << "ERROR: Could not open file '" << filenameToken << "'" << std::endl;
        return;
    }

    fh.currentCharPos = fh.stream.tellg();
    fh.stream.seekg(0, std::ios::end);
    fh.lastCharPos = fh.stream.tellg();
    fh.stream.seekg(fh.currentCharPos);
    fh.isFileOpen = true;

    openFiles[channel] = std::move(fh);
}


void executeCLOSE(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd, hashChannel;
    iss >> cmd >> hashChannel;

    if (hashChannel.front() == '#') {
        hashChannel = hashChannel.substr(1);
    }

    int channel = std::stoi(hashChannel);

    auto it = openFiles.find(channel);
    if (it == openFiles.end()) {
        std::cerr << "ERROR: CLOSE attempted on unopened channel #" << channel << std::endl;
        return;
    }
    it->second.isFileOpen = false;

    it->second.stream.close();
    openFiles.erase(it);
}
