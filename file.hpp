class File
{
private:

    struct
    {
        HANDLE fileHandle = (void*)-1;

        std::string fileName = nullptr;
    };

public:

    ~File() = default;

    File(const std::string& fileName)
        : fileName(fileName) {}

private:

    bool prepareFile(DWORD flag)
    {
        this->fileHandle = CreateFileA(this->fileName.c_str(),
            GENERIC_READ | GENERIC_WRITE, 0, nullptr,
            flag, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (this->fileHandle == INVALID_HANDLE_VALUE) //Validate.
        {
            return false; //Return false since handle is invalid.
        }

        return true; //Return true since all went as it expected.
    }

public:

    bool read(std::string& content)
    {
        if (!this->prepareFile(OPEN_ALWAYS)) //Prepare the file.
        {
            return false; //Return false since handle is invalid.
        }

        const auto fileSize = GetFileSize( //Retrieve file size.
            this->fileHandle, nullptr);

        auto buffer = new char[fileSize]; //Setup the buffer ptr.

        auto bytesRead = DWORD(); //Define readed bytes size obj.

        const auto result = ReadFile(this->fileHandle, buffer,
            fileSize, &bytesRead, nullptr);

        CloseHandle(this->fileHandle); //Close created file handle.

        if (result != 0) //Check if the readed results were valid.
        {
            content.assign(buffer, fileSize); //Assign buf to str.
        }

        delete[] buffer; //Delete the assigned buffer for content.

        return result != 0; //Check if the file results are valid.
    }

    bool write(const std::string& content)
    {
        if (!this->prepareFile(CREATE_ALWAYS)) //Prepare the file.
        {
            return false; //Return false since handle is invalid.
        }

        auto bytesWritten = DWORD(); //Define written bytes object.

        const auto result = WriteFile(this->fileHandle,
            content.c_str(), content.size(), &bytesWritten, NULL);

        CloseHandle(this->fileHandle); //Close created file handle.

        return result != 0; //Check if the file results are valid.
    }

    bool createDirectory(const std::string& path)
    {
        return CreateDirectoryA(path.c_str(), nullptr) != 0; //Do.
    }
};
